#include "gameplay/battle/Battle.h"

#include <algorithm>
#include <cmath>

namespace gameplay::battle {

namespace {

constexpr int kCtbBase = 100;
constexpr int kSkill1MpCost = 4;
constexpr int kSkill2MpCost = 6;

int BaseDelayFromAgility(const int agility) {
    const int safeAgility = std::max(1, agility);
    return std::max(1, static_cast<int>(std::ceil(static_cast<float>(kCtbBase) / static_cast<float>(safeAgility))));
}

bool IsAlive(const BattleUnit& unit) {
    if (unit.category == UnitCategory::Generic) {
        return unit.life > 0;
    }
    return unit.hp > 0;
}

} // namespace


BattleState BattleState::CreateForTests(std::vector<BattleUnit> units, const uint32_t seed) {
    BattleState state(seed);
    state.SetUnits(std::move(units));
    return state;
}

BattleState::BattleState(const uint32_t seed) : random_(seed) {}

bool BattleState::SetUnits(std::vector<BattleUnit> units) {
    int allies = 0;
    int enemies = 0;
    int assignedAllyLeaders = 0;
    int assignedEnemyLeaders = 0;

    for (auto& unit : units) {
        if (unit.side == TeamSide::Allies) {
            ++allies;
        } else {
            ++enemies;
        }

        unit.hp = std::clamp(unit.hp, 0, unit.stats.maxHp);
        unit.mp = std::clamp(unit.mp, 0, unit.stats.maxMp);
        unit.life = std::max(0, unit.life == 0 ? unit.stats.life : unit.life);
        unit.timeToAct = BaseDelayFromAgility(unit.stats.agility);
        unit.ko = !IsAlive(unit);
        unit.defending = false;
        unit.inReserve = false;
        if (unit.isAssignedLeader) {
            unit.stats.position = UnitPosition::Leader;
        }

        if (unit.isAssignedLeader) {
            if (unit.side == TeamSide::Allies) {
                ++assignedAllyLeaders;
            }
            else {
                ++assignedEnemyLeaders;
            }
        }
    }

    if (assignedAllyLeaders > 1 || assignedEnemyLeaders > 1) {
        return false;
    }

    if (allies == 0 || enemies == 0 || allies > kMaxUnitsPerSide || enemies > kMaxUnitsPerSide) {
        return false;
    }

    units_ = std::move(units);
    summary_ = {};
    AdvanceToNextTurn();
    return true;
}

bool BattleState::IsFinished() const {
    return summary_.finished;
}

const BattleSummary& BattleState::Summary() const {
    return summary_;
}

int BattleState::ActiveUnitIndex() const {
    return activeUnitIndex_;
}

const std::vector<BattleUnit>& BattleState::Units() const {
    return units_;
}

std::vector<int> BattleState::UpcomingTurnOrder(const int count) const {
    std::vector<int> order;
    order.reserve(std::max(0, count));

    struct ForecastEntry {
        int index;
        int time;
    };

    std::vector<ForecastEntry> forecast;
    forecast.reserve(units_.size());

    for (int i = 0; i < static_cast<int>(units_.size()); ++i) {
        if (!IsAliveAndActive(units_[i])) {
            continue;
        }
        forecast.push_back(ForecastEntry{i, units_[i].timeToAct});
    }

    for (int i = 0; i < count && !forecast.empty(); ++i) {
        std::sort(forecast.begin(), forecast.end(), [](const ForecastEntry& a, const ForecastEntry& b) {
            if (a.time == b.time) {
                return a.index < b.index;
            }
            return a.time < b.time;
        });

        order.push_back(forecast.front().index);

        const int elapsed = forecast.front().time;
        for (auto& entry : forecast) {
            entry.time -= elapsed;
        }
        forecast.front().time += BaseDelayFromAgility(units_[forecast.front().index].stats.agility);
    }

    return order;
}

const std::vector<BattleEvent>& BattleState::LastEvents() const
{
    return lastEvents_;
}

int BattleState::FindFirstTargetForActive() const {
    if (activeUnitIndex_ < 0 || activeUnitIndex_ >= static_cast<int>(units_.size())) {
        return -1;
    }

    const TeamSide targetSide = units_[activeUnitIndex_].side == TeamSide::Allies ? TeamSide::Enemies : TeamSide::Allies;
    for (int i = 0; i < static_cast<int>(units_.size()); ++i) {
        if (units_[i].side == targetSide && IsActionTargetable(units_[i])) {
            return i;
        }
    }

    return -1;
}

bool BattleState::ExecuteAction(const BattleActionType action, int targetIndex) {
    if (summary_.finished || activeUnitIndex_ < 0 || activeUnitIndex_ >= static_cast<int>(units_.size())) {
        return false;
    }
    lastEvents_.clear();
    BattleUnit& actor = units_[activeUnitIndex_];
    if (!IsAliveAndActive(actor)) {
        AdvanceToNextTurn();
        return false;
    }

    actor.defending = false;

    auto pushEvent = [&](const BattleEventType type,
        const BattleActionType eventAction,
        const std::string& actorName,
        const std::string& targetName,
        const int amount,
        const TeamSide winner,
        const std::string& infoText = {})
        {
            lastEvents_.push_back(BattleEvent{
                type,
                eventAction,
                actorName,
                targetName,
                infoText,
                amount,
                winner
                });
        };

    int resolvedTarget = targetIndex;
    const bool needsTarget = action == BattleActionType::Attack || action == BattleActionType::Skill1 || action == BattleActionType::Skill2;
    if (needsTarget && (resolvedTarget < 0 || resolvedTarget >= static_cast<int>(units_.size()) || !IsActionTargetable(units_[resolvedTarget]))) {
        resolvedTarget = FindFirstTargetForActive();
    }

    int damage = 0;
    bool waited = false;

    switch (action) {
    case BattleActionType::Attack:
        if (resolvedTarget < 0) {
            return false;
        }
        damage = ComputePhysicalDamage(actor, units_[resolvedTarget]);
        damage = ApplyDamage(units_[resolvedTarget], damage);
        pushEvent(BattleEventType::AttackResolved, action, 
            actor.name, units_[resolvedTarget].name,
            damage, TeamSide::Allies);

        break;
    case BattleActionType::Defend:
        actor.defending = true;
        pushEvent(BattleEventType::Defended, action, actor.name, "", 0, actor.side);
        break;
    case BattleActionType::Wait:
        waited = true;
        pushEvent(BattleEventType::Waited, action, actor.name, "", 0, actor.side);
        break;
    case BattleActionType::Skill1:
        if (resolvedTarget < 0) {
            return false;
        }
        if (actor.mp < kSkill1MpCost) {
            damage = ComputePhysicalDamage(actor, units_[resolvedTarget]);
            pushEvent(BattleEventType::Information, action,
                actor.name, units_[resolvedTarget].name, damage, actor.side, actor.name + " lacks MP, uses attack");
        } else {
            actor.mp -= kSkill1MpCost;
            damage = static_cast<int>(std::round(ComputePhysicalDamage(actor, units_[resolvedTarget]) * 1.25f));
        }
        damage = ApplyDamage(units_[resolvedTarget], damage);
        pushEvent(BattleEventType::SkillResolved, action,
            actor.name, units_[resolvedTarget].name,
            damage, actor.side);

        break;
    case BattleActionType::Skill2:
        if (resolvedTarget < 0) {
            return false;
        }
        if (actor.mp < kSkill2MpCost) {
            damage = ComputePhysicalDamage(actor, units_[resolvedTarget]);
            pushEvent(BattleEventType::Information, action,
                actor.name, units_[resolvedTarget].name, damage, actor.side, actor.name + " lacks MP, uses attack");
        } else {
            actor.mp -= kSkill2MpCost;
            damage = ComputeMagicDamage(actor, units_[resolvedTarget], 1.5f);
        }
        damage = ApplyDamage(units_[resolvedTarget], damage);
        pushEvent(BattleEventType::SkillResolved, action,
            actor.name, units_[resolvedTarget].name,
            damage, actor.side);
        break;
    }

    if (resolvedTarget >= 0 && resolvedTarget < static_cast<int>(units_.size()) && !IsAlive(units_[resolvedTarget])) {
        ShiftPositions(units_[resolvedTarget].side);
    }

    FinalizeIfFinished();

    if (!summary_.finished) {
        actor.timeToAct = PostActionDelay(actor, resolvedTarget >= 0 ? units_[resolvedTarget] : actor, waited);
        AdvanceToNextTurn();
    }

    return true;
}

bool BattleState::IsAliveAndActive(const BattleUnit& unit) const {
    return IsAlive(unit);
}

bool BattleState::IsActionTargetable(const BattleUnit& unit) const {
    return IsAliveAndActive(unit);
}

int BattleState::FindLeaderIndex(const TeamSide side) const {
    for (int i = 0; i < static_cast<int>(units_.size()); ++i) {
        if (units_[i].side == side && units_[i].isAssignedLeader) {
            return i;
        }
    }
    return -1;
}

int BattleState::EffectiveStat(const BattleUnit& unit, const TeamSide side, int UnitStats::*field) const {
    int result = unit.stats.*field;
    const int leaderIndex = FindLeaderIndex(side);
    if (leaderIndex >= 0 && leaderIndex < static_cast<int>(units_.size())) {
        const BattleUnit& leader = units_[leaderIndex];
        if (IsAlive(leader)) {
            result += leader.stats.*field;
        }
    }

    return std::max(0, result);
}

int BattleState::ComputePhysicalDamage(const BattleUnit& attacker, const BattleUnit& defender) {
    const int minDamage = std::min(attacker.stats.minDamage, attacker.stats.maxDamage);
    const int maxDamage = std::max(attacker.stats.minDamage, attacker.stats.maxDamage);
    std::uniform_int_distribution<int> distribution(minDamage, maxDamage);

    const int rolled = distribution(random_);
    const int baseDamage = rolled * std::max(1, attacker.life);

    const int attackStat = EffectiveStat(attacker, attacker.side, &UnitStats::attack);
    const int defenseStat = EffectiveStat(defender, defender.side, &UnitStats::defense);
    const int diff = attackStat - defenseStat;

    float modifier = 1.0f;
    if (diff > 0) {
        modifier += static_cast<float>(diff) * 0.1f;
    } else if (diff < 0) {
        modifier += static_cast<float>(diff) * 0.05f;
    }

    modifier = std::clamp(modifier, 0.1f, 3.0f);

    const int finalDamage = std::max(1, static_cast<int>(std::round(static_cast<float>(baseDamage) * modifier)));
    return finalDamage;
}

int BattleState::ComputeMagicDamage(const BattleUnit& attacker, const BattleUnit& defender, const float power) {
    const int magicStat = EffectiveStat(attacker, attacker.side, &UnitStats::magic);
    const int resistanceStat = EffectiveStat(defender, defender.side, &UnitStats::resistance);
    const int diff = magicStat - resistanceStat;

    float modifier = 1.0f;
    if (diff > 0) {
        modifier += static_cast<float>(diff) * 0.1f;
    } else if (diff < 0) {
        modifier += static_cast<float>(diff) * 0.05f;
    }

    const float baseMagic = static_cast<float>(magicStat) * power;
    const int finalDamage = std::max(1, static_cast<int>(std::round(baseMagic * modifier)));
    return finalDamage;
}

int BattleState::ApplyDamage(BattleUnit& target, int damage) {
    if (damage <= 0 || !IsAlive(target)) {
        return 0;
    }

    if (target.defending) {
        damage = std::max(1, damage / 2);
        target.defending = false;
    }

    int applied = 0;

    if (target.category == UnitCategory::Generic) {
        while (damage > 0 && target.life > 0) {
            if (damage < target.hp) {
                target.hp -= damage;
                applied += damage;
                damage = 0;
            } else {
                const int used = target.hp;
                damage -= used;
                applied += used;
                --target.life;

                if (target.life > 0) {
                    target.hp = target.stats.maxHp;
                } else {
                    target.hp = 0;
                    target.ko = true;
                }
            }
        }
    } else {
        const int used = std::min(target.hp, damage);
        target.hp -= used;
        applied += used;
        if (target.hp <= 0) {
            target.hp = 0;
            target.ko = true;
        }
    }

    return applied;
}

int BattleState::PostActionDelay(const BattleUnit& attacker, const BattleUnit& defender, const bool waited) const {
    const float penalty = AgilityPenalty(attacker, defender);
    const int effectiveAgility = std::max(1, static_cast<int>(std::round(static_cast<float>(attacker.stats.agility) * (1.0f - penalty))));

    int delay = BaseDelayFromAgility(effectiveAgility);
    if (waited) {
        delay = std::max(1, static_cast<int>(std::round(static_cast<float>(delay) * 0.7f)));
    }

    return delay;
}

float BattleState::AgilityPenalty(const BattleUnit& attacker, const BattleUnit& defender) const {
    const int rangeValue = static_cast<int>(attacker.stats.range);
    const int positionValue = static_cast<int>(defender.stats.position);

    const int penaltySteps = positionValue - rangeValue;
    if (penaltySteps <= 0) {
        return 0.0f;
    }

    if (penaltySteps == 1) {
        return 0.5f;
    }

    return 0.75f;
}

void BattleState::AdvanceToNextTurn() {
    activeUnitIndex_ = -1;

    std::vector<int> activeIndices;
    activeIndices.reserve(units_.size());

    for (int i = 0; i < static_cast<int>(units_.size()); ++i) {
        if (IsAliveAndActive(units_[i])) {
            activeIndices.push_back(i);
        }
    }

    if (activeIndices.empty()) {
        FinalizeIfFinished();
        return;
    }

    int minTime = units_[activeIndices.front()].timeToAct;
    for (const int index : activeIndices) {
        minTime = std::min(minTime, units_[index].timeToAct);
    }

    for (const int index : activeIndices) {
        units_[index].timeToAct -= minTime;
    }

    std::sort(activeIndices.begin(), activeIndices.end(), [this](const int a, const int b) {
        if (units_[a].timeToAct == units_[b].timeToAct) {
            return a < b;
        }
        return units_[a].timeToAct < units_[b].timeToAct;
    });

    activeUnitIndex_ = activeIndices.front();
}

void BattleState::ShiftPositions(const TeamSide side) {
    std::vector<BattleUnit*> line;
    line.reserve(3);

    for (auto& unit : units_) {
        if (unit.side == side && !unit.isAssignedLeader && IsAliveAndActive(unit)) {
            line.push_back(&unit);
        }
    }

    std::sort(line.begin(), line.end(), [](const BattleUnit* a, const BattleUnit* b) {
        return static_cast<int>(a->stats.position) < static_cast<int>(b->stats.position);
    });

    for (int i = 0; i < static_cast<int>(line.size()); ++i) {
        if (i == 0) {
            line[i]->stats.position = UnitPosition::Front;
        } else if (i == 1) {
            line[i]->stats.position = UnitPosition::Middle;
        } else {
            line[i]->stats.position = UnitPosition::Back;
        }
    }
}

void BattleState::FinalizeIfFinished() {
    bool alliesAlive = false;
    bool enemiesAlive = false;

    for (auto& unit : units_) {
        if (IsAlive(unit)) {
            if (unit.side == TeamSide::Allies) {
                alliesAlive = true;
            } else {
                enemiesAlive = true;
            }
        }
    }

    if (alliesAlive && enemiesAlive) {
        return;
    }

    summary_.finished = true;
    summary_.alliesWon = alliesAlive && !enemiesAlive;
    summary_.enemiesWon = enemiesAlive && !alliesAlive;

    if (summary_.alliesWon) {
        for (auto& unit : units_) {
            if (unit.side == TeamSide::Allies && unit.isPlayerCharacter && unit.hp <= 0) {
                unit.hp = 1;
                unit.ko = false;
                summary_.playerSetToOneHp = true;
            }
        }
    }

    for (auto& unit : units_) {
        if (unit.category == UnitCategory::Hero && unit.ko) {
            unit.lostAfterBattle = true;
        }
    }
}

} // namespace gameplay::battle

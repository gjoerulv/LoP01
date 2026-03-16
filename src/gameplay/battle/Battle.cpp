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

BattleState BattleState::CreateDebugBattle(const uint32_t seed) {
    std::vector<BattleUnit> units;
    units.reserve(6);

    BattleUnit playerLeader;
    playerLeader.id = "hero_player";
    playerLeader.name = "Wanderer";
    playerLeader.side = TeamSide::Allies;
    playerLeader.category = UnitCategory::Leader;
    playerLeader.isPlayerCharacter = true;
    playerLeader.stats = UnitStats{6, 5, 5, 4, 6, 8, 36, 18, 8, 1, UnitPosition::Leader, UnitRange::LongRanged};
    playerLeader.hp = playerLeader.stats.maxHp;
    playerLeader.mp = playerLeader.stats.maxMp;
    playerLeader.life = 1;
    playerLeader.inReserve = false;

    BattleUnit ally;
    ally.id = "unit_guard";
    ally.name = "Guard Recruit";
    ally.side = TeamSide::Allies;
    ally.category = UnitCategory::Generic;
    ally.stats = UnitStats{4, 3, 1, 1, 4, 6, 14, 0, 10, 3, UnitPosition::Front, UnitRange::Melee};
    ally.hp = ally.stats.maxHp;
    ally.mp = 0;
    ally.life = ally.stats.life;

    BattleUnit allyHero;
    allyHero.id = "hero_mira";
    allyHero.name = "Mira";
    allyHero.side = TeamSide::Allies;
    allyHero.category = UnitCategory::Hero;
    allyHero.stats = UnitStats{5, 3, 6, 3, 5, 7, 24, 14, 11, 1, UnitPosition::Back, UnitRange::LongRanged};
    allyHero.hp = allyHero.stats.maxHp;
    allyHero.mp = allyHero.stats.maxMp;
    allyHero.life = 1;

    BattleUnit enemyLeader;
    enemyLeader.id = "enemy_captain";
    enemyLeader.name = "Checkpoint Warden";
    enemyLeader.side = TeamSide::Enemies;
    enemyLeader.category = UnitCategory::Leader;
    enemyLeader.stats = UnitStats{5, 4, 4, 4, 5, 7, 30, 14, 7, 1, UnitPosition::Leader, UnitRange::LongRanged};
    enemyLeader.hp = enemyLeader.stats.maxHp;
    enemyLeader.mp = enemyLeader.stats.maxMp;
    enemyLeader.life = 1;

    BattleUnit enemyA;
    enemyA.id = "enemy_lancer";
    enemyA.name = "Lancer";
    enemyA.side = TeamSide::Enemies;
    enemyA.category = UnitCategory::Generic;
    enemyA.stats = UnitStats{4, 2, 1, 1, 3, 5, 12, 0, 9, 2, UnitPosition::Front, UnitRange::Melee};
    enemyA.hp = enemyA.stats.maxHp;
    enemyA.mp = 0;
    enemyA.life = enemyA.stats.life;

    BattleUnit enemyB;
    enemyB.id = "enemy_longbow";
    enemyB.name = "Longbow";
    enemyB.side = TeamSide::Enemies;
    enemyB.category = UnitCategory::Generic;
    enemyB.stats = UnitStats{3, 2, 1, 1, 3, 4, 10, 0, 10, 2, UnitPosition::Back, UnitRange::LongRanged};
    enemyB.hp = enemyB.stats.maxHp;
    enemyB.mp = 0;
    enemyB.life = enemyB.stats.life;

    units.push_back(playerLeader);
    units.push_back(ally);
    units.push_back(allyHero);
    units.push_back(enemyLeader);
    units.push_back(enemyA);
    units.push_back(enemyB);

    return CreateForTests(std::move(units), seed);
}

BattleState BattleState::CreateForTests(std::vector<BattleUnit> units, const uint32_t seed) {
    BattleState state(seed);
    state.SetUnits(std::move(units));
    return state;
}

BattleState::BattleState(const uint32_t seed) : random_(seed) {}

bool BattleState::SetUnits(std::vector<BattleUnit> units) {
    int allies = 0;
    int enemies = 0;

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
        if (unit.category == UnitCategory::Leader) {
            unit.stats.position = UnitPosition::Leader;
        }
    }

    for (auto& unit : units) {
        if (unit.category != UnitCategory::Leader) {
            continue;
        }

        bool hasNonLeader = false;
        for (const auto& other : units) {
            if (other.side == unit.side && other.category != UnitCategory::Leader && IsAlive(other)) {
                hasNonLeader = true;
                break;
            }
        }

        unit.inReserve = hasNonLeader;
    }

    if (allies == 0 || enemies == 0 || allies > kMaxUnitsPerSide || enemies > kMaxUnitsPerSide) {
        return false;
    }

    units_ = std::move(units);
    summary_ = {};
    lastActionText_ = "Battle started";
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

std::string BattleState::LastActionText() const {
    return lastActionText_;
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

    BattleUnit& actor = units_[activeUnitIndex_];
    if (!IsAliveAndActive(actor)) {
        AdvanceToNextTurn();
        return false;
    }

    actor.defending = false;

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
        lastActionText_ = actor.name + " attacks " + units_[resolvedTarget].name + " for " + std::to_string(damage);
        break;
    case BattleActionType::Defend:
        actor.defending = true;
        lastActionText_ = actor.name + " defends";
        break;
    case BattleActionType::Wait:
        waited = true;
        lastActionText_ = actor.name + " waits";
        break;
    case BattleActionType::Skill1:
        if (resolvedTarget < 0) {
            return false;
        }
        if (actor.mp < kSkill1MpCost) {
            lastActionText_ = actor.name + " lacks MP, uses attack";
            damage = ComputePhysicalDamage(actor, units_[resolvedTarget]);
        } else {
            actor.mp -= kSkill1MpCost;
            damage = static_cast<int>(std::round(ComputePhysicalDamage(actor, units_[resolvedTarget]) * 1.25f));
        }
        damage = ApplyDamage(units_[resolvedTarget], damage);
        lastActionText_ = actor.name + " uses Skill1 on " + units_[resolvedTarget].name + " for " + std::to_string(damage);
        break;
    case BattleActionType::Skill2:
        if (resolvedTarget < 0) {
            return false;
        }
        if (actor.mp < kSkill2MpCost) {
            lastActionText_ = actor.name + " lacks MP, uses attack";
            damage = ComputePhysicalDamage(actor, units_[resolvedTarget]);
        } else {
            actor.mp -= kSkill2MpCost;
            damage = ComputeMagicDamage(actor, units_[resolvedTarget], 1.5f);
        }
        damage = ApplyDamage(units_[resolvedTarget], damage);
        lastActionText_ = actor.name + " uses Skill2 on " + units_[resolvedTarget].name + " for " + std::to_string(damage);
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
    return IsAlive(unit) && !unit.inReserve;
}

bool BattleState::IsActionTargetable(const BattleUnit& unit) const {
    if (!IsAliveAndActive(unit)) {
        return false;
    }

    if (unit.category != UnitCategory::Leader) {
        return true;
    }

    for (const auto& other : units_) {
        if (other.side != unit.side || &other == &unit) {
            continue;
        }
        if (other.category != UnitCategory::Leader && IsAliveAndActive(other)) {
            return false;
        }
    }

    return true;
}

int BattleState::FindLeaderIndex(const TeamSide side) const {
    for (int i = 0; i < static_cast<int>(units_.size()); ++i) {
        if (units_[i].side == side && units_[i].category == UnitCategory::Leader) {
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
    for (auto& leader : units_) {
        if (leader.category != UnitCategory::Leader || !leader.inReserve || !IsAlive(leader)) {
            continue;
        }

        bool hasNonLeader = false;
        for (const auto& other : units_) {
            if (other.side == leader.side && other.category != UnitCategory::Leader && IsAliveAndActive(other)) {
                hasNonLeader = true;
                break;
            }
        }

        if (!hasNonLeader) {
            leader.inReserve = false;
            leader.timeToAct = std::max(1, leader.timeToAct);
        }
    }

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
        if (unit.side == side && unit.category != UnitCategory::Leader && IsAliveAndActive(unit)) {
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

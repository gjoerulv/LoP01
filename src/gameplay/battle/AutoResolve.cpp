#include "gameplay/battle/AutoResolve.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace gameplay::battle {

namespace {

// CTB gauge threshold. A unit acts when its gauge reaches this; gauge accrues by
// the unit's (clamped) agility each tick. The absolute value is irrelevant to the
// result — only agility ratios matter — but it must exceed any sane agility so a
// single tick-jump never makes a unit act twice.
constexpr long long kGaugeThreshold = 100000;

struct SimUnit {
    int index = 0;            // stable index within its own side
    bool defender = false;
    bool leader = false;
    UnitStats stats;
    long long maxHpPerBody = 1;
    long long currentHp = 1;  // quantity * maxHpPerBody, decremented as it takes damage
    int agility = 1;
    long long gauge = 0;

    [[nodiscard]] bool Alive() const { return currentHp > 0; }
    // Current stack count (Life) = bodies still standing.
    [[nodiscard]] long long Life() const {
        if (currentHp <= 0) return 0;
        return (currentHp + maxHpPerBody - 1) / maxHpPerBody;  // ceil
    }
};

// Leader aura: the living assigned leader's stat is added to every ally's stat
// (the leader buffs itself too — matches BattleState::EffectiveStat). Returns the
// leader's contribution for `field`, or 0 when that side has no living leader.
long long SideLeaderStat(const std::vector<SimUnit>& units, bool defenderSide,
                         int UnitStats::* field) {
    for (const auto& u : units) {
        if (u.defender == defenderSide && u.leader && u.Alive()) {
            return u.stats.*field;
        }
    }
    return 0;
}

long long EffectiveStat(const std::vector<SimUnit>& units, const SimUnit& unit,
                        int UnitStats::* field) {
    const long long base = unit.stats.*field;
    const long long aura = SideLeaderStat(units, unit.defender, field);
    return std::max<long long>(0, base + aura);
}

// Expected physical damage `attacker` deals to `defender`, mirroring
// BattleState::ComputePhysicalDamage but with the average roll instead of RNG.
long long ExpectedDamage(const std::vector<SimUnit>& units, const SimUnit& attacker,
                         const SimUnit& defender) {
    const int minD = std::min(attacker.stats.minDamage, attacker.stats.maxDamage);
    const int maxD = std::max(attacker.stats.minDamage, attacker.stats.maxDamage);
    const double avgRoll = (static_cast<double>(minD) + static_cast<double>(maxD)) / 2.0;
    const double base = avgRoll * static_cast<double>(std::max<long long>(1, attacker.Life()));

    const long long atk = EffectiveStat(units, attacker, &UnitStats::attack);
    const long long def = EffectiveStat(units, defender, &UnitStats::defense);
    const long long diff = atk - def;

    double modifier = 1.0;
    if (diff > 0) {
        modifier += static_cast<double>(diff) * 0.1;
    } else if (diff < 0) {
        modifier += static_cast<double>(diff) * 0.05;
    }
    modifier = std::clamp(modifier, 0.1, 3.0);

    const long long dmg = static_cast<long long>(std::llround(base * modifier));
    return std::max<long long>(1, dmg);
}

// Lowest-current-HP living enemy of `actor`, ties broken by stable index. Returns
// -1 when no living enemy remains.
int FindTarget(const std::vector<SimUnit>& units, const SimUnit& actor) {
    int best = -1;
    for (int i = 0; i < static_cast<int>(units.size()); ++i) {
        const SimUnit& u = units[i];
        if (u.defender == actor.defender || !u.Alive()) {
            continue;
        }
        if (best < 0 || u.currentHp < units[best].currentHp ||
            (u.currentHp == units[best].currentHp && i < best)) {
            best = i;
        }
    }
    return best;
}

std::vector<SimUnit> BuildSide(const std::vector<AutoResolveUnit>& source, bool defender) {
    std::vector<SimUnit> out;
    out.reserve(source.size());
    int idx = 0;
    for (const auto& src : source) {
        const int qty = std::max(0, src.quantity);
        if (qty <= 0) {
            continue;   // empty stacks contribute nothing
        }
        SimUnit u;
        u.index = idx++;
        u.defender = defender;
        u.leader = src.isLeader;
        u.stats = src.stats;
        u.maxHpPerBody = std::max<long long>(1, src.stats.maxHp);
        u.currentHp = u.maxHpPerBody * static_cast<long long>(qty);
        u.agility = std::clamp(src.stats.agility, 1, static_cast<int>(kGaugeThreshold));
        out.push_back(u);
    }
    return out;
}

bool SideAlive(const std::vector<SimUnit>& units, bool defender) {
    for (const auto& u : units) {
        if (u.defender == defender && u.Alive()) {
            return true;
        }
    }
    return false;
}

void Summarize(const std::vector<SimUnit>& units, bool defender,
               int& survivingUnits, long long& survivingQuantity) {
    survivingUnits = 0;
    survivingQuantity = 0;
    for (const auto& u : units) {
        if (u.defender == defender && u.Alive()) {
            ++survivingUnits;
            survivingQuantity += u.Life();
        }
    }
}

} // namespace

AutoResolveOutcome ResolveAuto(const std::vector<AutoResolveUnit>& attackers,
                               const std::vector<AutoResolveUnit>& defenders) {
    AutoResolveOutcome outcome;

    std::vector<SimUnit> units = BuildSide(attackers, /*defender=*/false);
    {
        std::vector<SimUnit> def = BuildSide(defenders, /*defender=*/true);
        units.insert(units.end(), def.begin(), def.end());
    }

    for (const auto& u : units) {
        if (u.defender) { ++outcome.defenderStartUnits; } else { ++outcome.attackerStartUnits; }
    }

    const bool attackersAlive = SideAlive(units, /*defender=*/false);
    const bool defendersAlive = SideAlive(units, /*defender=*/true);
    if (!attackersAlive || !defendersAlive) {
        // Empty/absent attackers => defenders win (tie-to-defender). Empty
        // defenders => attackers win. Both empty => defenders win.
        outcome.winner = (!defendersAlive && attackersAlive)
            ? AutoResolveWinner::Attackers
            : AutoResolveWinner::Defenders;
        Summarize(units, false, outcome.attackerSurvivingUnits, outcome.attackerSurvivingQuantity);
        Summarize(units, true, outcome.defenderSurvivingUnits, outcome.defenderSurvivingQuantity);
        return outcome;
    }

    // Hard action cap: every action removes >= 1 HP from one side, and total HP
    // never increases, so the battle terminates within the combined starting HP.
    long long totalHp = 0;
    for (const auto& u : units) {
        totalHp += u.currentHp;
    }
    const long long actionCap = totalHp + static_cast<long long>(units.size()) + 1;

    while (outcome.actionsTaken < actionCap) {
        if (!SideAlive(units, false) || !SideAlive(units, true)) {
            break;
        }

        // Advance time to the next ready unit (min ticks across living units).
        long long minTicks = -1;
        for (const auto& u : units) {
            if (!u.Alive()) continue;
            const long long remaining = kGaugeThreshold - u.gauge;
            const long long ticks = remaining <= 0 ? 0 : (remaining + u.agility - 1) / u.agility;
            if (minTicks < 0 || ticks < minTicks) {
                minTicks = ticks;
            }
        }
        if (minTicks < 0) {
            break;
        }
        for (auto& u : units) {
            if (u.Alive()) {
                u.gauge += minTicks * u.agility;
            }
        }

        // Collect ready units, ordered: highest gauge first, then defenders before
        // attackers, then by stable index — a fully deterministic order.
        std::vector<int> ready;
        for (int i = 0; i < static_cast<int>(units.size()); ++i) {
            if (units[i].Alive() && units[i].gauge >= kGaugeThreshold) {
                ready.push_back(i);
            }
        }
        std::sort(ready.begin(), ready.end(), [&](int a, int b) {
            if (units[a].gauge != units[b].gauge) return units[a].gauge > units[b].gauge;
            if (units[a].defender != units[b].defender) return units[a].defender;  // defender first
            return units[a].index < units[b].index;
        });

        for (const int actorIdx : ready) {
            SimUnit& actor = units[actorIdx];
            if (!actor.Alive()) {
                continue;
            }
            actor.gauge -= kGaugeThreshold;
            const int targetIdx = FindTarget(units, actor);
            if (targetIdx < 0) {
                break;   // no living enemy: battle is over
            }
            const long long dmg = ExpectedDamage(units, actor, units[targetIdx]);
            units[targetIdx].currentHp = std::max<long long>(0, units[targetIdx].currentHp - dmg);
            ++outcome.actionsTaken;
            if (!SideAlive(units, false) || !SideAlive(units, true)) {
                break;
            }
        }
    }

    const bool finalDefendersAlive = SideAlive(units, true);
    outcome.winner = finalDefendersAlive ? AutoResolveWinner::Defenders : AutoResolveWinner::Attackers;
    Summarize(units, false, outcome.attackerSurvivingUnits, outcome.attackerSurvivingQuantity);
    Summarize(units, true, outcome.defenderSurvivingUnits, outcome.defenderSurvivingQuantity);
    return outcome;
}

} // namespace gameplay::battle

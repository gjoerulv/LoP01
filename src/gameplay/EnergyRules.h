#pragma once

#include <algorithm>

namespace gameplay {

// Pure daily-starting Energy formula (docs/core_loop_rules.md §6):
//
//   1000 + (X * 100) + Y + Z
//
//   X = Agility of the lowest-Agility unit in the entire traveling party
//   Y = leader passive-skill Energy bonus
//   Z = leader equipped-item/artifact Energy bonus
//
// Y is supplied by the caller from the current leader's LeaderEnergy passive
// effects. Z is still a zero-valued seam (leader item/artifact Energy effects do
// not exist yet); the parameter stays so it can be filled without changing call
// sites. Never fake Z in content or UI.
//
// An empty traveling party yields lowestPartyAgility == 0 -> base 1000.
// Negative agility is guarded to 0 so a malformed unit can never reduce the
// base below 1000.
[[nodiscard]] inline int ComputeDailyStartingEnergy(
    int lowestPartyAgility,
    int leaderPassiveEnergyBonus,
    int leaderItemEnergyBonus)
{
    const int agility = std::max(0, lowestPartyAgility);
    return 1000 + (agility * 100) + leaderPassiveEnergyBonus + leaderItemEnergyBonus;
}

} // namespace gameplay

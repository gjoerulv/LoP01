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
// M14 passes Y and Z as 0 (zero-valued seams) because the skill system and
// item/artifact Energy effects do not exist yet. The parameters exist so a
// later milestone can supply real values without changing call sites.
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

#pragma once

#include <string>
#include <vector>

namespace data {

// Artifact slot kinds per docs/core_loop_rules.md §22:
//   each hero has 1 Attack slot + 1 Defense slot + 3 Misc slots.
// The Misc kind covers all three misc slot positions; the runtime layer (M13-b)
// disambiguates by named slot field (misc1/misc2/misc3).
enum class ArtifactSlotKind { Attack, Defense, Misc };

inline bool ArtifactSlotKindFromString(const std::string& value, ArtifactSlotKind& out) {
    if (value == "Attack")  { out = ArtifactSlotKind::Attack;  return true; }
    if (value == "Defense") { out = ArtifactSlotKind::Defense; return true; }
    if (value == "Misc")    { out = ArtifactSlotKind::Misc;    return true; }
    return false;
}

// Affected stat for a statBonus effect. Mirrors UnitStatsDefinition core stats.
// M13 implements only statBonus effects on artifacts; the doc's `specialEffect`
// enum is rejected at validation time as an unsupported effect type.
enum class ArtifactStatBonusStat { Attack, Defense, Magic, Resistance };

inline bool ArtifactStatBonusStatFromString(const std::string& value, ArtifactStatBonusStat& out) {
    if (value == "Attack")     { out = ArtifactStatBonusStat::Attack;     return true; }
    if (value == "Defense")    { out = ArtifactStatBonusStat::Defense;    return true; }
    if (value == "Magic")      { out = ArtifactStatBonusStat::Magic;      return true; }
    if (value == "Resistance") { out = ArtifactStatBonusStat::Resistance; return true; }
    return false;
}

struct ArtifactStatBonus {
    ArtifactStatBonusStat stat = ArtifactStatBonusStat::Attack;
    int amount = 0;
};

struct ArtifactDefinition {
    std::string id;
    std::string name;                              // English name
    std::string icon;
    std::vector<ArtifactSlotKind> allowedSlots;    // non-empty after validation
    std::string rarity;                            // free-form in M13 (enum deferred)
    int tier = 0;
    int baseValue = 0;
    bool combinable = false;
    std::vector<ArtifactStatBonus> statBonuses;
};

} // namespace data

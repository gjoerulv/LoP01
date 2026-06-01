#pragma once

#include <set>
#include <string>
#include <vector>

#include "data/definitions/CampaignDefinition.h"

namespace gameplay::campaign {

// Small campaign/domain snapshot of carry-over-eligible runtime state (M16).
// These structs are owned by the campaign module and intentionally mirror the
// GameSession runtime shapes WITHOUT depending on core::SaveData (the
// persistence schema) or pulling in the heavy GameSession.h. GameSession adapts
// to/from this snapshot when capturing and re-applying carry-over.

struct CarryRosterStack {
    std::string stackId;
    std::string unitId;
    int quantity = 0;
};

struct CarryItemStack {
    std::string itemId;
    int quantity = 0;
};

struct CarryArtifactStack {
    std::string artifactId;
    int quantity = 0;
};

// One hero's equipped-artifact ids, keyed by hero/unit id (heroes are unique).
struct CarryHeroEquipment {
    std::string heroId;
    std::string attackArtifactId;
    std::string defenseArtifactId;
    std::string misc1ArtifactId;
    std::string misc2ArtifactId;
    std::string misc3ArtifactId;
};

struct CampaignCarrySet {
    int gold = 0;
    std::vector<CarryRosterStack> rosterStacks;
    // Positional slot ids (empty string = empty slot), mirroring GameSession's
    // active/reserve slot vectors. A dropped roster stack clears its slot.
    std::vector<std::string> activeSlotStackIds;
    std::vector<std::string> reserveSlotStackIds;
    std::vector<CarryItemStack> items;
    std::vector<CarryArtifactStack> artifacts;       // unequipped only
    std::vector<CarryHeroEquipment> heroEquipment;
    std::set<std::string> storyFlags;                // scenario story flags
};

// Pure carry-over filtering. Raylib-free and deterministic.
//
// Returns a new CampaignCarrySet holding only the state the rule permits to
// survive the scenario transition:
//   - Player hero (playerHeroUnitId) is ALWAYS retained in the roster and its
//     active-slot placement, regardless of carryRoster.
//   - carryRoster=true  => keep all roster stacks whose unitId is in validUnitIds;
//     carryRoster=false  => keep only the player hero stack.
//     Dropped stacks are also cleared from the active/reserve slot id lists.
//   - carryItems=false     => items cleared.
//   - carryArtifacts=false => unequipped artifacts AND hero equipment cleared.
//     When true, hero equipment is kept only for heroes whose stack is retained.
//   - carryGold=false      => gold reset to fallbackGold.
//   - carryStoryFlags      => only the named flags present in `from` are kept.
[[nodiscard]] CampaignCarrySet ApplyCarryOver(
    const data::CarryOverRule& rule,
    const CampaignCarrySet& from,
    const std::string& playerHeroUnitId,
    int fallbackGold,
    const std::set<std::string>& validUnitIds);

} // namespace gameplay::campaign

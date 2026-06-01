#include "gameplay/campaign/CampaignCarryover.h"

#include <algorithm>

namespace gameplay::campaign {

CampaignCarrySet ApplyCarryOver(
    const data::CarryOverRule& rule,
    const CampaignCarrySet& from,
    const std::string& playerHeroUnitId,
    int fallbackGold,
    const std::set<std::string>& validUnitIds)
{
    CampaignCarrySet result;

    // --- Roster -------------------------------------------------------------
    // Player hero is always retained. Other stacks are retained per rule, gated
    // by valid unit ids.
    std::set<std::string> retainedStackIds;
    for (const auto& stack : from.rosterStacks) {
        const bool isPlayerHero = !playerHeroUnitId.empty() && stack.unitId == playerHeroUnitId;
        const bool unitIsValid  = validUnitIds.count(stack.unitId) > 0;

        bool retain = false;
        if (isPlayerHero) {
            retain = true;                       // always carried
        } else if (rule.carryRoster && unitIsValid) {
            retain = true;
        }

        if (retain) {
            result.rosterStacks.push_back(stack);
            retainedStackIds.insert(stack.stackId);
        }
    }

    // Positional slots: keep retained stack ids, clear (empty string) otherwise.
    auto filterSlots = [&](const std::vector<std::string>& slots) {
        std::vector<std::string> out;
        out.reserve(slots.size());
        for (const auto& slotStackId : slots) {
            if (!slotStackId.empty() && retainedStackIds.count(slotStackId) > 0) {
                out.push_back(slotStackId);
            } else {
                out.push_back("");
            }
        }
        return out;
    };
    result.activeSlotStackIds  = filterSlots(from.activeSlotStackIds);
    result.reserveSlotStackIds = filterSlots(from.reserveSlotStackIds);

    // --- Items --------------------------------------------------------------
    if (rule.carryItems) {
        result.items = from.items;
    }

    // --- Artifacts (unequipped) + hero equipment ----------------------------
    if (rule.carryArtifacts) {
        result.artifacts = from.artifacts;
        // Retain equipment only for heroes whose roster stack survived.
        std::set<std::string> retainedHeroUnitIds;
        for (const auto& stack : result.rosterStacks) {
            retainedHeroUnitIds.insert(stack.unitId);
        }
        for (const auto& eq : from.heroEquipment) {
            if (retainedHeroUnitIds.count(eq.heroId) > 0) {
                result.heroEquipment.push_back(eq);
            }
        }
    }

    // --- Gold ---------------------------------------------------------------
    result.gold = rule.carryGold ? from.gold : fallbackGold;

    // --- Scenario story flags ----------------------------------------------
    for (const auto& flag : rule.carryStoryFlags) {
        if (from.storyFlags.count(flag) > 0) {
            result.storyFlags.insert(flag);
        }
    }

    return result;
}

} // namespace gameplay::campaign

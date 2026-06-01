#include <catch2/catch_test_macros.hpp>

#include <algorithm>

#include "data/definitions/CampaignDefinition.h"
#include "gameplay/campaign/CampaignCarryover.h"

using namespace gameplay::campaign;

namespace {

// player hero "hero", an ally "ally", and an unknown/invalid unit "ghost".
CampaignCarrySet MakeSourceState() {
    CampaignCarrySet s;
    s.gold = 999;
    s.rosterStacks = {
        {"stk_1", "hero",  1},
        {"stk_2", "ally",  3},
        {"stk_3", "ghost", 2},
    };
    s.activeSlotStackIds  = {"stk_1", "stk_2", "", "", ""};
    s.reserveSlotStackIds = {"stk_3", "", "", "", "", "", "", ""};
    s.items     = {{"item_ration", 1}};
    s.artifacts = {{"artifact_sword", 1}};
    s.heroEquipment = {
        {"hero", "artifact_sword", "", "", "", ""},
        {"ally", "", "artifact_shield", "", "", ""},
    };
    s.storyFlags = {"flag_keep", "flag_drop"};
    return s;
}

const std::set<std::string> kValidUnitIds = {"hero", "ally"};   // "ghost" is invalid

bool HasStack(const CampaignCarrySet& s, const std::string& unitId) {
    return std::any_of(s.rosterStacks.begin(), s.rosterStacks.end(),
        [&](const CarryRosterStack& st) { return st.unitId == unitId; });
}

bool HasEquipmentFor(const CampaignCarrySet& s, const std::string& heroId) {
    return std::any_of(s.heroEquipment.begin(), s.heroEquipment.end(),
        [&](const CarryHeroEquipment& e) { return e.heroId == heroId; });
}

} // namespace

TEST_CASE("Carry-over: carryRoster=true keeps valid units, drops invalid unit ids") {
    data::CarryOverRule rule;
    rule.id = "r";
    rule.carryRoster = true;
    const auto out = ApplyCarryOver(rule, MakeSourceState(), "hero", 100, kValidUnitIds);

    REQUIRE(HasStack(out, "hero"));
    REQUIRE(HasStack(out, "ally"));
    REQUIRE_FALSE(HasStack(out, "ghost"));   // unit id not in catalog -> dropped
    // The dropped stack's reserve slot is cleared.
    REQUIRE(out.reserveSlotStackIds[0].empty());
    REQUIRE(out.activeSlotStackIds[0] == "stk_1");
    REQUIRE(out.activeSlotStackIds[1] == "stk_2");
}

TEST_CASE("Carry-over: carryRoster=false keeps only the player hero") {
    data::CarryOverRule rule;
    rule.id = "r";
    rule.carryRoster = false;
    const auto out = ApplyCarryOver(rule, MakeSourceState(), "hero", 100, kValidUnitIds);

    REQUIRE(HasStack(out, "hero"));
    REQUIRE_FALSE(HasStack(out, "ally"));
    REQUIRE_FALSE(HasStack(out, "ghost"));
    REQUIRE(out.activeSlotStackIds[0] == "stk_1");
    REQUIRE(out.activeSlotStackIds[1].empty());   // ally slot cleared
}

TEST_CASE("Carry-over: player hero always retained even when not in valid set") {
    data::CarryOverRule rule;
    rule.id = "r";
    rule.carryRoster = true;
    // Empty valid set: only the player hero should survive.
    const auto out = ApplyCarryOver(rule, MakeSourceState(), "hero", 100, {});
    REQUIRE(HasStack(out, "hero"));
    REQUIRE_FALSE(HasStack(out, "ally"));
}

TEST_CASE("Carry-over: carryItems toggles item inventory") {
    data::CarryOverRule rule;
    rule.id = "r";
    rule.carryItems = true;
    REQUIRE_FALSE(ApplyCarryOver(rule, MakeSourceState(), "hero", 100, kValidUnitIds).items.empty());

    rule.carryItems = false;
    REQUIRE(ApplyCarryOver(rule, MakeSourceState(), "hero", 100, kValidUnitIds).items.empty());
}

TEST_CASE("Carry-over: carryArtifacts governs unequipped artifacts and hero equipment") {
    data::CarryOverRule rule;
    rule.id = "r";
    rule.carryRoster = true;
    rule.carryArtifacts = true;
    {
        const auto out = ApplyCarryOver(rule, MakeSourceState(), "hero", 100, kValidUnitIds);
        REQUIRE_FALSE(out.artifacts.empty());
        REQUIRE(HasEquipmentFor(out, "hero"));
        REQUIRE(HasEquipmentFor(out, "ally"));
    }
    rule.carryArtifacts = false;
    {
        const auto out = ApplyCarryOver(rule, MakeSourceState(), "hero", 100, kValidUnitIds);
        REQUIRE(out.artifacts.empty());
        REQUIRE(out.heroEquipment.empty());
    }
}

TEST_CASE("Carry-over: equipment for dropped heroes is not carried") {
    data::CarryOverRule rule;
    rule.id = "r";
    rule.carryRoster = false;    // drop ally
    rule.carryArtifacts = true;
    const auto out = ApplyCarryOver(rule, MakeSourceState(), "hero", 100, kValidUnitIds);
    REQUIRE(HasEquipmentFor(out, "hero"));
    REQUIRE_FALSE(HasEquipmentFor(out, "ally"));   // ally dropped -> its equipment gone
}

TEST_CASE("Carry-over: carryGold false resets to fallback") {
    data::CarryOverRule rule;
    rule.id = "r";
    rule.carryGold = false;
    REQUIRE(ApplyCarryOver(rule, MakeSourceState(), "hero", 100, kValidUnitIds).gold == 100);

    rule.carryGold = true;
    REQUIRE(ApplyCarryOver(rule, MakeSourceState(), "hero", 100, kValidUnitIds).gold == 999);
}

TEST_CASE("Carry-over: only named story flags are preserved") {
    data::CarryOverRule rule;
    rule.id = "r";
    rule.carryStoryFlags = {"flag_keep", "flag_absent"};
    const auto out = ApplyCarryOver(rule, MakeSourceState(), "hero", 100, kValidUnitIds);
    REQUIRE(out.storyFlags.count("flag_keep") == 1);
    REQUIRE(out.storyFlags.count("flag_drop") == 0);     // not in allow-list
    REQUIRE(out.storyFlags.count("flag_absent") == 0);   // allow-listed but absent in source
}

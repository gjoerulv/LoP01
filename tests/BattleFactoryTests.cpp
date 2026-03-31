#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "data/ContentRepository.h"
#include "gameplay/battle/BattleFactory.h"

namespace {

void WriteTextFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream output(path, std::ios::trunc);
    output << content;
}

std::filesystem::path BuildBattleFactoryTestContent(
    const std::string& folderName,
    const std::string& battleScenariosJson = R"({"battle_scenarios":[{"id":"test_scenario","name":"Test Scenario","seed":7,"allies":[{"unit_id":"hero_player"},{"unit_id":"hero_mira"}],"enemies":[{"unit_id":"enemy_captain"},{"unit_id":"unit_longbow"}]}]})") {
    const std::filesystem::path root = std::filesystem::path("saves") / folderName;
    std::filesystem::create_directories(root);

    WriteTextFile(root / "regions.json", R"({"regions":[{"id":"ashvale_heartland","name":"Ashvale Heartland","unlocked":true,"nodes":[{"location_id":"test_location","x":0,"y":0,"discovered":true,"travel_available":true}],"links":[]}]})");
    WriteTextFile(root / "locations.json", R"({"locations":[{"id":"test_location","name":"Test Location","type":"combat","allows_sleep":false,"overworld_destination":true,"scene_id":"test_scene","battle_scenario_id":"test_scenario"}]})");
    WriteTextFile(root / "location_scenes.json", R"({"location_scenes":[{"id":"test_scene","spawn":{"x":0,"y":0,"width":1,"height":1},"blocking_rects":[],"zones":[]}]})");
    WriteTextFile(root / "units.json", R"({"units":[{"id":"hero_player","name":"Wanderer","category":"leader","is_player_character":true,"attack":6,"defense":5,"magic":5,"resistance":4,"min_damage":6,"max_damage":8,"max_hp":36,"max_mp":18,"agility":8,"life":1,"position":"leader","range":"long_ranged"},{"id":"hero_mira","name":"Mira","category":"hero","is_player_character":false,"attack":5,"defense":3,"magic":6,"resistance":3,"min_damage":5,"max_damage":7,"max_hp":24,"max_mp":14,"agility":11,"life":1,"position":"back","range":"long_ranged"},{"id":"unit_guard","name":"Guard Recruit","category":"generic","is_player_character":false,"attack":4,"defense":3,"magic":1,"resistance":1,"min_damage":4,"max_damage":6,"max_hp":14,"max_mp":0,"agility":10,"life":3,"position":"front","range":"melee"},{"id":"unit_medic","name":"Field Medic","category":"generic","is_player_character":false,"attack":2,"defense":2,"magic":4,"resistance":3,"min_damage":2,"max_damage":4,"max_hp":11,"max_mp":8,"agility":8,"life":2,"position":"back","range":"ranged"},{"id":"enemy_captain","name":"Enemy Captain","category":"leader","is_player_character":false,"attack":5,"defense":4,"magic":4,"resistance":4,"min_damage":5,"max_damage":7,"max_hp":30,"max_mp":14,"agility":7,"life":1,"position":"leader","range":"long_ranged"},{"id":"unit_longbow","name":"Longbow","category":"generic","is_player_character":false,"attack":3,"defense":2,"magic":1,"resistance":1,"min_damage":3,"max_damage":4,"max_hp":10,"max_mp":0,"agility":10,"life":2,"position":"back","range":"long_ranged"}]})");
    WriteTextFile(root / "battle_scenarios.json", battleScenariosJson);
    WriteTextFile(root / "enemy_groups.json", R"({"enemy_groups":[]})");
    WriteTextFile(root / "quests.json", R"({"quests":[]})");
    WriteTextFile(root / "location_services.json", R"({"location_services":[]})");

    return root;
}

std::string FindAssignedLeaderIdBySide(
    const std::vector<gameplay::battle::BattleUnit>& units,
    const gameplay::battle::TeamSide side) {
    for (const auto& unit : units) {
        if (unit.side == side && unit.isAssignedLeader) {
            return unit.id;
        }
    }

    return "";
}

int CountAssignedLeadersBySide(
    const std::vector<gameplay::battle::BattleUnit>& units,
    const gameplay::battle::TeamSide side) {
    int count = 0;
    for (const auto& unit : units) {
        if (unit.side == side && unit.isAssignedLeader) {
            ++count;
        }
    }

    return count;
}

std::vector<std::string> CollectUnitIdsBySide(
    const std::vector<gameplay::battle::BattleUnit>& units,
    const gameplay::battle::TeamSide side) {
    std::vector<std::string> ids;
    for (const auto& unit : units) {
        if (unit.side == side) {
            ids.push_back(unit.id);
        }
    }

    return ids;
}

int CountPlayerCharacterAllies(const std::vector<gameplay::battle::BattleUnit>& units) {
    int count = 0;
    for (const auto& unit : units) {
        if (unit.side == gameplay::battle::TeamSide::Allies && unit.isPlayerCharacter) {
            ++count;
        }
    }

    return count;
}

std::vector<gameplay::battle::PlayerBattleEntry> BuildActiveEntries(
    const std::vector<std::string>& stackIds,
    const std::vector<std::string>& unitIds,
    const std::vector<int>& quantities) {
    std::vector<gameplay::battle::PlayerBattleEntry> entries;
    const size_t count = std::min(stackIds.size(), std::min(unitIds.size(), quantities.size()));
    entries.reserve(count);

    for (size_t i = 0; i < count; ++i) {
        entries.push_back(gameplay::battle::PlayerBattleEntry{
            static_cast<int>(i),
            stackIds[i],
            unitIds[i],
            quantities[i]
        });
    }

    return entries;
}

} // namespace

TEST_CASE("EmptyActiveParty_UsesScenarioAuthoredAllies") {
    const auto root = BuildBattleFactoryTestContent("battle_factory_test_empty_active");

    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));

    const auto battle = gameplay::battle::BattleFactory::CreateFromScenario(content, "test_scenario", {}, 7);
    REQUIRE(battle.has_value());

    const auto allies = CollectUnitIdsBySide(battle->Units(), gameplay::battle::TeamSide::Allies);
    REQUIRE(allies == std::vector<std::string>{"hero_player", "hero_mira"});

    std::filesystem::remove_all(root);
}

TEST_CASE("LeaderlessPlayerActiveOverride_FailsBattleCreation") {
    const auto root = BuildBattleFactoryTestContent("battle_factory_test_leaderless_player_override_failure");

    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));

    const auto activeParty = BuildActiveEntries(
        {"stk_1", "stk_2"},
        {"unit_guard", "unit_medic"},
        {3, 2});

    const auto battle = gameplay::battle::BattleFactory::CreateFromScenario(content, "test_scenario", activeParty, 7);
    REQUIRE_FALSE(battle.has_value());

    std::filesystem::remove_all(root);
}

TEST_CASE("ActivePartyOverride_PreservesDeterministicOrderForUpToFiveActiveEntries") {
    const auto root = BuildBattleFactoryTestContent("battle_factory_test_order_five_active");

    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));

    const auto activeParty = BuildActiveEntries(
        {"stk_1", "stk_2", "stk_3", "stk_4", "stk_5"},
        {"hero_mira", "unit_guard", "unit_medic", "unit_guard", "unit_medic"},
        {1, 2, 2, 1, 1});
    const auto battle = gameplay::battle::BattleFactory::CreateFromScenario(content, "test_scenario", activeParty, 7);
    REQUIRE(battle.has_value());

    const auto allies = CollectUnitIdsBySide(battle->Units(), gameplay::battle::TeamSide::Allies);
    REQUIRE(allies == std::vector<std::string>{"hero_mira", "unit_guard", "unit_medic", "unit_guard", "unit_medic"});

    std::filesystem::remove_all(root);
}

TEST_CASE("ValidActiveParty_FullyOverridesScenarioAuthoredAllies") {
    const auto root = BuildBattleFactoryTestContent("battle_factory_test_valid_override");

    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));

    const auto activeParty = BuildActiveEntries(
        {"stk_1", "stk_2"},
        {"hero_mira", "unit_guard"},
        {1, 3});
    const auto battle = gameplay::battle::BattleFactory::CreateFromScenario(content, "test_scenario", activeParty, 7);
    REQUIRE(battle.has_value());

    const auto allies = CollectUnitIdsBySide(battle->Units(), gameplay::battle::TeamSide::Allies);
    REQUIRE(allies == std::vector<std::string>{"hero_mira", "unit_guard"});

    std::filesystem::remove_all(root);
}

TEST_CASE("MixedValidAndInvalidActiveParty_UsesOnlyValidResolvedActivePartyAllies") {
    const auto root = BuildBattleFactoryTestContent("battle_factory_test_mixed_override");

    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));

    const auto activeParty = BuildActiveEntries(
        {"stk_1", "stk_2", "stk_3"},
        {"hero_mira", "unit_missing", "unit_guard"},
        {1, 1, 3});
    const auto battle = gameplay::battle::BattleFactory::CreateFromScenario(content, "test_scenario", activeParty, 7);
    REQUIRE(battle.has_value());

    const auto allies = CollectUnitIdsBySide(battle->Units(), gameplay::battle::TeamSide::Allies);
    REQUIRE(allies == std::vector<std::string>{"hero_mira", "unit_guard"});

    std::filesystem::remove_all(root);
}

TEST_CASE("AllInvalidActiveParty_FallsBackToScenarioAuthoredAllies") {
    const auto root = BuildBattleFactoryTestContent("battle_factory_test_all_invalid");

    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));

    const auto activeParty = BuildActiveEntries(
        {"stk_1", "stk_2"},
        {"unit_missing_a", "unit_missing_b"},
        {1, 1});
    const auto battle = gameplay::battle::BattleFactory::CreateFromScenario(content, "test_scenario", activeParty, 7);
    REQUIRE(battle.has_value());

    const auto allies = CollectUnitIdsBySide(battle->Units(), gameplay::battle::TeamSide::Allies);
    REQUIRE(allies == std::vector<std::string>{"hero_player", "hero_mira"});

    std::filesystem::remove_all(root);
}

TEST_CASE("ActivePartyOverride_EnemiesRemainScenarioAuthored") {
    const auto root = BuildBattleFactoryTestContent("battle_factory_test_enemy_authored");

    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));

    const auto activeParty = BuildActiveEntries(
        {"stk_1", "stk_2"},
        {"hero_mira", "unit_guard"},
        {1, 3});
    const auto battle = gameplay::battle::BattleFactory::CreateFromScenario(content, "test_scenario", activeParty, 7);
    REQUIRE(battle.has_value());

    const auto enemies = CollectUnitIdsBySide(battle->Units(), gameplay::battle::TeamSide::Enemies);
    REQUIRE(enemies == std::vector<std::string>{"enemy_captain", "unit_longbow"});

    std::filesystem::remove_all(root);
}

TEST_CASE("ActivePartyOverride_PreservesOrderAndDuplicates") {
    const auto root = BuildBattleFactoryTestContent("battle_factory_test_order_duplicates");

    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));

    const auto activeParty = BuildActiveEntries(
        {"stk_1", "stk_2", "stk_3"},
        {"hero_mira", "unit_guard", "unit_guard"},
        {1, 2, 1});
    const auto battle = gameplay::battle::BattleFactory::CreateFromScenario(content, "test_scenario", activeParty, 7);
    REQUIRE(battle.has_value());

    const auto allies = CollectUnitIdsBySide(battle->Units(), gameplay::battle::TeamSide::Allies);
    REQUIRE(allies == std::vector<std::string>{"hero_mira", "unit_guard", "unit_guard"});

    std::filesystem::remove_all(root);
}

TEST_CASE("ActivePartyGenericOnly_IsRejectedWhenNoLeaderCapableAllyExists") {
    const auto root = BuildBattleFactoryTestContent("battle_factory_test_generic_only");

    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));

    const auto activeParty = BuildActiveEntries(
        {"stk_1", "stk_2"},
        {"unit_guard", "unit_medic"},
        {3, 2});
    const auto battle = gameplay::battle::BattleFactory::CreateFromScenario(content, "test_scenario", activeParty, 7);
    REQUIRE_FALSE(battle.has_value());

    std::filesystem::remove_all(root);
}

TEST_CASE("ActivePartyEntries_PreserveRosterStackIdAndQuantityToBattleLife") {
    const auto root = BuildBattleFactoryTestContent("battle_factory_test_roster_identity_life");

    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));

    const auto activeParty = BuildActiveEntries(
        {"stk_10", "stk_11"},
        {"hero_mira", "unit_guard"},
        {1, 5});
    const auto battle = gameplay::battle::BattleFactory::CreateFromScenario(content, "test_scenario", activeParty, 7);
    REQUIRE(battle.has_value());

    std::vector<std::string> allyStackIds;
    std::vector<int> allyLife;
    for (const auto& unit : battle->Units()) {
        if (unit.side == gameplay::battle::TeamSide::Allies) {
            allyStackIds.push_back(unit.rosterStackId);
            allyLife.push_back(unit.life);
        }
    }

    REQUIRE(allyStackIds == std::vector<std::string>{"stk_10", "stk_11"});
    REQUIRE(allyLife == std::vector<int>{1, 5});

    std::filesystem::remove_all(root);
}

TEST_CASE("EnemyUnits_DoNotCarryRosterStackIdentity") {
    const auto root = BuildBattleFactoryTestContent("battle_factory_test_enemy_no_roster_identity");

    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));

    const auto activeParty = BuildActiveEntries(
        {"stk_1", "stk_2"},
        {"hero_mira", "unit_guard"},
        {1, 3});
    const auto battle = gameplay::battle::BattleFactory::CreateFromScenario(content, "test_scenario", activeParty, 7);
    REQUIRE(battle.has_value());

    for (const auto& unit : battle->Units()) {
        if (unit.side == gameplay::battle::TeamSide::Enemies) {
            REQUIRE(unit.rosterStackId.empty());
        }
    }

    std::filesystem::remove_all(root);
}

TEST_CASE("LeaderAssignment_PlayerCharacterIsPreferredWhenPresent") {
    const auto root = BuildBattleFactoryTestContent("battle_factory_test_player_character_leader_preferred");

    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));

    const auto activeParty = BuildActiveEntries(
        {"stk_1", "stk_2", "stk_3"},
        {"hero_mira", "hero_player", "unit_guard"},
        {1, 1, 3});
    const auto battle = gameplay::battle::BattleFactory::CreateFromScenario(content, "test_scenario", activeParty, 7);
    REQUIRE(battle.has_value());

    REQUIRE(CountAssignedLeadersBySide(battle->Units(), gameplay::battle::TeamSide::Allies) == 1);
    REQUIRE(FindAssignedLeaderIdBySide(battle->Units(), gameplay::battle::TeamSide::Allies) == "hero_player");
    for (const auto& unit : battle->Units()) {
        if (unit.side == gameplay::battle::TeamSide::Allies && unit.isAssignedLeader) {
            REQUIRE(unit.stats.position == gameplay::battle::UnitPosition::Leader);
        }
    }

    std::filesystem::remove_all(root);
}

TEST_CASE("LeaderAssignment_PlayerFallsBackToFirstLeaderCapableByActiveOrder") {
    const auto root = BuildBattleFactoryTestContent("battle_factory_test_player_leader_fallback_order");

    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));

    const auto activeParty = BuildActiveEntries(
        {"stk_1", "stk_2"},
        {"hero_mira", "unit_guard"},
        {1, 3});
    const auto battle = gameplay::battle::BattleFactory::CreateFromScenario(content, "test_scenario", activeParty, 7);
    REQUIRE(battle.has_value());

    REQUIRE(CountAssignedLeadersBySide(battle->Units(), gameplay::battle::TeamSide::Allies) == 1);
    REQUIRE(FindAssignedLeaderIdBySide(battle->Units(), gameplay::battle::TeamSide::Allies) == "hero_mira");

    std::filesystem::remove_all(root);
}

TEST_CASE("LeaderAssignment_EnemyUsesFirstLeaderCapableByResolvedOrder") {
    const auto root = BuildBattleFactoryTestContent("battle_factory_test_enemy_leader_assignment");

    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));

    const auto battle = gameplay::battle::BattleFactory::CreateFromScenario(content, "test_scenario", {}, 7);
    REQUIRE(battle.has_value());

    REQUIRE(CountAssignedLeadersBySide(battle->Units(), gameplay::battle::TeamSide::Enemies) == 1);
    REQUIRE(FindAssignedLeaderIdBySide(battle->Units(), gameplay::battle::TeamSide::Enemies) == "enemy_captain");
    for (const auto& unit : battle->Units()) {
        if (unit.side == gameplay::battle::TeamSide::Enemies && unit.isAssignedLeader) {
            REQUIRE(unit.stats.position == gameplay::battle::UnitPosition::Leader);
        }
    }

    std::filesystem::remove_all(root);
}

TEST_CASE("EnemyTeamWithoutLeaderCapableUnits_IsValidAndHasNoAssignedLeader") {
    const auto root = BuildBattleFactoryTestContent(
        "battle_factory_test_enemy_no_leader",
        R"({"battle_scenarios":[{"id":"test_scenario","name":"Test Scenario","seed":7,"allies":[{"unit_id":"hero_player"},{"unit_id":"hero_mira"}],"enemies":[{"unit_id":"unit_longbow"}]}]})");

    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));

    const auto battle = gameplay::battle::BattleFactory::CreateFromScenario(content, "test_scenario", {}, 7);
    REQUIRE(battle.has_value());
    REQUIRE(CountAssignedLeadersBySide(battle->Units(), gameplay::battle::TeamSide::Enemies) == 0);

    std::filesystem::remove_all(root);
}

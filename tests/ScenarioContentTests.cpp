#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "data/ContentRepository.h"

namespace {

void WriteTextFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream output(path, std::ios::trunc);
    output << content;
}

// Two-region baseline (alpha, beta), mirroring WorldMapContentTests.
void WriteBaselineContent(const std::filesystem::path& root) {
    std::filesystem::create_directories(root);
    WriteTextFile(root / "regions.json", R"({"schemaVersion":1,"kind":"RegionCollection","id":"regions","regions":[
        {"id":"alpha","name":"Alpha","unlocked":true,"arrivalNodeId":"alpha_arrival","nodes":[
            {"location_id":"alpha_arrival","x":0,"y":0},{"location_id":"alpha_exit","x":1,"y":0}],
            "links":[["alpha_arrival","alpha_exit"]]},
        {"id":"beta","name":"Beta","unlocked":true,"arrivalNodeId":"beta_arrival","nodes":[
            {"location_id":"beta_arrival","x":0,"y":0}],"links":[]}
    ]})");
    WriteTextFile(root / "locations.json", R"({"schemaVersion":1,"kind":"LocationCollection","id":"locations","locations":[
        {"id":"alpha_arrival","name":"Alpha Arrival","type":"town","allows_sleep":false,"overworld_destination":true},
        {"id":"alpha_exit","name":"Alpha Exit","type":"town","allows_sleep":false,"overworld_destination":true},
        {"id":"beta_arrival","name":"Beta Arrival","type":"town","allows_sleep":false,"overworld_destination":true}
    ]})");
    WriteTextFile(root / "location_scenes.json", R"({"schemaVersion":1,"kind":"LocationSceneCollection","id":"location_scenes","location_scenes":[]})");
    WriteTextFile(root / "units.json", R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[{"id":"hero","name":"Hero","category":"hero","is_player_character":true,"attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":1,"life":1,"position":"front","range":"melee"}]})");
    WriteTextFile(root / "battle_scenarios.json", R"({"schemaVersion":1,"kind":"BattleScenarioCollection","id":"battle_scenarios","battle_scenarios":[]})");
    WriteTextFile(root / "enemy_groups.json", R"({"schemaVersion":1,"kind":"EnemyGroupCollection","id":"enemy_groups","enemy_groups":[]})");
    WriteTextFile(root / "quests.json", R"({"schemaVersion":1,"kind":"QuestCollection","id":"quests","quests":[]})");
    WriteTextFile(root / "location_services.json", R"({"schemaVersion":1,"kind":"LocationServiceCollection","id":"location_services","location_services":[]})");
}

bool HasMessageWithCode(const std::vector<ValidationMessage>& msgs, const std::string& code) {
    return std::any_of(msgs.begin(), msgs.end(),
        [&](const ValidationMessage& m) { return m.code == code; });
}

} // namespace

TEST_CASE("Scenario content: missing scenarios.json yields no scenarios (legal)") {
    const std::filesystem::path root = "saves/scenario_missing_test";
    WriteBaselineContent(root);
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(root));
    REQUIRE(repo.Scenarios().empty());
    std::filesystem::remove_all(root);
}

TEST_CASE("Scenario content: a valid scenario loads its fields") {
    const std::filesystem::path root = "saves/scenario_valid_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "scenarios.json", R"({"schemaVersion":1,"kind":"ScenarioCollection","id":"scenarios","scenarios":[
        {"id":"s_intro","name":"Intro","startRegionId":"alpha","startNodeId":"alpha_exit","startGold":1234,"standaloneSelectable":true}
    ]})");

    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(root));
    const auto* s = repo.FindScenarioById("s_intro");
    REQUIRE(s != nullptr);
    REQUIRE(s->name == "Intro");
    REQUIRE(s->startRegionId == "alpha");
    REQUIRE(s->startNodeId == "alpha_exit");
    REQUIRE(s->startGold.has_value());
    REQUIRE(*s->startGold == 1234);
    REQUIRE(s->standaloneSelectable);
    REQUIRE_FALSE(s->hasInlineOutcome);
    std::filesystem::remove_all(root);
}

TEST_CASE("Scenario content: inline outcome key presence sets hasInlineOutcome") {
    const std::filesystem::path root = "saves/scenario_inline_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "scenarios.json", R"({"schemaVersion":1,"kind":"ScenarioCollection","id":"scenarios","scenarios":[
        {"id":"s_inline","startRegionId":"alpha","victoryConditions":[{"type":"storyFlagSet","flag":"won"}]}
    ]})");

    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(root));
    const auto* s = repo.FindScenarioById("s_inline");
    REQUIRE(s != nullptr);
    REQUIRE(s->hasInlineOutcome);
    REQUIRE(s->victoryConditions.size() == 1);
    std::filesystem::remove_all(root);
}

TEST_CASE("Scenario content: empty id is rejected") {
    const std::filesystem::path root = "saves/scenario_emptyid_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "scenarios.json", R"({"schemaVersion":1,"kind":"ScenarioCollection","id":"scenarios","scenarios":[
        {"id":"","startRegionId":"alpha"}
    ]})");
    data::ContentRepository repo;
    (void)repo.LoadFromDirectory(root);
    REQUIRE(HasMessageWithCode(repo.ValidationMessages(), "SCENARIO_ID_EMPTY"));
    REQUIRE(repo.Scenarios().empty());
    std::filesystem::remove_all(root);
}

TEST_CASE("Scenario content: duplicate id is rejected") {
    const std::filesystem::path root = "saves/scenario_dup_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "scenarios.json", R"({"schemaVersion":1,"kind":"ScenarioCollection","id":"scenarios","scenarios":[
        {"id":"s1","startRegionId":"alpha"},
        {"id":"s1","startRegionId":"beta"}
    ]})");
    data::ContentRepository repo;
    (void)repo.LoadFromDirectory(root);
    REQUIRE(HasMessageWithCode(repo.ValidationMessages(), "SCENARIO_DUPLICATE"));
    REQUIRE(repo.Scenarios().size() == 1);
    std::filesystem::remove_all(root);
}

TEST_CASE("Scenario content: unknown start region is rejected") {
    const std::filesystem::path root = "saves/scenario_badregion_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "scenarios.json", R"({"schemaVersion":1,"kind":"ScenarioCollection","id":"scenarios","scenarios":[
        {"id":"s1","startRegionId":"nowhere"}
    ]})");
    data::ContentRepository repo;
    (void)repo.LoadFromDirectory(root);
    REQUIRE(HasMessageWithCode(repo.ValidationMessages(), "SCENARIO_START_REGION_UNKNOWN"));
    REQUIRE(repo.Scenarios().empty());
    std::filesystem::remove_all(root);
}

TEST_CASE("Scenario content: unknown start node is rejected") {
    const std::filesystem::path root = "saves/scenario_badnode_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "scenarios.json", R"({"schemaVersion":1,"kind":"ScenarioCollection","id":"scenarios","scenarios":[
        {"id":"s1","startRegionId":"alpha","startNodeId":"not_a_node"}
    ]})");
    data::ContentRepository repo;
    (void)repo.LoadFromDirectory(root);
    REQUIRE(HasMessageWithCode(repo.ValidationMessages(), "SCENARIO_START_NODE_UNKNOWN"));
    REQUIRE(repo.Scenarios().empty());
    std::filesystem::remove_all(root);
}

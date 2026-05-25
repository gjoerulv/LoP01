#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>

#include "data/ContentRepository.h"

namespace {

void WriteTextFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream output(path, std::ios::trunc);
    output << content;
}

// Writes the bare-minimum set of content files required for ContentRepository
// to load successfully. The caller may then add or overwrite scenario_outcome.json.
void WriteBaselineContent(const std::filesystem::path& root) {
    std::filesystem::create_directories(root);
    WriteTextFile(root / "regions.json", R"({"schemaVersion":1,"kind":"RegionCollection","id":"regions","regions":[{"id":"r","name":"R","unlocked":true,"nodes":[{"location_id":"loc","x":0,"y":0,"discovered":true,"travel_available":true}],"links":[]}]})");
    WriteTextFile(root / "locations.json", R"({"schemaVersion":1,"kind":"LocationCollection","id":"locations","locations":[{"id":"loc","name":"Loc","type":"combat","allows_sleep":false,"overworld_destination":true,"battle_scenario_id":"debug_intro_battle"}]})");
    WriteTextFile(root / "location_scenes.json", R"({"schemaVersion":1,"kind":"LocationSceneCollection","id":"location_scenes","location_scenes":[]})");
    WriteTextFile(root / "units.json", R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[{"id":"hero","name":"Hero","category":"hero","is_player_character":true,"attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":1,"life":1,"position":"front","range":"melee"}]})");
    WriteTextFile(root / "battle_scenarios.json", R"({"schemaVersion":1,"kind":"BattleScenarioCollection","id":"battle_scenarios","battle_scenarios":[{"id":"debug_intro_battle","name":"Debug","seed":7,"allies":[{"unit_id":"hero"}],"enemies":[{"unit_id":"hero"}]}]})");
    WriteTextFile(root / "enemy_groups.json", R"({"schemaVersion":1,"kind":"EnemyGroupCollection","id":"enemy_groups","enemy_groups":[]})");
    WriteTextFile(root / "quests.json", R"({"schemaVersion":1,"kind":"QuestCollection","id":"quests","quests":[]})");
    WriteTextFile(root / "location_services.json", R"({"schemaVersion":1,"kind":"LocationServiceCollection","id":"location_services","location_services":[]})");
}

bool HasMessageWithCode(const std::vector<ValidationMessage>& msgs, const std::string& code) {
    return std::any_of(msgs.begin(), msgs.end(),
        [&](const ValidationMessage& m) { return m.code == code; });
}

} // namespace

TEST_CASE("ContentRepository: missing scenario_outcome.json yields empty outcome (legal)") {
    const std::filesystem::path root = "saves/scenario_outcome_missing_test";
    WriteBaselineContent(root);

    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(root));

    const auto& outcome = repo.ScenarioOutcome();
    REQUIRE(outcome.victoryConditions.empty());
    REQUIRE(outcome.defeatConditions.empty());

    std::filesystem::remove_all(root);
}

TEST_CASE("ContentRepository: scenario_outcome.json with victory + defeat conditions loads") {
    const std::filesystem::path root = "saves/scenario_outcome_authored_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "scenario_outcome.json", R"({
        "schemaVersion": 1,
        "kind": "ScenarioOutcome",
        "id": "scenario_outcome",
        "victoryConditions": [
            {"type": "storyFlagSet", "flag": "ashvale_cleansed"}
        ],
        "defeatConditions": [
            {"type": "storyFlagSet", "flag": "ashvale_lost"}
        ]
    })");

    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(root));

    const auto& outcome = repo.ScenarioOutcome();
    REQUIRE(outcome.victoryConditions.size() == 1);
    REQUIRE(outcome.defeatConditions.size() == 1);
    REQUIRE(outcome.victoryConditions[0].leafType == "storyFlagSet");
    REQUIRE(outcome.defeatConditions[0].leafType == "storyFlagSet");

    std::filesystem::remove_all(root);
}

TEST_CASE("ContentRepository: scenario_outcome.json with unknown condition leaf type emits error") {
    const std::filesystem::path root = "saves/scenario_outcome_unknown_leaf_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "scenario_outcome.json", R"({
        "schemaVersion": 1,
        "kind": "ScenarioOutcome",
        "id": "scenario_outcome",
        "victoryConditions": [
            {"type": "unknownLeafType"}
        ]
    })");

    data::ContentRepository repo;
    static_cast<void>(repo.LoadFromDirectory(root));
    REQUIRE(HasMessageWithCode(repo.ValidationMessages(), "EVENT_CONDITION_TYPE_UNKNOWN"));

    std::filesystem::remove_all(root);
}

TEST_CASE("ContentRepository: composite All/Not condition parses successfully") {
    const std::filesystem::path root = "saves/scenario_outcome_composite_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "scenario_outcome.json", R"({
        "schemaVersion": 1,
        "kind": "ScenarioOutcome",
        "id": "scenario_outcome",
        "victoryConditions": [
            {"all": [
                {"type": "storyFlagSet", "flag": "a"},
                {"not": {"type": "storyFlagSet", "flag": "b"}}
            ]}
        ]
    })");

    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(root));

    const auto& outcome = repo.ScenarioOutcome();
    REQUIRE(outcome.victoryConditions.size() == 1);
    REQUIRE(outcome.victoryConditions[0].operands.size() == 2);

    std::filesystem::remove_all(root);
}

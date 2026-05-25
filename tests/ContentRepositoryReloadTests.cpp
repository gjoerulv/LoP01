#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <string>

#include "data/ContentRepository.h"

// M13-a cleanup: optional content files (events.json, scenario_outcome.json,
// items.json, artifacts.json) are only loaded when present. Earlier code did
// not clear their corresponding repository state up front, so a second
// LoadFromDirectory() on the same instance retained the previous state when
// the new content directory omitted the optional file. These regression tests
// pin the cleared-on-reload contract.

namespace {

void WriteTextFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream output(path, std::ios::trunc);
    output << content;
}

// Writes the baseline (required) content set. Optional files are NOT written
// here — callers add or omit them as the test needs.
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

// Writes all four optional files with at least one entry each — a populated
// "with-optionals" content root.
void WriteOptionalContent(const std::filesystem::path& root) {
    WriteTextFile(root / "items.json", R"({
        "schemaVersion": 1,
        "kind": "ItemCollection",
        "id": "items",
        "items": [
            { "id": "item_alpha", "name": {"en":"Alpha"}, "icon": "x",
              "subtype": "material", "stackCap": 999, "baseValue": 1 }
        ]
    })");
    WriteTextFile(root / "artifacts.json", R"({
        "schemaVersion": 1,
        "kind": "ArtifactCollection",
        "id": "artifacts",
        "artifacts": [
            { "id": "artifact_alpha", "name": {"en":"Alpha"}, "icon": "x",
              "allowedSlots": ["Attack"], "rarity": "minor", "tier": 1, "baseValue": 1,
              "combinable": false,
              "effects": [ { "type": "statBonus", "stat": "Attack", "amount": 1 } ] }
        ]
    })");
    WriteTextFile(root / "events.json", R"({
        "schemaVersion": 1,
        "kind": "EventCollection",
        "id": "events",
        "events": [
            { "id": "evt_alpha", "trigger": { "type": "startOfDay" },
              "actions": [], "repeat": { "mode": "once" } }
        ]
    })");
    WriteTextFile(root / "scenario_outcome.json", R"({
        "schemaVersion": 1,
        "kind": "ScenarioOutcome",
        "id": "scenario_outcome",
        "victoryConditions": [ { "type": "storyFlagSet", "flag": "win" } ],
        "defeatConditions":  [ { "type": "storyFlagSet", "flag": "lose" } ]
    })");
}

} // namespace

TEST_CASE("ContentRepository: reload without items.json clears previously loaded items") {
    const std::filesystem::path withOptionals = "saves/repo_reload_with_items";
    const std::filesystem::path withoutOptionals = "saves/repo_reload_without_items";
    WriteBaselineContent(withOptionals);
    WriteOptionalContent(withOptionals);
    WriteBaselineContent(withoutOptionals); // no items.json

    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(withOptionals));
    REQUIRE_FALSE(repo.Items().empty());

    REQUIRE(repo.LoadFromDirectory(withoutOptionals));
    REQUIRE(repo.Items().empty());
    REQUIRE(repo.FindItemById("item_alpha") == nullptr);

    std::filesystem::remove_all(withOptionals);
    std::filesystem::remove_all(withoutOptionals);
}

TEST_CASE("ContentRepository: reload without artifacts.json clears previously loaded artifacts") {
    const std::filesystem::path withOptionals = "saves/repo_reload_with_artifacts";
    const std::filesystem::path withoutOptionals = "saves/repo_reload_without_artifacts";
    WriteBaselineContent(withOptionals);
    WriteOptionalContent(withOptionals);
    WriteBaselineContent(withoutOptionals); // no artifacts.json

    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(withOptionals));
    REQUIRE_FALSE(repo.Artifacts().empty());

    REQUIRE(repo.LoadFromDirectory(withoutOptionals));
    REQUIRE(repo.Artifacts().empty());
    REQUIRE(repo.FindArtifactById("artifact_alpha") == nullptr);

    std::filesystem::remove_all(withOptionals);
    std::filesystem::remove_all(withoutOptionals);
}

TEST_CASE("ContentRepository: reload without events.json clears previously loaded events") {
    const std::filesystem::path withOptionals = "saves/repo_reload_with_events";
    const std::filesystem::path withoutOptionals = "saves/repo_reload_without_events";
    WriteBaselineContent(withOptionals);
    WriteOptionalContent(withOptionals);
    WriteBaselineContent(withoutOptionals); // no events.json

    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(withOptionals));
    REQUIRE_FALSE(repo.EventDefinitions().empty());

    REQUIRE(repo.LoadFromDirectory(withoutOptionals));
    REQUIRE(repo.EventDefinitions().empty());

    std::filesystem::remove_all(withOptionals);
    std::filesystem::remove_all(withoutOptionals);
}

TEST_CASE("ContentRepository: reload without scenario_outcome.json clears previously loaded outcome") {
    const std::filesystem::path withOptionals = "saves/repo_reload_with_outcome";
    const std::filesystem::path withoutOptionals = "saves/repo_reload_without_outcome";
    WriteBaselineContent(withOptionals);
    WriteOptionalContent(withOptionals);
    WriteBaselineContent(withoutOptionals); // no scenario_outcome.json

    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(withOptionals));
    REQUIRE_FALSE(repo.ScenarioOutcome().victoryConditions.empty());
    REQUIRE_FALSE(repo.ScenarioOutcome().defeatConditions.empty());

    REQUIRE(repo.LoadFromDirectory(withoutOptionals));
    REQUIRE(repo.ScenarioOutcome().victoryConditions.empty());
    REQUIRE(repo.ScenarioOutcome().defeatConditions.empty());

    std::filesystem::remove_all(withOptionals);
    std::filesystem::remove_all(withoutOptionals);
}

TEST_CASE("ContentRepository: single reload clears all four optional containers in one pass") {
    const std::filesystem::path withOptionals = "saves/repo_reload_all_with";
    const std::filesystem::path withoutOptionals = "saves/repo_reload_all_without";
    WriteBaselineContent(withOptionals);
    WriteOptionalContent(withOptionals);
    WriteBaselineContent(withoutOptionals); // none of the four optional files

    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(withOptionals));
    REQUIRE_FALSE(repo.Items().empty());
    REQUIRE_FALSE(repo.Artifacts().empty());
    REQUIRE_FALSE(repo.EventDefinitions().empty());
    REQUIRE_FALSE(repo.ScenarioOutcome().victoryConditions.empty());

    REQUIRE(repo.LoadFromDirectory(withoutOptionals));
    REQUIRE(repo.Items().empty());
    REQUIRE(repo.Artifacts().empty());
    REQUIRE(repo.EventDefinitions().empty());
    REQUIRE(repo.ScenarioOutcome().victoryConditions.empty());
    REQUIRE(repo.ScenarioOutcome().defeatConditions.empty());

    std::filesystem::remove_all(withOptionals);
    std::filesystem::remove_all(withoutOptionals);
}

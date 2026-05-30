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

// Two-region baseline (alpha, beta) with arrival + exit nodes and matching
// locations. Tests overwrite regions.json / world_map.json as needed.
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

TEST_CASE("WorldMap content: missing world_map.json yields an empty world map (legal)") {
    const std::filesystem::path root = "saves/worldmap_missing_test";
    WriteBaselineContent(root);

    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(root));
    REQUIRE(repo.WorldMap().entries.empty());
    REQUIRE(repo.WorldMap().adjacency.empty());

    std::filesystem::remove_all(root);
}

TEST_CASE("WorldMap content: happy path loads entries and adjacency") {
    const std::filesystem::path root = "saves/worldmap_happy_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "world_map.json", R"({
        "schemaVersion": 1, "kind": "WorldMap", "id": "world_map", "name": {"en": "Ashvale"},
        "entries": [
            {"id": "alpha", "unlocked": true, "exitNodeIds": ["alpha_exit"]},
            {"id": "beta",  "unlocked": true, "exitNodeIds": []}
        ],
        "adjacency": [["alpha", "beta"]]
    })");

    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(root));
    REQUIRE(repo.WorldMap().entries.size() == 2);
    REQUIRE(repo.WorldMap().adjacency.size() == 1);

    const auto* alpha = repo.FindWorldMapRegionEntry("alpha");
    REQUIRE(alpha != nullptr);
    REQUIRE(alpha->unlocked);
    REQUIRE(alpha->exitNodeIds.size() == 1);
    REQUIRE(alpha->exitNodeIds[0] == "alpha_exit");
    REQUIRE(repo.FindWorldMapRegionEntry("nope") == nullptr);

    std::filesystem::remove_all(root);
}

TEST_CASE("WorldMap content: unknown region reference is rejected") {
    const std::filesystem::path root = "saves/worldmap_unknown_region_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "world_map.json", R"({
        "schemaVersion": 1, "kind": "WorldMap", "id": "world_map",
        "entries": [{"id": "ghost_region", "unlocked": true, "exitNodeIds": []}]
    })");

    data::ContentRepository repo;
    static_cast<void>(repo.LoadFromDirectory(root));
    REQUIRE(HasMessageWithCode(repo.ValidationMessages(), "WORLDMAP_REGION_UNKNOWN"));
    REQUIRE(repo.WorldMap().entries.empty());

    std::filesystem::remove_all(root);
}

TEST_CASE("WorldMap content: region with empty arrivalNodeId is rejected") {
    const std::filesystem::path root = "saves/worldmap_missing_arrival_test";
    WriteBaselineContent(root);
    // Overwrite beta to have no arrivalNodeId.
    WriteTextFile(root / "regions.json", R"({"schemaVersion":1,"kind":"RegionCollection","id":"regions","regions":[
        {"id":"alpha","name":"Alpha","unlocked":true,"arrivalNodeId":"alpha_arrival","nodes":[
            {"location_id":"alpha_arrival","x":0,"y":0}],"links":[]},
        {"id":"beta","name":"Beta","unlocked":true,"nodes":[
            {"location_id":"beta_arrival","x":0,"y":0}],"links":[]}
    ]})");
    WriteTextFile(root / "world_map.json", R"({
        "schemaVersion": 1, "kind": "WorldMap", "id": "world_map",
        "entries": [
            {"id": "alpha", "unlocked": true, "exitNodeIds": []},
            {"id": "beta",  "unlocked": true, "exitNodeIds": []}
        ]
    })");

    data::ContentRepository repo;
    static_cast<void>(repo.LoadFromDirectory(root));
    REQUIRE(HasMessageWithCode(repo.ValidationMessages(), "WORLDMAP_ARRIVAL_NODE_MISSING"));

    std::filesystem::remove_all(root);
}

TEST_CASE("WorldMap content: arrival node not present in region nodes is rejected") {
    const std::filesystem::path root = "saves/worldmap_bad_arrival_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "regions.json", R"({"schemaVersion":1,"kind":"RegionCollection","id":"regions","regions":[
        {"id":"alpha","name":"Alpha","unlocked":true,"arrivalNodeId":"ghost_node","nodes":[
            {"location_id":"alpha_arrival","x":0,"y":0}],"links":[]}
    ]})");
    WriteTextFile(root / "world_map.json", R"({
        "schemaVersion": 1, "kind": "WorldMap", "id": "world_map",
        "entries": [{"id": "alpha", "unlocked": true, "exitNodeIds": []}]
    })");

    data::ContentRepository repo;
    static_cast<void>(repo.LoadFromDirectory(root));
    REQUIRE(HasMessageWithCode(repo.ValidationMessages(), "WORLDMAP_ARRIVAL_NODE_UNKNOWN"));

    std::filesystem::remove_all(root);
}

TEST_CASE("WorldMap content: exit node not present in source region is rejected") {
    const std::filesystem::path root = "saves/worldmap_bad_exit_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "world_map.json", R"({
        "schemaVersion": 1, "kind": "WorldMap", "id": "world_map",
        "entries": [{"id": "alpha", "unlocked": true, "exitNodeIds": ["not_a_node"]}]
    })");

    data::ContentRepository repo;
    static_cast<void>(repo.LoadFromDirectory(root));
    REQUIRE(HasMessageWithCode(repo.ValidationMessages(), "WORLDMAP_EXIT_NODE_UNKNOWN"));
    REQUIRE(repo.WorldMap().entries.empty());

    std::filesystem::remove_all(root);
}

TEST_CASE("WorldMap content: duplicate region entry is rejected") {
    const std::filesystem::path root = "saves/worldmap_duplicate_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "world_map.json", R"({
        "schemaVersion": 1, "kind": "WorldMap", "id": "world_map",
        "entries": [
            {"id": "alpha", "unlocked": true, "exitNodeIds": []},
            {"id": "alpha", "unlocked": true, "exitNodeIds": []}
        ]
    })");

    data::ContentRepository repo;
    static_cast<void>(repo.LoadFromDirectory(root));
    REQUIRE(HasMessageWithCode(repo.ValidationMessages(), "WORLDMAP_ENTRY_DUPLICATE"));
    REQUIRE(repo.WorldMap().entries.size() == 1); // first kept

    std::filesystem::remove_all(root);
}

TEST_CASE("WorldMap content: adjacency referencing an unknown entry is rejected") {
    const std::filesystem::path root = "saves/worldmap_bad_adjacency_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "world_map.json", R"({
        "schemaVersion": 1, "kind": "WorldMap", "id": "world_map",
        "entries": [{"id": "alpha", "unlocked": true, "exitNodeIds": []}],
        "adjacency": [["alpha", "beta"]]
    })");

    data::ContentRepository repo;
    static_cast<void>(repo.LoadFromDirectory(root));
    REQUIRE(HasMessageWithCode(repo.ValidationMessages(), "WORLDMAP_ADJACENCY_UNKNOWN_ENTRY"));
    REQUIRE(repo.WorldMap().adjacency.empty());

    std::filesystem::remove_all(root);
}

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

#include "data/ContentRepository.h"
#include "data/definitions/RegionDefinition.h"
#include "gameplay/EnemyOccupationRules.h"
#include "gameplay/EnemyTeamState.h"
#include "gameplay/GameSession.h"

namespace {

void WriteTextFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream output(path, std::ios::trunc);
    output << content;
}

} // namespace

TEST_CASE("EnemyGroupDefinition typed loading from enemy_groups.json shape") {
    const std::filesystem::path root = "saves/enemy_team_test_group_loading";
    std::filesystem::create_directories(root);

    WriteTextFile(root / "regions.json",          R"({"schemaVersion":1,"kind":"RegionCollection","id":"regions","regions":[]})");
    WriteTextFile(root / "locations.json",         R"({"schemaVersion":1,"kind":"LocationCollection","id":"locations","locations":[]})");
    WriteTextFile(root / "location_scenes.json",   R"({"schemaVersion":1,"kind":"LocationSceneCollection","id":"location_scenes","location_scenes":[]})");
    WriteTextFile(root / "units.json",             R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[]})");
    WriteTextFile(root / "battle_scenarios.json",  R"({"schemaVersion":1,"kind":"BattleScenarioCollection","id":"battle_scenarios","battle_scenarios":[]})");
    WriteTextFile(root / "quests.json",            R"({"schemaVersion":1,"kind":"QuestCollection","id":"quests","quests":[]})");
    WriteTextFile(root / "location_services.json", R"({"schemaVersion":1,"kind":"LocationServiceCollection","id":"location_services","location_services":[]})");
    WriteTextFile(root / "enemy_groups.json", R"({
        "schemaVersion": 1,
        "kind": "EnemyGroupCollection",
        "id": "enemy_groups",
        "enemy_groups": [
            { "id": "eg_patrol_01", "name": "Rust Patrol A", "units": ["unit_guard", "unit_scout"] },
            { "id": "eg_patrol_02", "name": "Rust Patrol B", "units": ["unit_guard"] }
        ]
    })");

    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(root));

    const auto& groups = repo.EnemyGroups();
    REQUIRE(groups.size() == 2);

    const auto* first = repo.FindEnemyGroupById("eg_patrol_01");
    REQUIRE(first != nullptr);
    REQUIRE(first->name == "Rust Patrol A");
    REQUIRE(first->unitIds.size() == 2);
    REQUIRE(first->unitIds[0] == "unit_guard");
    REQUIRE(first->unitIds[1] == "unit_scout");

    REQUIRE(repo.FindEnemyGroupById("eg_patrol_02") != nullptr);
    REQUIRE(repo.FindEnemyGroupById("nonexistent") == nullptr);
}

TEST_CASE("EnemyTeamState stores expected runtime fields") {
    gameplay::EnemyTeamState team;
    team.teamColor = "Red";
    team.nodeId = "bridge_node";
    team.personality = gameplay::Personality::Builder;
    team.aggression = gameplay::Aggression::Careful;
    team.patrol.enabled = true;
    team.patrol.centerNodeId = "bridge_node";
    team.patrol.radius = 2;
    team.alliances = { "Blue" };
    team.active = true;
    team.energy = 500;
    team.cooldownExpiresAtMinutes = 180;

    REQUIRE(team.teamColor == "Red");
    REQUIRE(team.nodeId == "bridge_node");
    REQUIRE(team.personality == gameplay::Personality::Builder);
    REQUIRE(team.aggression == gameplay::Aggression::Careful);
    REQUIRE(team.patrol.enabled);
    REQUIRE(team.patrol.centerNodeId == "bridge_node");
    REQUIRE(team.patrol.radius == 2);
    REQUIRE(team.alliances.size() == 1);
    REQUIRE(team.alliances[0] == "Blue");
    REQUIRE(team.active);
    REQUIRE(team.energy == 500);
    REQUIRE(team.cooldownExpiresAtMinutes == 180);
}

TEST_CASE("SetEnemyTeams stores explicit runtime teams") {
    gameplay::GameSession session;

    gameplay::EnemyTeamState a;
    a.teamColor = "Red";
    a.active = true;

    gameplay::EnemyTeamState b;
    b.teamColor = "Blue";
    b.active = true;

    session.SetEnemyTeams({ a, b });

    REQUIRE(session.EnemyTeams().size() == 2);
}

TEST_CASE("Leaderless enemy team is valid") {
    gameplay::GameSession session;

    gameplay::EnemyTeamState team;
    team.teamColor = "Yellow";
    team.active = true;

    session.SetEnemyTeams({ team });

    const auto results = session.ProcessEnemyPhase({});
    REQUIRE(results.size() == 1);
    REQUIRE(results[0].teamColor == "Yellow");
}

TEST_CASE("ProcessEnemyPhase returns one idle result per active team") {
    gameplay::GameSession session;

    gameplay::EnemyTeamState a; a.teamColor = "Red";    a.active = true;
    gameplay::EnemyTeamState b; b.teamColor = "Blue";   b.active = true;
    gameplay::EnemyTeamState c; c.teamColor = "Yellow"; c.active = true;

    session.SetEnemyTeams({ a, b, c });

    const auto results = session.ProcessEnemyPhase({});
    REQUIRE(results.size() == 3);
    for (const auto& r : results) {
        REQUIRE(r.actionType == "idle");
    }
}

TEST_CASE("Inactive teams are skipped in ProcessEnemyPhase") {
    gameplay::GameSession session;

    gameplay::EnemyTeamState a; a.teamColor = "Red";    a.active = true;
    gameplay::EnemyTeamState b; b.teamColor = "Blue";   b.active = false;
    gameplay::EnemyTeamState c; c.teamColor = "Yellow"; c.active = true;

    session.SetEnemyTeams({ a, b, c });

    const auto results = session.ProcessEnemyPhase({});
    REQUIRE(results.size() == 2);
}

TEST_CASE("ProcessEnemyPhase results are in fixed color order") {
    gameplay::GameSession session;

    // Inserted out of canonical order: Blue, Red, Yellow
    gameplay::EnemyTeamState blue;   blue.teamColor   = "Blue";   blue.active   = true;
    gameplay::EnemyTeamState red;    red.teamColor    = "Red";    red.active    = true;
    gameplay::EnemyTeamState yellow; yellow.teamColor = "Yellow"; yellow.active = true;

    session.SetEnemyTeams({ blue, red, yellow });

    // Canonical order: Green, Red, Blue, Yellow, Purple, Orange, Teal, White
    // Expected output order: Red, Blue, Yellow
    const auto results = session.ProcessEnemyPhase({});
    REQUIRE(results.size() == 3);
    REQUIRE(results[0].teamColor == "Red");
    REQUIRE(results[1].teamColor == "Blue");
    REQUIRE(results[2].teamColor == "Yellow");
}

TEST_CASE("ProcessEnemyPhase returns empty list when no teams are active") {
    gameplay::GameSession session;

    const auto results = session.ProcessEnemyPhase({});
    REQUIRE(results.empty());
}

TEST_CASE("ProcessEnemyPhase moves patrol team to adjacent node within radius") {
    // Graph: A-B-C chain. Team at A, patrol center=A, radius=1.
    // Candidate from A: B (1 hop from A <= 1). Valid.
    std::vector<data::RegionLinkDefinition> links = {
        { "node_a", "node_b" },
        { "node_b", "node_c" }
    };

    gameplay::GameSession session;
    gameplay::EnemyTeamState team;
    team.teamColor = "Red";
    team.nodeId = "node_a";
    team.patrol.enabled = true;
    team.patrol.centerNodeId = "node_a";
    team.patrol.radius = 1;
    team.active = true;

    session.SetEnemyTeams({ team });
    const auto results = session.ProcessEnemyPhase(links);

    REQUIRE(results.size() == 1);
    REQUIRE(results[0].actionType == "moved");
    REQUIRE(session.EnemyTeams()[0].nodeId == "node_b");
}

TEST_CASE("ProcessEnemyPhase keeps patrol team idle when adjacent nodes exceed radius") {
    // Graph: A-B-C chain. Team at A, patrol center=A, radius=0.
    // Candidate from A: B. FindHopCount(A, B) = 1 > 0. No valid candidates.
    std::vector<data::RegionLinkDefinition> links = {
        { "node_a", "node_b" },
        { "node_b", "node_c" }
    };

    gameplay::GameSession session;
    gameplay::EnemyTeamState team;
    team.teamColor = "Red";
    team.nodeId = "node_a";
    team.patrol.enabled = true;
    team.patrol.centerNodeId = "node_a";
    team.patrol.radius = 0;
    team.active = true;

    session.SetEnemyTeams({ team });
    const auto results = session.ProcessEnemyPhase(links);

    REQUIRE(results.size() == 1);
    REQUIRE(results[0].actionType == "idle");
    REQUIRE(session.EnemyTeams()[0].nodeId == "node_a");
}

TEST_CASE("ProcessEnemyPhase team with no patrol stays idle") {
    std::vector<data::RegionLinkDefinition> links = {
        { "node_a", "node_b" }
    };

    gameplay::GameSession session;
    gameplay::EnemyTeamState team;
    team.teamColor = "Green";
    team.nodeId = "node_a";
    team.patrol.enabled = false;
    team.active = true;

    session.SetEnemyTeams({ team });
    const auto results = session.ProcessEnemyPhase(links);

    REQUIRE(results.size() == 1);
    REQUIRE(results[0].actionType == "idle");
    REQUIRE(session.EnemyTeams()[0].nodeId == "node_a");
}

// ---------------------------------------------------------------------------
// IsBlockedByHostileOccupation
// ---------------------------------------------------------------------------

TEST_CASE("IsBlockedByHostileOccupation hostile occupied normal node is blocked") {
    REQUIRE(gameplay::IsBlockedByHostileOccupation("node_a", "", { "node_a", "node_b" }));
}

TEST_CASE("IsBlockedByHostileOccupation hostile occupied arrival node is not blocked") {
    REQUIRE_FALSE(gameplay::IsBlockedByHostileOccupation("node_a", "node_a", { "node_a" }));
}

TEST_CASE("IsBlockedByHostileOccupation unoccupied node is not blocked") {
    REQUIRE_FALSE(gameplay::IsBlockedByHostileOccupation("node_c", "", { "node_a", "node_b" }));
}

// ---------------------------------------------------------------------------
// HostileOccupiedNodeIds
// ---------------------------------------------------------------------------

TEST_CASE("HostileOccupiedNodeIds returns nodeId of active hostile team") {
    gameplay::GameSession session;
    gameplay::EnemyTeamState team;
    team.teamColor = "Red";
    team.nodeId = "bridge_node";
    team.active = true;
    session.SetEnemyTeams({ team });

    const auto result = session.HostileOccupiedNodeIds("Green");
    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == "bridge_node");
}

TEST_CASE("HostileOccupiedNodeIds excludes player-color team") {
    gameplay::GameSession session;
    gameplay::EnemyTeamState team;
    team.teamColor = "Green";
    team.nodeId = "bridge_node";
    team.active = true;
    session.SetEnemyTeams({ team });

    REQUIRE(session.HostileOccupiedNodeIds("Green").empty());
}

TEST_CASE("HostileOccupiedNodeIds excludes team allied with player") {
    gameplay::GameSession session;
    gameplay::EnemyTeamState team;
    team.teamColor = "Red";
    team.nodeId = "bridge_node";
    team.active = true;
    team.alliances = { "Green" };
    session.SetEnemyTeams({ team });

    REQUIRE(session.HostileOccupiedNodeIds("Green").empty());
}

TEST_CASE("HostileOccupiedNodeIds excludes inactive team") {
    gameplay::GameSession session;
    gameplay::EnemyTeamState team;
    team.teamColor = "Red";
    team.nodeId = "bridge_node";
    team.active = false;
    session.SetEnemyTeams({ team });

    REQUIRE(session.HostileOccupiedNodeIds("Green").empty());
}

TEST_CASE("HostileOccupiedNodeIds excludes team with empty nodeId") {
    gameplay::GameSession session;
    gameplay::EnemyTeamState team;
    team.teamColor = "Red";
    team.nodeId = "";
    team.active = true;
    session.SetEnemyTeams({ team });

    REQUIRE(session.HostileOccupiedNodeIds("Green").empty());
}

// ---------------------------------------------------------------------------
// RegionDefinition arrivalNodeId JSON loading
// ---------------------------------------------------------------------------

TEST_CASE("RegionDefinition loads arrivalNodeId from JSON") {
    const std::filesystem::path root = "saves/enemy_team_test_arrival_node";
    std::filesystem::create_directories(root);

    WriteTextFile(root / "regions.json", R"({
        "schemaVersion": 1,
        "kind": "RegionCollection",
        "id": "regions",
        "regions": [
            {
                "id": "region_01",
                "name": "Test Region",
                "arrivalNodeId": "node_start",
                "nodes": [],
                "links": []
            }
        ]
    })");
    WriteTextFile(root / "locations.json",         R"({"schemaVersion":1,"kind":"LocationCollection","id":"locations","locations":[]})");
    WriteTextFile(root / "location_scenes.json",   R"({"schemaVersion":1,"kind":"LocationSceneCollection","id":"location_scenes","location_scenes":[]})");
    WriteTextFile(root / "units.json",             R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[]})");
    WriteTextFile(root / "battle_scenarios.json",  R"({"schemaVersion":1,"kind":"BattleScenarioCollection","id":"battle_scenarios","battle_scenarios":[]})");
    WriteTextFile(root / "quests.json",            R"({"schemaVersion":1,"kind":"QuestCollection","id":"quests","quests":[]})");
    WriteTextFile(root / "location_services.json", R"({"schemaVersion":1,"kind":"LocationServiceCollection","id":"location_services","location_services":[]})");
    WriteTextFile(root / "enemy_groups.json",      R"({"schemaVersion":1,"kind":"EnemyGroupCollection","id":"enemy_groups","enemy_groups":[]})");

    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(root));

    const auto* region = repo.FindRegionById("region_01");
    REQUIRE(region != nullptr);
    REQUIRE(region->arrivalNodeId == "node_start");
}

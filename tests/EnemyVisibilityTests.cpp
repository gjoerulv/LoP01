#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "app/mappers/RegionModelMapper.h"
#include "data/ContentRepository.h"
#include "gameplay/EnemyTeamState.h"
#include "gameplay/GameSession.h"

// M32 enemy visibility: Region read models gate hostile-team presence by reveal.
// A hostile team on an UNREVEALED node is not surfaced (no marker, no estimate);
// a hostile team on a REVEALED node is surfaced with a bounded presence estimate.
// Travel mechanics keep using the true hostile set and are not affected here.

namespace {

void WriteTextFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream output(path, std::ios::trunc);
    output << content;
}

// alpha is a 4-node chain a_arr -> a_b -> a_c -> a_d. From the start (a_arr) the
// radius-2 reveal covers a_arr/a_b/a_c; a_d remains unknown.
void WriteVisibilityBaseline(const std::filesystem::path& root) {
    std::filesystem::create_directories(root);
    WriteTextFile(root / "regions.json", R"({"schemaVersion":1,"kind":"RegionCollection","id":"regions","regions":[
        {"id":"alpha","name":"Alpha","unlocked":true,"arrivalNodeId":"a_arr","nodes":[
            {"location_id":"a_arr","x":0,"y":0},{"location_id":"a_b","x":1,"y":0},
            {"location_id":"a_c","x":2,"y":0},{"location_id":"a_d","x":3,"y":0}],
            "links":[["a_arr","a_b"],["a_b","a_c"],["a_c","a_d"]]}
    ]})");
    WriteTextFile(root / "locations.json", R"({"schemaVersion":1,"kind":"LocationCollection","id":"locations","locations":[
        {"id":"a_arr","name":"A Arrival","type":"town","allows_sleep":false,"overworld_destination":true},
        {"id":"a_b","name":"A B","type":"town","allows_sleep":false,"overworld_destination":true},
        {"id":"a_c","name":"A C","type":"town","allows_sleep":false,"overworld_destination":true},
        {"id":"a_d","name":"A D","type":"town","allows_sleep":false,"overworld_destination":true}
    ]})");
    WriteTextFile(root / "location_scenes.json", R"({"schemaVersion":1,"kind":"LocationSceneCollection","id":"location_scenes","location_scenes":[]})");
    WriteTextFile(root / "units.json", R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[
        {"id":"hero_player","name":"PC","category":"leader","is_player_character":true,"attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":5,"life":1,"position":"leader","range":"melee"}]})");
    WriteTextFile(root / "battle_scenarios.json", R"({"schemaVersion":1,"kind":"BattleScenarioCollection","id":"battle_scenarios","battle_scenarios":[]})");
    WriteTextFile(root / "enemy_groups.json", R"({"schemaVersion":1,"kind":"EnemyGroupCollection","id":"enemy_groups","enemy_groups":[]})");
    WriteTextFile(root / "quests.json", R"({"schemaVersion":1,"kind":"QuestCollection","id":"quests","quests":[]})");
    WriteTextFile(root / "location_services.json", R"({"schemaVersion":1,"kind":"LocationServiceCollection","id":"location_services","location_services":[]})");
    WriteTextFile(root / "scenarios.json", R"({"schemaVersion":1,"kind":"ScenarioCollection","id":"scenarios","scenarios":[
        {"id":"s1","startRegionId":"alpha"}]})");
}

gameplay::GameSession WireSession(const data::ContentRepository& repo, const std::string& enemyNodeId) {
    gameplay::GameSession session;
    session.SetPlayerColor("Green");
    session.SetUnitCatalog(repo.Units());
    session.SetLeaderCapableUnitIds({"hero_player"});
    REQUIRE(session.AddOwnedUnit("hero_player", 1));
    REQUIRE(session.TryAddUnitToActiveParty("hero_player"));
    session.SetRegionCatalog(repo.Regions());
    session.SetWorldMap(repo.WorldMap());
    session.SetScenarioCatalog(repo.Scenarios());
    session.SetCampaignCatalog(repo.Campaigns());
    session.SetLocationServiceCatalog(repo.LocationServices());

    gameplay::EnemyTeamState team{};
    team.teamColor = "Red";
    team.nodeId = enemyNodeId;
    team.active = true;
    session.SetEnemyTeams({team});
    session.StartStandaloneScenario("s1");
    return session;
}

const ashvale::rendering::RegionNodeView* FindNode(
    const ashvale::rendering::RegionRenderModel& model, const std::string& id) {
    for (const auto& n : model.nodes) {
        if (n.id == id) return &n;
    }
    return nullptr;
}

} // namespace

TEST_CASE("EnemyVisibility: a hostile team on an unrevealed node is hidden from the read model") {
    const std::filesystem::path root = "saves/vis_hidden";
    WriteVisibilityBaseline(root);
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(root));

    auto session = WireSession(repo, "a_d");   // a_d is 3 hops away => unrevealed
    REQUIRE_FALSE(session.IsNodeRevealed("alpha", "a_d"));

    app::mappers::RegionModelMapper mapper;
    const auto model = mapper.Map(repo, session, session.Snapshot(), 0, {});
    const auto* node = FindNode(model, "a_d");
    REQUIRE(node != nullptr);
    REQUIRE_FALSE(node->revealed);
    REQUIRE_FALSE(node->hostileOccupied);
    REQUIRE(node->enemyEstimate.empty());
    std::filesystem::remove_all(root);
}

TEST_CASE("EnemyVisibility: a hostile team on a revealed node shows presence and a bounded estimate") {
    const std::filesystem::path root = "saves/vis_revealed";
    WriteVisibilityBaseline(root);
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(root));

    auto session = WireSession(repo, "a_c");   // a_c is within radius 2 => revealed
    REQUIRE(session.IsNodeRevealed("alpha", "a_c"));

    app::mappers::RegionModelMapper mapper;
    const auto model = mapper.Map(repo, session, session.Snapshot(), 0, {});
    const auto* node = FindNode(model, "a_c");
    REQUIRE(node != nullptr);
    REQUIRE(node->revealed);
    REQUIRE(node->hostileOccupied);
    REQUIRE_FALSE(node->enemyEstimate.empty());
    // M33: a revealed hostile node also surfaces a bounded threat band.
    REQUIRE(node->enemyEstimate.find("Threat:") != std::string::npos);
    std::filesystem::remove_all(root);
}

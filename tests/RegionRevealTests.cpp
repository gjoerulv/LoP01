#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "data/ContentRepository.h"
#include "data/definitions/RegionDefinition.h"
#include "gameplay/GameSession.h"
#include "gameplay/world/RegionRevealRules.h"

// M32 fog/reveal foundation: a HoMM-persistent per-Region reveal layer seeded at
// scenario start (radius-2 around the start node + start-owned-service nodes),
// extended on legal movement and World Map arrival, and persisted across save/load.

namespace {

void WriteTextFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream output(path, std::ios::trunc);
    output << content;
}

// alpha is a 4-node chain (a_arr -> a_b -> a_c -> a_d); beta is a 2-node chain.
// a_arr is alpha's arrival AND World Map exit node.
void WriteRevealBaseline(const std::filesystem::path& root) {
    std::filesystem::create_directories(root);
    WriteTextFile(root / "regions.json", R"({"schemaVersion":1,"kind":"RegionCollection","id":"regions","regions":[
        {"id":"alpha","name":"Alpha","unlocked":true,"arrivalNodeId":"a_arr","nodes":[
            {"location_id":"a_arr","x":0,"y":0},{"location_id":"a_b","x":1,"y":0},
            {"location_id":"a_c","x":2,"y":0},{"location_id":"a_d","x":3,"y":0}],
            "links":[["a_arr","a_b"],["a_b","a_c"],["a_c","a_d"]]},
        {"id":"beta","name":"Beta","unlocked":true,"arrivalNodeId":"b_arr","nodes":[
            {"location_id":"b_arr","x":0,"y":0},{"location_id":"b_b","x":1,"y":0}],
            "links":[["b_arr","b_b"]]}
    ]})");
    WriteTextFile(root / "locations.json", R"({"schemaVersion":1,"kind":"LocationCollection","id":"locations","locations":[
        {"id":"a_arr","name":"A Arrival","type":"town","allows_sleep":false,"overworld_destination":true},
        {"id":"a_b","name":"A B","type":"town","allows_sleep":false,"overworld_destination":true},
        {"id":"a_c","name":"A C","type":"town","allows_sleep":false,"overworld_destination":true},
        {"id":"a_d","name":"A D","type":"town","allows_sleep":false,"overworld_destination":true},
        {"id":"b_arr","name":"B Arrival","type":"town","allows_sleep":false,"overworld_destination":true},
        {"id":"b_b","name":"B B","type":"town","allows_sleep":false,"overworld_destination":true}
    ]})");
    WriteTextFile(root / "location_scenes.json", R"({"schemaVersion":1,"kind":"LocationSceneCollection","id":"location_scenes","location_scenes":[]})");
    WriteTextFile(root / "units.json", R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[
        {"id":"hero_player","name":"PC","category":"leader","is_player_character":true,"attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":5,"life":1,"position":"leader","range":"melee"}]})");
    WriteTextFile(root / "battle_scenarios.json", R"({"schemaVersion":1,"kind":"BattleScenarioCollection","id":"battle_scenarios","battle_scenarios":[]})");
    WriteTextFile(root / "enemy_groups.json", R"({"schemaVersion":1,"kind":"EnemyGroupCollection","id":"enemy_groups","enemy_groups":[]})");
    WriteTextFile(root / "quests.json", R"({"schemaVersion":1,"kind":"QuestCollection","id":"quests","quests":[]})");
    WriteTextFile(root / "location_services.json", R"({"schemaVersion":1,"kind":"LocationServiceCollection","id":"location_services","location_services":[]})");
    WriteTextFile(root / "world_map.json", R"({"schemaVersion":1,"kind":"WorldMap","id":"world_map","name":{"en":"WM"},"entries":[
        {"id":"alpha","unlocked":true,"exitNodeIds":["a_arr"],"x":0,"y":0},
        {"id":"beta","unlocked":true,"exitNodeIds":["b_arr"],"x":1,"y":0}],"adjacency":[["alpha","beta"]]})");
    WriteTextFile(root / "scenarios.json", R"({"schemaVersion":1,"kind":"ScenarioCollection","id":"scenarios","scenarios":[
        {"id":"s1","startRegionId":"alpha"}]})");
}

gameplay::GameSession WireSession(const data::ContentRepository& repo) {
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
    return session;
}

data::RegionDefinition MakeChainRegion() {
    data::RegionDefinition region;
    region.id = "chain";
    region.arrivalNodeId = "A";
    region.nodes = {{"A"}, {"B"}, {"C"}, {"D"}};
    region.links = {{"A", "B"}, {"B", "C"}, {"C", "D"}};
    return region;
}

bool Contains(const std::vector<std::string>& v, const std::string& s) {
    return std::find(v.begin(), v.end(), s) != v.end();
}

} // namespace

TEST_CASE("RegionReveal rules: graph radius BFS is bounded and inclusive") {
    const auto region = MakeChainRegion();

    const auto r2 = gameplay::world::NodesWithinGraphRadius(region, "A", 2);
    REQUIRE(r2.size() == 3);
    REQUIRE(Contains(r2, "A"));
    REQUIRE(Contains(r2, "B"));
    REQUIRE(Contains(r2, "C"));
    REQUIRE_FALSE(Contains(r2, "D"));

    REQUIRE(gameplay::world::NodesWithinGraphRadius(region, "A", 1).size() == 2);
    REQUIRE(gameplay::world::NodesWithinGraphRadius(region, "A", 0) == std::vector<std::string>{"A"});
    REQUIRE(gameplay::world::NodesWithinGraphRadius(region, "ghost", 2).empty());
}

TEST_CASE("RegionReveal: scenario start seeds reveal around the start node only") {
    const std::filesystem::path root = "saves/reveal_seed";
    WriteRevealBaseline(root);
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(root));

    auto session = WireSession(repo);
    session.StartStandaloneScenario("s1");

    REQUIRE(session.IsNodeRevealed("alpha", "a_arr"));
    REQUIRE(session.IsNodeRevealed("alpha", "a_b"));
    REQUIRE(session.IsNodeRevealed("alpha", "a_c"));     // radius 2
    REQUIRE_FALSE(session.IsNodeRevealed("alpha", "a_d")); // 3 hops away => unknown
    REQUIRE_FALSE(session.IsNodeRevealed("beta", "b_arr"));
    std::filesystem::remove_all(root);
}

TEST_CASE("RegionReveal: moving to a node reveals its neighborhood") {
    const std::filesystem::path root = "saves/reveal_move";
    WriteRevealBaseline(root);
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(root));

    auto session = WireSession(repo);
    session.StartStandaloneScenario("s1");
    REQUIRE_FALSE(session.IsNodeRevealed("alpha", "a_d"));

    session.SetDestination("a_c");   // arriving at a_c reveals a_b..a_d
    REQUIRE(session.IsNodeRevealed("alpha", "a_d"));
    std::filesystem::remove_all(root);
}

TEST_CASE("RegionReveal: World Map travel seeds reveal in the arrival region and keeps prior reveal") {
    const std::filesystem::path root = "saves/reveal_worldmap";
    WriteRevealBaseline(root);
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(root));

    auto session = WireSession(repo);
    session.StartStandaloneScenario("s1");   // start at alpha/a_arr (exit node)
    REQUIRE(session.IsNodeRevealed("alpha", "a_arr"));

    const auto travel = session.TravelToRegion("beta");
    REQUIRE(travel.success);
    REQUIRE(session.Snapshot().regionId == "beta");
    REQUIRE(session.IsNodeRevealed("beta", "b_arr"));
    REQUIRE(session.IsNodeRevealed("beta", "b_b"));      // 1 hop from arrival
    // Prior region reveal persists (HoMM model).
    REQUIRE(session.IsNodeRevealed("alpha", "a_arr"));
    std::filesystem::remove_all(root);
}

TEST_CASE("RegionReveal: reveal state survives save/load") {
    const std::filesystem::path root = "saves/reveal_saveload";
    WriteRevealBaseline(root);
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(root));

    auto session = WireSession(repo);
    session.StartStandaloneScenario("s1");
    session.SetDestination("a_c");           // reveals a_d
    REQUIRE(session.IsNodeRevealed("alpha", "a_d"));

    auto restored = WireSession(repo);
    restored.ApplySaveData(session.ToSaveData());
    REQUIRE(restored.IsNodeRevealed("alpha", "a_d"));
    REQUIRE(restored.IsNodeRevealed("alpha", "a_arr"));
    std::filesystem::remove_all(root);
}

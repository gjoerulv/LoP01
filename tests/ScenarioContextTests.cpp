#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "app/mappers/WorldMapModelMapper.h"
#include "data/ContentRepository.h"
#include "gameplay/GameSession.h"

// M32 Scenario Context: a thin scenario may author a `regions` list naming the
// Region ids it exposes. Empty/absent => the default context of ALL loaded
// Regions (backward compatible). The start Region must be in the context, and
// read models (World Map) plus GameSession::TravelToRegion never expose Regions
// outside the active context.

namespace {

void WriteTextFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream output(path, std::ios::trunc);
    output << content;
}

// Two-region baseline (alpha, beta) with a World Map linking them. Everything but
// scenarios.json is written here so each test supplies its own scenario object.
void WriteContextBaseline(const std::filesystem::path& root) {
    std::filesystem::create_directories(root);
    WriteTextFile(root / "regions.json", R"({"schemaVersion":1,"kind":"RegionCollection","id":"regions","regions":[
        {"id":"alpha","name":"Alpha","unlocked":true,"arrivalNodeId":"a_arr","nodes":[
            {"location_id":"a_arr","x":0,"y":0},{"location_id":"a_mid","x":1,"y":0}],"links":[["a_arr","a_mid"]]},
        {"id":"beta","name":"Beta","unlocked":true,"arrivalNodeId":"b_arr","nodes":[
            {"location_id":"b_arr","x":0,"y":0}],"links":[]}
    ]})");
    WriteTextFile(root / "locations.json", R"({"schemaVersion":1,"kind":"LocationCollection","id":"locations","locations":[
        {"id":"a_arr","name":"Alpha Arrival","type":"town","allows_sleep":false,"overworld_destination":true},
        {"id":"a_mid","name":"Alpha Mid","type":"town","allows_sleep":false,"overworld_destination":true},
        {"id":"b_arr","name":"Beta Arrival","type":"town","allows_sleep":false,"overworld_destination":true}
    ]})");
    WriteTextFile(root / "location_scenes.json", R"({"schemaVersion":1,"kind":"LocationSceneCollection","id":"location_scenes","location_scenes":[]})");
    WriteTextFile(root / "units.json", R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[{"id":"hero","name":"Hero","category":"leader","is_player_character":true,"attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":1,"life":1,"position":"leader","range":"melee"}]})");
    WriteTextFile(root / "battle_scenarios.json", R"({"schemaVersion":1,"kind":"BattleScenarioCollection","id":"battle_scenarios","battle_scenarios":[]})");
    WriteTextFile(root / "enemy_groups.json", R"({"schemaVersion":1,"kind":"EnemyGroupCollection","id":"enemy_groups","enemy_groups":[]})");
    WriteTextFile(root / "quests.json", R"({"schemaVersion":1,"kind":"QuestCollection","id":"quests","quests":[]})");
    WriteTextFile(root / "location_services.json", R"({"schemaVersion":1,"kind":"LocationServiceCollection","id":"location_services","location_services":[]})");
    WriteTextFile(root / "world_map.json", R"({"schemaVersion":1,"kind":"WorldMap","id":"world_map","name":{"en":"WM"},"entries":[
        {"id":"alpha","unlocked":true,"exitNodeIds":["a_arr"],"x":0,"y":0},
        {"id":"beta","unlocked":true,"exitNodeIds":["b_arr"],"x":1,"y":0}],"adjacency":[["alpha","beta"]]})");
}

bool HasCode(const std::vector<ValidationMessage>& msgs, const std::string& code) {
    return std::any_of(msgs.begin(), msgs.end(),
        [&](const ValidationMessage& m) { return m.code == code; });
}
bool HasError(const std::vector<ValidationMessage>& msgs) {
    return std::any_of(msgs.begin(), msgs.end(),
        [](const ValidationMessage& m) { return m.severity == Severity::Error; });
}

std::vector<ValidationMessage> LoadScenario(
    const std::filesystem::path& root, const std::string& scenarioObject, data::ContentRepository& repo) {
    WriteContextBaseline(root);
    WriteTextFile(root / "scenarios.json",
        R"({"schemaVersion":1,"kind":"ScenarioCollection","id":"scenarios","scenarios":[)" +
        scenarioObject + "]}");
    (void)repo.LoadFromDirectory(root);
    return repo.ValidationMessages();
}

gameplay::GameSession WireSession(const data::ContentRepository& repo) {
    gameplay::GameSession session;
    session.SetPlayerColor("Green");
    session.SetUnitCatalog(repo.Units());
    session.SetLeaderCapableUnitIds({"hero"});
    REQUIRE(session.AddOwnedUnit("hero", 1));
    REQUIRE(session.TryAddUnitToActiveParty("hero"));
    session.SetRegionCatalog(repo.Regions());
    session.SetWorldMap(repo.WorldMap());
    session.SetScenarioCatalog(repo.Scenarios());
    session.SetCampaignCatalog(repo.Campaigns());
    session.SetLocationServiceCatalog(repo.LocationServices());
    return session;
}

} // namespace

TEST_CASE("ScenarioContext: an authored regions list parses and constrains context") {
    const std::filesystem::path root = "saves/ctx_valid";
    data::ContentRepository repo;
    const auto msgs = LoadScenario(root,
        R"({"id":"s1","startRegionId":"alpha","regions":["alpha"]})", repo);
    REQUIRE_FALSE(HasError(msgs));
    const auto* s = repo.FindScenarioById("s1");
    REQUIRE(s != nullptr);
    REQUIRE(s->regionIds == std::vector<std::string>{"alpha"});
    std::filesystem::remove_all(root);
}

TEST_CASE("ScenarioContext: invalid region references are rejected") {
    struct Case { std::string name; std::string scenario; std::string code; };
    const std::vector<Case> cases = {
        {"unknown", R"({"id":"s1","startRegionId":"alpha","regions":["alpha","ghost"]})", "SCENARIO_REGION_UNKNOWN"},
        {"duplicate", R"({"id":"s1","startRegionId":"alpha","regions":["alpha","alpha"]})", "SCENARIO_REGION_DUPLICATE"},
        {"start_not_in", R"({"id":"s1","startRegionId":"alpha","regions":["beta"]})", "SCENARIO_START_REGION_NOT_IN_CONTEXT"},
        {"type", R"({"id":"s1","startRegionId":"alpha","regions":5})", "SCENARIO_REGIONS_TYPE_INVALID"},
        {"field", R"({"id":"s1","startRegionId":"alpha","regions":[5]})", "SCENARIO_REGION_FIELD_INVALID"},
    };
    for (const auto& c : cases) {
        const std::filesystem::path root = "saves/ctx_bad_" + c.name;
        data::ContentRepository repo;
        const auto msgs = LoadScenario(root, c.scenario, repo);
        INFO(c.name << " expected " << c.code);
        REQUIRE(HasCode(msgs, c.code));
        std::filesystem::remove_all(root);
    }
}

TEST_CASE("ScenarioContext: starting a scenario binds its region context") {
    const std::filesystem::path root = "saves/ctx_runtime";
    data::ContentRepository repo;
    REQUIRE_FALSE(HasError(LoadScenario(root,
        R"({"id":"s1","startRegionId":"alpha","regions":["alpha"]})", repo)));

    auto session = WireSession(repo);
    session.StartStandaloneScenario("s1");

    REQUIRE(session.ScenarioRegionIds() == std::vector<std::string>{"alpha"});
    REQUIRE(session.IsRegionInScenarioContext("alpha"));
    REQUIRE_FALSE(session.IsRegionInScenarioContext("beta"));

    // World Map read model never lists the out-of-context Region.
    app::mappers::WorldMapModelMapper mapper;
    const auto model = mapper.Map(repo, session, 0, false);
    for (const auto& dest : model.destinations) {
        REQUIRE(dest.regionId != "beta");
    }

    // Travel to an out-of-context Region is refused (no state change).
    const auto before = session.Snapshot();
    const auto result = session.TravelToRegion("beta");
    REQUIRE_FALSE(result.success);
    REQUIRE(session.Snapshot().regionId == before.regionId);

    std::filesystem::remove_all(root);
}

TEST_CASE("ScenarioContext: no authored regions => default context exposes all regions") {
    const std::filesystem::path root = "saves/ctx_default";
    data::ContentRepository repo;
    REQUIRE_FALSE(HasError(LoadScenario(root,
        R"({"id":"s1","startRegionId":"alpha"})", repo)));

    auto session = WireSession(repo);
    session.StartStandaloneScenario("s1");

    REQUIRE(session.ScenarioRegionIds().empty());
    REQUIRE(session.IsRegionInScenarioContext("alpha"));
    REQUIRE(session.IsRegionInScenarioContext("beta"));

    app::mappers::WorldMapModelMapper mapper;
    const auto model = mapper.Map(repo, session, 0, false);
    const bool listsBeta = std::any_of(model.destinations.begin(), model.destinations.end(),
        [](const auto& d) { return d.regionId == "beta"; });
    REQUIRE(listsBeta);

    std::filesystem::remove_all(root);
}

TEST_CASE("ScenarioContext: context is re-derived on save/load") {
    const std::filesystem::path root = "saves/ctx_saveload";
    data::ContentRepository repo;
    REQUIRE_FALSE(HasError(LoadScenario(root,
        R"({"id":"s1","startRegionId":"alpha","regions":["alpha"]})", repo)));

    auto session = WireSession(repo);
    session.StartStandaloneScenario("s1");

    auto restored = WireSession(repo);
    restored.ApplySaveData(session.ToSaveData());
    REQUIRE(restored.ScenarioRegionIds() == std::vector<std::string>{"alpha"});
    REQUIRE_FALSE(restored.IsRegionInScenarioContext("beta"));

    std::filesystem::remove_all(root);
}

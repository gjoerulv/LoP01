#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "core/GameClock.h"
#include "data/ContentRepository.h"
#include "gameplay/GameSession.h"

// M32 scenario-authored starting roster (playerStart.roster). When a scenario
// authors a roster, the scenario start REBUILDS the active/reserve roster from it
// (and resets inventory + clock) instead of inheriting the previous run. When no
// roster is authored the existing roster is preserved exactly as M16/M31 did.

namespace {

void WriteTextFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream output(path, std::ios::trunc);
    output << content;
}

// Baseline with a player character (leader), one extra hero, and two generics.
void WriteRosterBaseline(const std::filesystem::path& root) {
    std::filesystem::create_directories(root);
    WriteTextFile(root / "regions.json", R"({"schemaVersion":1,"kind":"RegionCollection","id":"regions","regions":[
        {"id":"alpha","name":"Alpha","unlocked":true,"arrivalNodeId":"a_arr","nodes":[
            {"location_id":"a_arr","x":0,"y":0}],"links":[]}
    ]})");
    WriteTextFile(root / "locations.json", R"({"schemaVersion":1,"kind":"LocationCollection","id":"locations","locations":[
        {"id":"a_arr","name":"Alpha Arrival","type":"town","allows_sleep":false,"overworld_destination":true}
    ]})");
    WriteTextFile(root / "location_scenes.json", R"({"schemaVersion":1,"kind":"LocationSceneCollection","id":"location_scenes","location_scenes":[]})");
    WriteTextFile(root / "units.json", R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[
        {"id":"hero_player","name":"PC","category":"leader","is_player_character":true,"attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":5,"life":1,"position":"leader","range":"melee"},
        {"id":"hero_ally","name":"Ally","category":"hero","is_player_character":false,"attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":5,"life":1,"position":"front","range":"melee"},
        {"id":"grunt","name":"Grunt","category":"generic","is_player_character":false,"attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":5,"life":2,"position":"front","range":"melee"},
        {"id":"grunt2","name":"Grunt2","category":"generic","is_player_character":false,"attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":5,"life":2,"position":"front","range":"melee"}
    ]})");
    WriteTextFile(root / "battle_scenarios.json", R"({"schemaVersion":1,"kind":"BattleScenarioCollection","id":"battle_scenarios","battle_scenarios":[]})");
    WriteTextFile(root / "enemy_groups.json", R"({"schemaVersion":1,"kind":"EnemyGroupCollection","id":"enemy_groups","enemy_groups":[]})");
    WriteTextFile(root / "quests.json", R"({"schemaVersion":1,"kind":"QuestCollection","id":"quests","quests":[]})");
    WriteTextFile(root / "location_services.json", R"({"schemaVersion":1,"kind":"LocationServiceCollection","id":"location_services","location_services":[]})");
}

bool HasCode(const std::vector<ValidationMessage>& msgs, const std::string& code) {
    return std::any_of(msgs.begin(), msgs.end(),
        [&](const ValidationMessage& m) { return m.code == code; });
}
bool HasError(const std::vector<ValidationMessage>& msgs) {
    return std::any_of(msgs.begin(), msgs.end(),
        [](const ValidationMessage& m) { return m.severity == Severity::Error; });
}

// Loads the baseline plus a scenarios.json holding `scenarioArray` (the inner
// array contents, without the surrounding [ ]).
std::vector<ValidationMessage> LoadScenarios(
    const std::filesystem::path& root, const std::string& scenarioArray, data::ContentRepository& repo) {
    WriteRosterBaseline(root);
    WriteTextFile(root / "scenarios.json",
        R"({"schemaVersion":1,"kind":"ScenarioCollection","id":"scenarios","scenarios":[)" +
        scenarioArray + "]}");
    (void)repo.LoadFromDirectory(root);
    return repo.ValidationMessages();
}

gameplay::GameSession WireSession(const data::ContentRepository& repo) {
    gameplay::GameSession session;
    session.SetPlayerColor("Green");
    session.SetUnitCatalog(repo.Units());
    session.SetLeaderCapableUnitIds({"hero_player", "hero_ally"});
    REQUIRE(session.AddOwnedUnit("hero_player", 1));
    REQUIRE(session.TryAddUnitToActiveParty("hero_player"));
    session.SetRegionCatalog(repo.Regions());
    session.SetWorldMap(repo.WorldMap());
    session.SetScenarioCatalog(repo.Scenarios());
    session.SetCampaignCatalog(repo.Campaigns());
    session.SetLocationServiceCatalog(repo.LocationServices());
    return session;
}

bool ActiveHas(const gameplay::GameSession& s, const std::string& unitId) {
    const auto& ids = s.ActivePartyUnitIds();
    return std::find(ids.begin(), ids.end(), unitId) != ids.end();
}

int StackQuantity(const gameplay::GameSession& s, const std::string& unitId) {
    for (const auto& st : s.RosterStacks()) {
        if (st.unitId == unitId) return st.quantity;
    }
    return 0;
}

} // namespace

TEST_CASE("ScenarioRoster: a valid authored roster loads and binds active/reserve") {
    const std::filesystem::path root = "saves/roster_valid";
    data::ContentRepository repo;
    const auto msgs = LoadScenarios(root,
        R"({"id":"s1","startRegionId":"alpha","playerStart":{"roster":{)"
        R"("active":[{"unitId":"hero_player"},{"unitId":"grunt","quantity":5}],)"
        R"("reserve":[{"unitId":"hero_ally"},{"unitId":"grunt2","quantity":3}]}}})", repo);
    REQUIRE_FALSE(HasError(msgs));

    const auto* s = repo.FindScenarioById("s1");
    REQUIRE(s != nullptr);
    REQUIRE(s->hasAuthoredRoster);
    REQUIRE(s->startActiveRoster.size() == 2);
    REQUIRE(s->startReserveRoster.size() == 2);

    auto session = WireSession(repo);
    session.StartStandaloneScenario("s1");

    REQUIRE(ActiveHas(session, "hero_player"));
    REQUIRE(ActiveHas(session, "grunt"));
    REQUIRE(session.ReserveUnitCount("hero_ally") == 1);
    REQUIRE(session.ReserveUnitCount("grunt2") == 3);
    REQUIRE(StackQuantity(session, "grunt") == 5);
    REQUIRE(StackQuantity(session, "grunt2") == 3);
    std::filesystem::remove_all(root);
}

TEST_CASE("ScenarioRoster: invalid authored rosters are rejected") {
    struct Case { std::string name; std::string roster; std::string code; };
    const std::vector<Case> cases = {
        {"type", R"("roster":5)", "SCENARIO_START_ROSTER_TYPE_INVALID"},
        {"list_type", R"("roster":{"active":5})", "SCENARIO_START_ROSTER_TYPE_INVALID"},
        {"entry_type", R"("roster":{"active":[5]})", "SCENARIO_START_ROSTER_ENTRY_TYPE_INVALID"},
        {"field", R"("roster":{"active":[{"quantity":1}]})", "SCENARIO_START_ROSTER_FIELD_INVALID"},
        {"unknown_unit", R"("roster":{"active":[{"unitId":"hero_player"},{"unitId":"ghost"}]})", "SCENARIO_START_ROSTER_UNIT_UNKNOWN"},
        {"qty_zero", R"("roster":{"active":[{"unitId":"hero_player"},{"unitId":"grunt","quantity":0}]})", "SCENARIO_START_ROSTER_QUANTITY_INVALID"},
        {"qty_negative", R"("roster":{"active":[{"unitId":"hero_player"},{"unitId":"grunt","quantity":-2}]})", "SCENARIO_START_ROSTER_QUANTITY_INVALID"},
        {"qty_nonint", R"("roster":{"active":[{"unitId":"hero_player"},{"unitId":"grunt","quantity":"x"}]})", "SCENARIO_START_ROSTER_QUANTITY_INVALID"},
        {"hero_qty", R"("roster":{"active":[{"unitId":"hero_player"},{"unitId":"hero_ally","quantity":2}]})", "SCENARIO_START_ROSTER_HERO_QUANTITY_INVALID"},
        {"hero_dup", R"("roster":{"active":[{"unitId":"hero_player"},{"unitId":"hero_ally"}],"reserve":[{"unitId":"hero_ally"}]})", "SCENARIO_START_ROSTER_HERO_DUPLICATE"},
        {"pc_dup", R"("roster":{"active":[{"unitId":"hero_player"}],"reserve":[{"unitId":"hero_player"}]})", "SCENARIO_START_ROSTER_HERO_DUPLICATE"},
        {"pc_missing", R"("roster":{"active":[{"unitId":"hero_ally"}]})", "SCENARIO_START_ROSTER_PLAYER_CHARACTER_MISSING"},
        {"no_active_leader", R"("roster":{"active":[{"unitId":"grunt"}],"reserve":[{"unitId":"hero_player"}]})", "SCENARIO_START_ROSTER_NO_ACTIVE_LEADER"},
        {"active_overflow", R"("roster":{"active":[{"unitId":"hero_player"},{"unitId":"grunt"},{"unitId":"grunt2"},{"unitId":"grunt"},{"unitId":"grunt2"},{"unitId":"grunt"}]})", "SCENARIO_START_ROSTER_ACTIVE_OVERFLOW"},
        {"reserve_overflow", R"("roster":{"active":[{"unitId":"hero_player"}],"reserve":[{"unitId":"grunt"},{"unitId":"grunt"},{"unitId":"grunt"},{"unitId":"grunt"},{"unitId":"grunt"},{"unitId":"grunt"},{"unitId":"grunt"},{"unitId":"grunt"},{"unitId":"grunt"}]})", "SCENARIO_START_ROSTER_RESERVE_OVERFLOW"},
    };
    for (const auto& c : cases) {
        const std::filesystem::path root = "saves/roster_bad_" + c.name;
        data::ContentRepository repo;
        const auto msgs = LoadScenarios(root,
            R"({"id":"s1","startRegionId":"alpha","playerStart":{)" + c.roster + "}}", repo);
        INFO(c.name << " expected " << c.code);
        REQUIRE(HasCode(msgs, c.code));
        std::filesystem::remove_all(root);
    }
}

TEST_CASE("ScenarioRoster: a second New Game does not inherit the previous run's roster") {
    const std::filesystem::path root = "saves/roster_leak";
    data::ContentRepository repo;
    const auto msgs = LoadScenarios(root,
        R"({"id":"s1","startRegionId":"alpha","playerStart":{"roster":{)"
        R"("active":[{"unitId":"hero_player"},{"unitId":"grunt","quantity":5}]}}},)"
        R"({"id":"s2","startRegionId":"alpha","playerStart":{"roster":{)"
        R"("active":[{"unitId":"hero_player"}],"reserve":[{"unitId":"grunt2","quantity":2}]}}})", repo);
    REQUIRE_FALSE(HasError(msgs));

    auto session = WireSession(repo);
    session.StartStandaloneScenario("s1");
    REQUIRE(StackQuantity(session, "grunt") == 5);

    // Second New Game (different scenario) must rebuild from s2's authored roster.
    session.StartStandaloneScenario("s2");
    REQUIRE(ActiveHas(session, "hero_player"));
    REQUIRE(session.OwnedUnitCount("grunt") == 0);     // s1's grunts gone
    REQUIRE(session.ReserveUnitCount("grunt2") == 2);
    std::filesystem::remove_all(root);
}

TEST_CASE("ScenarioRoster: scenario start resets the day/clock") {
    const std::filesystem::path root = "saves/roster_clock";
    data::ContentRepository repo;
    REQUIRE_FALSE(HasError(LoadScenarios(root,
        R"({"id":"s1","startRegionId":"alpha","playerStart":{"roster":{)"
        R"("active":[{"unitId":"hero_player"}]}}})", repo)));

    auto session = WireSession(repo);
    session.AddMinutes(core::GameClock::kMinutesPerSliceDay * 2);   // advance to day 3
    REQUIRE(session.Snapshot().day == 3);

    session.StartStandaloneScenario("s1");
    REQUIRE(session.Snapshot().day == 1);
    std::filesystem::remove_all(root);
}

TEST_CASE("ScenarioRoster: a scenario without an authored roster preserves the existing roster") {
    const std::filesystem::path root = "saves/roster_noauthor";
    data::ContentRepository repo;
    REQUIRE_FALSE(HasError(LoadScenarios(root,
        R"({"id":"s1","startRegionId":"alpha"})", repo)));

    auto session = WireSession(repo);
    REQUIRE(session.AddOwnedUnit("grunt", 3));   // a pre-existing reserve stack

    session.StartStandaloneScenario("s1");
    // No authored roster => the prebuilt roster is kept (M16/M31 behavior).
    REQUIRE(session.OwnedUnitCount("grunt") == 3);
    REQUIRE(ActiveHas(session, "hero_player"));
    std::filesystem::remove_all(root);
}

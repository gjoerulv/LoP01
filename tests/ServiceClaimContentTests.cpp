#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "core/GameClock.h"
#include "core/SaveGame.h"
#include "data/ContentRepository.h"
#include "data/definitions/LocationServiceDefinition.h"
#include "gameplay/EnemyTeamState.h"
#include "gameplay/GameSession.h"
#include "gameplay/ResourceState.h"

// Authored-content proof for M23: a self-contained slice where a Region node /
// Location with a battle scenario hosts an UNOWNED mine and is guarded by a
// hostile team spawned through an authored `spawnTeam` event. Loading validates
// cleanly; activating the guard occupies the mine node; and after the guard is
// defeated the player claims the mine, which then pays at the daily payout. Uses
// a temp content dir rather than expanding shipped content.

using data::LocationServiceKind;
using gameplay::ResourceType;

namespace {

constexpr int kOneDay = core::GameClock::kMinutesPerSliceDay;

void WriteTextFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream output(path, std::ios::trunc);
    output << content;
}

bool HasErrorMessage(const std::vector<ValidationMessage>& msgs) {
    return std::any_of(msgs.begin(), msgs.end(),
        [](const ValidationMessage& m) { return m.severity == Severity::Error; });
}

// Region with an arrival node ("home") and a guarded mine node ("deep_mine").
// The mine Location carries a battle scenario (hostile contact) and a scene/zone
// for the mine service. An authored spawnTeam event guards deep_mine when the
// player enters home.
void WriteClaimProofContent(const std::filesystem::path& root) {
    std::filesystem::create_directories(root);

    WriteTextFile(root / "regions.json", R"({"schemaVersion":1,"kind":"RegionCollection","id":"regions","regions":[
        {"id":"r1","name":"Contested Vale","unlocked":true,"arrivalNodeId":"home","nodes":[
            {"location_id":"home","x":0,"y":0,"discovered":true,"travel_available":true},
            {"location_id":"deep_mine","x":1,"y":0,"discovered":true,"travel_available":true}],
         "links":[["home","deep_mine"]]}
    ]})");

    WriteTextFile(root / "locations.json", R"({"schemaVersion":1,"kind":"LocationCollection","id":"locations","locations":[
        {"id":"home","name":"Home","type":"home","allows_sleep":true,"overworld_destination":true,"scene_id":"home_scene"},
        {"id":"deep_mine","name":"Deep Mine","type":"dungeon","allows_sleep":false,"overworld_destination":true,"scene_id":"mine_scene","battle_scenario_id":"guard_battle"}
    ]})");

    WriteTextFile(root / "location_scenes.json", R"({"schemaVersion":1,"kind":"LocationSceneCollection","id":"location_scenes","location_scenes":[
        {"id":"home_scene","spawn":{"x":0,"y":0,"width":1,"height":1},"blocking_rects":[],"zones":[
            {"id":"door","type":"recruit","area":{"x":0,"y":0,"width":1,"height":1},"prompt_text":"","result_text":"","failure_text":"","time_cost_minutes":0,"gold_cost":0,"recruit_count":0,"dialogue_choice_time_cost_minutes":1,"dialogue_choices":[]}]},
        {"id":"mine_scene","spawn":{"x":0,"y":0,"width":1,"height":1},"blocking_rects":[],"zones":[
            {"id":"mine_face","type":"recruit","area":{"x":0,"y":0,"width":1,"height":1},"prompt_text":"","result_text":"","failure_text":"","time_cost_minutes":0,"gold_cost":0,"recruit_count":0,"dialogue_choice_time_cost_minutes":1,"dialogue_choices":[]}]}
    ]})");

    WriteTextFile(root / "units.json", R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[
        {"id":"hero","name":"Hero","category":"hero","is_player_character":true,"attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":1,"life":1,"position":"front","range":"melee"},
        {"id":"kobold","name":"Kobold","category":"generic","is_player_character":false,"attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":1,"life":1,"position":"front","range":"melee"}
    ]})");

    WriteTextFile(root / "battle_scenarios.json", R"({"schemaVersion":1,"kind":"BattleScenarioCollection","id":"battle_scenarios","battle_scenarios":[
        {"id":"guard_battle","name":"Mine Guard","seed":7,"allies":[{"unit_id":"hero"}],"enemies":[{"unit_id":"kobold"}]}
    ]})");

    WriteTextFile(root / "enemy_groups.json", R"({"schemaVersion":1,"kind":"EnemyGroupCollection","id":"enemy_groups","enemy_groups":[]})");
    WriteTextFile(root / "quests.json", R"({"schemaVersion":1,"kind":"QuestCollection","id":"quests","quests":[]})");

    WriteTextFile(root / "location_services.json", R"({"schemaVersion":1,"kind":"LocationServiceCollection","id":"location_services","location_services":[
        {"id":"deep_mine_svc","location_id":"deep_mine","zone_id":"mine_face","kind":"mine","mine_outputs":[{"resource":"Stone","amount":2},{"resource":"Gold","amount":1000}]}
    ]})");

    // Authored spawnTeam: entering home guards the mine node with the Red team.
    WriteTextFile(root / "events.json", R"({"schemaVersion":1,"kind":"EventCollection","id":"events","events":[
        {"id":"evt_guard_mine","trigger":{"type":"regionNodeEntry","nodeId":"home"},
         "actions":[{"type":"spawnTeam","teamColor":"Red","nodeId":"deep_mine"}],
         "repeat":{"mode":"once"}}
    ]})");
}

core::SaveData BaseSave() {
    core::SaveData s;
    s.schemaVersion = 5;
    s.day = 1;
    s.minutesIntoSliceDay = 0;
    s.gold = 500;
    s.mode = "region_mode";
    s.regionId = "r1";
    s.destinationId = "home";
    s.hasCanonicalRoster = true;
    s.rosterStacks = {core::RosterStackSaveState{"stk_1", "hero", 1}};
    s.activeSlotStackIds = {"stk_1", "", "", "", ""};
    s.reserveSlotStackIds = {"", "", "", "", "", "", "", ""};
    s.nextStackIdCounter = 2;
    return s;
}

} // namespace

TEST_CASE("ServiceClaim content: authored guarded mine validates, spawns, is claimed, and pays") {
    const std::filesystem::path root = "saves/service_claim_content";
    WriteClaimProofContent(root);

    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(root));
    REQUIRE_FALSE(HasErrorMessage(repo.ValidationMessages()));

    gameplay::GameSession session;
    session.SetPlayerColor("Green");
    session.SetUnitCatalog(repo.Units());
    session.SetRegionCatalog(repo.Regions());
    session.SetLocationServiceCatalog(repo.LocationServices());
    session.InitializeEventDefinitions(repo.EventDefinitions());
    session.ApplySaveData(BaseSave());

    // The contesting system seeds teams at runtime; the authored spawnTeam event
    // activates a seeded team of that color at the target node. Seed after
    // ApplySaveData so it is not overwritten.
    gameplay::EnemyTeamState seed;
    seed.teamColor = "Red";
    seed.active = false;
    session.SetEnemyTeams({seed});

    // Authored spawnTeam fires on entering home and guards the mine node.
    static_cast<void>(session.NotifyRegionNodeEntry("home"));
    const auto occupied = session.HostileOccupiedNodeIds("Green");
    REQUIRE(std::find(occupied.begin(), occupied.end(), "deep_mine") != occupied.end());

    // The mine starts unowned (not in playerStart / ownedServices).
    REQUIRE(session.FindOwnedService("deep_mine_svc") == nullptr);

    // Defeat the guard, then claim the now-uncontested mine.
    session.ClearEnemyTeamByColor("Red");
    const auto claimed = session.ClaimContestedServicesAtNode("deep_mine");
    REQUIRE(claimed.size() == 1);
    REQUIRE(claimed.front() == "deep_mine_svc");
    const auto* owned = session.FindOwnedService("deep_mine_svc");
    REQUIRE(owned != nullptr);
    REQUIRE(owned->ownerTeamColor == "Green");

    // The claimed mine pays the player at the next daily payout.
    const int goldBefore = session.Snapshot().gold;
    REQUIRE(session.ResourceCount(ResourceType::Stone) == 0);
    session.AddMinutes(kOneDay);
    REQUIRE(session.ResourceCount(ResourceType::Stone) == 2);
    REQUIRE(session.Snapshot().gold == goldBefore + 1000);

    std::filesystem::remove_all(root);
}

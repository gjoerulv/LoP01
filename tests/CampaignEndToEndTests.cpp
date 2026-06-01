#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <string>

#include "data/ContentRepository.h"
#include "gameplay/GameSession.h"

using namespace gameplay;

namespace {

void WriteTextFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream output(path, std::ios::trunc);
    output << content;
}

// Two-region baseline + a two-scenario campaign. No scenario_outcome.json and no
// inline scenario outcomes => the global fallback is empty => default victory
// fires (no hostile enemy teams), which drives the campaign forward.
void WriteCampaignContent(const std::filesystem::path& root) {
    std::filesystem::create_directories(root);
    WriteTextFile(root / "regions.json", R"({"schemaVersion":1,"kind":"RegionCollection","id":"regions","regions":[
        {"id":"alpha","name":"Alpha","unlocked":true,"arrivalNodeId":"alpha_arrival","nodes":[
            {"location_id":"alpha_arrival","x":0,"y":0}],"links":[]},
        {"id":"beta","name":"Beta","unlocked":true,"arrivalNodeId":"beta_arrival","nodes":[
            {"location_id":"beta_arrival","x":0,"y":0}],"links":[]}
    ]})");
    WriteTextFile(root / "locations.json", R"({"schemaVersion":1,"kind":"LocationCollection","id":"locations","locations":[
        {"id":"alpha_arrival","name":"Alpha Arrival","type":"town","allows_sleep":false,"overworld_destination":true},
        {"id":"beta_arrival","name":"Beta Arrival","type":"town","allows_sleep":false,"overworld_destination":true}
    ]})");
    WriteTextFile(root / "location_scenes.json", R"({"schemaVersion":1,"kind":"LocationSceneCollection","id":"location_scenes","location_scenes":[]})");
    WriteTextFile(root / "units.json", R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[
        {"id":"hero","name":"Hero","category":"hero","is_player_character":true,"attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":5,"life":1,"position":"front","range":"melee"},
        {"id":"ally","name":"Ally","category":"hero","is_player_character":false,"attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":2,"life":1,"position":"front","range":"melee"}
    ]})");
    WriteTextFile(root / "battle_scenarios.json", R"({"schemaVersion":1,"kind":"BattleScenarioCollection","id":"battle_scenarios","battle_scenarios":[]})");
    WriteTextFile(root / "enemy_groups.json", R"({"schemaVersion":1,"kind":"EnemyGroupCollection","id":"enemy_groups","enemy_groups":[]})");
    WriteTextFile(root / "quests.json", R"({"schemaVersion":1,"kind":"QuestCollection","id":"quests","quests":[]})");
    WriteTextFile(root / "location_services.json", R"({"schemaVersion":1,"kind":"LocationServiceCollection","id":"location_services","location_services":[]})");
    WriteTextFile(root / "scenarios.json", R"({"schemaVersion":1,"kind":"ScenarioCollection","id":"scenarios","scenarios":[
        {"id":"s_intro","startRegionId":"alpha","startNodeId":"alpha_arrival","startGold":500},
        {"id":"s_second","startRegionId":"beta","startNodeId":"beta_arrival","startGold":900}
    ]})");
    WriteTextFile(root / "campaigns.json", R"({"schemaVersion":1,"kind":"CampaignCollection","id":"campaigns","campaigns":[
        {"id":"camp_demo","name":"Demo","startScenarioId":"s_intro",
         "carryOverRules":[{"id":"carry_all","carryGold":true,"carryRoster":true,"carryItems":true,"carryArtifacts":true}],
         "scenarios":[
            {"scenarioId":"s_intro","nextScenarioIds":["s_second"],"carryOverRuleId":"carry_all"},
            {"scenarioId":"s_second","nextScenarioIds":[],"carryOverRuleId":""}
         ]}
    ]})");
}

// Mirrors the App startup wiring needed for campaign play.
GameSession WireSessionFromContent(const data::ContentRepository& content) {
    GameSession session;
    session.SetUnitCatalog(content.Units());
    session.SetLeaderCapableUnitIds({"hero", "ally"});
    REQUIRE(session.AddOwnedUnit("hero", 1));
    REQUIRE(session.TryAddUnitToActiveParty("hero"));
    REQUIRE(session.AddOwnedUnit("ally", 1));
    REQUIRE(session.TryAddUnitToActiveParty("ally"));
    session.SetScenarioOutcomeDefinition(content.ScenarioOutcome());
    session.SetRegionCatalog(content.Regions());
    session.SetWorldMap(content.WorldMap());
    session.SetScenarioCatalog(content.Scenarios());
    session.SetCampaignCatalog(content.Campaigns());
    return session;
}

} // namespace

TEST_CASE("Campaign end-to-end: a two-scenario campaign plays sequentially with carry-over") {
    const std::filesystem::path root = "saves/campaign_e2e_test";
    WriteCampaignContent(root);

    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));
    REQUIRE(content.Campaigns().size() == 1);

    GameSession session = WireSessionFromContent(content);

    session.StartCampaign("camp_demo");
    REQUIRE(session.IsCampaignActive());
    REQUIRE(session.CurrentScenarioId() == "s_intro");
    REQUIRE(session.Snapshot().regionId == "alpha");
    REQUIRE(session.Snapshot().gold == 500);

    // Default victory (no hostile teams, empty fallback) latches immediately.
    session.CheckAndLatchOutcome();
    REQUIRE(session.IsScenarioEnded());

    session.ResolveCampaignAfterOutcome();
    REQUIRE(session.CurrentScenarioId() == "s_second");
    REQUIRE(session.Snapshot().regionId == "beta");
    REQUIRE(session.GetCampaignState() == CampaignState::InProgress);
    // carryGold=true: gold carried from s_intro (500), not reset to s_second's 900.
    REQUIRE(session.Snapshot().gold == 500);
    // Carried roster (player + ally) present.
    REQUIRE(session.RosterStacks().size() == 2);

    // Win the final scenario -> campaign Completed.
    session.CheckAndLatchOutcome();
    session.ResolveCampaignAfterOutcome();
    REQUIRE(session.GetCampaignState() == CampaignState::Completed);

    std::filesystem::remove_all(root);
}

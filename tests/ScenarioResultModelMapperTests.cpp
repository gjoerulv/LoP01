#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <string>

#include "app/mappers/ScenarioResultModelMapper.h"
#include "data/ContentRepository.h"
#include "data/definitions/ScenarioOutcomeDefinition.h"
#include "gameplay/GameSession.h"
#include "gameplay/events/EventDefinition.h"

using namespace gameplay;

namespace {

void WriteTextFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream output(path, std::ios::trunc);
    output << content;
}

// Two regions, three named scenarios, and two campaigns: a two-scenario linear
// campaign (default victory drives it) and a single-scenario campaign whose only
// scenario inline-defeats immediately. Enough to exercise every result-screen
// next-step branch without enemy teams or battle.
void WriteResultMapperContent(const std::filesystem::path& root) {
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
        {"id":"s_intro","name":"Intro","startRegionId":"alpha","startNodeId":"alpha_arrival","startGold":500},
        {"id":"s_second","name":"Second","startRegionId":"beta","startNodeId":"beta_arrival","startGold":900},
        {"id":"s_doom","name":"Doom","startRegionId":"alpha","startNodeId":"alpha_arrival","startGold":100,
         "defeatConditions":[{"type":"always"}]}
    ]})");
    WriteTextFile(root / "campaigns.json", R"({"schemaVersion":1,"kind":"CampaignCollection","id":"campaigns","campaigns":[
        {"id":"camp","name":"Linear","startScenarioId":"s_intro",
         "carryOverRules":[{"id":"carry_all","carryGold":true,"carryRoster":true,"carryItems":true,"carryArtifacts":true}],
         "scenarios":[
            {"scenarioId":"s_intro","nextScenarioIds":["s_second"],"carryOverRuleId":"carry_all"},
            {"scenarioId":"s_second","nextScenarioIds":[],"carryOverRuleId":""}
         ]},
        {"id":"camp_doom","name":"Doomed","startScenarioId":"s_doom",
         "scenarios":[
            {"scenarioId":"s_doom","nextScenarioIds":[],"carryOverRuleId":""}
         ]}
    ]})");
}

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

data::ContentRepository LoadContent(const std::filesystem::path& root) {
    WriteResultMapperContent(root);
    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));
    return content;
}

} // namespace

TEST_CASE("ScenarioResultModelMapper: campaign Victory with a next scenario shows Next") {
    const std::filesystem::path root = "saves/result_mapper_next";
    data::ContentRepository content = LoadContent(root);
    GameSession session = WireSessionFromContent(content);

    session.StartCampaign("camp");
    session.CheckAndLatchOutcome();   // default victory (no enemies, empty conditions)
    REQUIRE(session.IsScenarioEnded());

    app::mappers::ScenarioResultModelMapper mapper;
    const auto model = mapper.Map(content, session);

    REQUIRE(model.victory);
    REQUIRE(model.outcomeLabel == "Victory!");
    REQUIRE(model.title == "Intro");
    REQUIRE(model.nextStepText == "Next: Second");

    std::filesystem::remove_all(root);
}

TEST_CASE("ScenarioResultModelMapper: winning the final scenario shows Campaign complete") {
    const std::filesystem::path root = "saves/result_mapper_complete";
    data::ContentRepository content = LoadContent(root);
    GameSession session = WireSessionFromContent(content);

    session.StartCampaign("camp");
    session.CheckAndLatchOutcome();
    session.ResolveCampaignAfterOutcome();         // advance to the final scenario
    REQUIRE(session.CurrentScenarioId() == "s_second");
    session.CheckAndLatchOutcome();                // win the final scenario
    REQUIRE(session.IsScenarioEnded());

    app::mappers::ScenarioResultModelMapper mapper;
    const auto model = mapper.Map(content, session);

    REQUIRE(model.victory);
    REQUIRE(model.title == "Second");
    REQUIRE(model.nextStepText == "Campaign complete");

    std::filesystem::remove_all(root);
}

TEST_CASE("ScenarioResultModelMapper: campaign Defeat shows Campaign failed") {
    const std::filesystem::path root = "saves/result_mapper_failed";
    data::ContentRepository content = LoadContent(root);
    GameSession session = WireSessionFromContent(content);

    session.StartCampaign("camp_doom");
    session.CheckAndLatchOutcome();   // inline always-defeat latches Defeat
    REQUIRE(session.IsScenarioEnded());

    app::mappers::ScenarioResultModelMapper mapper;
    const auto model = mapper.Map(content, session);

    REQUIRE_FALSE(model.victory);
    REQUIRE(model.outcomeLabel == "Defeat.");
    REQUIRE(model.title == "Doom");
    REQUIRE(model.nextStepText == "Campaign failed");

    std::filesystem::remove_all(root);
}

TEST_CASE("ScenarioResultModelMapper: standalone scenario end shows Scenario ended") {
    const std::filesystem::path root = "saves/result_mapper_standalone";
    data::ContentRepository content = LoadContent(root);
    GameSession session = WireSessionFromContent(content);

    // No campaign: latch an explicit always-victory outcome directly.
    data::ScenarioOutcomeDefinition def;
    events::EventCondition always;
    always.kind = events::EventConditionKind::Leaf;
    always.leafType = "always";
    def.victoryConditions = {always};
    session.SetScenarioOutcomeDefinition(def);
    session.CheckAndLatchOutcome();
    REQUIRE(session.IsScenarioEnded());
    REQUIRE_FALSE(session.IsCampaignActive());

    app::mappers::ScenarioResultModelMapper mapper;
    const auto model = mapper.Map(content, session);

    REQUIRE(model.victory);
    REQUIRE(model.nextStepText == "Scenario ended");

    std::filesystem::remove_all(root);
}

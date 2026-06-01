#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "data/ContentRepository.h"
#include "gameplay/GameSession.h"

#ifndef LOP01_PROJECT_ROOT
#define LOP01_PROJECT_ROOT "."
#endif

namespace {

std::filesystem::path RealContentDir() {
    return std::filesystem::path(LOP01_PROJECT_ROOT) / "content";
}

bool HasErrorMessage(const std::vector<ValidationMessage>& msgs) {
    return std::any_of(msgs.begin(), msgs.end(),
        [](const ValidationMessage& m) { return m.severity == Severity::Error; });
}

void WriteTextFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream output(path, std::ios::trunc);
    output << content;
}

// Baseline content WITHOUT scenarios.json / campaigns.json (no campaign).
void WriteNoCampaignContent(const std::filesystem::path& root) {
    std::filesystem::create_directories(root);
    WriteTextFile(root / "regions.json", R"({"schemaVersion":1,"kind":"RegionCollection","id":"regions","regions":[
        {"id":"alpha","name":"Alpha","unlocked":true,"arrivalNodeId":"alpha_arrival","nodes":[
            {"location_id":"alpha_arrival","x":0,"y":0}],"links":[]}
    ]})");
    WriteTextFile(root / "locations.json", R"({"schemaVersion":1,"kind":"LocationCollection","id":"locations","locations":[
        {"id":"alpha_arrival","name":"Alpha Arrival","type":"town","allows_sleep":false,"overworld_destination":true}
    ]})");
    WriteTextFile(root / "location_scenes.json", R"({"schemaVersion":1,"kind":"LocationSceneCollection","id":"location_scenes","location_scenes":[]})");
    WriteTextFile(root / "units.json", R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[{"id":"hero","name":"Hero","category":"hero","is_player_character":true,"attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":1,"life":1,"position":"front","range":"melee"}]})");
    WriteTextFile(root / "battle_scenarios.json", R"({"schemaVersion":1,"kind":"BattleScenarioCollection","id":"battle_scenarios","battle_scenarios":[]})");
    WriteTextFile(root / "enemy_groups.json", R"({"schemaVersion":1,"kind":"EnemyGroupCollection","id":"enemy_groups","enemy_groups":[]})");
    WriteTextFile(root / "quests.json", R"({"schemaVersion":1,"kind":"QuestCollection","id":"quests","quests":[]})");
    WriteTextFile(root / "location_services.json", R"({"schemaVersion":1,"kind":"LocationServiceCollection","id":"location_services","location_services":[]})");
}

} // namespace

TEST_CASE("Campaign startup: no campaigns loaded leaves the standalone front-end flow unchanged") {
    const std::filesystem::path root = "saves/campaign_startup_none_test";
    WriteNoCampaignContent(root);

    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));
    REQUIRE(content.Campaigns().empty());

    gameplay::GameSession session;
    session.SetScenarioCatalog(content.Scenarios());
    session.SetCampaignCatalog(content.Campaigns());

    // The standalone front-end mode machine is unchanged: Title advances to the
    // OpeningSequence, then to RegionMode. No campaign is ever active.
    REQUIRE(session.Snapshot().mode == gameplay::GameMode::Title);
    session.AdvanceMode();
    REQUIRE(session.Snapshot().mode == gameplay::GameMode::OpeningSequence);
    session.AdvanceMode();
    REQUIRE(session.Snapshot().mode == gameplay::GameMode::RegionMode);
    REQUIRE_FALSE(session.IsCampaignActive());

    std::filesystem::remove_all(root);
}

TEST_CASE("Campaign startup: real content campaign starts at its authored start scenario") {
    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(RealContentDir()));
    REQUIRE_FALSE(HasErrorMessage(content.ValidationMessages()));
    REQUIRE(content.FindCampaignById("campaign_ashvale") != nullptr);

    gameplay::GameSession session;
    session.SetUnitCatalog(content.Units());
    session.SetLeaderCapableUnitIds({"hero_player"});
    REQUIRE(session.AddOwnedUnit("hero_player", 1));
    REQUIRE(session.TryAddUnitToActiveParty("hero_player"));
    session.SetScenarioOutcomeDefinition(content.ScenarioOutcome());
    session.SetRegionCatalog(content.Regions());
    session.SetWorldMap(content.WorldMap());
    session.SetScenarioCatalog(content.Scenarios());
    session.SetCampaignCatalog(content.Campaigns());

    session.StartCampaign("campaign_ashvale");

    REQUIRE(session.IsCampaignActive());
    REQUIRE(session.GetCampaignState() == gameplay::CampaignState::InProgress);
    REQUIRE(session.CurrentScenarioId() == "scenario_intro");
    REQUIRE(session.Snapshot().mode == gameplay::GameMode::RegionMode);
    REQUIRE(session.Snapshot().regionId == "ashvale_heartland");
    REQUIRE(session.Snapshot().destinationId == "home_base");   // region arrival node
}

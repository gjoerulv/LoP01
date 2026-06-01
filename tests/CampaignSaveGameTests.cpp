#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>

#include "core/SaveGame.h"
#include "data/definitions/CampaignDefinition.h"
#include "data/definitions/ScenarioDefinition.h"
#include "data/definitions/UnitDefinition.h"
#include "gameplay/GameSession.h"

using namespace gameplay;

namespace {

data::UnitDefinition MakeUnit(const std::string& id, int agility, bool player) {
    data::UnitDefinition u;
    u.id = id; u.name = id;
    u.category = data::UnitDefinitionCategory::Hero;
    u.isPlayerCharacter = player;
    u.stats.agility = agility;
    return u;
}

data::ScenarioDefinition MakeScenario(const std::string& id, const std::string& region) {
    data::ScenarioDefinition s;
    s.id = id; s.startRegionId = region; s.startNodeId = "n";
    s.hasInlineOutcome = true;
    events::EventCondition always;
    always.kind = events::EventConditionKind::Leaf;
    always.leafType = "always";
    s.victoryConditions = {always};
    return s;
}

data::CampaignDefinition MakeCampaign() {
    data::CampaignDefinition c;
    c.id = "camp"; c.startScenarioId = "s1";
    data::CarryOverRule rule; rule.id = "carry"; rule.carryRoster = true;
    c.carryOverRules = {rule};
    c.scenarios = {{"s1", {"s2"}, "carry"}, {"s2", {}, ""}};
    return c;
}

GameSession MakeWiredSession() {
    GameSession session;
    session.SetUnitCatalog({MakeUnit("hero", 5, true)});
    session.SetLeaderCapableUnitIds({"hero"});
    REQUIRE(session.AddOwnedUnit("hero", 1));
    REQUIRE(session.TryAddUnitToActiveParty("hero"));
    session.SetScenarioCatalog({MakeScenario("s1", "alpha"), MakeScenario("s2", "beta")});
    session.SetCampaignCatalog({MakeCampaign()});
    return session;
}

} // namespace

TEST_CASE("Campaign save/load: campaign progression round-trips through SaveData") {
    GameSession a = MakeWiredSession();
    a.StartCampaign("camp");
    a.CheckAndLatchOutcome();
    a.AdvanceCampaignOnVictory();
    REQUIRE(a.CurrentScenarioId() == "s2");

    const core::SaveData save = a.ToSaveData();
    REQUIRE(save.campaignId == "camp");
    REQUIRE(save.currentScenarioId == "s2");
    REQUIRE(save.campaignState == "in_progress");
    REQUIRE(save.completedScenarioIds == std::vector<std::string>{"s1"});

    GameSession b = MakeWiredSession();
    b.ApplySaveData(save);
    REQUIRE(b.CampaignId() == "camp");
    REQUIRE(b.CurrentScenarioId() == "s2");
    REQUIRE(b.GetCampaignState() == CampaignState::InProgress);
    REQUIRE(b.CompletedScenarioIds() == std::vector<std::string>{"s1"});
}

TEST_CASE("Campaign save/load: JSON file round-trips campaign keys") {
    GameSession a = MakeWiredSession();
    a.StartCampaign("camp");
    a.CheckAndLatchOutcome();
    a.AdvanceCampaignOnVictory();

    const std::filesystem::path path = "saves/campaign_savegame_test.json";
    core::SaveGameRepository repo;
    REQUIRE(repo.SaveToFile(a.ToSaveData(), path.string()));
    const auto loaded = repo.LoadFromFile(path.string());
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->campaignId == "camp");
    REQUIRE(loaded->currentScenarioId == "s2");
    REQUIRE(loaded->campaignState == "in_progress");
    REQUIRE(loaded->completedScenarioIds == std::vector<std::string>{"s1"});
    std::filesystem::remove(path);
}

TEST_CASE("Campaign save/load: legacy save without campaign keys loads as no campaign") {
    GameSession a = MakeWiredSession();
    a.StartCampaign("camp");

    const std::filesystem::path path = "saves/campaign_legacy_test.json";
    core::SaveGameRepository repo;
    REQUIRE(repo.SaveToFile(a.ToSaveData(), path.string()));

    // Simulate a pre-M16 save by stripping the campaign keys from the file.
    nlohmann::json j;
    {
        std::ifstream in(path);
        in >> j;
    }
    for (const char* key : {"campaign_id", "current_scenario_id",
                            "completed_scenario_ids", "campaign_flags", "campaign_state"}) {
        j.erase(key);
    }
    {
        std::ofstream out(path, std::ios::trunc);
        out << j.dump(2);
    }

    const auto legacy = repo.LoadFromFile(path.string());
    REQUIRE(legacy.has_value());
    REQUIRE(legacy->campaignId.empty());
    REQUIRE(legacy->campaignState.empty());

    GameSession b = MakeWiredSession();
    b.ApplySaveData(*legacy);
    REQUIRE(b.GetCampaignState() == CampaignState::None);
    REQUIRE(b.CampaignId().empty());
    REQUIRE_FALSE(b.IsCampaignActive());
    std::filesystem::remove(path);
}

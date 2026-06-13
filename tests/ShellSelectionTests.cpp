#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <string>
#include <vector>

#include "app/CampaignController.h"
#include "app/ShellSelectionRules.h"
#include "app/input/InputState.h"
#include "app/mappers/ShellModelMapper.h"
#include "data/ContentRepository.h"

// M31 shell selection: pure playability rules, the shell screen models, and the
// shared list controller. Invalid/unplayable entries must never read as
// playable; the standalone list must enforce standaloneSelectable; disabled
// states carry player-facing reasons (docs/game_shell_flow.md §9/§11/§12).

#ifndef LOP01_PROJECT_ROOT
#define LOP01_PROJECT_ROOT "."
#endif

namespace {

std::filesystem::path RealContentDir() {
    return std::filesystem::path(LOP01_PROJECT_ROOT) / "content";
}

data::ContentRepository& SharedRepo() {
    static data::ContentRepository repo;
    static bool loaded = false;
    if (!loaded) {
        REQUIRE(repo.LoadFromDirectory(RealContentDir()));
        loaded = true;
    }
    return repo;
}

const app::shell::ShellEntryStatus kOpenGate{true, ""};

} // namespace

TEST_CASE("ShellRules - content gate blocks unloaded or invalid installs") {
    using app::shell::ContentGateStatus;
    REQUIRE(ContentGateStatus(true, 0).playable);
    REQUIRE_FALSE(ContentGateStatus(false, 0).playable);
    REQUIRE_FALSE(ContentGateStatus(true, 3).playable);
    REQUIRE(ContentGateStatus(true, 3).reason.find("3 error(s)") != std::string::npos);
}

TEST_CASE("ShellRules - shipped campaign and intro scenario are playable") {
    auto& repo = SharedRepo();
    const auto* campaign = repo.FindCampaignById("campaign_ashvale");
    REQUIRE(campaign != nullptr);
    REQUIRE(app::shell::CampaignPlayability(repo, *campaign).playable);

    const auto* intro = repo.FindScenarioById("scenario_intro");
    REQUIRE(intro != nullptr);
    REQUIRE(app::shell::ScenarioPlayability(repo, *intro).playable);
}

TEST_CASE("ShellRules - broken references are unplayable with readable reasons") {
    auto& repo = SharedRepo();

    data::ScenarioDefinition noRegion;
    noRegion.id = "s_broken";
    noRegion.startRegionId = "region_missing";
    const auto regionStatus = app::shell::ScenarioPlayability(repo, noRegion);
    REQUIRE_FALSE(regionStatus.playable);
    REQUIRE(regionStatus.reason == "Missing starting region.");

    data::ScenarioDefinition badNode;
    badNode.id = "s_bad_node";
    badNode.startRegionId = "ashvale_heartland";
    badNode.startNodeId = "node_missing";
    REQUIRE_FALSE(app::shell::ScenarioPlayability(repo, badNode).playable);

    data::CampaignDefinition noStart;
    noStart.id = "c_broken";
    const auto campaignStatus = app::shell::CampaignPlayability(repo, noStart);
    REQUIRE_FALSE(campaignStatus.playable);
    REQUIRE(campaignStatus.reason == "No starting scenario authored.");

    data::CampaignDefinition missingScenario;
    missingScenario.id = "c_missing";
    missingScenario.startScenarioId = "scenario_missing";
    REQUIRE_FALSE(app::shell::CampaignPlayability(repo, missingScenario).playable);
}

TEST_CASE("ShellRules - standalone list enforces standaloneSelectable") {
    auto& repo = SharedRepo();
    const auto standalone = app::shell::StandaloneScenarios(repo);
    REQUIRE(standalone.size() == 1);
    REQUIRE(standalone[0]->id == "scenario_intro");  // scenario_second is campaign-only
}

TEST_CASE("ShellMapper - main menu reflects save and content availability") {
    app::mappers::ShellModelMapper mapper;

    const auto withSave = mapper.MapMainMenu(true, true, 0, "");
    REQUIRE(withSave.menuItems ==
        std::vector<std::string>{"Continue", "New Game", "Quit"});
    REQUIRE(withSave.menuItemEnabled == std::vector<bool>{true, true, true});

    const auto noSave = mapper.MapMainMenu(false, true, 1, "No save found.");
    REQUIRE(noSave.menuItemEnabled == std::vector<bool>{false, true, true});
    REQUIRE(noSave.selectedIndex == 1);
    REQUIRE(noSave.statusText == "No save found.");

    const auto noContent = mapper.MapMainMenu(false, false, 0, "");
    REQUIRE(noContent.menuItemEnabled == std::vector<bool>{false, false, true});
}

TEST_CASE("ShellMapper - game mode select lists Campaign and Standalone Scenario") {
    app::mappers::ShellModelMapper mapper;
    const auto model = mapper.MapGameModeSelect(SharedRepo(), 1, "");

    REQUIRE(model.campaigns.size() == 2);
    REQUIRE(model.campaigns[0].name == "Campaign");
    REQUIRE(model.campaigns[0].enabled);
    REQUIRE_FALSE(model.campaigns[0].selected);
    REQUIRE(model.campaigns[1].name == "Standalone Scenario");
    REQUIRE(model.campaigns[1].enabled);
    REQUIRE(model.campaigns[1].selected);
}

TEST_CASE("ShellMapper - campaign list shows shipped campaign as playable") {
    app::mappers::ShellModelMapper mapper;
    const auto model = mapper.MapCampaignSelect(SharedRepo(), kOpenGate, 0, "");

    REQUIRE(model.campaigns.size() == 1);
    REQUIRE(model.campaigns[0].id == "campaign_ashvale");
    REQUIRE(model.campaigns[0].name == "Ashvale");          // player-facing name, not id
    REQUIRE_FALSE(model.campaigns[0].description.empty());
    REQUIRE(model.campaigns[0].enabled);
    REQUIRE(model.campaigns[0].selected);
}

TEST_CASE("ShellMapper - scenario list shows only standalone entries with region context") {
    app::mappers::ShellModelMapper mapper;
    const auto model = mapper.MapScenarioSelect(SharedRepo(), kOpenGate, 0, "");

    REQUIRE(model.campaigns.size() == 1);
    REQUIRE(model.campaigns[0].id == "scenario_intro");
    REQUIRE(model.campaigns[0].name == "The Cleansing of Ashvale");
    REQUIRE(model.campaigns[0].description == "Starts in Ashvale Heartland.");
    REQUIRE(model.campaigns[0].enabled);
}

TEST_CASE("ShellMapper - a failed content gate disables every row with the reason") {
    app::mappers::ShellModelMapper mapper;
    const app::shell::ShellEntryStatus gate{false, "Installed content failed validation (2 error(s))."};

    const auto campaigns = mapper.MapCampaignSelect(SharedRepo(), gate, 0, "");
    REQUIRE_FALSE(campaigns.campaigns[0].enabled);
    REQUIRE(campaigns.campaigns[0].statusText == gate.reason);

    const auto scenarios = mapper.MapScenarioSelect(SharedRepo(), gate, 0, "");
    REQUIRE_FALSE(scenarios.campaigns[0].enabled);
    REQUIRE(scenarios.campaigns[0].statusText == gate.reason);
}

TEST_CASE("ShellController - Up/Down navigates shell lists like Left/Right") {
    app::CampaignController controller;

    app::input::InputState down{};
    down.targetNext = true;
    REQUIRE(controller.Update(down, 3, 0).selectedIndex == 1);

    app::input::InputState up{};
    up.targetPrev = true;
    REQUIRE(controller.Update(up, 3, 0).selectedIndex == 2);  // wraps

    app::input::InputState confirm{};
    confirm.confirm = true;
    const auto result = controller.Update(confirm, 3, 1);
    REQUIRE(result.confirmed);
    REQUIRE(result.selectedIndex == 1);
}

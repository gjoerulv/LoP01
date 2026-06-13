#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include "core/SaveGame.h"
#include "data/ContentRepository.h"
#include "gameplay/GameSession.h"
#include "gameplay/ResourceState.h"

// M31 standalone scenario start: GameSession::StartStandaloneScenario enters a
// scenario through the same transition chokepoint StartCampaign uses (scenario
// reset, playerStart economy/services, outcome selection, RegionMode) but with
// the session explicitly campaign-free. Selection legality (standaloneSelectable,
// validation) is the caller's job; the session only requires a valid id.

#ifndef LOP01_PROJECT_ROOT
#define LOP01_PROJECT_ROOT "."
#endif

namespace {

std::filesystem::path RealContentDir() {
    return std::filesystem::path(LOP01_PROJECT_ROOT) / "content";
}

gameplay::GameSession WireSession(const data::ContentRepository& content) {
    gameplay::GameSession session;
    session.SetPlayerColor("Green");
    session.SetUnitCatalog(content.Units());
    session.SetLeaderCapableUnitIds({"hero_player"});
    REQUIRE(session.AddOwnedUnit("hero_player", 1));
    REQUIRE(session.TryAddUnitToActiveParty("hero_player"));
    session.SetScenarioOutcomeDefinition(content.ScenarioOutcome());
    session.SetRegionCatalog(content.Regions());
    session.SetWorldMap(content.WorldMap());
    session.SetScenarioCatalog(content.Scenarios());
    session.SetCampaignCatalog(content.Campaigns());
    session.SetLocationServiceCatalog(content.LocationServices());
    session.SetTraderCurveCatalog(content.TraderCurves());
    session.SetEnemyGroupCatalog(content.EnemyGroups());
    return session;
}

bool OwnsService(const gameplay::GameSession& session, const std::string& serviceId) {
    const auto* owned = session.FindOwnedService(serviceId);
    return owned != nullptr && owned->ownerTeamColor == "Green";
}

} // namespace

TEST_CASE("StandaloneStart - starts the shipped intro scenario with playerStart applied") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));
    auto session = WireSession(repo);

    session.StartStandaloneScenario("scenario_intro");

    const auto snap = session.Snapshot();
    REQUIRE(snap.mode == gameplay::GameMode::RegionMode);
    REQUIRE(snap.regionId == "ashvale_heartland");
    REQUIRE(snap.destinationId == "home_base");  // region arrival node
    REQUIRE(snap.gold == 1200);                  // playerStart gold
    REQUIRE(session.ResourceCount(gameplay::ResourceType::Wood) == 50);
    REQUIRE(OwnsService(session, "home_base_trading_post"));
    REQUIRE(OwnsService(session, "home_base_storage"));
    REQUIRE(OwnsService(session, "river_depot_storage"));

    // Explicitly campaign-free.
    REQUIRE_FALSE(session.IsCampaignActive());
    REQUIRE(session.CampaignId().empty());
    REQUIRE(session.CurrentScenarioId() == "scenario_intro");
    REQUIRE_FALSE(session.IsScenarioEnded());
}

TEST_CASE("StandaloneStart - scenario with inline outcome and start node uses them") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));
    auto session = WireSession(repo);

    // scenario_second is not standaloneSelectable; the session method still
    // starts it (selection gating is the shell's job, by contract).
    session.StartStandaloneScenario("scenario_second");

    const auto snap = session.Snapshot();
    REQUIRE(snap.mode == gameplay::GameMode::RegionMode);
    REQUIRE(snap.regionId == "riverside_vale");
    REQUIRE(snap.destinationId == "vale_landing");  // authored startNodeId
    // Inline victory conditions replace the global outcome definition.
    REQUIRE(session.ActiveScenarioOutcomeDefinition().victoryConditions.size() == 1);
}

TEST_CASE("StandaloneStart - unknown scenario id is a strict no-op") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));
    auto session = WireSession(repo);
    const auto before = session.Snapshot();

    session.StartStandaloneScenario("scenario_missing");

    const auto after = session.Snapshot();
    REQUIRE(after.mode == before.mode);
    REQUIRE(after.regionId == before.regionId);
    REQUIRE(after.gold == before.gold);
    REQUIRE(session.CurrentScenarioId().empty());
}

TEST_CASE("StandaloneStart - clears a previous campaign run's identity") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));
    auto session = WireSession(repo);

    session.StartCampaign("campaign_ashvale");
    REQUIRE(session.IsCampaignActive());

    session.StartStandaloneScenario("scenario_intro");
    REQUIRE_FALSE(session.IsCampaignActive());
    REQUIRE(session.CampaignId().empty());
    REQUIRE(session.CompletedScenarioIds().empty());
    REQUIRE(session.CurrentScenarioId() == "scenario_intro");
}

TEST_CASE("StandaloneStart - state survives a save/load round-trip") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));
    auto session = WireSession(repo);
    session.StartStandaloneScenario("scenario_intro");

    auto restored = WireSession(repo);
    restored.ApplySaveData(session.ToSaveData());

    const auto snap = restored.Snapshot();
    REQUIRE(snap.mode == gameplay::GameMode::RegionMode);
    REQUIRE(snap.regionId == "ashvale_heartland");
    REQUIRE(snap.gold == 1200);
    REQUIRE_FALSE(restored.IsCampaignActive());
    REQUIRE(restored.CurrentScenarioId() == "scenario_intro");
    REQUIRE(OwnsService(restored, "river_depot_storage"));
}

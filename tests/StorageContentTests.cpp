#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include "core/SaveGame.h"
#include "data/ContentRepository.h"
#include "gameplay/GameSession.h"

// M28 storage against the SHIPPED content directory: the authored Home Base
// storage service is seeded player-owned via playerStart, validates without
// errors, and supports the store -> retrieve loop.

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

gameplay::GameSession WireCampaignSession(const data::ContentRepository& content) {
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
    return session;
}

} // namespace

TEST_CASE("Storage content: shipped content validates and seeds Home Base storage player-owned") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));
    REQUIRE_FALSE(HasErrorMessage(repo.ValidationMessages()));

    auto session = WireCampaignSession(repo);
    session.StartCampaign("campaign_ashvale");

    const auto* storage = session.FindOwnedService("home_base_storage");
    REQUIRE(storage != nullptr);
    REQUIRE(storage->ownerTeamColor == "Green");
    REQUIRE(storage->storedUnits.empty());
    REQUIRE(session.CanOpenStorageAtService("home_base_storage"));
}

TEST_CASE("Storage content: a reserve unit can be stored at Home Base and retrieved") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));
    auto session = WireCampaignSession(repo);
    session.StartCampaign("campaign_ashvale");

    // Recruit a generic into the reserve, then find its stack id.
    REQUIRE(session.AddOwnedUnit("unit_guard", 1));
    std::string guardStackId;
    for (const auto& stack : session.RosterStacks()) {
        if (stack.unitId == "unit_guard") {
            guardStackId = stack.stackId;
            break;
        }
    }
    REQUIRE_FALSE(guardStackId.empty());

    // Store it at the authored Home Base storage.
    REQUIRE(session.TryStoreStackAtService("home_base_storage", guardStackId));
    const auto* storage = session.FindOwnedService("home_base_storage");
    REQUIRE(storage->storedUnits.size() == 1);
    REQUIRE(storage->storedUnits[0].stackId == guardStackId);
    REQUIRE(session.OwnedUnitCount("unit_guard") == 1);  // still owned

    // Retrieve it back to reserve (same stack id).
    REQUIRE(session.TryRetrieveStackFromService("home_base_storage", guardStackId));
    REQUIRE(session.FindOwnedService("home_base_storage")->storedUnits.empty());
    REQUIRE(session.FindRosterStackById(guardStackId) != nullptr);
    REQUIRE(session.OwnedUnitCount("unit_guard") == 1);
}

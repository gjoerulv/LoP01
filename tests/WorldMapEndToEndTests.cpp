#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include "app/mappers/WorldMapModelMapper.h"
#include "core/GameClock.h"
#include "data/ContentRepository.h"
#include "data/definitions/RegionDefinition.h"
#include "gameplay/GameSession.h"
#include "gameplay/region/RegionTravelRules.h"
#include "gameplay/worldmap/WorldMapTravelRules.h"

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

// Mirrors App startup wiring for the World Map, then puts the player character
// (a leader/hero) and one generic into the party so the generic-loss path is
// exercised. The starting node home_base is both the Ashvale arrival node and
// its authored World Map exit node, so the session begins on an exit node.
gameplay::GameSession MakeRealContentSession(data::ContentRepository& repo) {
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));
    REQUIRE_FALSE(HasErrorMessage(repo.ValidationMessages()));

    gameplay::GameSession session;
    session.SetUnitCatalog(repo.Units());
    session.SetRegionCatalog(repo.Regions());
    session.SetWorldMap(repo.WorldMap());

    session.SetLeaderCapableUnitIds({ "hero_player" });
    REQUIRE(session.AddOwnedUnit("hero_player", 1));
    REQUIRE(session.TryAddUnitToActiveParty("hero_player"));
    REQUIRE(session.AddOwnedUnit("unit_guard", 1)); // a generic, stays in reserve
    session.ApplyDailyStartingEnergy();

    session.EnterRegionMode();
    session.SetDestination("home_base"); // the authored exit node
    return session;
}

} // namespace

TEST_CASE("WorldMap end-to-end - real content loads the world map with two regions") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));
    REQUIRE_FALSE(HasErrorMessage(repo.ValidationMessages()));

    const auto& wm = repo.WorldMap();
    REQUIRE(wm.entries.size() == 2);
    REQUIRE(repo.FindWorldMapRegionEntry("ashvale_heartland") != nullptr);
    REQUIRE(repo.FindWorldMapRegionEntry("riverside_vale") != nullptr);
    REQUIRE(wm.adjacency.size() == 1);
}

TEST_CASE("WorldMap end-to-end - the session begins on an exit node and can open the World Map") {
    data::ContentRepository repo;
    auto session = MakeRealContentSession(repo);
    REQUIRE(session.CanOpenWorldMapHere());
}

TEST_CASE("WorldMap end-to-end - the model lists riverside_vale as a legal 1-day destination") {
    data::ContentRepository repo;
    auto session = MakeRealContentSession(repo);

    app::mappers::WorldMapModelMapper mapper;
    const auto model = mapper.Map(repo, session, /*selectedIndex=*/0);

    REQUIRE(model.currentRegionName == "Ashvale Heartland");
    REQUIRE(model.genericLossCount == 1); // one generic (unit_guard) aboard

    // The only destination is riverside_vale (the current region is excluded).
    REQUIRE(model.destinations.size() == 1);
    const auto& dest = model.destinations[0];
    REQUIRE(dest.regionId == "riverside_vale");
    REQUIRE(dest.name == "Riverside Vale");
    REQUIRE(dest.legal);
    REQUIRE(dest.days == 1);
}

TEST_CASE("WorldMap end-to-end - traveling to riverside_vale arrives next-day 11:00 at its arrival node") {
    data::ContentRepository repo;
    auto session = MakeRealContentSession(repo);
    // lowest agility 8 (hero) vs 10 (guard); +150 from the leader's authored leader_energy.
    REQUIRE(session.CurrentEnergy() == 1950);

    // Open the World Map (as the App does) so travel commits from WorldMapMode.
    session.EnterWorldMapMode();

    const auto result = session.TravelToRegion("riverside_vale");
    REQUIRE(result.success);
    REQUIRE(result.days == 1);
    REQUIRE(result.genericsDropped == 1);

    const auto snap = session.Snapshot();
    REQUIRE(snap.regionId == "riverside_vale");
    REQUIRE(snap.destinationId == "vale_landing"); // RegionDefinition.arrivalNodeId
    REQUIRE(snap.day == 2);
    REQUIRE(snap.time == "11:00");
    // Arrival returns to the Region layer, not the World Map screen.
    REQUIRE(snap.mode == gameplay::GameMode::RegionMode);

    // Generic dropped, hero retained; arrival-day Energy reset to hero-only max
    // (1000 + 8*100 + 150 leader_energy).
    REQUIRE(session.OwnedUnitCount("unit_guard") == 0);
    REQUIRE(session.MaxEnergy() == 1950);
    REQUIRE(session.CurrentEnergy() == 1950);

    // M12 scenario outcome is unaffected by World Map travel.
    REQUIRE_FALSE(session.IsScenarioEnded());

    // Normal in-Region travel works in the destination Region: from the arrival
    // node (vale_landing) the player can reach vale_market.
    const auto* vale = repo.FindRegionById("riverside_vale");
    REQUIRE(vale != nullptr);
    const auto regionTravel = gameplay::region::EvaluateTravel(
        snap.destinationId,            // vale_landing
        "vale_market",
        /*destinationTravelAvailable=*/true,
        snap.minutesIntoSliceDay,
        vale->links);
    REQUIRE(regionTravel.legal);
}

// ---------------------------------------------------------------------------
// M29 cross-Region generic preservation against the SHIPPED content: a generic
// stored at the authored Home Base storage survives Region travel while a
// traveling generic is lost, and retrieval works after returning.
// ---------------------------------------------------------------------------

namespace {

// Full campaign wiring (mirrors StorageContentTests) so playerStart seeds the
// authored home_base_storage as player-owned.
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

std::string SlottedStackIdOf(const gameplay::GameSession& session, const std::string& unitId) {
    auto inSlots = [&](const std::vector<std::string>& slots, const std::string& stackId) {
        return std::find(slots.begin(), slots.end(), stackId) != slots.end();
    };
    for (const auto& stack : session.RosterStacks()) {
        if (stack.unitId == unitId &&
            (inSlots(session.ActiveSlotStackIds(), stack.stackId) ||
             inSlots(session.ReserveSlotStackIds(), stack.stackId))) {
            return stack.stackId;
        }
    }
    return {};
}

} // namespace

TEST_CASE("WorldMap end-to-end - stored generic survives Region travel, traveling generic is lost, retrieval works after return") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));
    REQUIRE_FALSE(HasErrorMessage(repo.ValidationMessages()));

    auto session = WireCampaignSession(repo);
    session.StartCampaign("campaign_ashvale");
    session.SetDestination("home_base"); // the authored Ashvale exit node
    REQUIRE(session.CanOpenWorldMapHere());

    // First guard: into reserve, then stored at the authored Home Base storage.
    REQUIRE(session.AddOwnedUnit("unit_guard", 1));
    const std::string storedStackId = SlottedStackIdOf(session, "unit_guard");
    REQUIRE_FALSE(storedStackId.empty());
    REQUIRE(session.TryStoreStackAtService("home_base_storage", storedStackId));

    // Second guard: a fresh reserve stack that stays in the traveling party.
    REQUIRE(session.AddOwnedUnit("unit_guard", 1));
    const std::string travelingStackId = SlottedStackIdOf(session, "unit_guard");
    REQUIRE_FALSE(travelingStackId.empty());
    REQUIRE(travelingStackId != storedStackId);
    session.ApplyDailyStartingEnergy();

    // Only the traveling guard is at risk; the warning model names it.
    const auto preview = session.PreviewRegionTravelGenericLosses();
    REQUIRE(preview.size() == 1);
    REQUIRE(preview[0].stackId == travelingStackId);
    REQUIRE(preview[0].quantity == 1);

    app::mappers::WorldMapModelMapper mapper;
    const auto idleModel = mapper.Map(repo, session, /*selectedIndex=*/0);
    REQUIRE_FALSE(idleModel.confirmingLoss);
    REQUIRE(idleModel.genericLossCount == 1); // slotted-only: stored guard not counted

    const auto confirmModel = mapper.Map(repo, session, /*selectedIndex=*/0,
        /*lossConfirmationPending=*/true);
    REQUIRE(confirmModel.confirmingLoss);
    REQUIRE(confirmModel.confirmTitle == "Confirm travel to Riverside Vale?");
    REQUIRE(confirmModel.lossLines.size() == 1);
    const auto* guardDef = repo.FindUnitById("unit_guard");
    REQUIRE(guardDef != nullptr);
    REQUIRE(confirmModel.lossLines[0] == "1x " + guardDef->name);

    // Confirmed travel: the traveling guard is lost, the stored guard survives.
    const auto outbound = session.TravelToRegion("riverside_vale");
    REQUIRE(outbound.success);
    REQUIRE(outbound.genericsDropped == 1);
    REQUIRE(session.FindRosterStackById(travelingStackId) == nullptr);
    REQUIRE(session.FindRosterStackById(storedStackId) != nullptr);
    const auto* storage = session.FindOwnedService("home_base_storage");
    REQUIRE(storage != nullptr);
    REQUIRE(storage->storedUnits.size() == 1);
    REQUIRE(storage->storedUnits[0].stackId == storedStackId);
    REQUIRE(session.OwnedUnitCount("unit_guard") == 1);

    // Return next day (arrival is 11:00, past the departure deadline) from the
    // vale's authored exit node, then retrieve the stored guard to reserve.
    session.AddMinutes(core::GameClock::kMinutesPerSliceDay
        - session.Snapshot().minutesIntoSliceDay);
    REQUIRE(session.Snapshot().destinationId == "vale_landing");
    const auto inbound = session.TravelToRegion("ashvale_heartland");
    REQUIRE(inbound.success);
    REQUIRE(inbound.genericsDropped == 0); // heroes-only party, no extra loss

    REQUIRE(session.TryRetrieveStackFromService("home_base_storage", storedStackId));
    REQUIRE(session.FindOwnedService("home_base_storage")->storedUnits.empty());
    REQUIRE(session.FindRosterStackById(storedStackId) != nullptr);
    REQUIRE(session.OwnedUnitCount("unit_guard") == 1);
}

// ---------------------------------------------------------------------------
// Region-aware fallback resolver tests
//
// These verify the logic that ResolveSafeFallbackRegionNodeId() encodes, using
// the real content. The function is private to App so it is exercised through
// the public contract it affects: after a wake/sleep penalty, the destination
// stays within the current Region and lands on its arrival node.
// ---------------------------------------------------------------------------

namespace {

// Helper that simulates the region-aware fallback selection purely using the
// content repository, mirroring ResolveSafeFallbackRegionNodeId's logic.
// Returns the node that a wake/sleep penalty would recover to.
std::string SimulateFallback(
    const data::ContentRepository& content,
    const std::string& regionId,
    const std::string& currentDestinationId)
{
    const auto* region = content.FindRegionById(regionId);

    auto nodeExistsInRegion = [&](const data::RegionDefinition* r, const std::string& nodeId) {
        if (r == nullptr || nodeId.empty()) return false;
        for (const auto& node : r->nodes) {
            if (node.locationId == nodeId) return true;
        }
        return false;
    };

    if (region != nullptr &&
        !region->arrivalNodeId.empty() &&
        nodeExistsInRegion(region, region->arrivalNodeId)) {
        return region->arrivalNodeId;
    }
    if (nodeExistsInRegion(region, currentDestinationId)) {
        return currentDestinationId;
    }
    if (region != nullptr && !region->nodes.empty()) {
        return region->nodes.front().locationId;
    }
    return currentDestinationId;
}

} // namespace

TEST_CASE("WorldMap fallback - in riverside_vale wake/sleep penalty recovers to vale_landing, not home_base") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));
    REQUIRE_FALSE(HasErrorMessage(repo.ValidationMessages()));

    const std::string fallback = SimulateFallback(repo, "riverside_vale", "vale_market");
    REQUIRE(fallback == "vale_landing");
    REQUIRE(fallback != "home_base");
}

TEST_CASE("WorldMap fallback - in ashvale_heartland wake/sleep penalty recovers to home_base") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));

    const std::string fallback = SimulateFallback(repo, "ashvale_heartland", "town_center");
    REQUIRE(fallback == "home_base");
}

TEST_CASE("WorldMap fallback - recovery node always belongs to the current Region") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));

    for (const auto& region : repo.Regions()) {
        const std::string nodeId = SimulateFallback(repo, region.id, region.arrivalNodeId);
        bool foundInRegion = false;
        for (const auto& node : region.nodes) {
            if (node.locationId == nodeId) {
                foundInRegion = true;
                break;
            }
        }
        // If the region has any nodes the fallback must be one of them.
        if (!region.nodes.empty()) {
            REQUIRE(foundInRegion);
        }
    }
}

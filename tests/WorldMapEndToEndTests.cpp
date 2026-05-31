#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include "app/mappers/WorldMapModelMapper.h"
#include "data/ContentRepository.h"
#include "gameplay/GameSession.h"
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
    REQUIRE(session.CurrentEnergy() == 1800); // lowest agility 8 (hero) vs 10 (guard)

    const auto result = session.TravelToRegion("riverside_vale");
    REQUIRE(result.success);
    REQUIRE(result.days == 1);
    REQUIRE(result.genericsDropped == 1);

    const auto snap = session.Snapshot();
    REQUIRE(snap.regionId == "riverside_vale");
    REQUIRE(snap.destinationId == "vale_landing"); // RegionDefinition.arrivalNodeId
    REQUIRE(snap.day == 2);
    REQUIRE(snap.time == "11:00");

    // Generic dropped, hero retained; arrival-day Energy reset to hero-only max.
    REQUIRE(session.OwnedUnitCount("unit_guard") == 0);
    REQUIRE(session.MaxEnergy() == 1800);
    REQUIRE(session.CurrentEnergy() == 1800);

    // M12 scenario outcome is unaffected by World Map travel.
    REQUIRE_FALSE(session.IsScenarioEnded());
}

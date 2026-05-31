#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <string>
#include <vector>

#include "core/GameClock.h"
#include "core/SaveGame.h"
#include "data/definitions/RegionDefinition.h"
#include "data/definitions/UnitDefinition.h"
#include "data/definitions/WorldMapDefinition.h"
#include "gameplay/GameSession.h"
#include "gameplay/worldmap/WorldMapTravelRules.h"

using namespace gameplay;
using worldmap::WorldMapTravelBlockReason;

namespace {

data::RegionNodeDefinition Node(const std::string& id) {
    data::RegionNodeDefinition n;
    n.locationId = id;
    return n;
}

data::UnitDefinition Unit(const std::string& id, data::UnitDefinitionCategory cat, int agility) {
    data::UnitDefinition def;
    def.id = id;
    def.name = id;
    def.category = cat;
    def.stats.agility = agility;
    return def;
}

// Session positioned in RegionMode on the exit node of "ashvale_heartland"
// (the constructor's default region), with "riverside_vale" adjacent + unlocked,
// a hero in the active party, and a generic grunt in reserve.
GameSession MakeTravelReadySession(bool destinationUnlocked = true) {
    GameSession session;

    data::RegionDefinition home;
    home.id = "ashvale_heartland";
    home.arrivalNodeId = "home_arrival";
    home.nodes = { Node("home_arrival"), Node("home_exit") };

    data::RegionDefinition vale;
    vale.id = "riverside_vale";
    vale.arrivalNodeId = "vale_arrival";
    vale.nodes = { Node("vale_arrival") };

    session.SetRegionCatalog({ home, vale });

    data::WorldMapDefinition wm;
    wm.id = "world_map";
    wm.entries = {
        { "ashvale_heartland", true, { "home_exit" }, 0.0f, 0.0f },
        { "riverside_vale", destinationUnlocked, {}, 0.0f, 0.0f },
    };
    wm.adjacency = { { "ashvale_heartland", "riverside_vale" } };
    session.SetWorldMap(wm);

    // Roster: hero (active, agility 8) + grunt (reserve, agility 3).
    session.SetLeaderCapableUnitIds({ "hero_a" });
    session.SetUnitCatalog({
        Unit("hero_a", data::UnitDefinitionCategory::Hero, 8),
        Unit("grunt", data::UnitDefinitionCategory::Generic, 3),
    });
    REQUIRE(session.AddOwnedUnit("hero_a", 1));
    REQUIRE(session.TryAddUnitToActiveParty("hero_a"));
    REQUIRE(session.AddOwnedUnit("grunt", 1)); // stays in reserve
    session.ApplyDailyStartingEnergy();         // lowest agility 3 -> 1300

    // Position on the Region layer at the exit node.
    session.EnterRegionMode();
    session.SetDestination("home_exit");
    return session;
}

bool ActivePartyHas(const GameSession& session, const std::string& unitId) {
    const auto& ids = session.ActivePartyUnitIds();
    return std::find(ids.begin(), ids.end(), unitId) != ids.end();
}

} // namespace

TEST_CASE("WorldMapTravel - legal trip switches region, advances to next-day 11:00, drops generics") {
    auto session = MakeTravelReadySession();
    REQUIRE(session.CanOpenWorldMapHere());
    REQUIRE(session.CurrentEnergy() == 1300); // lowest agility 3 before drop

    const auto result = session.TravelToRegion("riverside_vale");
    REQUIRE(result.success);
    REQUIRE(result.reason == WorldMapTravelBlockReason::None);
    REQUIRE(result.days == 1);
    REQUIRE(result.genericsDropped == 1);

    const auto snap = session.Snapshot();
    REQUIRE(snap.regionId == "riverside_vale");
    REQUIRE(snap.destinationId == "vale_arrival"); // destination arrival node
    REQUIRE(snap.day == 2);
    REQUIRE(snap.time == "11:00");

    // Generic dropped; hero retained.
    REQUIRE(ActivePartyHas(session, "hero_a"));
    REQUIRE(session.OwnedUnitCount("grunt") == 0);

    // Arrival-day Energy reset uses the post-drop (hero-only, agility 8) party.
    REQUIRE(session.MaxEnergy() == 1800);
    REQUIRE(session.CurrentEnergy() == 1800);
}

TEST_CASE("WorldMapTravel - not standing on an exit node yields NotAtExitNode and no mutation") {
    auto session = MakeTravelReadySession();
    session.SetDestination("home_arrival"); // not an exit node

    REQUIRE_FALSE(session.CanOpenWorldMapHere());
    const auto result = session.TravelToRegion("riverside_vale");
    REQUIRE_FALSE(result.success);
    REQUIRE(result.reason == WorldMapTravelBlockReason::NotAtExitNode);
    REQUIRE(session.Snapshot().regionId == "ashvale_heartland");
    REQUIRE(session.OwnedUnitCount("grunt") == 1); // unchanged
}

TEST_CASE("WorldMapTravel - locked destination is rejected with no mutation") {
    auto session = MakeTravelReadySession(/*destinationUnlocked=*/false);

    const auto result = session.TravelToRegion("riverside_vale");
    REQUIRE_FALSE(result.success);
    REQUIRE(result.reason == WorldMapTravelBlockReason::DestinationLocked);
    REQUIRE(session.Snapshot().regionId == "ashvale_heartland");
    REQUIRE(session.CurrentEnergy() == 1300); // not spent
}

TEST_CASE("WorldMapTravel - at/after 11:00 is past the departure deadline") {
    auto session = MakeTravelReadySession();
    session.AddMinutes((11 - core::GameClock::kDayStartHour) * 60); // advance to exactly 11:00
    REQUIRE(session.Snapshot().time == "11:00");

    const auto result = session.TravelToRegion("riverside_vale");
    REQUIRE_FALSE(result.success);
    REQUIRE(result.reason == WorldMapTravelBlockReason::PastDepartureDeadline);
    REQUIRE(session.Snapshot().regionId == "ashvale_heartland");
}

TEST_CASE("WorldMapTravel - insufficient energy is rejected with no mutation") {
    auto session = MakeTravelReadySession();
    REQUIRE(session.TrySpendEnergy(400)); // 1300 -> 900, below the 1000 cost

    const auto result = session.TravelToRegion("riverside_vale");
    REQUIRE_FALSE(result.success);
    REQUIRE(result.reason == WorldMapTravelBlockReason::InsufficientEnergy);
    REQUIRE(session.Snapshot().regionId == "ashvale_heartland");
    REQUIRE(session.CurrentEnergy() == 900); // unchanged by the failed trip
}

TEST_CASE("WorldMapTravel - succeeds from WorldMapMode when on an exit node") {
    // Regression: opening the World Map switches mode to WorldMapMode. Confirming
    // travel must still succeed (the exit-node gate is mode-flexible: RegionMode
    // OR WorldMapMode), not fail with NotAtExitNode.
    auto session = MakeTravelReadySession();
    session.EnterWorldMapMode();          // still standing on home_exit
    REQUIRE_FALSE(session.CanOpenWorldMapHere()); // opening gate is RegionMode-only

    const auto result = session.TravelToRegion("riverside_vale");
    REQUIRE(result.success);
    REQUIRE(result.days == 1);
    REQUIRE(session.Snapshot().regionId == "riverside_vale");
    REQUIRE(session.Snapshot().destinationId == "vale_arrival");
    // Arrival drops back onto the Region layer, not the World Map screen.
    REQUIRE(session.Snapshot().mode == gameplay::GameMode::RegionMode);
}

TEST_CASE("WorldMapTravel - WorldMapMode but not on an exit node fails with no mutation") {
    auto session = MakeTravelReadySession();
    session.SetDestination("home_arrival"); // not an exit node
    session.EnterWorldMapMode();

    const auto result = session.TravelToRegion("riverside_vale");
    REQUIRE_FALSE(result.success);
    REQUIRE(result.reason == WorldMapTravelBlockReason::NotAtExitNode);
    REQUIRE(session.Snapshot().regionId == "ashvale_heartland");
    REQUIRE(session.CurrentEnergy() == 1300);   // not spent
    REQUIRE(session.OwnedUnitCount("grunt") == 1); // generics not dropped
}

TEST_CASE("WorldMapTravel - travel is illegal from Location or Battle mode") {
    auto session = MakeTravelReadySession(); // on home_exit
    session.EnterBattleMode();
    REQUIRE_FALSE(session.TravelToRegion("riverside_vale").success);
    REQUIRE(session.TravelToRegion("riverside_vale").reason
        == WorldMapTravelBlockReason::NotAtExitNode);
    REQUIRE(session.Snapshot().regionId == "ashvale_heartland");
}

TEST_CASE("WorldMapTravel - same region is AlreadyHere") {
    auto session = MakeTravelReadySession();
    const auto result = session.TravelToRegion("ashvale_heartland");
    REQUIRE_FALSE(result.success);
    REQUIRE(result.reason == WorldMapTravelBlockReason::AlreadyHere);
}

TEST_CASE("WorldMapTravel - unlocked-region set round-trips through save/load") {
    // Unlock a third region at runtime so the saved set differs from authored.
    auto session = MakeTravelReadySession();
    REQUIRE(session.IsRegionUnlocked("ashvale_heartland"));
    REQUIRE(session.IsRegionUnlocked("riverside_vale"));

    const auto saveData = session.ToSaveData();
    REQUIRE(saveData.unlockedRegionIds.size() == 2);

    // Restore into a fresh session that was seeded with the same world map but a
    // DIFFERENT authored unlock state (only the start region unlocked). The saved
    // non-empty set must override the seed.
    GameSession restored;
    data::WorldMapDefinition wm;
    wm.id = "world_map";
    wm.entries = {
        { "ashvale_heartland", true, {}, 0.0f, 0.0f },
        { "riverside_vale", false, {}, 0.0f, 0.0f }, // locked in authored state
    };
    restored.SetWorldMap(wm);
    REQUIRE_FALSE(restored.IsRegionUnlocked("riverside_vale")); // seeded locked

    restored.ApplySaveData(saveData);
    REQUIRE(restored.IsRegionUnlocked("ashvale_heartland"));
    REQUIRE(restored.IsRegionUnlocked("riverside_vale")); // saved set restored
}

TEST_CASE("WorldMapTravel - legacy save without unlocked set keeps the authored seed") {
    auto source = MakeTravelReadySession();
    core::SaveData saveData = source.ToSaveData();
    saveData.unlockedRegionIds.clear(); // simulate a pre-M15 save

    GameSession restored;
    data::WorldMapDefinition wm;
    wm.id = "world_map";
    wm.entries = {
        { "ashvale_heartland", true, {}, 0.0f, 0.0f },
        { "riverside_vale", true, {}, 0.0f, 0.0f },
    };
    restored.SetWorldMap(wm); // authored seed: both unlocked
    restored.ApplySaveData(saveData);

    // Empty saved set -> authored seed preserved (not wiped).
    REQUIRE(restored.IsRegionUnlocked("ashvale_heartland"));
    REQUIRE(restored.IsRegionUnlocked("riverside_vale"));
}

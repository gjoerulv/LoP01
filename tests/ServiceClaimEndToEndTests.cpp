#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

#include "core/GameClock.h"
#include "core/SaveGame.h"
#include "data/definitions/LocationServiceDefinition.h"
#include "gameplay/EnemyTeamState.h"
#include "gameplay/GameSession.h"
#include "gameplay/ResourceState.h"

// GameSession-level proof of M23 service claiming: defeating the hostile team
// guarding a node claims the eligible ownable services there, and the existing
// payout / trader-tier consumers then reflect the new ownership. Builds catalogs
// and runtime state directly (no content loader) to keep each case focused.

using data::LocationServiceKind;
using gameplay::ResourceType;

namespace {

constexpr int kOneDay = core::GameClock::kMinutesPerSliceDay;

data::LocationServiceDefinition MakeMine(const std::string& id, const std::string& locationId) {
    data::LocationServiceDefinition d;
    d.id = id;
    d.locationId = locationId;
    d.zoneId = "z";
    d.kind = LocationServiceKind::Mine;
    d.mineOutputs = {{"Stone", 2}, {"Gold", 1000}};
    return d;
}

data::LocationServiceDefinition MakeTradingPost(const std::string& id, const std::string& locationId) {
    data::LocationServiceDefinition d;
    d.id = id;
    d.locationId = locationId;
    d.zoneId = "z";
    d.kind = LocationServiceKind::TradingPost;
    return d;
}

gameplay::EnemyTeamState MakeTeam(const std::string& color, const std::string& nodeId,
    bool active = true, std::vector<std::string> alliances = {}) {
    gameplay::EnemyTeamState t;
    t.teamColor = color;
    t.nodeId = nodeId;
    t.active = active;
    t.alliances = std::move(alliances);
    return t;
}

// Baseline runtime save: a two-stack roster (so stationed refs can be valid),
// some gold, day 1, in Region mode. Caller fills ownedServices/destination.
core::SaveData BaseSave() {
    core::SaveData s;
    s.schemaVersion = 5;
    s.day = 1;
    s.minutesIntoSliceDay = 0;
    s.gold = 1000;
    s.mode = "region_mode";
    s.regionId = "r1";
    s.destinationId = "mine_node";
    s.hasCanonicalRoster = true;
    s.rosterStacks = {
        core::RosterStackSaveState{"stk_1", "hero", 1},
        core::RosterStackSaveState{"stk_2", "kobold", 1}
    };
    s.activeSlotStackIds = {"stk_1", "", "", "", ""};
    s.reserveSlotStackIds = {"stk_2", "", "", "", "", "", "", ""};
    s.nextStackIdCounter = 3;
    return s;
}

gameplay::GameSession MakeSession(const std::vector<data::LocationServiceDefinition>& catalog) {
    gameplay::GameSession session;
    session.SetPlayerColor("Green");
    session.SetLocationServiceCatalog(catalog);
    return session;
}

} // namespace

TEST_CASE("ServiceClaim e2e: unowned guarded mine becomes player-owned after defeat") {
    auto session = MakeSession({MakeMine("mine_svc", "mine_node")});
    session.ApplySaveData(BaseSave());
    session.SetEnemyTeams({MakeTeam("Red", "mine_node")});

    session.ClearEnemyTeamByColor("Red");
    const auto claimed = session.ClaimContestedServicesAtNode("mine_node");

    REQUIRE(claimed.size() == 1);
    REQUIRE(claimed.front() == "mine_svc");
    const auto* owned = session.FindOwnedService("mine_svc");
    REQUIRE(owned != nullptr);
    REQUIRE(owned->ownerTeamColor == "Green");
}

TEST_CASE("ServiceClaim e2e: enemy-owned guarded Trading Post becomes player-owned with tier") {
    auto session = MakeSession({MakeTradingPost("tp_svc", "tp_node")});
    auto save = BaseSave();
    save.destinationId = "tp_node";
    save.ownedServices = {
        core::OwnedServiceSaveState{"tp_svc", "Red", false, false, {}}
    };
    session.ApplySaveData(save);
    session.SetEnemyTeams({MakeTeam("Red", "tp_node")});

    session.ClearEnemyTeamByColor("Red");
    const auto claimed = session.ClaimContestedServicesAtNode("tp_node");

    REQUIRE(claimed.size() == 1);
    const auto* owned = session.FindOwnedService("tp_svc");
    REQUIRE(owned != nullptr);
    REQUIRE(owned->ownerTeamColor == "Green");
    // The claim feeds the existing trader-tier consumer unchanged.
    REQUIRE(session.OwnedTraderServiceTierForService("tp_svc") == 1);
}

TEST_CASE("ServiceClaim e2e: already player-owned service is unchanged and keeps stationed units") {
    auto session = MakeSession({MakeMine("mine_svc", "mine_node")});
    auto save = BaseSave();
    save.ownedServices = {
        core::OwnedServiceSaveState{"mine_svc", "Green", false, false,
            {core::StationedUnitSaveState{"kobold", "stk_2"}}}
    };
    session.ApplySaveData(save);
    session.SetEnemyTeams({MakeTeam("Red", "mine_node")});

    session.ClearEnemyTeamByColor("Red");
    const auto claimed = session.ClaimContestedServicesAtNode("mine_node");

    REQUIRE(claimed.empty());
    const auto* owned = session.FindOwnedService("mine_svc");
    REQUIRE(owned != nullptr);
    REQUIRE(owned->ownerTeamColor == "Green");
    REQUIRE(owned->stationedUnits.size() == 1);   // preserved (not touched)
}

TEST_CASE("ServiceClaim e2e: allied-owned service is not claimed") {
    auto session = MakeSession({MakeMine("mine_svc", "mine_node")});
    auto save = BaseSave();
    save.ownedServices = {
        core::OwnedServiceSaveState{"mine_svc", "Blue", false, false, {}}
    };
    session.ApplySaveData(save);
    // Red guards the mine node; Blue is an active team allied to the player.
    session.SetEnemyTeams({
        MakeTeam("Red", "mine_node"),
        MakeTeam("Blue", "other_node", true, {"Green"})
    });

    session.ClearEnemyTeamByColor("Red");
    const auto claimed = session.ClaimContestedServicesAtNode("mine_node");

    REQUIRE(claimed.empty());
    const auto* owned = session.FindOwnedService("mine_svc");
    REQUIRE(owned != nullptr);
    REQUIRE(owned->ownerTeamColor == "Blue");   // unchanged
}

TEST_CASE("ServiceClaim e2e: locked and destroyed services are not claimed") {
    auto session = MakeSession({MakeMine("locked_svc", "locked_node"),
                                MakeMine("destroyed_svc", "destroyed_node")});
    auto save = BaseSave();
    save.ownedServices = {
        core::OwnedServiceSaveState{"locked_svc", "", true, false, {}},
        core::OwnedServiceSaveState{"destroyed_svc", "", false, true, {}}
    };
    session.ApplySaveData(save);
    session.SetEnemyTeams({MakeTeam("Red", "locked_node"), MakeTeam("Blue", "destroyed_node")});

    session.ClearEnemyTeamByColor("Red");
    session.ClearEnemyTeamByColor("Blue");

    REQUIRE(session.ClaimContestedServicesAtNode("locked_node").empty());
    REQUIRE(session.ClaimContestedServicesAtNode("destroyed_node").empty());
    REQUIRE(session.FindOwnedService("locked_svc")->ownerTeamColor.empty());
    REQUIRE(session.FindOwnedService("destroyed_svc")->ownerTeamColor.empty());
}

TEST_CASE("ServiceClaim e2e: stationed units are cleared on ownership transfer") {
    auto session = MakeSession({MakeMine("mine_svc", "mine_node")});
    auto save = BaseSave();
    save.ownedServices = {
        core::OwnedServiceSaveState{"mine_svc", "Red", false, false,
            {core::StationedUnitSaveState{"kobold", "stk_2"}}}
    };
    session.ApplySaveData(save);
    session.SetEnemyTeams({MakeTeam("Red", "mine_node")});

    session.ClearEnemyTeamByColor("Red");
    const auto claimed = session.ClaimContestedServicesAtNode("mine_node");

    REQUIRE(claimed.size() == 1);
    const auto* owned = session.FindOwnedService("mine_svc");
    REQUIRE(owned != nullptr);
    REQUIRE(owned->ownerTeamColor == "Green");
    REQUIRE(owned->stationedUnits.empty());
}

TEST_CASE("ServiceClaim e2e: a still-contested node claims nothing") {
    auto session = MakeSession({MakeMine("mine_svc", "mine_node")});
    session.ApplySaveData(BaseSave());
    session.SetEnemyTeams({MakeTeam("Red", "mine_node"), MakeTeam("Purple", "mine_node")});

    session.ClearEnemyTeamByColor("Red");   // one guard down, Purple still holds the node
    const auto claimed = session.ClaimContestedServicesAtNode("mine_node");

    REQUIRE(claimed.empty());
    REQUIRE(session.FindOwnedService("mine_svc") == nullptr);
}

TEST_CASE("ServiceClaim e2e: a claimed mine pays the player on the next daily payout") {
    auto session = MakeSession({MakeMine("mine_svc", "mine_node")});
    session.ApplySaveData(BaseSave());
    session.SetEnemyTeams({MakeTeam("Red", "mine_node")});

    session.ClearEnemyTeamByColor("Red");
    REQUIRE(session.ClaimContestedServicesAtNode("mine_node").size() == 1);

    const int goldBefore = session.Snapshot().gold;
    REQUIRE(session.ResourceCount(ResourceType::Stone) == 0);

    session.AddMinutes(kOneDay);

    REQUIRE(session.ResourceCount(ResourceType::Stone) == 2);
    REQUIRE(session.Snapshot().gold == goldBefore + 1000);
}

TEST_CASE("ServiceClaim e2e: claimed ownership persists through save/load") {
    const std::vector<data::LocationServiceDefinition> catalog = {MakeMine("mine_svc", "mine_node")};
    auto session = MakeSession(catalog);
    session.ApplySaveData(BaseSave());
    session.SetEnemyTeams({MakeTeam("Red", "mine_node")});
    session.ClearEnemyTeamByColor("Red");
    REQUIRE(session.ClaimContestedServicesAtNode("mine_node").size() == 1);

    const auto saved = session.ToSaveData();

    gameplay::GameSession restored = MakeSession(catalog);
    restored.ApplySaveData(saved);

    const auto* owned = restored.FindOwnedService("mine_svc");
    REQUIRE(owned != nullptr);
    REQUIRE(owned->ownerTeamColor == "Green");
}

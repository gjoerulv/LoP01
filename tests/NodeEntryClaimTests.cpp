#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

#include "core/SaveGame.h"
#include "data/definitions/LocationServiceDefinition.h"
#include "gameplay/EnemyTeamState.h"
#include "gameplay/GameSession.h"

// M26 GameSession-level proof of GENERAL player-side claiming: legally entering a
// node with claimable, unguarded ownable services claims them immediately
// (ResolveNodeEntryClaims), while hostile-occupied nodes claim nothing until the
// guard is cleared. Builds catalogs/runtime state directly (no content loader).

using data::LocationServiceKind;

namespace {

data::LocationServiceDefinition MakeService(const std::string& id,
    const std::string& locationId, LocationServiceKind kind) {
    data::LocationServiceDefinition d;
    d.id = id;
    d.locationId = locationId;
    d.zoneId = "z";
    d.kind = kind;
    if (kind == LocationServiceKind::Mine) {
        d.mineOutputs = {{"Stone", 2}, {"Gold", 1000}};
    }
    return d;
}

data::LocationServiceDefinition MakeMine(const std::string& id, const std::string& locationId) {
    return MakeService(id, locationId, LocationServiceKind::Mine);
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

TEST_CASE("NodeEntry claim: peaceful entry claims an unowned unguarded ownable service") {
    auto session = MakeSession({MakeMine("mine_svc", "mine_node")});
    session.ApplySaveData(BaseSave());
    // No hostile team at the node: legal peaceful entry.

    const auto claimed = session.ResolveNodeEntryClaims("mine_node");

    REQUIRE(claimed.size() == 1);
    REQUIRE(claimed.front() == "mine_svc");
    const auto* owned = session.FindOwnedService("mine_svc");
    REQUIRE(owned != nullptr);
    REQUIRE(owned->ownerTeamColor == "Green");
}

TEST_CASE("NodeEntry claim: a hostile-occupied node claims nothing on entry (pre-battle)") {
    auto session = MakeSession({MakeMine("mine_svc", "mine_node")});
    session.ApplySaveData(BaseSave());
    session.SetEnemyTeams({MakeTeam("Red", "mine_node")});  // guard NOT cleared

    const auto claimed = session.ResolveNodeEntryClaims("mine_node");

    REQUIRE(claimed.empty());
    REQUIRE(session.FindOwnedService("mine_svc") == nullptr);  // nothing claimed pre-battle
}

TEST_CASE("NodeEntry claim: re-entering an owned node preserves the player's stationed units") {
    auto session = MakeSession({MakeMine("mine_svc", "mine_node")});
    auto save = BaseSave();
    save.ownedServices = {
        core::OwnedServiceSaveState{"mine_svc", "Green", false, false,
            {core::StationedUnitSaveState{"kobold", "stk_2"}}}
    };
    session.ApplySaveData(save);

    // Re-entry twice: the player already owns the service, so it is skipped and the
    // stationed units must never be cleared (M25 invariant).
    REQUIRE(session.ResolveNodeEntryClaims("mine_node").empty());
    REQUIRE(session.ResolveNodeEntryClaims("mine_node").empty());

    const auto* owned = session.FindOwnedService("mine_svc");
    REQUIRE(owned != nullptr);
    REQUIRE(owned->ownerTeamColor == "Green");
    REQUIRE(owned->stationedUnits.size() == 1);  // preserved across re-entry
}

TEST_CASE("NodeEntry claim: locked / destroyed services are not claimed on entry") {
    auto session = MakeSession({MakeMine("locked_svc", "locked_node"),
                                MakeMine("destroyed_svc", "destroyed_node")});
    auto save = BaseSave();
    save.ownedServices = {
        core::OwnedServiceSaveState{"locked_svc", "", true, false, {}},
        core::OwnedServiceSaveState{"destroyed_svc", "", false, true, {}}
    };
    session.ApplySaveData(save);

    REQUIRE(session.ResolveNodeEntryClaims("locked_node").empty());
    REQUIRE(session.ResolveNodeEntryClaims("destroyed_node").empty());
    REQUIRE(session.FindOwnedService("locked_svc")->ownerTeamColor.empty());
    REQUIRE(session.FindOwnedService("destroyed_svc")->ownerTeamColor.empty());
}

TEST_CASE("NodeEntry claim: non-ownable services are not claimed on entry") {
    auto session = MakeSession({MakeService("rest_svc", "rest_node", LocationServiceKind::Rest),
                                MakeService("shop_svc", "rest_node", LocationServiceKind::Shop)});
    session.ApplySaveData(BaseSave());

    const auto claimed = session.ResolveNodeEntryClaims("rest_node");

    REQUIRE(claimed.empty());
    REQUIRE(session.FindOwnedService("rest_svc") == nullptr);
    REQUIRE(session.FindOwnedService("shop_svc") == nullptr);
}

TEST_CASE("NodeEntry claim: services at unrelated nodes are not claimed") {
    auto session = MakeSession({MakeMine("here_svc", "here_node"),
                                MakeMine("there_svc", "there_node")});
    session.ApplySaveData(BaseSave());

    const auto claimed = session.ResolveNodeEntryClaims("here_node");

    REQUIRE(claimed.size() == 1);
    REQUIRE(claimed.front() == "here_svc");
    REQUIRE(session.FindOwnedService("there_svc") == nullptr);  // other node untouched
}

TEST_CASE("NodeEntry claim: ResolveNodeEntryClaims and ClaimContestedServicesAtNode are equivalent") {
    const std::vector<data::LocationServiceDefinition> catalog = {MakeMine("mine_svc", "mine_node")};

    auto viaResolve = MakeSession(catalog);
    viaResolve.ApplySaveData(BaseSave());
    const auto resolved = viaResolve.ResolveNodeEntryClaims("mine_node");

    auto viaAlias = MakeSession(catalog);
    viaAlias.ApplySaveData(BaseSave());
    const auto aliased = viaAlias.ClaimContestedServicesAtNode("mine_node");

    REQUIRE(resolved == aliased);
    REQUIRE(viaResolve.FindOwnedService("mine_svc")->ownerTeamColor ==
            viaAlias.FindOwnedService("mine_svc")->ownerTeamColor);
}

TEST_CASE("NodeEntry claim: guard cleared then entry claims (post-battle parity)") {
    auto session = MakeSession({MakeMine("mine_svc", "mine_node")});
    session.ApplySaveData(BaseSave());
    session.SetEnemyTeams({MakeTeam("Red", "mine_node")});

    REQUIRE(session.ResolveNodeEntryClaims("mine_node").empty());  // blocked while guarded
    session.ClearEnemyTeamByColor("Red");
    const auto claimed = session.ResolveNodeEntryClaims("mine_node");  // now claimable

    REQUIRE(claimed.size() == 1);
    REQUIRE(session.FindOwnedService("mine_svc")->ownerTeamColor == "Green");
}

TEST_CASE("NodeEntry claim: peaceful claim + stationing round-trips through save/load with no schema bump") {
    auto session = MakeSession({MakeMine("mine_svc", "mine_node")});
    auto save = BaseSave();
    save.ownedServices = {
        core::OwnedServiceSaveState{"mine_svc", "Green", false, false,
            {core::StationedUnitSaveState{"kobold", "stk_2"}}}
    };
    session.ApplySaveData(save);
    REQUIRE(session.ResolveNodeEntryClaims("mine_node").empty());  // idempotent re-entry

    const auto saved = session.ToSaveData();
    REQUIRE(saved.schemaVersion == 5);  // no bump

    gameplay::GameSession restored = MakeSession({MakeMine("mine_svc", "mine_node")});
    restored.ApplySaveData(saved);
    const auto* owned = restored.FindOwnedService("mine_svc");
    REQUIRE(owned != nullptr);
    REQUIRE(owned->ownerTeamColor == "Green");
    REQUIRE(owned->stationedUnits.size() == 1);
}

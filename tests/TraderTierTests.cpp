#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

#include "core/SaveGame.h"
#include "data/definitions/LocationServiceDefinition.h"
#include "gameplay/EnemyTeamState.h"
#include "gameplay/GameSession.h"

using data::LocationServiceKind;

namespace {

data::LocationServiceDefinition MakeService(const std::string& id,
    const std::string& locationId, LocationServiceKind kind) {
    data::LocationServiceDefinition svc;
    svc.id = id;
    svc.locationId = locationId;
    svc.kind = kind;
    return svc;
}

core::SaveData MakeBaseSave() {
    core::SaveData s;
    s.schemaVersion = 5;
    s.day = 1;
    s.minutesIntoSliceDay = 0;
    s.gold = 2500;
    s.mode = "region_mode";
    s.regionId = "ashvale_heartland";
    s.destinationId = "home_base";
    s.hasCanonicalRoster = true;
    s.rosterStacks = { core::RosterStackSaveState{"stk_1", "hero", 1} };
    s.activeSlotStackIds = {"stk_1", "", "", "", ""};
    s.reserveSlotStackIds = {"", "", "", "", "", "", "", ""};
    s.nextStackIdCounter = 2;
    return s;
}

core::OwnedServiceSaveState Owned(const std::string& serviceId, const std::string& owner,
    bool locked = false, bool destroyed = false) {
    return core::OwnedServiceSaveState{serviceId, owner, locked, destroyed, {}};
}

gameplay::GameSession MakeSession(
    std::vector<data::LocationServiceDefinition> serviceCatalog,
    std::vector<core::OwnedServiceSaveState> ownedServices) {
    auto save = MakeBaseSave();
    save.ownedServices = std::move(ownedServices);
    gameplay::GameSession session;
    session.SetLocationServiceCatalog(std::move(serviceCatalog));
    session.ApplySaveData(save);
    return session;
}

} // namespace

TEST_CASE("TraderTierSession - counts player-owned trader services per type") {
    auto session = MakeSession(
        {
            MakeService("market_a", "loc_a", LocationServiceKind::Market),
            MakeService("market_b", "loc_b", LocationServiceKind::Market),
            MakeService("tp_a", "loc_c", LocationServiceKind::TradingPost)
        },
        {
            Owned("market_a", "Green"),
            Owned("market_b", "Green"),
            Owned("tp_a", "Green")
        });

    // Per type: 2 Markets, 1 Trading Post — neither affects the other.
    REQUIRE(session.OwnedTraderServiceTier(LocationServiceKind::Market) == 2);
    REQUIRE(session.OwnedTraderServiceTier(LocationServiceKind::TradingPost) == 1);
    REQUIRE(session.OwnedTraderServiceTier(LocationServiceKind::BlackMarket) == 0);
}

TEST_CASE("TraderTierSession - allied, enemy, neutral and unowned services do not count") {
    auto session = MakeSession(
        {
            MakeService("m_player", "loc_a", LocationServiceKind::Market),
            MakeService("m_ally", "loc_b", LocationServiceKind::Market),
            MakeService("m_enemy", "loc_c", LocationServiceKind::Market),
            MakeService("m_neutral", "loc_d", LocationServiceKind::Market),
            MakeService("m_unowned", "loc_e", LocationServiceKind::Market)
        },
        {
            Owned("m_player", "Green"),
            Owned("m_ally", "Blue"),
            Owned("m_enemy", "Red"),
            Owned("m_neutral", "Neutral"),
            Owned("m_unowned", "")
        });

    REQUIRE(session.OwnedTraderServiceTier(LocationServiceKind::Market) == 1);
}

TEST_CASE("TraderTierSession - locked and destroyed services do not count") {
    auto session = MakeSession(
        {
            MakeService("m_ok", "loc_a", LocationServiceKind::Market),
            MakeService("m_locked", "loc_b", LocationServiceKind::Market),
            MakeService("m_destroyed", "loc_c", LocationServiceKind::Market)
        },
        {
            Owned("m_ok", "Green"),
            Owned("m_locked", "Green", /*locked=*/true),
            Owned("m_destroyed", "Green", false, /*destroyed=*/true)
        });

    REQUIRE(session.OwnedTraderServiceTier(LocationServiceKind::Market) == 1);
}

TEST_CASE("TraderTierSession - hostile-occupied service does not count") {
    auto session = MakeSession(
        {
            MakeService("m_ok", "loc_a", LocationServiceKind::Market),
            MakeService("m_occupied", "loc_b", LocationServiceKind::Market)
        },
        {
            Owned("m_ok", "Green"),
            Owned("m_occupied", "Green")
        });

    gameplay::EnemyTeamState enemy;
    enemy.teamColor = "Red";
    enemy.nodeId = "loc_b";  // occupies the second market's node
    enemy.active = true;
    session.SetEnemyTeams({enemy});

    REQUIRE(session.OwnedTraderServiceTier(LocationServiceKind::Market) == 1);
}

TEST_CASE("TraderTierSession - tier caps at 8") {
    std::vector<data::LocationServiceDefinition> catalog;
    std::vector<core::OwnedServiceSaveState> owned;
    for (int i = 0; i < 11; ++i) {
        const std::string id = "m_" + std::to_string(i);
        catalog.push_back(MakeService(id, "loc_" + std::to_string(i), LocationServiceKind::Market));
        owned.push_back(Owned(id, "Green"));
    }
    auto session = MakeSession(std::move(catalog), std::move(owned));

    REQUIRE(session.OwnedTraderServiceTier(LocationServiceKind::Market) == 8);
}

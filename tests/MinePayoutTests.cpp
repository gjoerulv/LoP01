#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

#include "core/GameClock.h"
#include "core/SaveGame.h"
#include "data/definitions/LocationServiceDefinition.h"
#include "data/definitions/UnitDefinition.h"
#include "gameplay/EnemyTeamState.h"
#include "gameplay/GameSession.h"
#include "gameplay/ResourceState.h"

using gameplay::ResourceType;

namespace {

constexpr int kOneDay = core::GameClock::kMinutesPerSliceDay;

data::UnitDefinition MakeUnit(const std::string& id) {
    data::UnitDefinition u;
    u.id = id;
    u.name = id;
    u.category = data::UnitDefinitionCategory::Generic;
    return u;
}

data::UnitDefinition MakeUnitWithPassive(const std::string& id,
    const std::string& resource, int amount) {
    auto u = MakeUnit(id);
    u.passiveEffects.push_back(data::UnitPassiveEffect{
        data::PassiveEffectKind::MineProduction, resource, "mine", amount});
    return u;
}

std::vector<data::UnitDefinition> MakeUnitCatalog() {
    return {
        MakeUnit("plain"),
        MakeUnitWithPassive("kobold", "Stone", 1),
        MakeUnitWithPassive("stonewright", "Stone", 2),
        MakeUnitWithPassive("clayworker", "Clay", 1)
    };
}

data::LocationServiceDefinition MakeMine(const std::string& id,
    const std::string& locationId, std::vector<data::MineOutputDefinition> outputs) {
    data::LocationServiceDefinition svc;
    svc.id = id;
    svc.locationId = locationId;
    svc.zoneId = "mine_face";
    svc.kind = data::LocationServiceKind::Mine;
    svc.mineOutputs = std::move(outputs);
    return svc;
}

// Canonical save with a roster of the test units. Caller fills ownedServices.
//   stk_1 -> kobold (3), stk_2 -> stonewright (1), stk_3 -> clayworker (2)
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
    s.rosterStacks = {
        core::RosterStackSaveState{"stk_1", "kobold", 3},
        core::RosterStackSaveState{"stk_2", "stonewright", 1},
        core::RosterStackSaveState{"stk_3", "clayworker", 2}
    };
    s.activeSlotStackIds = {"stk_1", "", "", "", ""};
    s.reserveSlotStackIds = {"stk_2", "stk_3", "", "", "", "", "", ""};
    s.nextStackIdCounter = 4;
    return s;
}

// Builds a session with the given service catalog and owned services, loaded and
// ready at day 1. playerColor defaults to "Green".
gameplay::GameSession MakeSession(
    std::vector<data::LocationServiceDefinition> serviceCatalog,
    std::vector<core::OwnedServiceSaveState> ownedServices) {
    auto save = MakeBaseSave();
    save.ownedServices = std::move(ownedServices);

    gameplay::GameSession session;
    session.SetUnitCatalog(MakeUnitCatalog());
    session.SetLocationServiceCatalog(std::move(serviceCatalog));
    session.ApplySaveData(save);
    return session;
}

core::OwnedServiceSaveState Owned(const std::string& serviceId,
    const std::string& owner, bool locked, bool destroyed,
    std::vector<core::StationedUnitSaveState> stationed = {}) {
    return core::OwnedServiceSaveState{serviceId, owner, locked, destroyed, std::move(stationed)};
}

} // namespace

TEST_CASE("MinePayout - owned gold mine pays its base output at the day boundary") {
    auto session = MakeSession(
        {MakeMine("gold_mine_svc", "gold_mine", {{"Gold", 1000}})},
        {Owned("gold_mine_svc", "Green", false, false)});

    const int goldBefore = session.Snapshot().gold;
    session.AddMinutes(kOneDay);  // cross one day boundary

    REQUIRE(session.Snapshot().gold == goldBefore + 1000);
    // Gold source of truth is shared: the resource API observes the same value.
    REQUIRE(session.ResourceCount(ResourceType::Gold) == goldBefore + 1000);
}

TEST_CASE("MinePayout - non-gold output accrues into the resource pool") {
    auto session = MakeSession(
        {MakeMine("stone_mine_svc", "stone_mine", {{"Stone", 2}})},
        {Owned("stone_mine_svc", "Green", false, false)});

    REQUIRE(session.ResourceCount(ResourceType::Stone) == 0);
    session.AddMinutes(kOneDay);
    REQUIRE(session.ResourceCount(ResourceType::Stone) == 2);
}

TEST_CASE("MinePayout - a stationed passive increases output") {
    auto session = MakeSession(
        {MakeMine("stone_mine_svc", "stone_mine", {{"Stone", 2}})},
        {Owned("stone_mine_svc", "Green", false, false,
            {core::StationedUnitSaveState{"kobold", "stk_1"}})});  // +1 Stone

    session.AddMinutes(kOneDay);
    REQUIRE(session.ResourceCount(ResourceType::Stone) == 3);  // 2 + 1
}

TEST_CASE("MinePayout - strongest stationed passive wins") {
    auto session = MakeSession(
        {MakeMine("stone_mine_svc", "stone_mine", {{"Stone", 2}})},
        {Owned("stone_mine_svc", "Green", false, false,
            {
                core::StationedUnitSaveState{"kobold", "stk_1"},      // +1
                core::StationedUnitSaveState{"stonewright", "stk_2"}  // +2 (strongest)
            })});

    session.AddMinutes(kOneDay);
    REQUIRE(session.ResourceCount(ResourceType::Stone) == 4);  // 2 + strongest 2, not +1+2
}

TEST_CASE("MinePayout - equal-strength passives do not stack") {
    // Two stationed refs both resolve to a +1 Stone passive (same stack); the
    // two equal +1 contributions must collapse to a single +1.
    auto session = MakeSession(
        {MakeMine("stone_mine_svc", "stone_mine", {{"Stone", 2}})},
        {Owned("stone_mine_svc", "Green", false, false,
            {
                core::StationedUnitSaveState{"kobold", "stk_1"},
                core::StationedUnitSaveState{"kobold", "stk_1"}
            })});

    session.AddMinutes(kOneDay);
    REQUIRE(session.ResourceCount(ResourceType::Stone) == 3);  // 2 + 1, not 2 + 1 + 1
}

TEST_CASE("MinePayout - multi-resource outputs resolve independently") {
    auto session = MakeSession(
        {MakeMine("mixed_mine_svc", "mixed_mine", {{"Stone", 2}, {"Clay", 1}})},
        {Owned("mixed_mine_svc", "Green", false, false,
            {
                core::StationedUnitSaveState{"stonewright", "stk_2"},  // +2 Stone
                core::StationedUnitSaveState{"clayworker", "stk_3"}    // +1 Clay
            })});

    session.AddMinutes(kOneDay);
    REQUIRE(session.ResourceCount(ResourceType::Stone) == 4);  // 2 + 2
    REQUIRE(session.ResourceCount(ResourceType::Clay) == 2);   // 1 + 1
}

TEST_CASE("MinePayout - service with no owner pays nothing") {
    auto session = MakeSession(
        {MakeMine("stone_mine_svc", "stone_mine", {{"Stone", 2}})},
        {Owned("stone_mine_svc", "", false, false)});  // no owner

    session.AddMinutes(kOneDay);
    REQUIRE(session.ResourceCount(ResourceType::Stone) == 0);
}

TEST_CASE("MinePayout - locked service pays nothing") {
    auto session = MakeSession(
        {MakeMine("stone_mine_svc", "stone_mine", {{"Stone", 2}})},
        {Owned("stone_mine_svc", "Green", /*locked=*/true, false)});

    session.AddMinutes(kOneDay);
    REQUIRE(session.ResourceCount(ResourceType::Stone) == 0);
}

TEST_CASE("MinePayout - destroyed service pays nothing") {
    auto session = MakeSession(
        {MakeMine("stone_mine_svc", "stone_mine", {{"Stone", 2}})},
        {Owned("stone_mine_svc", "Green", false, /*destroyed=*/true)});

    session.AddMinutes(kOneDay);
    REQUIRE(session.ResourceCount(ResourceType::Stone) == 0);
}

TEST_CASE("MinePayout - hostile-occupied service pays nothing") {
    auto session = MakeSession(
        {MakeMine("stone_mine_svc", "stone_mine", {{"Stone", 2}})},
        {Owned("stone_mine_svc", "Green", false, false)});

    // A hostile (non-allied, active) enemy team occupies the mine's node.
    gameplay::EnemyTeamState enemy;
    enemy.teamColor = "Red";
    enemy.nodeId = "stone_mine";
    enemy.active = true;
    session.SetEnemyTeams({enemy});

    session.AddMinutes(kOneDay);
    REQUIRE(session.ResourceCount(ResourceType::Stone) == 0);
}

TEST_CASE("MinePayout - non-mine owned service pays nothing") {
    data::LocationServiceDefinition shop;
    shop.id = "shop_svc";
    shop.locationId = "town";
    shop.kind = data::LocationServiceKind::Shop;  // not a mine, no outputs

    auto session = MakeSession({shop}, {Owned("shop_svc", "Green", false, false)});

    const int goldBefore = session.Snapshot().gold;
    session.AddMinutes(kOneDay);
    REQUIRE(session.Snapshot().gold == goldBefore);
}

TEST_CASE("MinePayout - fires exactly once for a normal day boundary") {
    auto session = MakeSession(
        {MakeMine("gold_mine_svc", "gold_mine", {{"Gold", 1000}})},
        {Owned("gold_mine_svc", "Green", false, false)});

    const int goldBefore = session.Snapshot().gold;
    session.AddMinutes(kOneDay);  // exactly one crossing
    REQUIRE(session.Snapshot().gold == goldBefore + 1000);
}

TEST_CASE("MinePayout - does not fire when time advances without crossing a day boundary") {
    auto session = MakeSession(
        {MakeMine("gold_mine_svc", "gold_mine", {{"Gold", 1000}})},
        {Owned("gold_mine_svc", "Green", false, false)});

    const int goldBefore = session.Snapshot().gold;
    session.AddMinutes(60);  // same day
    REQUIRE(session.Snapshot().gold == goldBefore);
}

TEST_CASE("MinePayout - a multi-day jump pays exactly once per crossing") {
    auto session = MakeSession(
        {MakeMine("gold_mine_svc", "gold_mine", {{"Gold", 1000}})},
        {Owned("gold_mine_svc", "Green", false, false)});

    const int goldBefore = session.Snapshot().gold;
    session.AddMinutes(kOneDay * 2);  // day 1 -> 3 in one advance: one detected crossing
    REQUIRE(session.Snapshot().gold == goldBefore + 1000);  // not +2000
}

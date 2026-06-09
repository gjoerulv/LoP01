#include <catch2/catch_test_macros.hpp>

#include "data/definitions/LocationServiceDefinition.h"
#include "gameplay/economy/StationingRules.h"

using data::LocationServiceKind;
using gameplay::economy::IsLegalSplitQuantity;
using gameplay::economy::kMaxStationedUnitsPerService;
using gameplay::economy::ServiceCanReceiveStationedUnit;
using gameplay::economy::StackIsStationable;

TEST_CASE("StationingRules - a player-owned, available mine below capacity can receive") {
    REQUIRE(ServiceCanReceiveStationedUnit(
        LocationServiceKind::Mine, /*playerOwned=*/true, /*locked=*/false,
        /*destroyed=*/false, /*currentStationedCount=*/0));
}

TEST_CASE("StationingRules - non-mine kinds are not stationing targets in M25") {
    REQUIRE_FALSE(ServiceCanReceiveStationedUnit(
        LocationServiceKind::TradingPost, true, false, false, 0));
    REQUIRE_FALSE(ServiceCanReceiveStationedUnit(
        LocationServiceKind::Market, true, false, false, 0));
    REQUIRE_FALSE(ServiceCanReceiveStationedUnit(
        LocationServiceKind::Rest, true, false, false, 0));
}

TEST_CASE("StationingRules - non-owned / locked / destroyed mines refuse units") {
    REQUIRE_FALSE(ServiceCanReceiveStationedUnit(
        LocationServiceKind::Mine, /*playerOwned=*/false, false, false, 0));
    REQUIRE_FALSE(ServiceCanReceiveStationedUnit(
        LocationServiceKind::Mine, true, /*locked=*/true, false, 0));
    REQUIRE_FALSE(ServiceCanReceiveStationedUnit(
        LocationServiceKind::Mine, true, false, /*destroyed=*/true, 0));
}

TEST_CASE("StationingRules - capacity is bounded at the per-service maximum") {
    REQUIRE(kMaxStationedUnitsPerService == 5);
    REQUIRE(ServiceCanReceiveStationedUnit(
        LocationServiceKind::Mine, true, false, false,
        kMaxStationedUnitsPerService - 1));
    REQUIRE_FALSE(ServiceCanReceiveStationedUnit(
        LocationServiceKind::Mine, true, false, false,
        kMaxStationedUnitsPerService));
    REQUIRE_FALSE(ServiceCanReceiveStationedUnit(
        LocationServiceKind::Mine, true, false, false,
        kMaxStationedUnitsPerService + 1));
}

TEST_CASE("StationingRules - a real, non-PC, not-yet-stationed stack is stationable") {
    REQUIRE(StackIsStationable(
        /*stackExists=*/true, /*isPlayerCharacter=*/false, /*alreadyStationed=*/false));
}

TEST_CASE("StationingRules - missing, Player-Character, or already-stationed stacks are not stationable") {
    REQUIRE_FALSE(StackIsStationable(/*stackExists=*/false, false, false));
    REQUIRE_FALSE(StackIsStationable(true, /*isPlayerCharacter=*/true, false));
    REQUIRE_FALSE(StackIsStationable(true, false, /*alreadyStationed=*/true));
}

TEST_CASE("StationingRules - a legal split leaves at least one unit on each side") {
    REQUIRE(IsLegalSplitQuantity(/*quantity=*/1, /*stackQuantity=*/3));
    REQUIRE(IsLegalSplitQuantity(2, 3));
    REQUIRE_FALSE(IsLegalSplitQuantity(0, 3));   // nothing moved
    REQUIRE_FALSE(IsLegalSplitQuantity(3, 3));   // whole stack, not a split
    REQUIRE_FALSE(IsLegalSplitQuantity(4, 3));   // more than the stack holds
    REQUIRE_FALSE(IsLegalSplitQuantity(1, 1));   // a single unit cannot split
}

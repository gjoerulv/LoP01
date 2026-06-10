#include <catch2/catch_test_macros.hpp>

#include "data/definitions/LocationServiceDefinition.h"
#include "gameplay/economy/StorageRules.h"

using data::LocationServiceKind;
using gameplay::economy::kMaxStoredUnitsPerService;
using gameplay::economy::ServiceCanReceiveStoredUnit;
using gameplay::economy::StackIsStorable;

TEST_CASE("StorageRules - a player-owned, available storage below capacity can receive") {
    REQUIRE(ServiceCanReceiveStoredUnit(
        LocationServiceKind::Storage, /*playerOwned=*/true, /*locked=*/false,
        /*destroyed=*/false, /*currentStoredCount=*/0));
}

TEST_CASE("StorageRules - non-storage kinds are not storage targets") {
    REQUIRE_FALSE(ServiceCanReceiveStoredUnit(LocationServiceKind::Mine, true, false, false, 0));
    REQUIRE_FALSE(ServiceCanReceiveStoredUnit(LocationServiceKind::TradingPost, true, false, false, 0));
    REQUIRE_FALSE(ServiceCanReceiveStoredUnit(LocationServiceKind::Rest, true, false, false, 0));
}

TEST_CASE("StorageRules - non-owned / locked / destroyed storage refuses units") {
    REQUIRE_FALSE(ServiceCanReceiveStoredUnit(LocationServiceKind::Storage, /*playerOwned=*/false, false, false, 0));
    REQUIRE_FALSE(ServiceCanReceiveStoredUnit(LocationServiceKind::Storage, true, /*locked=*/true, false, 0));
    REQUIRE_FALSE(ServiceCanReceiveStoredUnit(LocationServiceKind::Storage, true, false, /*destroyed=*/true, 0));
}

TEST_CASE("StorageRules - capacity is bounded at 7, distinct from the mine cap of 5") {
    REQUIRE(kMaxStoredUnitsPerService == 7);
    REQUIRE(ServiceCanReceiveStoredUnit(LocationServiceKind::Storage, true, false, false, kMaxStoredUnitsPerService - 1));
    REQUIRE_FALSE(ServiceCanReceiveStoredUnit(LocationServiceKind::Storage, true, false, false, kMaxStoredUnitsPerService));
    REQUIRE_FALSE(ServiceCanReceiveStoredUnit(LocationServiceKind::Storage, true, false, false, kMaxStoredUnitsPerService + 1));
}

TEST_CASE("StorageRules - a real, non-PC, not-yet-placed stack is storable") {
    REQUIRE(StackIsStorable(/*stackExists=*/true, /*isPlayerCharacter=*/false, /*alreadyPlaced=*/false));
}

TEST_CASE("StorageRules - missing / Player-Character / already-placed stacks are not storable") {
    REQUIRE_FALSE(StackIsStorable(/*stackExists=*/false, false, false));
    REQUIRE_FALSE(StackIsStorable(true, /*isPlayerCharacter=*/true, false));
    REQUIRE_FALSE(StackIsStorable(true, false, /*alreadyPlaced=*/true));
}

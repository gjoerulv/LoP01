#include <catch2/catch_test_macros.hpp>

#include "data/definitions/LocationServiceDefinition.h"
#include "gameplay/economy/ServiceClaimRules.h"

using data::LocationServiceKind;
using gameplay::economy::ServiceIsClaimable;
using gameplay::economy::ServiceOwnerRelationship;

namespace {

// A claimable baseline: an unowned mine, not locked, not destroyed.
bool Claimable(LocationServiceKind kind = LocationServiceKind::Mine,
    bool locked = false,
    bool destroyed = false,
    ServiceOwnerRelationship rel = ServiceOwnerRelationship::Unowned) {
    return ServiceIsClaimable(kind, locked, destroyed, rel);
}

} // namespace

TEST_CASE("ServiceClaimRules - unowned ownable service is claimable") {
    REQUIRE(Claimable(LocationServiceKind::Mine));
    REQUIRE(Claimable(LocationServiceKind::TradingPost));
    REQUIRE(Claimable(LocationServiceKind::Market));
    REQUIRE(Claimable(LocationServiceKind::FreelancersGuild));
    REQUIRE(Claimable(LocationServiceKind::BlackMarket));
}

TEST_CASE("ServiceClaimRules - non-ownable service kinds are never claimable") {
    REQUIRE_FALSE(Claimable(LocationServiceKind::Rest));
    REQUIRE_FALSE(Claimable(LocationServiceKind::Shop));
    REQUIRE_FALSE(Claimable(LocationServiceKind::Recruit));
    REQUIRE_FALSE(Claimable(LocationServiceKind::Muster));
    REQUIRE_FALSE(Claimable(LocationServiceKind::Unknown));
}

TEST_CASE("ServiceClaimRules - hostile-owned service is claimable (contesting)") {
    REQUIRE(Claimable(LocationServiceKind::TradingPost, false, false,
        ServiceOwnerRelationship::HostileToPlayer));
}

TEST_CASE("ServiceClaimRules - already player-owned service is not claimed") {
    REQUIRE_FALSE(Claimable(LocationServiceKind::Mine, false, false,
        ServiceOwnerRelationship::Player));
}

TEST_CASE("ServiceClaimRules - allied-owned service is not claimed (conservative)") {
    REQUIRE_FALSE(Claimable(LocationServiceKind::Mine, false, false,
        ServiceOwnerRelationship::AlliedToPlayer));
}

TEST_CASE("ServiceClaimRules - locked service is not claimable") {
    REQUIRE_FALSE(Claimable(LocationServiceKind::Mine, /*locked=*/true));
    REQUIRE_FALSE(Claimable(LocationServiceKind::Mine, /*locked=*/true, false,
        ServiceOwnerRelationship::HostileToPlayer));
}

TEST_CASE("ServiceClaimRules - destroyed service is not claimable") {
    REQUIRE_FALSE(Claimable(LocationServiceKind::Mine, false, /*destroyed=*/true));
    REQUIRE_FALSE(Claimable(LocationServiceKind::Mine, false, /*destroyed=*/true,
        ServiceOwnerRelationship::HostileToPlayer));
}

#include <catch2/catch_test_macros.hpp>

#include "data/definitions/LocationServiceDefinition.h"
#include "gameplay/location/LocationServiceRules.h"

TEST_CASE("Rest service is recognized by typed service kind") {
    data::LocationServiceDefinition service;
    service.kind = data::LocationServiceKind::Rest;

    REQUIRE(gameplay::location::IsRestService(&service));
    REQUIRE_FALSE(gameplay::location::IsShopService(&service));
    REQUIRE_FALSE(gameplay::location::IsRecruitService(&service));
}

TEST_CASE("Shop service is recognized by typed service kind") {
    data::LocationServiceDefinition service;
    service.kind = data::LocationServiceKind::Shop;

    REQUIRE(gameplay::location::IsShopService(&service));
    REQUIRE_FALSE(gameplay::location::IsRestService(&service));
    REQUIRE_FALSE(gameplay::location::IsRecruitService(&service));
}

TEST_CASE("Recruit service is recognized by typed service kind") {
    data::LocationServiceDefinition service;
    service.kind = data::LocationServiceKind::Recruit;

    REQUIRE(gameplay::location::IsRecruitService(&service));
    REQUIRE_FALSE(gameplay::location::IsRestService(&service));
    REQUIRE_FALSE(gameplay::location::IsShopService(&service));
}
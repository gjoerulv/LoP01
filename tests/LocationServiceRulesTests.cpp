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

TEST_CASE("Recruit service prompt includes unit, cost, stock, and week") {
    data::LocationServiceDefinition service;
    service.kind = data::LocationServiceKind::Recruit;
    service.unitDisplayName = "Town Guard";
    service.goldCost = 120;
    service.weeklyStock = 3;

    const std::string prompt = gameplay::location::BuildServicePromptText(service, 2, 4, 0);
    REQUIRE(prompt == "E: Recruit Town Guard (120g) | Stock 2/3 | Week 4");
}

TEST_CASE("Rest service prompt uses authored text when present") {
    data::LocationServiceDefinition service;
    service.kind = data::LocationServiceKind::Rest;
    service.restKind = data::RestServiceKind::Inn;
    service.promptText = "E: Rent a Room at Old Inn (Pay 25g)";

    const std::string prompt = gameplay::location::BuildServicePromptText(service, 0, 1, 0);
    REQUIRE(prompt == service.promptText);
}

TEST_CASE("Daily supplies shop prompt shows effect and daily availability") {
    data::LocationServiceDefinition service;
    service.kind = data::LocationServiceKind::Shop;
    service.goldCost = 35;
    service.dailyUseLimit = 1;
    service.travelPrepDiscountMinutes = 20;
    service.travelPrepCharges = 1;

    const std::string availablePrompt = gameplay::location::BuildServicePromptText(service, 0, 1, 1);
    REQUIRE(availablePrompt == "E: Buy Trail Supplies (35g) | Next travel -20m | 1/day: Available today");

    const std::string usedPrompt = gameplay::location::BuildServicePromptText(service, 0, 1, 0);
    REQUIRE(usedPrompt == "E: Buy Trail Supplies (35g) | Next travel -20m | 1/day: Used today");
}

TEST_CASE("Home Base prep prompt uses authored label and free daily prep text") {
    data::LocationServiceDefinition service;
    service.kind = data::LocationServiceKind::Shop;
    service.promptText = "E: Prepare at Home Base";
    service.goldCost = 0;
    service.dailyUseLimit = 1;
    service.travelPrepDiscountMinutes = 20;
    service.travelPrepCharges = 1;

    const std::string prompt = gameplay::location::BuildServicePromptText(service, 0, 1, 1);
    REQUIRE(prompt == "E: Prepare at Home Base (Free) | Next travel -20m | 1/day: Available today");
}
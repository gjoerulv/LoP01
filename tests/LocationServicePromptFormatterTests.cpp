#include <catch2/catch_test_macros.hpp>

#include "app/formatters/LocationServicePromptFormatter.h"
#include "data/definitions/LocationServiceDefinition.h"

TEST_CASE("Recruit service prompt includes unit, cost, stock, and week") {
    data::LocationServiceDefinition service;
    service.kind = data::LocationServiceKind::Recruit;
    service.unitDisplayName = "Town Guard";
    service.goldCost = 120;
    service.weeklyStock = 3;

    app::LocationServicePromptContext context;
    context.remainingRecruitStock = 2;
    context.currentWeek = 4;

    const std::string prompt = app::BuildLocationServicePrompt(service, context);
    REQUIRE(prompt == "Recruit Town Guard\nCost: 120g | Stock: 2/3\nResets: Week 5");
}

TEST_CASE("Rest service prompt uses authored text when present") {
    data::LocationServiceDefinition service;
    service.kind = data::LocationServiceKind::Rest;
    service.restKind = data::RestServiceKind::Inn;
    service.promptText = "E: Rent a Room at Old Inn";
    service.goldCost = 25;

    app::LocationServicePromptContext context;
    const std::string prompt = app::BuildLocationServicePrompt(service, context);

    REQUIRE(prompt == "Rent a Room at Old Inn\nCost: 25g\nResets: N/A");
}

TEST_CASE("Daily supplies shop prompt shows effect and daily availability") {
    data::LocationServiceDefinition service;
    service.kind = data::LocationServiceKind::Shop;
    service.goldCost = 35;
    service.dailyUseLimit = 1;
    service.travelPrepDiscountMinutes = 20;
    service.travelPrepCharges = 1;

    app::LocationServicePromptContext availableContext;
    availableContext.remainingDailyUses = 1;

    const std::string availablePrompt = app::BuildLocationServicePrompt(service, availableContext);
    REQUIRE(availablePrompt == "Buy Trail Supplies\nCost: 35g | Effect: Next travel -20m\nResets: Next day | Available today");

    app::LocationServicePromptContext usedContext;
    usedContext.remainingDailyUses = 0;

    const std::string usedPrompt = app::BuildLocationServicePrompt(service, usedContext);
    REQUIRE(usedPrompt == "Buy Trail Supplies\nCost: 35g | Effect: Next travel -20m\nResets: Next day | Used today");
}

TEST_CASE("Home Base prep prompt uses authored label and free daily prep text") {
    data::LocationServiceDefinition service;
    service.kind = data::LocationServiceKind::Shop;
    service.promptText = "E: Prepare at Home Base";
    service.goldCost = 0;
    service.dailyUseLimit = 1;
    service.travelPrepDiscountMinutes = 20;
    service.travelPrepCharges = 1;

    app::LocationServicePromptContext context;
    context.remainingDailyUses = 1;

    const std::string prompt = app::BuildLocationServicePrompt(service, context);
    REQUIRE(prompt == "Prepare at Home Base\nCost: Free | Effect: Next travel -20m\nResets: Next day | Available today");
}

TEST_CASE("Travel prep prompt shows active-prep wording when prep is already active") {
    data::LocationServiceDefinition service;
    service.kind = data::LocationServiceKind::Shop;
    service.promptText = "E: Prepare at Home Base";
    service.goldCost = 0;
    service.dailyUseLimit = 1;
    service.travelPrepDiscountMinutes = 20;
    service.travelPrepCharges = 1;

    app::LocationServicePromptContext context;
    context.remainingDailyUses = 1;
    context.hasActiveTravelPrep = true;

    const std::string prompt = app::BuildLocationServicePrompt(service, context);
    REQUIRE(prompt == "Prepare at Home Base\nCost: Free | Effect: Next travel -20m\nResets: Next day | Prep already active");
}
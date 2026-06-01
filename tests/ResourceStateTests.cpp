#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <string>

#include "gameplay/GameSession.h"
#include "gameplay/ResourceState.h"

using gameplay::ResourceType;

// ---------------------------------------------------------------------------
// Pure ResourceType parse/format — strict, 1:1, no fuzzy matching.
// ---------------------------------------------------------------------------

TEST_CASE("ResourceType - canonical names round-trip through parse/format") {
    const ResourceType all[] = {
        ResourceType::Gold, ResourceType::Wood, ResourceType::Stone,
        ResourceType::Steel, ResourceType::Fiber, ResourceType::Clay,
        ResourceType::Gems
    };
    for (const auto type : all) {
        const std::string name = gameplay::ResourceTypeToString(type);
        ResourceType parsed;
        REQUIRE(gameplay::TryResourceTypeFromString(name, parsed));
        REQUIRE(parsed == type);
    }
}

TEST_CASE("ResourceType - exact expected canonical strings") {
    REQUIRE(std::string(gameplay::ResourceTypeToString(ResourceType::Gold)) == "Gold");
    REQUIRE(std::string(gameplay::ResourceTypeToString(ResourceType::Wood)) == "Wood");
    REQUIRE(std::string(gameplay::ResourceTypeToString(ResourceType::Stone)) == "Stone");
    REQUIRE(std::string(gameplay::ResourceTypeToString(ResourceType::Steel)) == "Steel");
    REQUIRE(std::string(gameplay::ResourceTypeToString(ResourceType::Fiber)) == "Fiber");
    REQUIRE(std::string(gameplay::ResourceTypeToString(ResourceType::Clay)) == "Clay");
    REQUIRE(std::string(gameplay::ResourceTypeToString(ResourceType::Gems)) == "Gems");
}

TEST_CASE("ResourceType - parse rejects unknown and non-canonical strings") {
    ResourceType parsed;
    REQUIRE_FALSE(gameplay::TryResourceTypeFromString("gold", parsed));   // case-sensitive
    REQUIRE_FALSE(gameplay::TryResourceTypeFromString("WOOD", parsed));
    REQUIRE_FALSE(gameplay::TryResourceTypeFromString("Mithril", parsed));
    REQUIRE_FALSE(gameplay::TryResourceTypeFromString("", parsed));
}

TEST_CASE("ResourceType - Gold is not part of the non-gold storage set") {
    REQUIRE(gameplay::IsGoldResource(ResourceType::Gold));
    REQUIRE(gameplay::kNonGoldResourceCount == 6);
    const bool goldListed = std::ranges::any_of(
        gameplay::kNonGoldResourceTypes,
        [](ResourceType t) { return gameplay::IsGoldResource(t); });
    REQUIRE_FALSE(goldListed);
}

// ---------------------------------------------------------------------------
// GameSession non-gold resource pool — add/spend/insufficient/zero-floor.
// ---------------------------------------------------------------------------

TEST_CASE("ResourceState - non-gold add and spend") {
    gameplay::GameSession session;
    REQUIRE(session.ResourceCount(ResourceType::Wood) == 0);

    session.AddResource(ResourceType::Wood, 5);
    REQUIRE(session.ResourceCount(ResourceType::Wood) == 5);

    REQUIRE(session.TrySpendResource(ResourceType::Wood, 3));
    REQUIRE(session.ResourceCount(ResourceType::Wood) == 2);
}

TEST_CASE("ResourceState - spending more than held fails and leaves state unchanged") {
    gameplay::GameSession session;
    session.AddResource(ResourceType::Stone, 2);

    REQUIRE_FALSE(session.TrySpendResource(ResourceType::Stone, 5));
    REQUIRE(session.ResourceCount(ResourceType::Stone) == 2);
}

TEST_CASE("ResourceState - zero or negative spend is a no-op that succeeds") {
    gameplay::GameSession session;
    session.AddResource(ResourceType::Gems, 1);

    REQUIRE(session.TrySpendResource(ResourceType::Gems, 0));
    REQUIRE(session.TrySpendResource(ResourceType::Gems, -10));
    REQUIRE(session.ResourceCount(ResourceType::Gems) == 1);
}

TEST_CASE("ResourceState - non-gold count never drops below zero (floor)") {
    gameplay::GameSession session;
    session.AddResource(ResourceType::Clay, 3);
    session.AddResource(ResourceType::Clay, -100);  // clamps, does not underflow
    REQUIRE(session.ResourceCount(ResourceType::Clay) == 0);
}

TEST_CASE("ResourceState - non-gold resources are independent") {
    gameplay::GameSession session;
    session.AddResource(ResourceType::Wood, 4);
    session.AddResource(ResourceType::Steel, 9);
    REQUIRE(session.ResourceCount(ResourceType::Wood) == 4);
    REQUIRE(session.ResourceCount(ResourceType::Steel) == 9);
    REQUIRE(session.ResourceCount(ResourceType::Fiber) == 0);
}

// ---------------------------------------------------------------------------
// Anti-split: ResourceType::Gold and the existing gold APIs are one value.
// ---------------------------------------------------------------------------

TEST_CASE("ResourceState - Gold resource API and gold_ APIs observe one value") {
    gameplay::GameSession session;
    const int startGold = session.Snapshot().gold;
    REQUIRE(session.ResourceCount(ResourceType::Gold) == startGold);

    // Add through the resource API; observe through the gold snapshot.
    session.AddResource(ResourceType::Gold, 100);
    REQUIRE(session.ResourceCount(ResourceType::Gold) == startGold + 100);
    REQUIRE(session.Snapshot().gold == startGold + 100);

    // Spend through the resource API; observe through the gold snapshot.
    REQUIRE(session.TrySpendResource(ResourceType::Gold, 50));
    REQUIRE(session.ResourceCount(ResourceType::Gold) == startGold + 50);
    REQUIRE(session.Snapshot().gold == startGold + 50);

    // Spend through the legacy gold API; observe through the resource API.
    REQUIRE(session.SpendGold(50));
    REQUIRE(session.ResourceCount(ResourceType::Gold) == startGold);
    REQUIRE(session.Snapshot().gold == startGold);
}

TEST_CASE("ResourceState - spending more gold than held fails through resource API") {
    gameplay::GameSession session;
    const int startGold = session.Snapshot().gold;
    REQUIRE_FALSE(session.TrySpendResource(ResourceType::Gold, startGold + 1));
    REQUIRE(session.Snapshot().gold == startGold);
}

// ---------------------------------------------------------------------------
// Gold is never serialized as a second resource entry.
// ---------------------------------------------------------------------------

TEST_CASE("ResourceState - Gold is not serialized into SaveData.resources") {
    gameplay::GameSession session;
    session.AddResource(ResourceType::Gold, 100);
    session.AddResource(ResourceType::Wood, 7);

    const core::SaveData save = session.ToSaveData();

    // Gold lives solely in the single gold field.
    REQUIRE(save.gold == session.Snapshot().gold);

    // No resource entry is ever named "Gold".
    const bool goldEntry = std::ranges::any_of(save.resources,
        [](const core::ResourceSaveState& r) { return r.resource == "Gold"; });
    REQUIRE_FALSE(goldEntry);

    // The non-gold resource is present exactly once.
    int woodEntries = 0;
    int woodAmount = 0;
    for (const auto& r : save.resources) {
        if (r.resource == "Wood") {
            ++woodEntries;
            woodAmount = r.amount;
        }
    }
    REQUIRE(woodEntries == 1);
    REQUIRE(woodAmount == 7);
}

// ---------------------------------------------------------------------------
// GameSession-level round-trip for resources and owned services.
// ---------------------------------------------------------------------------

TEST_CASE("ResourceState - resources round-trip through ToSaveData/ApplySaveData") {
    gameplay::GameSession source;
    source.AddResource(ResourceType::Wood, 3);
    source.AddResource(ResourceType::Gems, 12);

    const core::SaveData save = source.ToSaveData();

    gameplay::GameSession restored;
    restored.ApplySaveData(save);

    REQUIRE(restored.ResourceCount(ResourceType::Wood) == 3);
    REQUIRE(restored.ResourceCount(ResourceType::Gems) == 12);
    REQUIRE(restored.ResourceCount(ResourceType::Stone) == 0);
}

TEST_CASE("ResourceState - owned services round-trip through ApplySaveData/ToSaveData") {
    core::SaveData save;
    save.day = 1;
    save.minutesIntoSliceDay = 0;
    save.gold = 2500;
    save.mode = "region_mode";
    save.hasCanonicalRoster = true;
    save.activeSlotStackIds = {"", "", "", "", ""};
    save.reserveSlotStackIds = {"", "", "", "", "", "", "", ""};
    save.nextStackIdCounter = 1;
    save.ownedServices = {
        core::OwnedServiceSaveState{"stone_mine_svc", "Green", false, false},
        core::OwnedServiceSaveState{"vale_market_svc", "Green", true, false}
    };

    gameplay::GameSession session;
    session.ApplySaveData(save);

    REQUIRE(session.OwnedServices().size() == 2);
    const auto* mine = session.FindOwnedService("stone_mine_svc");
    REQUIRE(mine != nullptr);
    REQUIRE(mine->ownerTeamColor == "Green");
    REQUIRE_FALSE(mine->locked);

    const auto* market = session.FindOwnedService("vale_market_svc");
    REQUIRE(market != nullptr);
    REQUIRE(market->locked);

    // Re-serialize: stable fields survive the round-trip.
    const core::SaveData out = session.ToSaveData();
    REQUIRE(out.ownedServices.size() == 2);
    REQUIRE(out.ownedServices[0].serviceId == "stone_mine_svc");
    REQUIRE(out.ownedServices[1].locked);
}

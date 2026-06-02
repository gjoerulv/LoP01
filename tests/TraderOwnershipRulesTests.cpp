#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <string>
#include <vector>

#include "data/definitions/LocationServiceDefinition.h"
#include "data/definitions/TraderOwnershipCurve.h"
#include "gameplay/ResourceState.h"
#include "gameplay/economy/TraderOwnershipRules.h"

using data::LocationServiceKind;
using gameplay::ResourceType;
using gameplay::economy::CountOwnedServiceTier;
using gameplay::economy::DefaultTradingPostBarter;
using gameplay::economy::OwnedServiceTierCandidate;
using gameplay::economy::ResolvePriceFactor;
using gameplay::economy::ResolveTradingPostBarter;
using gameplay::economy::ResolvedExchange;

namespace {

OwnedServiceTierCandidate Cand(LocationServiceKind kind, const std::string& owner,
    bool locked = false, bool destroyed = false, bool hostile = false) {
    return OwnedServiceTierCandidate{kind, owner, locked, destroyed, hostile};
}

int CostFor(const std::vector<ResolvedExchange>& table, ResourceType from, ResourceType to) {
    for (const auto& e : table) {
        if (e.from == from && e.to == to) {
            return e.cost;
        }
    }
    return -1;
}

} // namespace

// ---------------------------------------------------------------------------
// Ownership-tier counting.
// ---------------------------------------------------------------------------

TEST_CASE("TraderTier - counts player-owned services of the requested type") {
    const std::vector<OwnedServiceTierCandidate> candidates = {
        Cand(LocationServiceKind::Market, "Green"),
        Cand(LocationServiceKind::Market, "Green"),
        Cand(LocationServiceKind::TradingPost, "Green")
    };
    REQUIRE(CountOwnedServiceTier(candidates, LocationServiceKind::Market, "Green") == 2);
}

TEST_CASE("TraderTier - tier is capped at 8") {
    std::vector<OwnedServiceTierCandidate> candidates;
    for (int i = 0; i < 12; ++i) {
        candidates.push_back(Cand(LocationServiceKind::Market, "Green"));
    }
    REQUIRE(CountOwnedServiceTier(candidates, LocationServiceKind::Market, "Green") == 8);
}

TEST_CASE("TraderTier - unowned and neutral services do not count") {
    const std::vector<OwnedServiceTierCandidate> candidates = {
        Cand(LocationServiceKind::Market, ""),         // unowned
        Cand(LocationServiceKind::Market, "Neutral")   // not the receiving team
    };
    REQUIRE(CountOwnedServiceTier(candidates, LocationServiceKind::Market, "Green") == 0);
}

TEST_CASE("TraderTier - allied-owned services do not count") {
    const std::vector<OwnedServiceTierCandidate> candidates = {
        Cand(LocationServiceKind::Market, "Blue")  // an allied team, not the receiver
    };
    REQUIRE(CountOwnedServiceTier(candidates, LocationServiceKind::Market, "Green") == 0);
}

TEST_CASE("TraderTier - enemy-owned services do not count") {
    const std::vector<OwnedServiceTierCandidate> candidates = {
        Cand(LocationServiceKind::Market, "Red")
    };
    REQUIRE(CountOwnedServiceTier(candidates, LocationServiceKind::Market, "Green") == 0);
}

TEST_CASE("TraderTier - locked and destroyed services do not count") {
    const std::vector<OwnedServiceTierCandidate> candidates = {
        Cand(LocationServiceKind::Market, "Green", /*locked=*/true),
        Cand(LocationServiceKind::Market, "Green", false, /*destroyed=*/true)
    };
    REQUIRE(CountOwnedServiceTier(candidates, LocationServiceKind::Market, "Green") == 0);
}

TEST_CASE("TraderTier - hostile-occupied services do not count") {
    const std::vector<OwnedServiceTierCandidate> candidates = {
        Cand(LocationServiceKind::Market, "Green", false, false, /*hostile=*/true)
    };
    REQUIRE(CountOwnedServiceTier(candidates, LocationServiceKind::Market, "Green") == 0);
}

TEST_CASE("TraderTier - counting is per type (Market ownership does not raise Trading Post tier)") {
    const std::vector<OwnedServiceTierCandidate> candidates = {
        Cand(LocationServiceKind::Market, "Green"),
        Cand(LocationServiceKind::Market, "Green"),
        Cand(LocationServiceKind::TradingPost, "Green")
    };
    REQUIRE(CountOwnedServiceTier(candidates, LocationServiceKind::TradingPost, "Green") == 1);
    REQUIRE(CountOwnedServiceTier(candidates, LocationServiceKind::Market, "Green") == 2);
}

// ---------------------------------------------------------------------------
// Trading Post barter matrix resolution and defaults.
// ---------------------------------------------------------------------------

TEST_CASE("TraderMatrix - default barter table matches the documented rates and forbids self-exchange") {
    const auto table = DefaultTradingPostBarter();

    // Documented examples (docs/core_loop_rules.md §23).
    REQUIRE(CostFor(table, ResourceType::Stone, ResourceType::Wood) == 10);   // pay Stone, buy Wood
    REQUIRE(CostFor(table, ResourceType::Wood, ResourceType::Steel) == 20);   // pay Wood, buy Steel
    REQUIRE(CostFor(table, ResourceType::Steel, ResourceType::Wood) == 5);    // pay Steel, buy Wood
    REQUIRE(CostFor(table, ResourceType::Wood, ResourceType::Gems) == 50);    // pay Wood, buy Gems
    REQUIRE(CostFor(table, ResourceType::Gems, ResourceType::Wood) == 2);     // pay Gems, buy Wood

    // No self-exchange and no Gold barter lines.
    const bool anySelf = std::ranges::any_of(table,
        [](const ResolvedExchange& e) { return e.from == e.to; });
    REQUIRE_FALSE(anySelf);
    const bool anyGold = std::ranges::any_of(table, [](const ResolvedExchange& e) {
        return e.from == ResourceType::Gold || e.to == ResourceType::Gold;
    });
    REQUIRE_FALSE(anyGold);
}

TEST_CASE("TraderMatrix - omitted authored matrix resolves to the default barter table") {
    // No curve authored -> default rates apply at any tier.
    const auto table = ResolveTradingPostBarter(nullptr, 5);
    REQUIRE(CostFor(table, ResourceType::Stone, ResourceType::Wood) == 10);
}

TEST_CASE("TraderMatrix - authored matrix resolves by tier (highest authored tier <= requested)") {
    data::TraderOwnershipCurve curve;
    curve.kind = LocationServiceKind::TradingPost;
    curve.rawType = "trading_post";

    data::TraderTierEntry t0;
    t0.tier = 0;
    t0.exchangeMatrix = {{"Stone", "Wood", 10}};
    data::TraderTierEntry t2;
    t2.tier = 2;
    t2.exchangeMatrix = {{"Stone", "Wood", 8}};  // better rate at higher ownership
    curve.tiers = {t0, t2};

    // Exact tier 2 -> tier-2 matrix.
    REQUIRE(CostFor(ResolveTradingPostBarter(&curve, 2), ResourceType::Stone, ResourceType::Wood) == 8);
    // Tier 1 -> highest authored <= 1 is tier 0.
    REQUIRE(CostFor(ResolveTradingPostBarter(&curve, 1), ResourceType::Stone, ResourceType::Wood) == 10);
    // Tier 5 -> highest authored <= 5 is tier 2.
    REQUIRE(CostFor(ResolveTradingPostBarter(&curve, 5), ResourceType::Stone, ResourceType::Wood) == 8);
}

TEST_CASE("TraderCurve - price factor resolves by tier with a default of 100") {
    data::TraderOwnershipCurve market;
    market.kind = LocationServiceKind::Market;
    market.rawType = "market";
    data::TraderTierEntry t1;
    t1.tier = 1;
    t1.priceFactor = 95;
    market.tiers = {t1};

    REQUIRE(ResolvePriceFactor(nullptr, 3) == 100);     // no curve -> default
    REQUIRE(ResolvePriceFactor(&market, 0) == 100);     // below authored -> default
    REQUIRE(ResolvePriceFactor(&market, 1) == 95);
    REQUIRE(ResolvePriceFactor(&market, 4) == 95);      // highest authored <= 4
}

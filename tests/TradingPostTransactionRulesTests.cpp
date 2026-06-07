#include <catch2/catch_test_macros.hpp>

#include <limits>
#include <vector>

#include "gameplay/ResourceState.h"
#include "gameplay/economy/TradingPostTransactionRules.h"
#include "gameplay/economy/TraderOwnershipRules.h"  // ResolvedExchange

using gameplay::ResourceType;
using gameplay::economy::BarterQuote;
using gameplay::economy::GoldTradeQuote;
using gameplay::economy::QuoteBarter;
using gameplay::economy::QuoteBuyResourceForGold;
using gameplay::economy::QuoteSellResourceForGold;
using gameplay::economy::ResolvedExchange;
using gameplay::economy::TradingPostResourceBaseValue;
using gameplay::economy::TradingPostUsable;

namespace {

constexpr int kMaxInt = std::numeric_limits<int>::max();

} // namespace

// ---------------------------------------------------------------------------
// Base values (README_DECISIONS §57).
// ---------------------------------------------------------------------------

TEST_CASE("TradingPostBaseValue - matches the documented gold reference values") {
    REQUIRE(TradingPostResourceBaseValue(ResourceType::Wood) == 100);
    REQUIRE(TradingPostResourceBaseValue(ResourceType::Stone) == 100);
    REQUIRE(TradingPostResourceBaseValue(ResourceType::Steel) == 200);
    REQUIRE(TradingPostResourceBaseValue(ResourceType::Fiber) == 200);
    REQUIRE(TradingPostResourceBaseValue(ResourceType::Clay) == 200);
    REQUIRE(TradingPostResourceBaseValue(ResourceType::Gems) == 500);
}

TEST_CASE("TradingPostBaseValue - Gold has no base value") {
    REQUIRE(TradingPostResourceBaseValue(ResourceType::Gold) == 0);
}

// ---------------------------------------------------------------------------
// Gold buy/sell at the default favorability factor (§57 default rates).
// ---------------------------------------------------------------------------

TEST_CASE("GoldTrade - buy at priceFactor 100 equals 5x base value") {
    REQUIRE(QuoteBuyResourceForGold(ResourceType::Wood, 1, 100).goldAmount == 500);
    REQUIRE(QuoteBuyResourceForGold(ResourceType::Stone, 1, 100).goldAmount == 500);
    REQUIRE(QuoteBuyResourceForGold(ResourceType::Steel, 1, 100).goldAmount == 1000);
    REQUIRE(QuoteBuyResourceForGold(ResourceType::Fiber, 1, 100).goldAmount == 1000);
    REQUIRE(QuoteBuyResourceForGold(ResourceType::Clay, 1, 100).goldAmount == 1000);
    REQUIRE(QuoteBuyResourceForGold(ResourceType::Gems, 1, 100).goldAmount == 2500);
}

TEST_CASE("GoldTrade - sell at priceFactor 100 equals 1/5 base value") {
    REQUIRE(QuoteSellResourceForGold(ResourceType::Wood, 1, 100).goldAmount == 20);
    REQUIRE(QuoteSellResourceForGold(ResourceType::Stone, 1, 100).goldAmount == 20);
    REQUIRE(QuoteSellResourceForGold(ResourceType::Steel, 1, 100).goldAmount == 40);
    REQUIRE(QuoteSellResourceForGold(ResourceType::Fiber, 1, 100).goldAmount == 40);
    REQUIRE(QuoteSellResourceForGold(ResourceType::Clay, 1, 100).goldAmount == 40);
    REQUIRE(QuoteSellResourceForGold(ResourceType::Gems, 1, 100).goldAmount == 100);
}

TEST_CASE("GoldTrade - quantity multiplies the gold amount") {
    REQUIRE(QuoteBuyResourceForGold(ResourceType::Wood, 3, 100).goldAmount == 1500);
    REQUIRE(QuoteSellResourceForGold(ResourceType::Gems, 4, 100).goldAmount == 400);
}

// ---------------------------------------------------------------------------
// Favorability scalar direction.
// ---------------------------------------------------------------------------

TEST_CASE("GoldTrade - priceFactor above 100 makes buying cheaper and selling richer") {
    const auto buy = QuoteBuyResourceForGold(ResourceType::Wood, 1, 200);
    const auto sell = QuoteSellResourceForGold(ResourceType::Wood, 1, 200);
    REQUIRE(buy.valid);
    REQUIRE(sell.valid);
    REQUIRE(buy.goldAmount == 250);   // cheaper than the 500 default
    REQUIRE(sell.goldAmount == 40);   // richer than the 20 default
}

TEST_CASE("GoldTrade - priceFactor below 100 makes buying dearer and selling poorer") {
    const auto buy = QuoteBuyResourceForGold(ResourceType::Wood, 1, 50);
    const auto sell = QuoteSellResourceForGold(ResourceType::Wood, 1, 50);
    REQUIRE(buy.valid);
    REQUIRE(sell.valid);
    REQUIRE(buy.goldAmount == 1000);  // dearer than the 500 default
    REQUIRE(sell.goldAmount == 10);   // poorer than the 20 default
}

// ---------------------------------------------------------------------------
// Rounding direction: the player never underpays on buy or over-receives on sell.
// ---------------------------------------------------------------------------

TEST_CASE("GoldTrade - buy rounds up") {
    // 100 * 5 * 1 * 100 / 150 = 50000 / 150 = 333.33 -> 334.
    REQUIRE(QuoteBuyResourceForGold(ResourceType::Wood, 1, 150).goldAmount == 334);
}

TEST_CASE("GoldTrade - sell rounds down") {
    // 100 * 1 * 133 / 500 = 13300 / 500 = 26.6 -> 26 (and richer than the 20 default).
    REQUIRE(QuoteSellResourceForGold(ResourceType::Wood, 1, 133).goldAmount == 26);
}

// ---------------------------------------------------------------------------
// Gold-trade invalid input.
// ---------------------------------------------------------------------------

TEST_CASE("GoldTrade - Gold is not a tradeable good") {
    REQUIRE_FALSE(QuoteBuyResourceForGold(ResourceType::Gold, 1, 100).valid);
    REQUIRE_FALSE(QuoteSellResourceForGold(ResourceType::Gold, 1, 100).valid);
}

TEST_CASE("GoldTrade - non-positive quantity is rejected") {
    REQUIRE_FALSE(QuoteBuyResourceForGold(ResourceType::Wood, 0, 100).valid);
    REQUIRE_FALSE(QuoteBuyResourceForGold(ResourceType::Wood, -1, 100).valid);
    REQUIRE_FALSE(QuoteSellResourceForGold(ResourceType::Wood, 0, 100).valid);
    REQUIRE_FALSE(QuoteSellResourceForGold(ResourceType::Wood, -1, 100).valid);
}

TEST_CASE("GoldTrade - non-positive priceFactor is rejected") {
    REQUIRE_FALSE(QuoteBuyResourceForGold(ResourceType::Wood, 1, 0).valid);
    REQUIRE_FALSE(QuoteBuyResourceForGold(ResourceType::Wood, 1, -5).valid);
    REQUIRE_FALSE(QuoteSellResourceForGold(ResourceType::Wood, 1, 0).valid);
    REQUIRE_FALSE(QuoteSellResourceForGold(ResourceType::Wood, 1, -5).valid);
}

TEST_CASE("GoldTrade - overflow yields an invalid quote, not a wrapped value") {
    // Buy result overflows int (fits in int64 but exceeds INT_MAX).
    REQUIRE_FALSE(QuoteBuyResourceForGold(ResourceType::Gems, kMaxInt, 1).valid);
    // Sell intermediate product overflows int64 (base * qty * priceFactor).
    REQUIRE_FALSE(QuoteSellResourceForGold(ResourceType::Gems, kMaxInt, kMaxInt).valid);
}

// ---------------------------------------------------------------------------
// Barter over a resolved matrix.
// ---------------------------------------------------------------------------

TEST_CASE("Barter - cost is the matrix rate times quantity") {
    const std::vector<ResolvedExchange> matrix = {
        ResolvedExchange{ResourceType::Wood, ResourceType::Stone, 7},
    };
    const auto quote = QuoteBarter(matrix, ResourceType::Wood, ResourceType::Stone, 3);
    REQUIRE(quote.valid);
    REQUIRE(quote.fromCost == 21);
}

TEST_CASE("Barter - Gold may not be the source") {
    const std::vector<ResolvedExchange> matrix = {
        ResolvedExchange{ResourceType::Gold, ResourceType::Stone, 7},
    };
    REQUIRE_FALSE(QuoteBarter(matrix, ResourceType::Gold, ResourceType::Stone, 1).valid);
}

TEST_CASE("Barter - Gold may not be the target") {
    const std::vector<ResolvedExchange> matrix = {
        ResolvedExchange{ResourceType::Wood, ResourceType::Gold, 7},
    };
    REQUIRE_FALSE(QuoteBarter(matrix, ResourceType::Wood, ResourceType::Gold, 1).valid);
}

TEST_CASE("Barter - a resource cannot be exchanged for itself") {
    const std::vector<ResolvedExchange> matrix = {
        ResolvedExchange{ResourceType::Wood, ResourceType::Wood, 7},
    };
    REQUIRE_FALSE(QuoteBarter(matrix, ResourceType::Wood, ResourceType::Wood, 1).valid);
}

TEST_CASE("Barter - an unknown pair is rejected") {
    const std::vector<ResolvedExchange> matrix = {
        ResolvedExchange{ResourceType::Wood, ResourceType::Stone, 7},
    };
    REQUIRE_FALSE(QuoteBarter(matrix, ResourceType::Steel, ResourceType::Gems, 1).valid);
}

TEST_CASE("Barter - non-positive quantity is rejected") {
    const std::vector<ResolvedExchange> matrix = {
        ResolvedExchange{ResourceType::Wood, ResourceType::Stone, 7},
    };
    REQUIRE_FALSE(QuoteBarter(matrix, ResourceType::Wood, ResourceType::Stone, 0).valid);
    REQUIRE_FALSE(QuoteBarter(matrix, ResourceType::Wood, ResourceType::Stone, -1).valid);
}

TEST_CASE("Barter - overflow yields an invalid quote") {
    const std::vector<ResolvedExchange> matrix = {
        ResolvedExchange{ResourceType::Wood, ResourceType::Stone, kMaxInt},
    };
    REQUIRE_FALSE(QuoteBarter(matrix, ResourceType::Wood, ResourceType::Stone, kMaxInt).valid);
}

// ---------------------------------------------------------------------------
// Usability gate (independent of ownership).
// ---------------------------------------------------------------------------

TEST_CASE("TradingPostUsable - usable only when no blocking flag is set") {
    REQUIRE(TradingPostUsable(false, false, false));
    REQUIRE_FALSE(TradingPostUsable(true, false, false));   // locked
    REQUIRE_FALSE(TradingPostUsable(false, true, false));   // destroyed
    REQUIRE_FALSE(TradingPostUsable(false, false, true));   // hostile-occupied
    REQUIRE_FALSE(TradingPostUsable(true, true, true));
}

#include "gameplay/economy/TradingPostTransactionRules.h"

#include <cstdint>
#include <limits>

namespace gameplay::economy {

namespace {

// Gold buy rate is 5x base value and the sell rate is 1/5 base value
// (README_DECISIONS §57). priceFactor (basis 100) scales favorability:
//   buy  = base * kGoldBuyRate  * 100 / priceFactor   (rounded up)
//   sell = base * priceFactor / (kGoldSellDivisor * 100)  (rounded down)
constexpr int64_t kGoldBuyRate = 5;
constexpr int64_t kGoldSellDivisor = 5;
constexpr int64_t kBasis = 100;

// Overflow-safe 64-bit multiply for the strictly-positive operands these quotes
// use (all inputs are validated > 0 before any arithmetic). The bound is checked
// BEFORE multiplying so the guard itself never triggers signed-overflow UB.
// Non-positive operands are rejected as a defensive contract violation.
[[nodiscard]] bool MulPositiveChecked(int64_t a, int64_t b, int64_t& out) {
    if (a <= 0 || b <= 0) {
        return false;
    }
    if (a > std::numeric_limits<int64_t>::max() / b) {
        return false;  // product would overflow; reject before multiplying
    }
    out = a * b;
    return true;
}

[[nodiscard]] bool FitsInt(int64_t value) {
    return value >= 0 && value <= std::numeric_limits<int>::max();
}

} // namespace

int TradingPostResourceBaseValue(ResourceType resource) {
    switch (resource) {
        case ResourceType::Wood:  return 100;
        case ResourceType::Stone: return 100;
        case ResourceType::Steel: return 200;
        case ResourceType::Fiber: return 200;
        case ResourceType::Clay:  return 200;
        case ResourceType::Gems:  return 500;
        case ResourceType::Gold:  return 0;  // currency, not a traded good
    }
    return 0;
}

GoldTradeQuote QuoteBuyResourceForGold(ResourceType resource, int quantity, int priceFactor) {
    GoldTradeQuote quote;
    if (IsGoldResource(resource) || quantity <= 0 || priceFactor <= 0) {
        return quote;
    }
    const int base = TradingPostResourceBaseValue(resource);
    if (base <= 0) {
        return quote;
    }

    int64_t numerator = base;
    if (!MulPositiveChecked(numerator, kGoldBuyRate, numerator) ||
        !MulPositiveChecked(numerator, quantity, numerator) ||
        !MulPositiveChecked(numerator, kBasis, numerator)) {
        return quote;
    }
    // Round up via quotient + remainder so the player never underpays, and so the
    // rounding cannot overflow (the quotient is <= numerator, and the +1 only
    // applies when priceFactor >= 2, keeping it far from the int64 ceiling).
    const int64_t cost =
        numerator / priceFactor + ((numerator % priceFactor != 0) ? 1 : 0);
    if (!FitsInt(cost)) {
        return quote;
    }
    quote.valid = true;
    quote.goldAmount = static_cast<int>(cost);
    return quote;
}

GoldTradeQuote QuoteSellResourceForGold(ResourceType resource, int quantity, int priceFactor) {
    GoldTradeQuote quote;
    if (IsGoldResource(resource) || quantity <= 0 || priceFactor <= 0) {
        return quote;
    }
    const int base = TradingPostResourceBaseValue(resource);
    if (base <= 0) {
        return quote;
    }

    int64_t numerator = base;
    if (!MulPositiveChecked(numerator, quantity, numerator) ||
        !MulPositiveChecked(numerator, priceFactor, numerator)) {
        return quote;
    }
    const int64_t gain = numerator / (kGoldSellDivisor * kBasis);  // round down
    if (!FitsInt(gain)) {
        return quote;
    }
    quote.valid = true;
    quote.goldAmount = static_cast<int>(gain);
    return quote;
}

BarterQuote QuoteBarter(
    const std::vector<ResolvedExchange>& matrix,
    ResourceType from, ResourceType to, int quantity) {
    BarterQuote quote;
    if (IsGoldResource(from) || IsGoldResource(to) || from == to || quantity <= 0) {
        return quote;
    }
    for (const auto& entry : matrix) {
        if (entry.from != from || entry.to != to) {
            continue;
        }
        if (entry.cost <= 0) {
            return quote;  // malformed matrix line; reject rather than give it away
        }
        int64_t cost = 0;
        if (!MulPositiveChecked(static_cast<int64_t>(entry.cost), quantity, cost) ||
            !FitsInt(cost)) {
            return quote;
        }
        quote.valid = true;
        quote.fromCost = static_cast<int>(cost);
        return quote;
    }
    return quote;  // pair not in the resolved matrix
}

bool TradingPostUsable(bool locked, bool destroyed, bool hostileOccupied) {
    return !locked && !destroyed && !hostileOccupied;
}

} // namespace gameplay::economy

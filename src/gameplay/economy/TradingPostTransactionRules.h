#pragma once

#include <vector>

#include "gameplay/ResourceState.h"
#include "gameplay/economy/TraderOwnershipRules.h"  // ResolvedExchange

namespace gameplay::economy {

// Trading Post pure transaction rules. Every function here is pure: no I/O, no
// global state, no GameSession dependency. They quote a single transaction; a
// later layer performs ownership/usability gating and the actual resource/Gold
// mutation. All integer math is overflow-guarded and never yields a negative
// quote; out-of-range or malformed input returns an invalid (valid == false)
// quote rather than a clamped or wrapped value.

// Gold reference value of a tradeable resource (README_DECISIONS §57). Gold is
// the currency, never a traded good, and has no base value (returns 0).
[[nodiscard]] int TradingPostResourceBaseValue(ResourceType resource);

struct GoldTradeQuote {
    bool valid = false;
    int goldAmount = 0;  // gold paid (buy) or received (sell)
};

// Gold cost to BUY `quantity` units of `resource`. `priceFactor` is the basis-100
// Gold-trade favorability scalar (README_DECISIONS §57/§58a): 100 = default rate
// (5x base value), > 100 = cheaper for the player. Rounds UP so the player never
// underpays. Invalid when `resource` is Gold, `quantity` <= 0, `priceFactor` <= 0,
// or the result would overflow int.
[[nodiscard]] GoldTradeQuote QuoteBuyResourceForGold(
    ResourceType resource, int quantity, int priceFactor);

// Gold received when SELLING `quantity` units of `resource`. `priceFactor` as
// above: 100 = default rate (1/5 base value), > 100 = richer for the player.
// Rounds DOWN so the player never over-receives. Same invalidity conditions as
// the buy quote.
[[nodiscard]] GoldTradeQuote QuoteSellResourceForGold(
    ResourceType resource, int quantity, int priceFactor);

struct BarterQuote {
    bool valid = false;
    int fromCost = 0;  // units of `from` spent to receive `quantity` of `to`
};

// Cost (in `from`) to barter for `quantity` of `to` over a resolved matrix.
// Barter is non-Gold resource-for-resource only (README_DECISIONS §56/§58a):
// Gold may not appear as `from` or `to`. Invalid when either side is Gold,
// `from == to`, `quantity` <= 0, the (from, to) pair is absent from `matrix` (or
// authored with a non-positive cost), or the result would overflow int.
[[nodiscard]] BarterQuote QuoteBarter(
    const std::vector<ResolvedExchange>& matrix,
    ResourceType from, ResourceType to, int quantity);

// A Trading Post is usable for a transaction iff none of these block it.
// Ownership never bypasses these gates (docs/core_loop_rules.md); ownership-tier
// benefit is resolved separately by the caller.
[[nodiscard]] bool TradingPostUsable(bool locked, bool destroyed, bool hostileOccupied);

} // namespace gameplay::economy

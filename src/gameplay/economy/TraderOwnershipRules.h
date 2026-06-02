#pragma once

#include <string>
#include <vector>

#include "data/definitions/LocationServiceDefinition.h"
#include "data/definitions/TraderOwnershipCurve.h"
#include "gameplay/ResourceState.h"

namespace gameplay::economy {

// Global ownership-tier cap (docs/core_loop_rules.md §23): owning more than 8
// services of a type grants no further tier.
inline constexpr int kMaxOwnershipTier = 8;

// One owned-service input for tier counting, already gated by the caller for
// everything except ownership/type (the caller resolves locked/destroyed from
// owned-service state and hostileOccupied from the occupation rules). Keeping
// these as pre-resolved fields keeps the counter pure and free of catalog or
// occupation lookups.
struct OwnedServiceTierCandidate {
    data::LocationServiceKind kind = data::LocationServiceKind::Unknown;
    std::string ownerTeamColor;
    bool locked = false;
    bool destroyed = false;
    bool hostileOccupied = false;
};

// Pure ownership-tier count for one trader type and one receiving team. Counts
// candidates that: match `traderKind`; are owned by `receivingTeamColor`
// (allied / enemy / neutral / unowned never count); and are not locked,
// destroyed, or hostile-occupied. Capped at kMaxOwnershipTier. Counting is per
// type, so other trader types never affect this result.
[[nodiscard]] int CountOwnedServiceTier(
    const std::vector<OwnedServiceTierCandidate>& candidates,
    data::LocationServiceKind traderKind,
    const std::string& receivingTeamColor);

// A resolved (typed) Trading Post barter line.
struct ResolvedExchange {
    ResourceType from = ResourceType::Wood;
    ResourceType to = ResourceType::Stone;
    int cost = 0;
};

// The documented default resource-for-resource barter table
// (docs/core_loop_rules.md §23). Used when no per-tier matrix is authored.
[[nodiscard]] std::vector<ResolvedExchange> DefaultTradingPostBarter();

// Resolves the effective Trading Post barter matrix for `tier`. Uses the
// authored curve's highest tier entry whose tier <= `tier` (if that entry has a
// non-empty matrix); otherwise falls back to DefaultTradingPostBarter(). A null
// curve always yields the default. Authored entries with an invalid resource are
// skipped defensively (validation rejects them at load).
[[nodiscard]] std::vector<ResolvedExchange> ResolveTradingPostBarter(
    const data::TraderOwnershipCurve* tradingPostCurve, int tier);

// Resolves the price/rate factor (basis-100) for a non-Trading-Post trader type
// at `tier`: the authored highest tier entry whose tier <= `tier`, else the
// default of 100 (no modifier). A null curve yields 100.
[[nodiscard]] int ResolvePriceFactor(
    const data::TraderOwnershipCurve* curve, int tier);

} // namespace gameplay::economy

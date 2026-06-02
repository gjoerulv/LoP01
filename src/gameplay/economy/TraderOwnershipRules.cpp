#include "gameplay/economy/TraderOwnershipRules.h"

#include <algorithm>
#include <array>

namespace gameplay::economy {

int CountOwnedServiceTier(
    const std::vector<OwnedServiceTierCandidate>& candidates,
    const data::LocationServiceKind traderKind,
    const std::string& receivingTeamColor) {
    int count = 0;
    for (const auto& c : candidates) {
        if (c.kind != traderKind) {
            continue;
        }
        if (c.ownerTeamColor.empty() || c.ownerTeamColor != receivingTeamColor) {
            continue;  // allied / enemy / neutral / unowned never count
        }
        if (c.locked || c.destroyed || c.hostileOccupied) {
            continue;
        }
        ++count;
    }
    return std::min(count, kMaxOwnershipTier);
}

namespace {

// Resource-class tiers for the default barter table: Wood/Stone = 0,
// Steel/Fiber/Clay = 1, Gems = 2.
[[nodiscard]] int ResourceClassTier(ResourceType r) {
    switch (r) {
        case ResourceType::Wood:
        case ResourceType::Stone:
            return 0;
        case ResourceType::Steel:
        case ResourceType::Fiber:
        case ResourceType::Clay:
            return 1;
        case ResourceType::Gems:
            return 2;
        case ResourceType::Gold:
            return -1;  // Gold is not part of resource-for-resource barter
    }
    return -1;
}

} // namespace

std::vector<ResolvedExchange> DefaultTradingPostBarter() {
    // Cost to buy 1 of target (column tier) paying with source (row tier),
    // straight from docs/core_loop_rules.md §23. Generated for every ordered
    // (from, to) pair of distinct non-gold resources.
    static constexpr int kCost[3][3] = {
        {10, 20, 50},
        { 5, 10, 25},
        { 2,  4, 10}
    };
    static constexpr std::array<ResourceType, 6> kBarterResources = {
        ResourceType::Wood, ResourceType::Stone, ResourceType::Steel,
        ResourceType::Fiber, ResourceType::Clay, ResourceType::Gems
    };

    std::vector<ResolvedExchange> table;
    table.reserve(kBarterResources.size() * (kBarterResources.size() - 1));
    for (const auto to : kBarterResources) {
        for (const auto from : kBarterResources) {
            if (from == to) {
                continue;  // a resource may not be exchanged for itself
            }
            const int payTier = ResourceClassTier(from);
            const int buyTier = ResourceClassTier(to);
            if (payTier < 0 || buyTier < 0) {
                continue;
            }
            table.push_back(ResolvedExchange{from, to, kCost[payTier][buyTier]});
        }
    }
    return table;
}

namespace {

// Highest authored tier entry whose tier <= requested; nullptr if none.
[[nodiscard]] const data::TraderTierEntry* ResolveTierEntry(
    const data::TraderOwnershipCurve& curve, int tier) {
    const data::TraderTierEntry* best = nullptr;
    for (const auto& entry : curve.tiers) {
        if (entry.tier <= tier && (best == nullptr || entry.tier > best->tier)) {
            best = &entry;
        }
    }
    return best;
}

} // namespace

std::vector<ResolvedExchange> ResolveTradingPostBarter(
    const data::TraderOwnershipCurve* tradingPostCurve, int tier) {
    if (tradingPostCurve != nullptr) {
        if (const auto* entry = ResolveTierEntry(*tradingPostCurve, tier);
            entry != nullptr && !entry->exchangeMatrix.empty()) {
            std::vector<ResolvedExchange> resolved;
            resolved.reserve(entry->exchangeMatrix.size());
            for (const auto& line : entry->exchangeMatrix) {
                ResourceType from;
                ResourceType to;
                if (TryResourceTypeFromString(line.from, from) &&
                    TryResourceTypeFromString(line.to, to) && from != to) {
                    resolved.push_back(ResolvedExchange{from, to, line.cost});
                }
            }
            return resolved;
        }
    }
    return DefaultTradingPostBarter();
}

int ResolvePriceFactor(const data::TraderOwnershipCurve* curve, int tier) {
    if (curve != nullptr) {
        if (const auto* entry = ResolveTierEntry(*curve, tier); entry != nullptr) {
            return entry->priceFactor;
        }
    }
    return 100;
}

} // namespace gameplay::economy

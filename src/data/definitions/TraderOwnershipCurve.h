#pragma once

#include <string>
#include <vector>

#include "data/definitions/LocationServiceDefinition.h"

namespace data
{
    // M17 Phase 4 authored trader ownership economy.
    //
    // Curves are per trader service type and capped at 8 owned services (the
    // global ownership-tier cap). Resources are authored as canonical
    // ResourceType names (validated; converted to the gameplay enum at use).

    // One Trading Post barter line: cost (in units of `from`) to buy one `to`.
    // A resource may never be exchanged for itself.
    struct TraderExchangeEntry
    {
        std::string from;
        std::string to;
        int cost = 0;
    };

    // One ownership-tier entry. For Trading Post, `exchangeMatrix` holds that
    // tier's barter rates. For the other trader types the curve shape is not
    // specified by the docs beyond "prices/rates", so `priceFactor` is a narrow
    // placeholder scalar in basis-100 (100 = no modifier).
    struct TraderTierEntry
    {
        int tier = 0;
        std::vector<TraderExchangeEntry> exchangeMatrix;
        int priceFactor = 100;
    };

    // An authored ownership curve for one trader service type. `kind` must be a
    // trader kind (see IsTraderServiceKind); `rawType` preserves the authored
    // string for validation messages.
    struct TraderOwnershipCurve
    {
        LocationServiceKind kind = LocationServiceKind::Unknown;
        std::string rawType;
        std::vector<TraderTierEntry> tiers;
    };
}

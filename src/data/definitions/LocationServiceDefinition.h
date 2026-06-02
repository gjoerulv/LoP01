#pragma once

#include <string>
#include <vector>

namespace data
{
    enum class LocationServiceKind
    {
        Unknown,
        Rest,
        Shop,
        Recruit,
        Muster,
        // M17 Phase 2: a resource-producing mine/resource service. Distinguishes
        // economy-foundation services from the existing interaction services
        // above. Trader service types (Market, Trading Post, ...) are a later
        // phase and are intentionally not represented here yet.
        Mine
    };

    inline LocationServiceKind LocationServiceKindFromString(const std::string& value)
    {
        if (value == "rest")    return LocationServiceKind::Rest;
        if (value == "shop")    return LocationServiceKind::Shop;
        if (value == "recruit") return LocationServiceKind::Recruit;
        if (value == "muster")  return LocationServiceKind::Muster;
        if (value == "mine")    return LocationServiceKind::Mine;
        return LocationServiceKind::Unknown;
    }

    // M17 Phase 2: one authored base daily output line for a mine/resource
    // service. `resource` is a canonical ResourceType name (e.g. "Stone",
    // "Gold"); it is stored as a string in the data layer and converted to the
    // gameplay ResourceType when the pure output rule runs. `amount` is the
    // authored base amount per day (validated positive).
    struct MineOutputDefinition
    {
        std::string resource;
        int amount = 0;
    };

    enum class RestServiceKind
    {
        Unknown,
        HomeBase,
        Inn
    };

    inline RestServiceKind RestServiceKindFromString(const std::string& value)
    {
        if (value == "home_base") return RestServiceKind::HomeBase;
        if (value == "inn")       return RestServiceKind::Inn;
        return RestServiceKind::Unknown;
    }

    struct LocationServiceDefinition
    {
        std::string id;
        std::string locationId;
        std::string zoneId;
        LocationServiceKind kind = LocationServiceKind::Unknown;

        std::string promptText;
        std::string successText;
        std::string failureText;

        int goldCost = 0;
        int timeCostMinutes = 0;

        RestServiceKind restKind = RestServiceKind::Unknown;

        std::string unitId;
        std::string unitDisplayName;
        int weeklyStock = 0;

        int dailyUseLimit = 0;
        int travelPrepDiscountMinutes = 0;
        int travelPrepCharges = 0;

        // M17 Phase 2: authored base daily outputs for mine/resource services.
        // Empty for all non-mine services (backward compatible). Runtime payout
        // is NOT computed here — see gameplay::economy::ComputeMineDailyOutput.
        std::vector<MineOutputDefinition> mineOutputs;
    };
}
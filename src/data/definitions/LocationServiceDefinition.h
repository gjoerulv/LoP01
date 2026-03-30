#pragma once

#include <string>

namespace data
{
    enum class LocationServiceKind
    {
        Unknown,
        Rest,
        Shop,
        Recruit,
        Muster
    };

    inline LocationServiceKind LocationServiceKindFromString(const std::string& value)
    {
        if (value == "rest")    return LocationServiceKind::Rest;
        if (value == "shop")    return LocationServiceKind::Shop;
        if (value == "recruit") return LocationServiceKind::Recruit;
        if (value == "muster")  return LocationServiceKind::Muster;
        return LocationServiceKind::Unknown;
    }

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
    };
}
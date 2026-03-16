#pragma once

#include <string>

namespace data
{
    enum class LocationType
    {
        Unknown,
        Home,
        Town,
        Inn,
        Dungeon,
        Service,
        Recruit,
        Combat
    };

    struct LocationDefinition
    {
        std::string id;
        std::string name;
        LocationType type = LocationType::Unknown;
        bool overworldDestination = true;
        bool enterable = true;
    };
}
#pragma once

#include <string>
#include <vector>

namespace data
{
    struct RegionNodeDefinition
    {
        std::string locationId;
        float x = 0.0f;
        float y = 0.0f;
        bool discovered = true;
        bool travelAvailable = true;
    };

    struct RegionLinkDefinition
    {
        std::string fromLocationId;
        std::string toLocationId;
    };

    struct RegionDefinition
    {
        std::string id;
        std::string name;
        bool unlocked = false;
        std::vector<RegionNodeDefinition> nodes;
        std::vector<RegionLinkDefinition> links;
    };
}
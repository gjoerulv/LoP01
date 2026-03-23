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

    inline LocationType LocationTypeFromString(const std::string& value)
    {
        if (value == "home")    return LocationType::Home;
        if (value == "town")    return LocationType::Town;
        if (value == "inn")     return LocationType::Inn;
        if (value == "dungeon") return LocationType::Dungeon;
        if (value == "service") return LocationType::Service;
        if (value == "recruit") return LocationType::Recruit;
        if (value == "combat")  return LocationType::Combat;
        return LocationType::Unknown;
    }

    inline std::string ToDisplayString(const LocationType type)
    {
        switch (type)
        {
        case LocationType::Home:    return "Home";
        case LocationType::Town:    return "Town";
        case LocationType::Inn:     return "Inn";
        case LocationType::Dungeon: return "Dungeon";
        case LocationType::Service: return "Service";
        case LocationType::Recruit: return "Recruit";
        case LocationType::Combat:  return "Combat";
        default:                    return "Unknown";
        }
    }

    inline bool SupportsBattleScenario(const LocationType type)
    {
        return type == LocationType::Combat ||
            type == LocationType::Dungeon;
    }

    struct LocationDefinition
    {
        std::string id;
        std::string name;
        LocationType type = LocationType::Unknown;
        bool allowsSleep = false;
        bool blocksTransitUntilCleared = false;
        bool overworldDestination = true;
        std::string sceneId;
        std::string battleScenarioId;
    };

    inline bool EntersLocationMode(const LocationDefinition& location)
    {
        return !location.sceneId.empty();
    }
}
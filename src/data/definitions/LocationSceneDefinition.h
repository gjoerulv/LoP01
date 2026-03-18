#pragma once

#include <string>
#include <vector>

namespace data
{
    enum class LocationSceneZoneType
    {
        Unknown,
        InnDoor,
        Shop,
        Recruit,
        Npc,
        Exit
    };

    inline LocationSceneZoneType LocationSceneZoneTypeFromString(const std::string& value)
    {
        if (value == "inn_door") return LocationSceneZoneType::InnDoor;
        if (value == "shop")     return LocationSceneZoneType::Shop;
        if (value == "recruit")  return LocationSceneZoneType::Recruit;
        if (value == "npc")      return LocationSceneZoneType::Npc;
        if (value == "exit")     return LocationSceneZoneType::Exit;
        return LocationSceneZoneType::Unknown;
    }

    struct SceneRectDefinition
    {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
    };

    struct LocationSceneZoneDefinition
    {
        std::string id;
        LocationSceneZoneType type = LocationSceneZoneType::Unknown;
        SceneRectDefinition area;

        std::string promptText;
        std::string resultText;
        std::string failureText;

        int timeCostMinutes = 0;
        int goldCost = 0;
        int recruitCount = 0;
        int dialogueChoiceTimeCostMinutes = 1;

        std::vector<std::string> dialogueChoices;
    };

    struct LocationSceneDefinition
    {
        std::string id;
        SceneRectDefinition spawn;
        std::vector<SceneRectDefinition> blockingRects;
        std::vector<LocationSceneZoneDefinition> zones;
    };
}
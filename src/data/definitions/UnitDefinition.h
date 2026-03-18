#pragma once

#include <string>

namespace data
{
    enum class UnitDefinitionCategory
    {
        Leader,
        Hero,
        Generic
    };

    enum class UnitDefinitionPosition
    {
        Front,
        Middle,
        Back,
        Leader
    };

    enum class UnitDefinitionRange
    {
        Melee,
        Ranged,
        LongRanged
    };

    inline UnitDefinitionCategory UnitDefinitionCategoryFromString(const std::string& value)
    {
        if (value == "leader") return UnitDefinitionCategory::Leader;
        if (value == "hero")   return UnitDefinitionCategory::Hero;
        return UnitDefinitionCategory::Generic;
    }

    inline UnitDefinitionPosition UnitDefinitionPositionFromString(const std::string& value)
    {
        if (value == "front")  return UnitDefinitionPosition::Front;
        if (value == "middle") return UnitDefinitionPosition::Middle;
        if (value == "back")   return UnitDefinitionPosition::Back;
        return UnitDefinitionPosition::Leader;
    }

    inline UnitDefinitionRange UnitDefinitionRangeFromString(const std::string& value)
    {
        if (value == "melee")       return UnitDefinitionRange::Melee;
        if (value == "ranged")      return UnitDefinitionRange::Ranged;
        return UnitDefinitionRange::LongRanged;
    }

    struct UnitStatsDefinition
    {
        int attack = 0;
        int defense = 0;
        int magic = 0;
        int resistance = 0;
        int minDamage = 1;
        int maxDamage = 1;
        int maxHp = 1;
        int maxMp = 0;
        int agility = 1;
        int life = 1;
        UnitDefinitionPosition position = UnitDefinitionPosition::Front;
        UnitDefinitionRange range = UnitDefinitionRange::Melee;
    };

    struct UnitDefinition
    {
        std::string id;
        std::string name;
        UnitDefinitionCategory category = UnitDefinitionCategory::Generic;
        bool isPlayerCharacter = false;
        UnitStatsDefinition stats;
    };
}
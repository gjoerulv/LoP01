#pragma once

#include <optional>
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

    // M17 Phase 3a: a narrow, flat unit passive used only for mine/resource
    // production. This is NOT a general passive system or skill tree — it is a
    // single optional production modifier per unit. A unit (hero, leader, or
    // generic) is a valid production-boosting stationed guard iff its definition
    // carries this passive; category is never consulted.
    //
    //   target   - producing-service kind the passive applies to. Only "mine"
    //              is meaningful in M17; validated explicitly.
    //   resource - canonical ResourceType name (e.g. "Stone") the bonus applies
    //              to; validated strict.
    //   amount   - additive per-day bonus to that resource's output; validated
    //              positive. Resolved strongest-only / non-stacking at payout.
    struct UnitMineProductionPassive
    {
        std::string target = "mine";
        std::string resource;
        int amount = 0;
    };

    struct UnitDefinition
    {
        std::string id;
        std::string name;
        UnitDefinitionCategory category = UnitDefinitionCategory::Generic;
        bool isPlayerCharacter = false;
        UnitStatsDefinition stats;

        // M17 Phase 3a: present only when the unit authored a mine-production
        // passive. Absent for every existing unit (backward compatible).
        std::optional<UnitMineProductionPassive> mineProductionPassive;
    };
}
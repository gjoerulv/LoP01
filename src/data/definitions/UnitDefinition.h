#pragma once

#include <string>
#include <vector>

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

    // Typed unit passive effect. Each effect's kind names its consumer; the
    // spine is intentionally narrow (only kinds with an active consumer exist).
    // Category (hero/leader/generic) is never consulted for eligibility.
    //
    //   MineProduction - additive per-day bonus (`amount`) to a mine/resource
    //     service's output of `resource`; `target` is the producing-service kind
    //     ("mine"). Resolved strongest-only / non-stacking per owned-service
    //     instance and output resource at payout.
    //   LeaderEnergy - additive bonus (`amount`) to the team's daily starting
    //     Energy, applied only when the unit is the current leader. Carries no
    //     resource/target.
    // `amount` is validated positive for both kinds.
    enum class PassiveEffectKind { Unknown, MineProduction, LeaderEnergy };

    inline PassiveEffectKind PassiveEffectKindFromString(const std::string& value)
    {
        if (value == "mine_production") return PassiveEffectKind::MineProduction;
        if (value == "leader_energy")   return PassiveEffectKind::LeaderEnergy;
        return PassiveEffectKind::Unknown;
    }

    struct UnitPassiveEffect
    {
        PassiveEffectKind kind = PassiveEffectKind::Unknown;
        std::string resource;   // MineProduction only
        std::string target;     // MineProduction only ("mine")
        int amount = 0;
    };

    struct UnitDefinition
    {
        std::string id;
        std::string name;
        UnitDefinitionCategory category = UnitDefinitionCategory::Generic;
        bool isPlayerCharacter = false;
        UnitStatsDefinition stats;

        // Canonical typed passive effects. Empty for units authoring none. The
        // legacy `mine_production_passive` JSON key is converted into a
        // MineProduction entry here at load; runtime reads only this vector.
        std::vector<UnitPassiveEffect> passiveEffects;
    };
}
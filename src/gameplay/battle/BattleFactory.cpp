#include "gameplay/battle/BattleFactory.h"

namespace gameplay::battle
{
    namespace
    {
        UnitCategory ToBattleCategory(const data::UnitDefinitionCategory category)
        {
            switch (category)
            {
            case data::UnitDefinitionCategory::Leader: return UnitCategory::Leader;
            case data::UnitDefinitionCategory::Hero:   return UnitCategory::Hero;
            default:                                   return UnitCategory::Generic;
            }
        }

        UnitPosition ToBattlePosition(const data::UnitDefinitionPosition position)
        {
            switch (position)
            {
            case data::UnitDefinitionPosition::Front:  return UnitPosition::Front;
            case data::UnitDefinitionPosition::Middle: return UnitPosition::Middle;
            case data::UnitDefinitionPosition::Back:   return UnitPosition::Back;
            default:                                   return UnitPosition::Leader;
            }
        }

        UnitRange ToBattleRange(const data::UnitDefinitionRange range)
        {
            switch (range)
            {
            case data::UnitDefinitionRange::Melee:      return UnitRange::Melee;
            case data::UnitDefinitionRange::Ranged:     return UnitRange::Ranged;
            default:                                    return UnitRange::LongRanged;
            }
        }

        UnitStats ToBattleStats(const data::UnitStatsDefinition& stats)
        {
            UnitStats result;
            result.attack = stats.attack;
            result.defense = stats.defense;
            result.magic = stats.magic;
            result.resistance = stats.resistance;
            result.minDamage = stats.minDamage;
            result.maxDamage = stats.maxDamage;
            result.maxHp = stats.maxHp;
            result.maxMp = stats.maxMp;
            result.agility = stats.agility;
            result.life = stats.life;
            result.position = ToBattlePosition(stats.position);
            result.range = ToBattleRange(stats.range);
            return result;
        }

        BattleUnit BuildBattleUnit(
            const data::UnitDefinition& definition,
            const TeamSide side,
            const int lifeOverride)
        {
            BattleUnit unit;
            unit.id = definition.id;
            unit.name = definition.name;
            unit.side = side;
            unit.category = ToBattleCategory(definition.category);
            unit.isPlayerCharacter = definition.isPlayerCharacter;
            unit.stats = ToBattleStats(definition.stats);
            unit.hp = unit.stats.maxHp;
            unit.mp = unit.stats.maxMp;
            unit.life = lifeOverride > 0 ? lifeOverride : unit.stats.life;
            return unit;
        }

        void AppendUnits(
            std::vector<BattleUnit>& output,
            const std::vector<data::BattleScenarioEntryDefinition>& entries,
            const TeamSide side,
            const data::ContentRepository& content)
        {
            for (const auto& entry : entries)
            {
                if (const auto* definition = content.FindUnitById(entry.unitId))
                {
                    output.push_back(BuildBattleUnit(*definition, side, entry.lifeOverride));
                }
            }
        }
    }

    std::optional<BattleState> BattleFactory::CreateFromScenario(
        const data::ContentRepository& content,
        const std::string& scenarioId,
        const std::vector<std::string>& activePartyUnitIds,
        const uint32_t seed)
    {
        const auto* scenario = content.FindBattleScenarioById(scenarioId);
        if (scenario == nullptr)
        {
            return std::nullopt;
        }

        std::vector<BattleUnit> resolvedActivePartyAllies;
        resolvedActivePartyAllies.reserve(activePartyUnitIds.size());
        for (const auto& unitId : activePartyUnitIds)
        {
            if (const auto* definition = content.FindUnitById(unitId))
            {
                resolvedActivePartyAllies.push_back(BuildBattleUnit(*definition, TeamSide::Allies, 0));
            }
        }

        const bool useActivePartyOverride = !resolvedActivePartyAllies.empty();

        std::vector<BattleUnit> units;
        const size_t allyCount = useActivePartyOverride
            ? resolvedActivePartyAllies.size()
            : scenario->allies.size();
        units.reserve(allyCount + scenario->enemies.size());

        if (useActivePartyOverride)
        {
            units.insert(units.end(), resolvedActivePartyAllies.begin(), resolvedActivePartyAllies.end());
        }
        else
        {
            AppendUnits(units, scenario->allies, TeamSide::Allies, content);
        }

        AppendUnits(units, scenario->enemies, TeamSide::Enemies, content);

        BattleState state(seed == 0 ? scenario->seed : seed);
        if (!state.SetUnits(std::move(units)))
        {
            return std::nullopt;
        }

        return state;
    }
}
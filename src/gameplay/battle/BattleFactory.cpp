#include "gameplay/battle/BattleFactory.h"

#include <algorithm>

namespace gameplay::battle
{
    namespace
    {
        bool IsLeaderCapable(const BattleUnit& unit)
        {
            return unit.category == UnitCategory::Leader || unit.category == UnitCategory::Hero;
        }

        bool AssignLeader(
            std::vector<BattleUnit>& team,
            const bool preferPlayerCharacter,
            const bool requireLeader)
        {
            for (auto& unit : team) {
                unit.isAssignedLeader = false;
            }

            int leaderIndex = -1;

            if (preferPlayerCharacter) {
                for (int i = 0; i < static_cast<int>(team.size()); ++i) {
                    if (team[i].isPlayerCharacter && IsLeaderCapable(team[i])) {
                        leaderIndex = i;
                        break;
                    }
                }
            }

            if (leaderIndex < 0) {
                for (int i = 0; i < static_cast<int>(team.size()); ++i) {
                    if (IsLeaderCapable(team[i])) {
                        leaderIndex = i;
                        break;
                    }
                }
            }

            if (leaderIndex < 0) {
                return !requireLeader;
            }

            team[leaderIndex].isAssignedLeader = true;
            team[leaderIndex].stats.position = UnitPosition::Leader;
            return true;
        }

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
            const int lifeOverride,
            const std::string& rosterStackId)
        {
            BattleUnit unit;
            unit.id = definition.id;
            unit.name = definition.name;
            unit.rosterStackId = rosterStackId;
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
                    output.push_back(BuildBattleUnit(*definition, side, entry.lifeOverride, ""));
                }
            }
        }
    }

    std::optional<BattleState> BattleFactory::CreateFromScenario(
        const data::ContentRepository& content,
        const std::string& scenarioId,
        const std::vector<PlayerBattleEntry>& activePartyEntries,
        const uint32_t seed)
    {
        const auto* scenario = content.FindBattleScenarioById(scenarioId);
        if (scenario == nullptr)
        {
            return std::nullopt;
        }

        std::vector<BattleUnit> resolvedActivePartyAllies;
        std::vector<PlayerBattleEntry> sortedEntries = activePartyEntries;
        std::sort(sortedEntries.begin(), sortedEntries.end(), [](const PlayerBattleEntry& left, const PlayerBattleEntry& right) {
            return left.activeSlotIndex < right.activeSlotIndex;
        });

        resolvedActivePartyAllies.reserve(sortedEntries.size());
        for (const auto& entry : sortedEntries)
        {
            if (entry.activeSlotIndex < 0 || entry.stackId.empty() || entry.unitId.empty() || entry.quantity <= 0) {
                continue;
            }

            if (const auto* definition = content.FindUnitById(entry.unitId))
            {
                resolvedActivePartyAllies.push_back(BuildBattleUnit(*definition, TeamSide::Allies, entry.quantity, entry.stackId));
            }
        }

        const bool useActivePartyOverride = !resolvedActivePartyAllies.empty();

        std::vector<BattleUnit> allies;
        if (useActivePartyOverride)
        {
            allies = std::move(resolvedActivePartyAllies);
        }
        else
        {
            AppendUnits(allies, scenario->allies, TeamSide::Allies, content);
        }

        std::vector<BattleUnit> enemies;
        AppendUnits(enemies, scenario->enemies, TeamSide::Enemies, content);

        if (!AssignLeader(allies, true, true)) {
            return std::nullopt;
        }

        if (!AssignLeader(enemies, false, false)) {
            return std::nullopt;
        }

        std::vector<BattleUnit> units;
        units.reserve(allies.size() + enemies.size());
        units.insert(units.end(), allies.begin(), allies.end());
        units.insert(units.end(), enemies.begin(), enemies.end());

        BattleState state(seed == 0 ? scenario->seed : seed);
        if (!state.SetUnits(std::move(units)))
        {
            return std::nullopt;
        }

        return state;
    }
}
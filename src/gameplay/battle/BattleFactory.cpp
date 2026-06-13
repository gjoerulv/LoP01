#include "gameplay/battle/BattleFactory.h"

#include <algorithm>

#include "gameplay/battle/BattleUnitConversion.h"

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

        BattleUnit BuildBattleUnit(
            const data::UnitDefinition& definition,
            const TeamSide side,
            const int lifeOverride,
            const std::string& rosterStackId,
            int artifactAttackBonus = 0,
            int artifactDefenseBonus = 0,
            int artifactMagicBonus = 0,
            int artifactResistanceBonus = 0)
        {
            BattleUnit unit;
            unit.id = definition.id;
            unit.name = definition.name;
            unit.rosterStackId = rosterStackId;
            unit.side = side;
            unit.category = ToBattleCategory(definition.category);
            unit.isPlayerCharacter = definition.isPlayerCharacter;
            unit.stats = ToBattleStats(definition.stats);
            // M13-b: equipped-artifact statBonus values add on top of the
            // unit's authored stats for this battle only. The persistent
            // UnitDefinition is never mutated. Bonuses are pre-summed by
            // GameSession::BuildActiveBattleStackEntries and forwarded
            // through PlayerBattleEntry.
            unit.stats.attack     += artifactAttackBonus;
            unit.stats.defense    += artifactDefenseBonus;
            unit.stats.magic      += artifactMagicBonus;
            unit.stats.resistance += artifactResistanceBonus;
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
                resolvedActivePartyAllies.push_back(BuildBattleUnit(
                    *definition,
                    TeamSide::Allies,
                    entry.quantity,
                    entry.stackId,
                    entry.artifactAttackBonus,
                    entry.artifactDefenseBonus,
                    entry.artifactMagicBonus,
                    entry.artifactResistanceBonus));
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
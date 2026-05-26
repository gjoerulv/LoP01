#pragma once

#include <optional>
#include <string>
#include <vector>

#include "data/ContentRepository.h"
#include "gameplay/battle/Battle.h"

namespace gameplay::battle
{
    struct PlayerBattleEntry {
        int activeSlotIndex = -1;
        std::string stackId;
        std::string unitId;
        int quantity = 0;
        // M13-b: pre-summed equipped-artifact stat bonuses applied to per-battle
        // hero stats during construction. Zero by default; the caller (App
        // copies from GameSession::BuildActiveBattleStackEntries) sets these
        // for hero entries that have equipped artifacts. The factory adds the
        // bonuses on top of the unit's authored stats without mutating the
        // persistent UnitDefinition.
        int artifactAttackBonus = 0;
        int artifactDefenseBonus = 0;
        int artifactMagicBonus = 0;
        int artifactResistanceBonus = 0;
    };

    class BattleFactory
    {
    public:
        [[nodiscard]] static std::optional<BattleState> CreateFromScenario(
            const data::ContentRepository& content,
            const std::string& scenarioId,
            const std::vector<PlayerBattleEntry>& activePartyEntries,
            uint32_t seed = 7);
    };
}
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
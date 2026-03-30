#pragma once

#include <optional>
#include <string>
#include <vector>

#include "data/ContentRepository.h"
#include "gameplay/battle/Battle.h"

namespace gameplay::battle
{
    class BattleFactory
    {
    public:
        [[nodiscard]] static std::optional<BattleState> CreateFromScenario(
            const data::ContentRepository& content,
            const std::string& scenarioId,
            const std::vector<std::string>& activePartyUnitIds,
            uint32_t seed = 7);
    };
}
#pragma once

#include "gameplay/battle/Battle.h"
#include "rendering/BattleRenderer.h"

namespace app::mappers
{
    class BattleModelMapper
    {
    public:
        [[nodiscard]] ashvale::rendering::BattleRenderModel Map(
            const gameplay::battle::BattleState& battle,
            int selectedActionIndex,
            int selectedTargetIndex,
            const std::string& statusText) const;
    };
}
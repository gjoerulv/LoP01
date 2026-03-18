#pragma once

#include "app/input/InputState.h"
#include "gameplay/battle/Battle.h"

namespace app
{
    struct BattleControllerState
    {
        int selectedActionIndex = 0;
        int selectedTargetIndex = -1;
    };

    struct BattleUpdateResult
    {
        BattleControllerState state;
        bool executeAction = false;
        gameplay::battle::BattleActionType action = gameplay::battle::BattleActionType::Attack;
        int targetIndex = -1;
    };

    class BattleController
    {
    public:
        [[nodiscard]] BattleUpdateResult Update(
            const input::InputState& input,
            const gameplay::battle::BattleState& battle,
            const BattleControllerState& state) const;
    };
}
#pragma once

#include "app/input/InputState.h"

namespace app
{
    struct OverworldUpdateResult
    {
        int selectedNodeIndex = 0;
        bool travelConfirmed = false;
        bool travelCancelled = false;
        bool requestDebugBattle = false;
    };

    class OverworldController
    {
    public:
        [[nodiscard]] OverworldUpdateResult Update(
            const input::InputState& input,
            int nodeCount,
            int currentIndex,
            int selectedIndex) const;
    };
}
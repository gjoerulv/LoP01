#pragma once

#include "app/input/InputState.h"

namespace app
{
    struct RegionUpdateResult
    {
        int selectedNodeIndex = 0;
        bool travelConfirmed = false;
        bool travelCancelled = false;
        bool requestDebugBattle = false;
    };

    class RegionController
    {
    public:
        [[nodiscard]] RegionUpdateResult Update(
            const input::InputState& input,
            int nodeCount,
            int currentIndex,
            int selectedIndex) const;
    };
}
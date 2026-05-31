#pragma once

#include "app/input/InputState.h"

namespace app
{
    struct WorldMapUpdateResult
    {
        int selectedIndex = 0;
        bool travelConfirmed = false;
        bool cancelled = false;
    };

    // Pure select-then-confirm controller for the World Map screen. Mirrors
    // RegionController: left/right wrap the selection, confirm commits, cancel
    // backs out. Holds no state.
    class WorldMapController
    {
    public:
        [[nodiscard]] WorldMapUpdateResult Update(
            const input::InputState& input,
            int destinationCount,
            int selectedIndex) const;
    };
}

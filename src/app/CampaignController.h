#pragma once

#include "app/input/InputState.h"

namespace app
{
    struct CampaignSelectUpdateResult
    {
        int selectedIndex = 0;
        bool confirmed = false;
        bool cancelled = false;
    };

    // Pure select-then-confirm controller for the Campaign Selection screen.
    // Mirrors WorldMapController: prev/next wrap the selection, confirm commits,
    // cancel backs out. Holds no state.
    class CampaignController
    {
    public:
        [[nodiscard]] CampaignSelectUpdateResult Update(
            const input::InputState& input,
            int campaignCount,
            int selectedIndex) const;
    };
}

#pragma once

#include "app/input/InputState.h"

namespace app
{
    struct WorldMapUpdateResult
    {
        int selectedIndex = 0;
        bool travelConfirmed = false;
        // Confirm pressed on a legal destination that would lose generic units:
        // show the loss warning instead of traveling (App enters pending state).
        bool requestLossConfirmation = false;
        // Leave the pending loss warning and stay on the World Map screen
        // (cancel pressed, or the selection moved off the warned destination).
        bool dismissLossConfirmation = false;
        bool cancelled = false;
    };

    // Pure select-then-confirm controller for the World Map screen. Mirrors
    // RegionController: left/right wrap the selection, confirm commits, cancel
    // backs out. Holds no state.
    //
    // M29 two-stage confirm: the App owns the pending flag and passes it back in.
    // While `lossConfirmationPending`, cancel/openWorldMap dismiss the warning
    // (they do NOT close the screen), a selection move dismisses it too, and
    // confirm commits the travel. While not pending, confirm on a destination
    // that requires the warning (`lossWarningRequired`, derived by the App from
    // destination legality + the at-risk preview) requests the confirmation
    // stage instead of traveling.
    class WorldMapController
    {
    public:
        [[nodiscard]] WorldMapUpdateResult Update(
            const input::InputState& input,
            int destinationCount,
            int selectedIndex,
            bool lossConfirmationPending = false,
            bool lossWarningRequired = false) const;
    };
}

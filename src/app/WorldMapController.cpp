#include "app/WorldMapController.h"

#include <algorithm>

namespace app
{
    WorldMapUpdateResult WorldMapController::Update(
        const input::InputState& input,
        const int destinationCount,
        const int selectedIndex) const
    {
        WorldMapUpdateResult result{};

        if (destinationCount <= 0)
        {
            // Nothing to select; cancel is still honored so the player can leave.
            result.selectedIndex = 0;
            result.cancelled = input.cancel || input.openWorldMap;
            return result;
        }

        int nextIndex = std::clamp(selectedIndex, 0, destinationCount - 1);

        if (input.selectPrev)
        {
            nextIndex = (nextIndex - 1 + destinationCount) % destinationCount;
        }
        if (input.selectNext)
        {
            nextIndex = (nextIndex + 1) % destinationCount;
        }

        result.selectedIndex = nextIndex;

        // Cancel, or pressing the open-world-map key again, closes the screen.
        if (input.cancel || input.openWorldMap)
        {
            result.cancelled = true;
        }
        else if (input.confirm)
        {
            result.travelConfirmed = true;
        }

        return result;
    }
}

#include "app/OverworldController.h"

#include <algorithm>

namespace app
{
    OverworldUpdateResult OverworldController::Update(
        const input::InputState& input,
        const int nodeCount,
        const int currentIndex,
        const int selectedIndex) const
    {
        OverworldUpdateResult result{};

        if (nodeCount <= 0)
        {
            result.selectedNodeIndex = 0;
            return result;
        }

        int nextIndex = std::clamp(selectedIndex, 0, nodeCount - 1);

        if (input.selectPrev)
        {
            nextIndex = (nextIndex - 1 + nodeCount) % nodeCount;
        }

        if (input.selectNext)
        {
            nextIndex = (nextIndex + 1) % nodeCount;
        }

        result.selectedNodeIndex = nextIndex;

        if (input.cancel)
        {
            result.selectedNodeIndex = std::clamp(currentIndex, 0, nodeCount - 1);
            result.travelCancelled = true;
        }

        if (input.confirm)
        {
            result.travelConfirmed = true;
        }

        if (input.debugBattle)
        {
            result.requestDebugBattle = true;
        }

        return result;
    }
}
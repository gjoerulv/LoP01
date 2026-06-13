#include "app/CampaignController.h"

#include <algorithm>

namespace app
{
    CampaignSelectUpdateResult CampaignController::Update(
        const input::InputState& input,
        const int campaignCount,
        const int selectedIndex) const
    {
        CampaignSelectUpdateResult result{};

        if (campaignCount <= 0)
        {
            // Nothing to select; cancel is still honored so the player can leave.
            result.selectedIndex = 0;
            result.cancelled = input.cancel;
            return result;
        }

        // Both axes navigate: Up/Down (target keys) matches the on-screen
        // "Up/Down to choose" hint; Left/Right (select keys) stays supported.
        int nextIndex = std::clamp(selectedIndex, 0, campaignCount - 1);
        if (input.selectPrev || input.targetPrev)
        {
            nextIndex = (nextIndex - 1 + campaignCount) % campaignCount;
        }
        if (input.selectNext || input.targetNext)
        {
            nextIndex = (nextIndex + 1) % campaignCount;
        }
        result.selectedIndex = nextIndex;

        if (input.cancel)
        {
            result.cancelled = true;
        }
        else if (input.confirm)
        {
            result.confirmed = true;
        }

        return result;
    }
}

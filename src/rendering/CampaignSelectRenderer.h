#pragma once

#include <string>
#include <vector>

#include "RenderContext.h"

namespace ashvale::rendering
{
    // One selectable row on a shell selection list (campaigns, standalone
    // scenarios, or game-mode entries — the model is list-shape generic).
    struct CampaignSelectRow
    {
        std::string id;
        std::string name;
        std::string description;
        bool selected = false;
        // Disabled rows draw muted with `statusText` as the player-facing
        // reason; they stay selectable so confirming reports the reason.
        bool enabled = true;
        std::string statusText;
    };

    struct CampaignSelectModel
    {
        std::string title = "Select Campaign";
        std::vector<CampaignSelectRow> campaigns;
        // Bounded screen-level status/error line ("" hides it).
        std::string statusText;
        std::string emptyText;   // shown when the list has no rows
        std::string footerHint = "Up/Down to choose, Enter to start, Esc to go back";
    };

    class CampaignSelectRenderer
    {
    public:
        void Draw(const RenderContext& context, const CampaignSelectModel& model) const;
    };
}

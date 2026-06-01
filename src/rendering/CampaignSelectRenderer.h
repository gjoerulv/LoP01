#pragma once

#include <string>
#include <vector>

#include "RenderContext.h"

namespace ashvale::rendering
{
    // One selectable campaign row on the Campaign Selection screen.
    struct CampaignSelectRow
    {
        std::string id;
        std::string name;
        std::string description;
        bool selected = false;
    };

    struct CampaignSelectModel
    {
        std::string title = "Select Campaign";
        std::vector<CampaignSelectRow> campaigns;
        std::string footerHint = "Up/Down to choose, Enter to start, Esc to go back";
    };

    class CampaignSelectRenderer
    {
    public:
        void Draw(const RenderContext& context, const CampaignSelectModel& model) const;
    };
}

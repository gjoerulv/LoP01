#pragma once

#include <string>
#include <vector>
#include "RenderContext.h"

namespace ashvale::rendering
{
    struct HudModel
    {
        std::string modeLabel;
        int day = 1;
        int week = 1;
        std::string timeText = "06:00";
        int gold = 0;
        int energy = 0;
        int maxEnergy = 0;
        std::string questCompactText;
        std::vector<std::string> activeBuffIcons;

        std::string primaryAreaLabel;
        std::string primaryAreaValue;

        std::string secondaryAreaLabel;
        std::string secondaryAreaValue;

        std::string statusText;
        bool showStatus = true;

        // M16-c: compact campaign/scenario status, empty for standalone play.
        std::string campaignText;
    };

    class HudRenderer
    {
    public:
        void Draw(const RenderContext& context, const HudModel& model) const;
    };
}
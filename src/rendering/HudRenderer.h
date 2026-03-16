#pragma once

#include <string>
#include "RenderContext.h"

namespace ashvale::rendering
{
    struct HudModel
    {
        std::string modeLabel;
        int day = 1;
        std::string timeText = "06:00";
        int gold = 0;

        std::string primaryAreaLabel;
        std::string primaryAreaValue;

        std::string secondaryAreaLabel;
        std::string secondaryAreaValue;

        std::string statusText;
        bool showStatus = true;
    };

    class HudRenderer
    {
    public:
        void Draw(const RenderContext& context, const HudModel& model) const;
    };
}
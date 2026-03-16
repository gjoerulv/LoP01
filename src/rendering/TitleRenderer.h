#pragma once

#include <string>
#include <vector>
#include "RenderContext.h"

namespace ashvale::rendering
{
    struct TitleScreenModel
    {
        std::string title = "Project Ashvale";
        std::string subtitle = "Vertical Slice";
        std::vector<std::string> menuItems{ "Start", "Load", "Quit" };
        int selectedIndex = 0;
        std::string footerHint = "Press Enter to confirm";
    };

    class TitleRenderer
    {
    public:
        void Draw(const RenderContext& context, const TitleScreenModel& model) const;
    };
}
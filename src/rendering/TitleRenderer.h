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
        // Per-item availability, parallel to menuItems. Empty => all enabled.
        // Disabled items draw muted and stay selectable so confirming them can
        // report a readable reason.
        std::vector<bool> menuItemEnabled;
        int selectedIndex = 0;
        // Bounded status/error line under the menu panel ("" hides it).
        std::string statusText;
        std::string footerHint = "Press Enter to confirm";
    };

    class TitleRenderer
    {
    public:
        void Draw(const RenderContext& context, const TitleScreenModel& model) const;
    };
}
#pragma once

#include <raylib.h>

namespace ashvale::rendering
{
    struct UiTheme
    {
        Color clearColor{ 10, 14, 20, 255 };
        Color panelColor{ 20, 28, 40, 230 };
        Color panelBorderColor{ 90, 120, 160, 255 };
        Color panelAccentColor{ 255, 194, 0, 255 };

        Color textColor{ 235, 240, 250, 255 };
        Color mutedTextColor{ 150, 165, 185, 255 };
        Color highlightTextColor{ 255, 220, 90, 255 };

        Color successColor{ 80, 210, 120, 255 };
        Color warningColor{ 255, 194, 0, 255 };
        Color dangerColor{ 230, 80, 80, 255 };
        Color selectionColor{ 90, 190, 255, 255 };

        Color overworldGroundA{ 27, 39, 52, 255 };
        Color overworldGroundB{ 35, 48, 66, 255 };
        Color locationFloorA{ 22, 33, 46, 255 };
        Color locationFloorB{ 30, 42, 58, 255 };
        Color battleBackA{ 18, 22, 30, 255 };
        Color battleBackB{ 36, 24, 32, 255 };

        float panelPadding = 12.0f;
        float sectionGap = 12.0f;
        float nodeRadius = 14.0f;
        float hudHeight = 56.0f;
        float dialogueHeight = 150.0f;
    };

    inline UiTheme MakeDefaultUiTheme()
    {
        return UiTheme{};
    }
}
#pragma once

#include <raylib.h>
#include "UiTheme.h"

namespace ashvale::rendering
{
    struct RenderContext
    {
        int screenWidth = 1280;
        int screenHeight = 720;

        bool debugEnabled = false;

        Font uiFont{};
        float titleFontSize = 42.0f;
        float normalFontSize = 24.0f;
        float smallFontSize = 18.0f;

        UiTheme theme = MakeDefaultUiTheme();
    };

    inline Font ResolveUiFont(const RenderContext& context)
    {
        return context.uiFont.texture.id != 0 ? context.uiFont : GetFontDefault();
    }
}
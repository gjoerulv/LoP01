#include "TitleRenderer.h"

namespace ashvale::rendering
{
    void TitleRenderer::Draw(const RenderContext& context, const TitleScreenModel& model) const
    {
        ClearBackground(context.theme.clearColor);

        DrawRectangle(0, 0, context.screenWidth, context.screenHeight / 2, Color{ 18, 24, 36, 255 });
        DrawRectangle(0, context.screenHeight / 2, context.screenWidth, context.screenHeight / 2, Color{ 10, 14, 20, 255 });

        const Font font = ResolveUiFont(context);

        DrawTextEx(font, model.title.c_str(), { 70.0f, 90.0f }, context.titleFontSize, 2.0f, context.theme.textColor);
        DrawTextEx(font, model.subtitle.c_str(), { 74.0f, 145.0f }, context.normalFontSize, 1.0f, context.theme.highlightTextColor);

        const Rectangle panel{
            70.0f,
            240.0f,
            340.0f,
            220.0f
        };

        DrawRectangleRounded(panel, 0.08f, 8, context.theme.panelColor);
        DrawRectangleRoundedLinesEx(panel, 0.08f, 8, 2.0f, context.theme.panelBorderColor);

        float y = panel.y + 24.0f;
        for (int i = 0; i < static_cast<int>(model.menuItems.size()); ++i)
        {
            const bool selected = (i == model.selectedIndex);
            const bool enabled = i >= static_cast<int>(model.menuItemEnabled.size())
                || model.menuItemEnabled[i];
            const Color color = !enabled
                ? context.theme.mutedTextColor
                : (selected ? context.theme.highlightTextColor : context.theme.textColor);

            if (selected)
            {
                DrawRectangle(static_cast<int>(panel.x + 16.0f), static_cast<int>(y - 2.0f), static_cast<int>(panel.width - 32.0f), 34, Fade(context.theme.selectionColor, 0.18f));
            }

            DrawTextEx(font, model.menuItems[i].c_str(), { panel.x + 24.0f, y }, context.normalFontSize, 1.0f, color);
            y += 46.0f;
        }

        if (!model.statusText.empty())
        {
            DrawTextEx(font, model.statusText.c_str(),
                { panel.x, panel.y + panel.height + 18.0f },
                context.smallFontSize, 1.0f, context.theme.mutedTextColor);
        }

        DrawTextEx(font, model.footerHint.c_str(),
            { 70.0f, static_cast<float>(context.screenHeight - 48) },
            context.smallFontSize,
            1.0f,
            context.theme.mutedTextColor);
    }
}
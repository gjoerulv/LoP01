#include "DebugOverlay.h"

#include <algorithm>
#include <string>

namespace ashvale::rendering
{
    void DebugOverlay::Draw(const RenderContext& context, const DebugOverlayModel& model) const
    {
        if (!model.visible)
        {
            return;
        }

        const int panelX = 12;
        const int panelY = static_cast<int>(context.theme.hudHeight) + 12;
        const int panelW = 420;
        const int lineHeight = 22;
        const int panelH = 16 + 28 + static_cast<int>(model.lines.size()) * lineHeight + 12;

        DrawRectangle(panelX, panelY, panelW, panelH, Fade(BLACK, 0.78f));
        DrawRectangleLines(panelX, panelY, panelW, panelH, context.theme.panelBorderColor);

        const Font font = ResolveUiFont(context);
        DrawTextEx(font, "DEBUG OVERLAY (F1)", { static_cast<float>(panelX + 10), static_cast<float>(panelY + 8) }, context.smallFontSize, 1.0f, context.theme.warningColor);

        float y = static_cast<float>(panelY + 36);
        for (const DebugLine& line : model.lines)
        {
            DrawTextEx(font, line.label.c_str(), { static_cast<float>(panelX + 10), y }, context.smallFontSize, 1.0f, context.theme.mutedTextColor);
            DrawTextEx(font, line.value.c_str(), { static_cast<float>(panelX + 160), y }, context.smallFontSize, 1.0f, context.theme.textColor);
            y += static_cast<float>(lineHeight);
        }
    }
}
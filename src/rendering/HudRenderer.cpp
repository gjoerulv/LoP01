#include "HudRenderer.h"

#include <string>

namespace ashvale::rendering
{
    namespace
    {
        void DrawLabelValue(const RenderContext& context, const std::string& label, const std::string& value, float x, float y)
        {
            const Font font = ResolveUiFont(context);
            DrawTextEx(font, label.c_str(), { x, y }, context.smallFontSize, 1.0f, context.theme.mutedTextColor);

            const Vector2 labelSize = MeasureTextEx(font, label.c_str(), context.smallFontSize, 1.0f);
            DrawTextEx(font, value.c_str(), { x + labelSize.x + 6.0f, y }, context.smallFontSize, 1.0f, context.theme.textColor);
        }
    }

    void HudRenderer::Draw(const RenderContext& context, const HudModel& model) const
    {
        const float h = context.theme.hudHeight;

        DrawRectangle(0, 0, context.screenWidth, static_cast<int>(h), context.theme.panelColor);
        DrawLine(0, static_cast<int>(h), context.screenWidth, static_cast<int>(h), context.theme.panelBorderColor);

        const Font font = ResolveUiFont(context);

        DrawTextEx(font, model.modeLabel.c_str(), { 16.0f, 10.0f }, context.normalFontSize, 1.0f, context.theme.highlightTextColor);

        DrawLabelValue(context, "Day:", std::to_string(model.day), 260.0f, 10.0f);
        DrawLabelValue(context, "Time:", model.timeText, 360.0f, 10.0f);
        DrawLabelValue(context, "Gold:", std::to_string(model.gold), 500.0f, 10.0f);

        if (!model.primaryAreaLabel.empty() && !model.primaryAreaValue.empty())
        {
            DrawLabelValue(context, model.primaryAreaLabel, model.primaryAreaValue, 650.0f, 10.0f);
        }

        if (!model.secondaryAreaLabel.empty() && !model.secondaryAreaValue.empty())
        {
            DrawLabelValue(context, model.secondaryAreaLabel, model.secondaryAreaValue, 940.0f, 10.0f);
        }

        if (model.showStatus && !model.statusText.empty())
        {
            DrawTextEx(font, model.statusText.c_str(), { 16.0f, 30.0f }, context.smallFontSize, 1.0f, context.theme.mutedTextColor);
        }
    }
}
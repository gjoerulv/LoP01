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

        if (!model.campaignText.empty())
        {
            DrawLabelValue(context, "Campaign:", model.campaignText, 16.0f, 10.0f);
        }

        DrawLabelValue(context, "Day:", std::to_string(model.day), 260.0f, 10.0f);
        DrawLabelValue(context, "Week:", std::to_string(model.week), 340.0f, 10.0f);
        DrawLabelValue(context, "Time:", model.timeText, 430.0f, 10.0f);
        DrawLabelValue(context, "Gold:", std::to_string(model.gold), 570.0f, 10.0f);
        if (!model.questCompactText.empty())
        {
            DrawLabelValue(context, "Quests:", model.questCompactText, 650.0f, 10.0f);
        }

        if (!model.primaryAreaLabel.empty() && !model.primaryAreaValue.empty())
        {
            DrawLabelValue(context, model.primaryAreaLabel, model.primaryAreaValue, 770.0f, 10.0f);
        }

        if (!model.secondaryAreaLabel.empty() && !model.secondaryAreaValue.empty())
        {
            DrawLabelValue(context, model.secondaryAreaLabel, model.secondaryAreaValue, 1020.0f, 10.0f);
        }

        if (model.showStatus && !model.statusText.empty())
        {
            DrawTextEx(font, model.statusText.c_str(), { 16.0f, 30.0f }, context.smallFontSize, 1.0f, context.theme.mutedTextColor);
        }

        if (!model.activeBuffIcons.empty())
        {
            const float iconY = h + 6.0f;
            const float iconSize = 18.0f;
            float iconX = 16.0f;

            for (const auto& iconId : model.activeBuffIcons)
            {
                const Rectangle iconRect{ iconX, iconY, iconSize, iconSize };
                DrawRectangleRounded(iconRect, 0.18f, 4, context.theme.panelColor);
                DrawRectangleRoundedLinesEx(iconRect, 0.18f, 4, 2.0f, context.theme.highlightTextColor);

                if (iconId == "travel_prep")
                {
                    DrawLineEx(
                        Vector2{ iconRect.x + 4.0f, iconRect.y + iconRect.height - 4.0f },
                        Vector2{ iconRect.x + iconRect.width - 4.0f, iconRect.y + 4.0f },
                        2.0f,
                        context.theme.highlightTextColor);
                    DrawLineEx(
                        Vector2{ iconRect.x + 4.0f, iconRect.y + 4.0f },
                        Vector2{ iconRect.x + iconRect.width - 4.0f, iconRect.y + 4.0f },
                        2.0f,
                        context.theme.highlightTextColor);
                }

                iconX += iconSize + 8.0f;
            }
        }
    }
}
#include "CampaignSelectRenderer.h"

#include <string>

namespace ashvale::rendering
{
    void CampaignSelectRenderer::Draw(const RenderContext& context, const CampaignSelectModel& model) const
    {
        ClearBackground(context.theme.clearColor);

        const Font font = ResolveUiFont(context);

        DrawTextEx(font, model.title.c_str(), { 70.0f, 70.0f }, context.titleFontSize, 2.0f,
            context.theme.textColor);

        if (model.campaigns.empty())
        {
            DrawTextEx(font, "No campaigns installed.", { 74.0f, 150.0f }, context.normalFontSize, 1.0f,
                context.theme.mutedTextColor);
        }

        float y = 150.0f;
        for (const auto& row : model.campaigns)
        {
            const Color color = row.selected
                ? context.theme.highlightTextColor
                : context.theme.textColor;

            if (row.selected)
            {
                DrawRectangle(60, static_cast<int>(y - 2.0f), 900, 34,
                    Fade(context.theme.selectionColor, 0.18f));
            }

            const std::string prefix = row.selected ? "> " : "  ";
            DrawTextEx(font, (prefix + row.name).c_str(), { 74.0f, y }, context.normalFontSize, 1.0f, color);
            if (!row.description.empty())
            {
                DrawTextEx(font, row.description.c_str(), { 110.0f, y + 26.0f }, context.smallFontSize, 1.0f,
                    context.theme.mutedTextColor);
            }
            y += 66.0f;
        }

        DrawTextEx(font, model.footerHint.c_str(), { 74.0f, y + 16.0f }, context.normalFontSize, 1.0f,
            context.theme.mutedTextColor);
    }
}

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
            const std::string emptyText =
                model.emptyText.empty() ? "Nothing available." : model.emptyText;
            DrawTextEx(font, emptyText.c_str(), { 74.0f, 150.0f }, context.normalFontSize, 1.0f,
                context.theme.mutedTextColor);
        }

        float y = 150.0f;
        for (const auto& row : model.campaigns)
        {
            const Color color = !row.enabled
                ? context.theme.mutedTextColor
                : (row.selected ? context.theme.highlightTextColor : context.theme.textColor);

            if (row.selected)
            {
                DrawRectangle(60, static_cast<int>(y - 2.0f), 900, 34,
                    Fade(context.theme.selectionColor, 0.18f));
            }

            const std::string prefix = row.selected ? "> " : "  ";
            DrawTextEx(font, (prefix + row.name).c_str(), { 74.0f, y }, context.normalFontSize, 1.0f, color);
            float detailY = y + 26.0f;
            if (!row.description.empty())
            {
                DrawTextEx(font, row.description.c_str(), { 110.0f, detailY }, context.smallFontSize, 1.0f,
                    context.theme.mutedTextColor);
                detailY += 22.0f;
                y += 22.0f;
            }
            if (!row.statusText.empty())
            {
                DrawTextEx(font, row.statusText.c_str(), { 110.0f, detailY }, context.smallFontSize, 1.0f,
                    context.theme.mutedTextColor);
                y += 22.0f;
            }
            y += 44.0f;
        }

        if (!model.statusText.empty())
        {
            DrawTextEx(font, model.statusText.c_str(), { 74.0f, y + 8.0f },
                context.smallFontSize, 1.0f, context.theme.mutedTextColor);
            y += 26.0f;
        }

        DrawTextEx(font, model.footerHint.c_str(), { 74.0f, y + 16.0f }, context.normalFontSize, 1.0f,
            context.theme.mutedTextColor);
    }
}

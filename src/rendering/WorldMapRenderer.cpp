#include "WorldMapRenderer.h"

#include <string>

namespace ashvale::rendering
{
    void WorldMapRenderer::Draw(const RenderContext& context, const WorldMapModel& model) const
    {
        ClearBackground(context.theme.clearColor);

        const Font font = ResolveUiFont(context);

        DrawTextEx(font, model.title.c_str(), { 70.0f, 70.0f }, context.titleFontSize, 2.0f,
            context.theme.textColor);

        const std::string here = "From: " + model.currentRegionName +
            "   Energy: " + std::to_string(model.energy) + "/" + std::to_string(model.maxEnergy);
        DrawTextEx(font, here.c_str(), { 74.0f, 130.0f }, context.normalFontSize, 1.0f,
            context.theme.highlightTextColor);

        if (model.genericLossCount > 0)
        {
            const std::string warn = "Warning: " + std::to_string(model.genericLossCount) +
                " generic unit(s) will be lost on departure.";
            DrawTextEx(font, warn.c_str(), { 74.0f, 162.0f }, context.normalFontSize, 1.0f,
                context.theme.mutedTextColor);
        }

        float y = 210.0f;
        for (const auto& dest : model.destinations)
        {
            const Color color = dest.selected
                ? context.theme.highlightTextColor
                : (dest.legal ? context.theme.textColor : context.theme.mutedTextColor);

            if (dest.selected)
            {
                DrawRectangle(60, static_cast<int>(y - 2.0f), 760, 34,
                    Fade(context.theme.selectionColor, 0.18f));
            }

            const std::string prefix = dest.selected ? "> " : "  ";
            const std::string line = prefix + dest.name + "  -  " + dest.statusText;
            DrawTextEx(font, line.c_str(), { 74.0f, y }, context.normalFontSize, 1.0f, color);
            y += 40.0f;
        }

        if (model.confirmingLoss)
        {
            y += 12.0f;
            DrawTextEx(font, model.confirmTitle.c_str(), { 74.0f, y },
                context.normalFontSize, 1.0f, context.theme.highlightTextColor);
            y += 28.0f;
            DrawTextEx(font, "These generic units will be LOST:", { 74.0f, y },
                context.normalFontSize, 1.0f, context.theme.textColor);
            y += 28.0f;
            for (const auto& lossLine : model.lossLines)
            {
                DrawTextEx(font, ("  " + lossLine).c_str(), { 74.0f, y },
                    context.normalFontSize, 1.0f, context.theme.textColor);
                y += 24.0f;
            }
            DrawTextEx(font, "(stored and stationed units stay behind and are safe)",
                { 74.0f, y }, context.normalFontSize, 1.0f, context.theme.mutedTextColor);
            y += 28.0f;
        }

        DrawTextEx(font, model.footerHint.c_str(), { 74.0f, y + 16.0f }, context.normalFontSize, 1.0f,
            context.theme.mutedTextColor);
    }
}

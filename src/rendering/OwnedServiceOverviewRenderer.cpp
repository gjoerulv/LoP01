#include "OwnedServiceOverviewRenderer.h"

#include <string>

namespace ashvale::rendering
{
    namespace
    {
        std::string JoinNames(const std::vector<std::string>& names)
        {
            std::string out;
            for (const auto& n : names)
            {
                if (!out.empty()) out += ", ";
                out += n;
            }
            return out;
        }
    }

    void OwnedServiceOverviewRenderer::Draw(const RenderContext& context,
        const OwnedServiceOverviewModel& model) const
    {
        ClearBackground(context.theme.clearColor);
        const Font font = ResolveUiFont(context);

        DrawTextEx(font, model.title.c_str(), { 70.0f, 50.0f }, context.titleFontSize, 2.0f,
            context.theme.textColor);

        float y = 130.0f;

        if (model.rows.empty())
        {
            DrawTextEx(font, model.emptyText.c_str(), { 74.0f, y }, context.normalFontSize, 1.0f,
                context.theme.mutedTextColor);
        }

        for (int i = 0; i < static_cast<int>(model.rows.size()); ++i)
        {
            const auto& row = model.rows[i];
            const bool selected = i == model.selectedIndex;
            const Color headColor = selected
                ? context.theme.selectionColor
                : context.theme.textColor;

            const std::string head =
                (selected ? "> " : "  ") + row.displayName + "   [" + row.kindLabel + "]";
            DrawTextEx(font, head.c_str(), { 70.0f, y }, context.normalFontSize, 1.0f, headColor);
            y += 30.0f;

            const std::string where = "   " + row.locationLabel + "   " + row.statusLabel;
            DrawTextEx(font, where.c_str(), { 74.0f, y }, context.smallFontSize, 1.0f,
                context.theme.mutedTextColor);
            y += 24.0f;

            // M33 bounded threat preview line (only when the node is contested).
            if (!row.threatLabel.empty())
            {
                DrawTextEx(font, ("   " + row.threatLabel).c_str(), { 74.0f, y },
                    context.smallFontSize, 1.0f, context.theme.dangerColor);
                y += 24.0f;
            }

            if (row.isMine)
            {
                std::string stationed = "   Stationed: " + std::to_string(row.stationedCount) +
                    "/" + std::to_string(row.stationedCapacity);
                if (!row.stationedUnitNames.empty())
                {
                    stationed += "  (" + JoinNames(row.stationedUnitNames) + ")";
                }
                DrawTextEx(font, stationed.c_str(), { 74.0f, y }, context.smallFontSize, 1.0f,
                    context.theme.textColor);
                y += 24.0f;

                for (const auto& line : row.outputLines)
                {
                    DrawTextEx(font, ("   " + line).c_str(), { 74.0f, y }, context.smallFontSize,
                        1.0f, context.theme.successColor);
                    y += 22.0f;
                }
            }
            else if (row.isTrader)
            {
                const std::string tier = "   Ownership tier: " + std::to_string(row.traderTier);
                DrawTextEx(font, tier.c_str(), { 74.0f, y }, context.smallFontSize, 1.0f,
                    context.theme.textColor);
                y += 24.0f;
            }
            else if (row.isStorage)
            {
                std::string stored = "   Stored: " + std::to_string(row.storedCount) +
                    "/" + std::to_string(row.storageCapacity);
                if (!row.storedUnitNames.empty())
                {
                    stored += "  (" + JoinNames(row.storedUnitNames) + ")";
                }
                DrawTextEx(font, stored.c_str(), { 74.0f, y }, context.smallFontSize, 1.0f,
                    context.theme.textColor);
                y += 24.0f;
            }

            if (context.debugEnabled && !row.serviceId.empty())
            {
                DrawTextEx(font, ("   id: " + row.serviceId).c_str(), { 74.0f, y },
                    context.smallFontSize, 1.0f, context.theme.mutedTextColor);
                y += 22.0f;
            }

            y += 10.0f;
        }

        // M30: Temporarily Unavailable heroes (bounded section; absent when empty).
        if (!model.unavailableHeroLines.empty())
        {
            y += 8.0f;
            DrawTextEx(font, "Temporarily unavailable:", { 70.0f, y },
                context.smallFontSize, 1.0f, context.theme.textColor);
            y += 24.0f;
            for (const auto& line : model.unavailableHeroLines)
            {
                DrawTextEx(font, ("   " + line).c_str(), { 74.0f, y },
                    context.smallFontSize, 1.0f, context.theme.mutedTextColor);
                y += 22.0f;
            }
        }

        // M30: recent service events (bounded, newest last; absent when empty).
        if (!model.eventLogLines.empty())
        {
            y += 8.0f;
            DrawTextEx(font, "Recent service events:", { 70.0f, y },
                context.smallFontSize, 1.0f, context.theme.textColor);
            y += 24.0f;
            for (const auto& line : model.eventLogLines)
            {
                DrawTextEx(font, ("   " + line).c_str(), { 74.0f, y },
                    context.smallFontSize, 1.0f, context.theme.mutedTextColor);
                y += 22.0f;
            }
        }

        DrawTextEx(font, model.footerHint.c_str(),
            { 70.0f, static_cast<float>(context.screenHeight - 44) },
            context.smallFontSize, 1.0f, context.theme.mutedTextColor);
    }
}

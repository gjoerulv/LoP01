#include "RegionRenderer.h"

#include <string>

namespace ashvale::rendering
{
    Color RegionRenderer::GetNodeColor(RegionNodeType type, const UiTheme& theme) const
    {
        switch (type)
        {
        case RegionNodeType::Home:    return theme.successColor;
        case RegionNodeType::Town:    return theme.selectionColor;
        case RegionNodeType::Dungeon: return theme.dangerColor;
        case RegionNodeType::Service: return theme.warningColor;
        case RegionNodeType::Recruit: return Color{ 255, 120, 210, 255 };
        case RegionNodeType::Combat:  return Color{ 230, 80, 80, 255 };
        default:                         return theme.textColor;
        }
    }

    void RegionRenderer::Draw(const RenderContext& context, const RegionRenderModel& model) const
    {
        const int hudBottom = static_cast<int>(context.theme.hudHeight);
        const Rectangle mapRect{ 20.0f, static_cast<float>(hudBottom + 20), 860.0f, 620.0f };
        const Rectangle sideRect{ 900.0f, static_cast<float>(hudBottom + 20), 360.0f, 620.0f };

        DrawRectangleGradientV(
            static_cast<int>(mapRect.x), static_cast<int>(mapRect.y),
            static_cast<int>(mapRect.width), static_cast<int>(mapRect.height),
            context.theme.overworldGroundA, context.theme.overworldGroundB);

        DrawRectangleRounded(sideRect, 0.04f, 8, context.theme.panelColor);
        DrawRectangleRoundedLinesEx(sideRect, 0.04f, 8, 2.0f, context.theme.panelBorderColor);

        const Font font = ResolveUiFont(context);

        DrawTextEx(font, model.regionName.c_str(), { sideRect.x + 16.0f, sideRect.y + 16.0f }, context.normalFontSize, 1.0f, context.theme.highlightTextColor);

        for (const auto& link : model.links)
        {
            if (link[0] < 0 || link[1] < 0 || link[0] >= static_cast<int>(model.nodes.size()) || link[1] >= static_cast<int>(model.nodes.size()))
            {
                continue;
            }

            const Vector2 a{ mapRect.x + model.nodes[link[0]].position.x, mapRect.y + model.nodes[link[0]].position.y };
            const Vector2 b{ mapRect.x + model.nodes[link[1]].position.x, mapRect.y + model.nodes[link[1]].position.y };
            DrawLineEx(a, b, 3.0f, Fade(context.theme.panelBorderColor, 0.55f));
        }

        for (const auto& node : model.nodes)
        {
            if (!node.discovered)
            {
                continue;
            }

            const Vector2 p{ mapRect.x + node.position.x, mapRect.y + node.position.y };
            const Color fill = GetNodeColor(node.type, context.theme);

            DrawCircleV(p, context.theme.nodeRadius, fill);
            DrawCircleLines(static_cast<int>(p.x), static_cast<int>(p.y), context.theme.nodeRadius, RAYWHITE);

            if (node.current)
            {
                DrawCircleLines(static_cast<int>(p.x), static_cast<int>(p.y), context.theme.nodeRadius + 6.0f, context.theme.successColor);
                DrawTextEx(font, "P", { p.x - 4.0f, p.y - 8.0f }, context.smallFontSize, 1.0f, BLACK);
            }

            if (node.selected)
            {
                DrawCircleLines(static_cast<int>(p.x), static_cast<int>(p.y), context.theme.nodeRadius + 11.0f, context.theme.selectionColor);
            }

            DrawTextEx(font, node.label.c_str(), { p.x - 20.0f, p.y + 22.0f }, context.smallFontSize, 1.0f, context.theme.textColor);
        }

        float y = sideRect.y + 64.0f;
        DrawTextEx(font, "Current", { sideRect.x + 16.0f, y }, context.smallFontSize, 1.0f, context.theme.mutedTextColor);
        y += 24.0f;
        DrawTextEx(font, model.currentNodeLabel.c_str(), { sideRect.x + 16.0f, y }, context.normalFontSize, 1.0f, context.theme.successColor);
        y += 40.0f;

        DrawTextEx(font, "Selected", { sideRect.x + 16.0f, y }, context.smallFontSize, 1.0f, context.theme.mutedTextColor);
        y += 26.0f;
        DrawTextEx(font, model.selectedNodeLabel.c_str(), { sideRect.x + 16.0f, y }, context.normalFontSize, 1.0f, context.theme.textColor);
        y += 34.0f;
        DrawTextEx(font, model.selectedNodeType.c_str(), { sideRect.x + 16.0f, y }, context.smallFontSize, 1.0f, context.theme.mutedTextColor);
        y += 24.0f;
        DrawTextEx(font, "Properties", { sideRect.x + 16.0f, y }, context.smallFontSize, 1.0f, context.theme.mutedTextColor);
        y += 20.0f;
        DrawTextEx(font, model.selectedNodeEnterable.c_str(), { sideRect.x + 16.0f, y }, context.smallFontSize, 1.0f, context.theme.mutedTextColor);
        y += 36.0f;

        DrawTextEx(font, "Travel Time", { sideRect.x + 16.0f, y }, context.smallFontSize, 1.0f, context.theme.mutedTextColor);
        y += 24.0f;
        DrawTextEx(font, model.travelTimeText.c_str(), { sideRect.x + 16.0f, y }, context.normalFontSize, 1.0f, context.theme.highlightTextColor);
    }
}
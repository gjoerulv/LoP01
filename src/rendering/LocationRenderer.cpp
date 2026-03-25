#include "LocationRenderer.h"

#include <algorithm>
#include <string>
#include <vector>

namespace ashvale::rendering
{
    namespace {
        std::vector<std::string> SplitLines(const std::string& text)
        {
            std::vector<std::string> lines;
            std::string current;
            for (const char ch : text)
            {
                if (ch == '\n')
                {
                    lines.push_back(current);
                    current.clear();
                    continue;
                }

                current.push_back(ch);
            }

            lines.push_back(current);
            return lines;
        }
    }

    Color LocationRenderer::GetZoneColor(LocationInteractableType type, const UiTheme& theme) const
    {
        switch (type)
        {
        case LocationInteractableType::Inn:     return theme.successColor;
        case LocationInteractableType::Shop:    return theme.warningColor;
        case LocationInteractableType::Recruit: return Color{ 255, 140, 220, 255 };
        case LocationInteractableType::Npc:     return theme.selectionColor;
        case LocationInteractableType::Exit:    return theme.dangerColor;
        case LocationInteractableType::Door:    return theme.textColor;
        default:                                return theme.mutedTextColor;
        }
    }

    void LocationRenderer::Draw(const RenderContext& context, const LocationRenderModel& model) const
    {
        const int hudBottom = static_cast<int>(context.theme.hudHeight);
        const int bottomUiTop = context.screenHeight - static_cast<int>(context.theme.dialogueHeight);

        for (int y = hudBottom; y < bottomUiTop; y += 40)
        {
            for (int x = 0; x < context.screenWidth; x += 40)
            {
                const bool alt = ((x / 40) + (y / 40)) % 2 == 0;
                DrawRectangle(x, y, 40, 40, alt ? context.theme.locationFloorA : context.theme.locationFloorB);
            }
        }

        for (const Rectangle& block : model.buildingBlocks)
        {
            const Rectangle roof{ block.x, block.y - 12.0f, block.width, 16.0f };
            DrawRectangleRec(roof, Color{ 132, 72, 58, 255 });

            DrawRectangleRec(block, Color{ 98, 82, 66, 255 });
            DrawRectangleLinesEx(block, 2.0f, context.theme.panelBorderColor);

            const Rectangle door{ block.x + block.width * 0.44f, block.y + block.height - 24.0f, block.width * 0.12f, 24.0f };
            DrawRectangleRec(door, Color{ 62, 44, 32, 255 });

            const Rectangle windowA{ block.x + 12.0f, block.y + 26.0f, 16.0f, 14.0f };
            const Rectangle windowB{ block.x + block.width - 28.0f, block.y + 26.0f, 16.0f, 14.0f };
            DrawRectangleRec(windowA, Color{ 124, 170, 214, 255 });
            DrawRectangleRec(windowB, Color{ 124, 170, 214, 255 });
        }

        for (const Rectangle& wall : model.walls)
        {
            DrawRectangleRec(wall, Color{ 62, 70, 84, 255 });
        }

        const Font font = ResolveUiFont(context);
        const LocationZoneView* highlightedZone = nullptr;

        for (const auto& zone : model.zones)
        {
            const Color zoneColor = GetZoneColor(zone.type, context.theme);
            DrawRectangleRec(zone.bounds, Fade(zoneColor, zone.highlighted ? 0.22f : 0.12f));
            DrawRectangleLinesEx(zone.bounds, zone.highlighted ? 3.0f : 2.0f, zone.highlighted ? context.theme.selectionColor : zoneColor);
            DrawTextEx(font, zone.label.c_str(), { zone.bounds.x, zone.bounds.y - 18.0f }, context.smallFontSize, 1.0f, zoneColor);

            if (zone.highlighted)
            {
                highlightedZone = &zone;
            }
        }

        for (const auto& npc : model.npcs)
        {
            DrawRectangleRec(npc.bounds, npc.highlighted ? context.theme.selectionColor : Color{ 100, 180, 255, 255 });
            DrawRectangleLinesEx(npc.bounds, 2.0f, RAYWHITE);
            DrawTextEx(font, npc.name.c_str(), { npc.bounds.x - 4.0f, npc.bounds.y - 18.0f }, context.smallFontSize, 1.0f, context.theme.textColor);
        }

        DrawRectangleRec(model.playerBounds, WHITE);

        if (model.showInteractPrompt && !model.interactPrompt.empty())
        {
            float promptX = 20.0f;
            float promptY = static_cast<float>(bottomUiTop - 56);

            if (highlightedZone != nullptr)
            {
                promptX = highlightedZone->bounds.x + highlightedZone->bounds.width + 8.0f;
                promptY = highlightedZone->bounds.y - 12.0f;
            }

            const std::vector<std::string> promptLines = SplitLines(model.interactPrompt);
            const float promptW = 380.0f;
            const float lineHeight = context.smallFontSize + 4.0f;
            const float promptH = std::max(36.0f, 10.0f + lineHeight * static_cast<float>(promptLines.size()));
            promptX = std::clamp(promptX, 8.0f, static_cast<float>(context.screenWidth) - promptW - 8.0f);
            promptY = std::clamp(promptY, static_cast<float>(hudBottom + 8), static_cast<float>(bottomUiTop) - promptH - 8.0f);

            const Rectangle promptRect{ promptX, promptY, promptW, promptH };
            const Color usablePanel = Fade(context.theme.panelColor, 0.95f);
            const Color unusablePanel = Fade(context.theme.panelColor, 0.75f);
            const Color usableBorder = context.theme.highlightTextColor;
            const Color unusableBorder = context.theme.mutedTextColor;
            DrawRectangleRounded(promptRect, 0.15f, 6, model.interactPromptUsable ? usablePanel : unusablePanel);
            DrawRectangleRoundedLinesEx(promptRect, 0.15f, 6, 2.0f, model.interactPromptUsable ? usableBorder : unusableBorder);

            float lineY = promptRect.y + 8.0f;
            for (const auto& line : promptLines)
            {
                DrawTextEx(
                    font,
                    line.c_str(),
                    { promptRect.x + 10.0f, lineY },
                    context.smallFontSize,
                    1.0f,
                    model.interactPromptUsable ? context.theme.textColor : context.theme.mutedTextColor);
                lineY += lineHeight;
            }
        }

        const Rectangle dialogueRect{
            0.0f,
            static_cast<float>(bottomUiTop),
            static_cast<float>(context.screenWidth),
            static_cast<float>(context.screenHeight - bottomUiTop)
        };

        DrawRectangleRec(dialogueRect, Fade(BLACK, 0.82f));
        DrawLine(0, bottomUiTop, context.screenWidth, bottomUiTop, context.theme.panelBorderColor);
        DrawRectangle(0, bottomUiTop, context.screenWidth, 26, Fade(context.theme.panelColor, 0.9f));

        DrawTextEx(font, model.locationName.c_str(), { 16.0f, static_cast<float>(bottomUiTop + 10) }, context.smallFontSize, 1.0f, context.theme.highlightTextColor);

        if (model.dialogue.visible)
        {
            DrawTextEx(font, model.dialogue.speaker.c_str(), { 16.0f, static_cast<float>(bottomUiTop + 40) }, context.smallFontSize, 1.0f, context.theme.selectionColor);
            DrawTextEx(font, model.dialogue.text.c_str(), { 16.0f, static_cast<float>(bottomUiTop + 68) }, context.smallFontSize, 1.0f, context.theme.textColor);

            float optionY = static_cast<float>(bottomUiTop + 100);
            for (int i = 0; i < static_cast<int>(model.dialogue.options.size()); ++i)
            {
                const bool selected = (i == model.dialogue.selectedOption);
                const Color color = selected ? context.theme.highlightTextColor : context.theme.mutedTextColor;

                DrawRectangleRounded(
                    Rectangle{ 20.0f, optionY - 2.0f, static_cast<float>(context.screenWidth - 40), 20.0f },
                    0.2f,
                    4,
                    selected ? Fade(context.theme.selectionColor, 0.2f) : Fade(context.theme.panelBorderColor, 0.08f));

                DrawTextEx(font, model.dialogue.options[i].c_str(), { 28.0f, optionY }, context.smallFontSize, 1.0f, color);
                optionY += 22.0f;
            }
        }
        else
        {
            DrawTextEx(font, "Explore, interact, and leave through the marked exit.", { 16.0f, static_cast<float>(bottomUiTop + 40) }, context.smallFontSize, 1.0f, context.theme.mutedTextColor);
        }
    }
}
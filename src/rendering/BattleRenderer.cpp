#include "BattleRenderer.h"

#include <algorithm>
#include <string>

namespace ashvale::rendering
{
    namespace
    {
        int SafeBarWidth(int value, int maxValue, int width)
        {
            if (maxValue <= 0)
            {
                return 0;
            }

            const float ratio = static_cast<float>(value) / static_cast<float>(maxValue);
            return static_cast<int>(std::max(0.0f, std::min(1.0f, ratio)) * static_cast<float>(width));
        }
    }

    void BattleRenderer::DrawUnitCard(const RenderContext& context, const BattleUnitView& unit) const
    {
        const Color cardColor = unit.team == BattleTeam::Allies
            ? Color{ 28, 44, 62, 255 }
        : Color{ 62, 30, 38, 255 };

        DrawRectangleRec(unit.bounds, cardColor);

        const Color borderColor =
            unit.selectedTarget ? context.theme.highlightTextColor :
            unit.active ? context.theme.selectionColor :
            unit.ko ? context.theme.dangerColor :
            context.theme.panelBorderColor;

        DrawRectangleLinesEx(unit.bounds, unit.active || unit.selectedTarget ? 3.0f : 2.0f, borderColor);

        if (unit.active)
        {
            DrawRectangleLinesEx(
                Rectangle{ unit.bounds.x - 3.0f, unit.bounds.y - 3.0f, unit.bounds.width + 6.0f, unit.bounds.height + 6.0f },
                2.0f,
                context.theme.highlightTextColor);
        }

        if (unit.selectedTarget)
        {
            DrawRectangle(
                static_cast<int>(unit.bounds.x + unit.bounds.width - 72.0f),
                static_cast<int>(unit.bounds.y + 6.0f),
                64,
                18,
                Fade(context.theme.dangerColor, 0.25f));
            DrawTextEx(ResolveUiFont(context), "TARGET", { unit.bounds.x + unit.bounds.width - 66.0f, unit.bounds.y + 9.0f }, 13.0f, 1.0f, context.theme.dangerColor);
        }

        const Font font = ResolveUiFont(context);

        DrawTextEx(font, unit.name.c_str(), { unit.bounds.x + 8.0f, unit.bounds.y + 8.0f }, context.smallFontSize, 1.0f, context.theme.textColor);

        const int barX = static_cast<int>(unit.bounds.x + 8.0f);
        const int hpY = static_cast<int>(unit.bounds.y + 38.0f);
        const int mpY = static_cast<int>(unit.bounds.y + 58.0f);
        const int barW = static_cast<int>(unit.bounds.width - 16.0f);

        DrawRectangle(barX, hpY, barW, 10, Fade(context.theme.panelBorderColor, 0.35f));
        DrawRectangle(barX, hpY, SafeBarWidth(unit.hp, unit.maxHp, barW), 10, context.theme.dangerColor);

        DrawRectangle(barX, mpY, barW, 10, Fade(context.theme.panelBorderColor, 0.35f));
        DrawRectangle(barX, mpY, SafeBarWidth(unit.mp, unit.maxMp, barW), 10, context.theme.selectionColor);

        DrawTextEx(font,
            TextFormat("HP %d/%d", unit.hp, unit.maxHp),
            { unit.bounds.x + 8.0f, unit.bounds.y + 76.0f },
            context.smallFontSize,
            1.0f,
            context.theme.textColor);

        DrawTextEx(font,
            TextFormat("MP %d/%d", unit.mp, unit.maxMp),
            { unit.bounds.x + 8.0f, unit.bounds.y + 96.0f },
            context.smallFontSize,
            1.0f,
            context.theme.textColor);

        DrawTextEx(font,
            TextFormat("Life %d", unit.life),
            { unit.bounds.x + 8.0f, unit.bounds.y + 116.0f },
            context.smallFontSize,
            1.0f,
            context.theme.mutedTextColor);

        if (unit.ko)
        {
            DrawTextEx(font, "KO", { unit.bounds.x + unit.bounds.width - 36.0f, unit.bounds.y + 8.0f }, context.smallFontSize, 1.0f, context.theme.dangerColor);
        }
    }

    void BattleRenderer::Draw(const RenderContext& context, const BattleRenderModel& model) const
    {
        const int hudBottom = static_cast<int>(context.theme.hudHeight);
        const Rectangle turnOrderRect{ 16.0f, static_cast<float>(hudBottom + 10), 1248.0f, 56.0f };
        const Rectangle actionRect{ 16.0f, 560.0f, 1248.0f, 144.0f };
        const Rectangle alliesArea{ 20.0f, static_cast<float>(hudBottom + 76), 612.0f, 470.0f };
        const Rectangle enemiesArea{ 648.0f, static_cast<float>(hudBottom + 76), 612.0f, 470.0f };

        DrawRectangleGradientV(0, hudBottom, context.screenWidth, context.screenHeight - hudBottom, context.theme.battleBackA, context.theme.battleBackB);

        DrawRectangleRounded(alliesArea, 0.04f, 8, Fade(Color{ 32, 56, 78, 255 }, 0.55f));
        DrawRectangleRoundedLinesEx(alliesArea, 0.04f, 8, 2.0f, Fade(context.theme.selectionColor, 0.6f));
        DrawRectangleRounded(enemiesArea, 0.04f, 8, Fade(Color{ 84, 36, 44, 255 }, 0.55f));
        DrawRectangleRoundedLinesEx(enemiesArea, 0.04f, 8, 2.0f, Fade(context.theme.dangerColor, 0.65f));

        DrawRectangleRounded(turnOrderRect, 0.08f, 8, context.theme.panelColor);
        DrawRectangleRoundedLinesEx(turnOrderRect, 0.08f, 8, 2.0f, context.theme.panelBorderColor);

        const Font font = ResolveUiFont(context);

        DrawTextEx(font, model.battleTitle.c_str(), { 24.0f, static_cast<float>(hudBottom + 18) }, context.smallFontSize, 1.0f, context.theme.highlightTextColor);
        DrawTextEx(font, "Allies", { alliesArea.x + 10.0f, alliesArea.y + 8.0f }, context.smallFontSize, 1.0f, context.theme.selectionColor);
        DrawTextEx(font, "Enemies", { enemiesArea.x + 10.0f, enemiesArea.y + 8.0f }, context.smallFontSize, 1.0f, context.theme.dangerColor);

        float chipX = 220.0f;
        int chipIndex = 0;
        for (const auto& entry : model.turnOrder)
        {
            const Rectangle chip{ chipX, static_cast<float>(hudBottom + 18), 140.0f, 28.0f };
            const bool next = chipIndex == 0;
            DrawRectangleRounded(chip, 0.18f, 6, next ? Fade(context.theme.highlightTextColor, 0.23f) : Fade(context.theme.selectionColor, 0.18f));
            DrawRectangleRoundedLinesEx(chip, 0.18f, 6, 1.0f, next ? context.theme.highlightTextColor : context.theme.selectionColor);
            DrawTextEx(font, TextFormat("%d", chipIndex + 1), { chip.x + 6.0f, chip.y + 5.0f }, 16.0f, 1.0f, context.theme.mutedTextColor);
            DrawTextEx(font, entry.c_str(), { chip.x + 24.0f, chip.y + 5.0f }, 16.0f, 1.0f, context.theme.textColor);
            chipX += 150.0f;
            ++chipIndex;
        }

        for (const auto& unit : model.units)
        {
            DrawUnitCard(context, unit);
        }

        DrawRectangleRounded(actionRect, 0.05f, 8, context.theme.panelColor);
        DrawRectangleRoundedLinesEx(actionRect, 0.05f, 8, 2.0f, context.theme.panelBorderColor);

        float actionX = actionRect.x + 18.0f;
        const float actionY = actionRect.y + 16.0f;

        for (const auto& action : model.actions)
        {
            const Rectangle button{ actionX, actionY, 140.0f, 42.0f };
            const Color fill = action.selected
                ? Fade(context.theme.selectionColor, 0.25f)
                : Fade(context.theme.panelBorderColor, 0.12f);
            const Color border = action.enabled
                ? (action.selected ? context.theme.selectionColor : context.theme.panelBorderColor)
                : context.theme.mutedTextColor;
            const Color text = action.enabled ? context.theme.textColor : context.theme.mutedTextColor;

            DrawRectangleRounded(button, 0.2f, 6, fill);
            DrawRectangleRoundedLinesEx(button, 0.2f, 6, 2.0f, border);
            DrawTextEx(font, action.label.c_str(), { button.x + 16.0f, button.y + 10.0f }, context.smallFontSize, 1.0f, text);

            actionX += 152.0f;
        }

        if (!model.statusText.empty())
        {
            DrawTextEx(font, model.statusText.c_str(), { actionRect.x + 18.0f, actionRect.y + 74.0f }, context.smallFontSize, 1.0f, context.theme.textColor);
        }

        if (!model.targetHint.empty())
        {
            DrawTextEx(font, model.targetHint.c_str(), { actionRect.x + 18.0f, actionRect.y + 102.0f }, context.smallFontSize, 1.0f, context.theme.mutedTextColor);
        }

        DrawTextEx(font,
            "Controls: Left/Right action | Up/Down target | Enter confirm",
            { actionRect.x + 500.0f, actionRect.y + 102.0f },
            context.smallFontSize,
            1.0f,
            context.theme.mutedTextColor);
    }
}
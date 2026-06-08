#include "ScenarioResultRenderer.h"

namespace ashvale::rendering
{
    void ScenarioResultRenderer::Draw(const RenderContext& context, const ScenarioResultModel& model) const
    {
        ClearBackground(context.theme.clearColor);

        const Font font = ResolveUiFont(context);

        DrawTextEx(font, model.title.c_str(), { 70.0f, 70.0f }, context.titleFontSize, 2.0f,
            context.theme.textColor);

        const Color outcomeColor = model.victory
            ? context.theme.successColor
            : context.theme.dangerColor;
        DrawTextEx(font, model.outcomeLabel.c_str(), { 74.0f, 150.0f }, context.titleFontSize, 2.0f,
            outcomeColor);

        float y = 210.0f;
        if (!model.reason.empty())
        {
            DrawTextEx(font, model.reason.c_str(), { 74.0f, y }, context.normalFontSize, 1.0f,
                context.theme.mutedTextColor);
            y += 44.0f;
        }

        if (!model.nextStepText.empty())
        {
            DrawTextEx(font, model.nextStepText.c_str(), { 74.0f, y }, context.normalFontSize, 1.0f,
                context.theme.highlightTextColor);
        }

        DrawTextEx(font, model.footerHint.c_str(),
            { 70.0f, static_cast<float>(context.screenHeight - 48) },
            context.smallFontSize, 1.0f, context.theme.mutedTextColor);
    }
}

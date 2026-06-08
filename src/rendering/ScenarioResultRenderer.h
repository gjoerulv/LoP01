#pragma once

#include <string>

#include "RenderContext.h"

namespace ashvale::rendering
{
    // Full-screen end-of-scenario result model. Built by ScenarioResultModelMapper
    // from the latched ScenarioOutcome and campaign context; pure presentation data.
    struct ScenarioResultModel
    {
        std::string title = "Scenario Result";
        std::string outcomeLabel;   // "Victory!" / "Defeat."
        bool victory = false;       // selects success vs danger accent color
        std::string reason;         // latched outcome reason (may be empty)
        std::string nextStepText;   // "Next: <scenario>" / "Campaign complete" / "Campaign failed" / "Scenario ended"
        std::string footerHint = "Press Enter to continue";
    };

    class ScenarioResultRenderer
    {
    public:
        void Draw(const RenderContext& context, const ScenarioResultModel& model) const;
    };
}

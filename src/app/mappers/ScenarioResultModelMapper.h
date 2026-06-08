#pragma once

#include "data/ContentRepository.h"
#include "gameplay/GameSession.h"
#include "rendering/ScenarioResultRenderer.h"

namespace app::mappers
{
    class ScenarioResultModelMapper
    {
    public:
        // Builds the end-of-scenario result screen model from the session's
        // latched outcome and campaign context. Pure read; mutates nothing.
        [[nodiscard]] ashvale::rendering::ScenarioResultModel Map(
            const data::ContentRepository& content,
            const gameplay::GameSession& session) const;
    };
}

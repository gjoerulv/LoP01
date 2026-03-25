#pragma once

#include <string>
#include <vector>

#include "gameplay/GameSession.h"
#include "gameplay/quests/QuestState.h"
#include "rendering/HudRenderer.h"
#include "data/ContentRepository.h"

namespace app::mappers
{
    class HudModelMapper
    {
    public:
        [[nodiscard]] ashvale::rendering::HudModel Map(
            const data::ContentRepository& content,
            const gameplay::GameSession& session,
            const gameplay::SessionSnapshot& snapshot,
            const std::string& statusText,
            const std::vector<gameplay::quests::QuestProgress>& quests) const;
    };
}
#pragma once

#include <string>

#include "gameplay/GameSession.h"
#include "rendering/HudRenderer.h"

namespace app::mappers
{
    class HudModelMapper
    {
    public:
        [[nodiscard]] ashvale::rendering::HudModel Map(
            const gameplay::SessionSnapshot& snapshot,
            const std::string& statusText) const;
    };
}
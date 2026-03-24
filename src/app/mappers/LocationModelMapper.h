#pragma once

#include <string>

#include "gameplay/GameSession.h"
#include "gameplay/location/LocationScene.h"
#include "rendering/LocationRenderer.h"
#include "data/ContentRepository.h"

namespace app::mappers
{
    class LocationModelMapper
    {
    public:
        [[nodiscard]] ashvale::rendering::LocationRenderModel Map(
            const data::ContentRepository& content,
            const gameplay::GameSession& session,
            const gameplay::SessionSnapshot& snapshot,
            const gameplay::location::LocationScene& scene,
            const std::string& statusText) const;
    };
}
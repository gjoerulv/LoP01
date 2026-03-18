#pragma once

#include <string>

#include "gameplay/GameSession.h"
#include "gameplay/location/LocationScene.h"
#include "rendering/LocationRenderer.h"

namespace app::mappers
{
    class LocationModelMapper
    {
    public:
        [[nodiscard]] ashvale::rendering::LocationRenderModel Map(
            const gameplay::SessionSnapshot& snapshot,
            const gameplay::location::LocationScene& scene,
            const std::string& statusText) const;
    };
}
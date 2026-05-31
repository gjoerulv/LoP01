#pragma once

#include "data/ContentRepository.h"
#include "gameplay/GameSession.h"
#include "rendering/WorldMapRenderer.h"

namespace app::mappers
{
    class WorldMapModelMapper
    {
    public:
        // Builds the World Map screen model: the destinations are every World Map
        // entry other than the current region, each annotated with travel
        // legality/days/cost (via the pure WorldMapTravelRules) and the
        // generic-loss warning count. `selectedIndex` highlights one destination.
        [[nodiscard]] ashvale::rendering::WorldMapModel Map(
            const data::ContentRepository& content,
            const gameplay::GameSession& session,
            int selectedIndex) const;
    };
}

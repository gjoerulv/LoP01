#pragma once

#include "data/ContentRepository.h"
#include "gameplay/GameSession.h"
#include "rendering/OwnedServiceOverviewRenderer.h"

namespace app::mappers
{
    // Assembles the M27 read-only owned-service overview model from existing
    // GameSession accessors and content display data. Pure read: it calls only
    // const session accessors (OwnedServices, FindLocationServiceById,
    // PreviewMineDailyOutput, OwnedTraderServiceTierForService,
    // HostileOccupiedNodeIds) and never mutates ownership, stationing, or payout.
    // Lists only services the player currently owns.
    class OwnedServiceOverviewModelMapper
    {
    public:
        [[nodiscard]] ashvale::rendering::OwnedServiceOverviewModel Map(
            const data::ContentRepository& content,
            const gameplay::GameSession& session) const;
    };
}

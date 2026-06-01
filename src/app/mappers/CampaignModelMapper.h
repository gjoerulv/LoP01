#pragma once

#include "data/ContentRepository.h"
#include "rendering/CampaignSelectRenderer.h"

namespace app::mappers
{
    class CampaignModelMapper
    {
    public:
        // Builds the Campaign Selection screen model from the loaded campaigns.
        // `selectedIndex` highlights one row.
        [[nodiscard]] ashvale::rendering::CampaignSelectModel Map(
            const data::ContentRepository& content,
            int selectedIndex) const;
    };
}

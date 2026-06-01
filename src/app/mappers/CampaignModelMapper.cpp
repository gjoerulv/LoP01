#include "app/mappers/CampaignModelMapper.h"

namespace app::mappers
{
    ashvale::rendering::CampaignSelectModel CampaignModelMapper::Map(
        const data::ContentRepository& content,
        const int selectedIndex) const
    {
        ashvale::rendering::CampaignSelectModel model;

        const auto& campaigns = content.Campaigns();
        for (int i = 0; i < static_cast<int>(campaigns.size()); ++i)
        {
            const auto& campaign = campaigns[i];
            ashvale::rendering::CampaignSelectRow row;
            row.id = campaign.id;
            row.name = campaign.name.empty() ? campaign.id : campaign.name;
            row.description = campaign.description;
            row.selected = (i == selectedIndex);
            model.campaigns.push_back(std::move(row));
        }

        return model;
    }
}

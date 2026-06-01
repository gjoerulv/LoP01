#include "app/mappers/HudModelMapper.h"

#include <algorithm>

namespace app::mappers
{
    ashvale::rendering::HudModel HudModelMapper::Map(
        const data::ContentRepository& content,
        const gameplay::GameSession& session,
        const gameplay::SessionSnapshot& snapshot,
        const std::string& statusText,
        const std::vector<gameplay::quests::QuestProgress>& quests) const
    {
        int completed = 0;
        std::vector<std::string> activeQuestNames;

        for (const auto& quest : quests)
        {
            if (quest.status == gameplay::quests::QuestStatus::Completed)
            {
                ++completed;
            }

            if (quest.status == gameplay::quests::QuestStatus::InProgress)
            {
                activeQuestNames.push_back(quest.name);
            }
        }

        std::string questInfo = "Quests " + std::to_string(completed) + "/" + std::to_string(quests.size()) + " complete";
        if (!activeQuestNames.empty())
        {
            questInfo += " | Active: ";
            const int maxQuestsToShow = std::min(2, static_cast<int>(activeQuestNames.size()));
            for (int i = 0; i < maxQuestsToShow; ++i)
            {
                if (i > 0)
                {
                    questInfo += ", ";
                }

                questInfo += activeQuestNames[i];
            }

            if (static_cast<int>(activeQuestNames.size()) > maxQuestsToShow)
            {
                questInfo += ", ...";
            }
        }

        ashvale::rendering::HudModel model;
        model.day = snapshot.day;
        model.week = ((snapshot.day - 1) / 7) + 1;
        model.timeText = snapshot.time;
        model.gold = snapshot.gold;
        model.energy = snapshot.energy;
        model.maxEnergy = snapshot.maxEnergy;
        model.questCompactText = "Q" + std::to_string(completed) + "/" + std::to_string(quests.size());
        if (session.HasActiveSameDayTravelPrep()) {
            model.activeBuffIcons.push_back("travel_prep");
        }

        model.primaryAreaLabel = "Region:";
        if (const auto* region = content.FindRegionById(snapshot.regionId)) {
            model.primaryAreaValue = region->name;
        }
        else {
            model.primaryAreaValue = snapshot.regionId;
        }

        model.secondaryAreaLabel = "Location:";
        if (const auto* location = content.FindLocationById(snapshot.destinationId)) {
            model.secondaryAreaValue = location->name;
        }
        else {
            model.secondaryAreaValue = snapshot.destinationId;
        }

        // M16-c: compact campaign/scenario status when a campaign is active.
        if (snapshot.campaignState != gameplay::CampaignState::None &&
            !snapshot.campaignId.empty()) {
            std::string stateLabel;
            switch (snapshot.campaignState) {
            case gameplay::CampaignState::InProgress: stateLabel = snapshot.currentScenarioId; break;
            case gameplay::CampaignState::Completed:  stateLabel = "COMPLETE"; break;
            case gameplay::CampaignState::Failed:     stateLabel = "FAILED"; break;
            case gameplay::CampaignState::None:       break;
            }
            model.campaignText = snapshot.campaignId;
            if (!stateLabel.empty()) {
                model.campaignText += " - " + stateLabel;
            }
        }

        (void)statusText;
        (void)questInfo;
        model.showStatus = false;
        return model;
    }
}
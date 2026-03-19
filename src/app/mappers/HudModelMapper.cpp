#include "app/mappers/HudModelMapper.h"

#include <algorithm>

namespace app::mappers
{
    ashvale::rendering::HudModel HudModelMapper::Map(
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
        model.modeLabel = gameplay::GameSession::ToString(snapshot.mode);
        model.day = snapshot.day;
        model.timeText = snapshot.time;
        model.gold = snapshot.gold;
        model.primaryAreaLabel = "Region:";
        model.primaryAreaValue = snapshot.regionId;
        model.secondaryAreaLabel = "Location:";
        model.secondaryAreaValue = snapshot.destinationId;
        model.statusText = statusText.empty() ? questInfo : statusText + " | " + questInfo;
        model.showStatus = true;
        return model;
    }
}
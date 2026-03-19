#include "app/mappers/HudModelMapper.h"

#include <algorithm>
#include <cctype>

namespace app::mappers
{
    namespace {
        std::string HumanizeId(std::string value)
        {
            bool nextUpper = true;
            for (char& ch : value)
            {
                if (ch == '_')
                {
                    ch = ' ';
                    nextUpper = true;
                    continue;
                }

                if (nextUpper)
                {
                    ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
                    nextUpper = false;
                }
            }

            return value;
        }
    }

    ashvale::rendering::HudModel HudModelMapper::Map(
        const gameplay::SessionSnapshot& snapshot,
        const std::string& statusText,
        const std::vector<gameplay::quests::QuestProgress>& quests) const
    {
        int completed = 0;
        std::vector<std::string> activeTargets;

        for (const auto& quest : quests)
        {
            if (quest.status == gameplay::quests::QuestStatus::Completed)
            {
                ++completed;
            }

            if (quest.status == gameplay::quests::QuestStatus::InProgress)
            {
                activeTargets.push_back(HumanizeId(quest.target));
            }
        }

        std::string questInfo = "Quests " + std::to_string(completed) + "/" + std::to_string(quests.size()) + " complete";
        if (!activeTargets.empty())
        {
            questInfo += " | Targets: ";
            const int maxTargetsToShow = std::min(2, static_cast<int>(activeTargets.size()));
            for (int i = 0; i < maxTargetsToShow; ++i)
            {
                if (i > 0)
                {
                    questInfo += ", ";
                }

                questInfo += activeTargets[i];
            }

            if (static_cast<int>(activeTargets.size()) > maxTargetsToShow)
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
#include "app/BattleEventTextFormatter.h"

#include <string>
#include <vector>

namespace app
{
    namespace
    {
        std::string WinnerText(const gameplay::battle::TeamSide side)
        {
            return side == gameplay::battle::TeamSide::Allies
                ? "Allies win the battle"
                : "Enemies win the battle";
        }
    }

    std::string BattleEventTextFormatter::FormatSummary(
        const std::vector<gameplay::battle::BattleEvent>& events) const
    {
        if (events.empty())
        {
            return {};
        }

        std::vector<std::string> parts;
        parts.reserve(events.size());

        for (const auto& event : events)
        {
            switch (event.type)
            {
            case gameplay::battle::BattleEventType::AttackResolved:
                parts.push_back(event.actorName + " attacks " + event.targetName + " for " + std::to_string(event.amount));
                break;

            case gameplay::battle::BattleEventType::SkillResolved:
                parts.push_back(event.actorName + " uses skill on " + event.targetName + " for " + std::to_string(event.amount));
                break;

            case gameplay::battle::BattleEventType::Defended:
                parts.push_back(event.actorName + " defends");
                break;

            case gameplay::battle::BattleEventType::Waited:
                parts.push_back(event.actorName + " waits");
                break;

            case gameplay::battle::BattleEventType::UnitKo:
                parts.push_back(event.targetName + " is knocked out");
                break;

            case gameplay::battle::BattleEventType::BattleFinished:
                parts.push_back(WinnerText(event.winner));
                break;
            case gameplay::battle::BattleEventType::Information:
                if (!event.infoText.empty())
                {
                    parts.push_back(event.infoText);
                }
                break;
            }
        }

        std::string result;
        for (size_t i = 0; i < parts.size(); ++i)
        {
            if (i > 0)
            {
                result += " | ";
            }
            result += parts[i];
        }

        return result;
    }
}
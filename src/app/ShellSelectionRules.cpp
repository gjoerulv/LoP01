#include "app/ShellSelectionRules.h"

namespace app::shell
{
    ShellEntryStatus ContentGateStatus(const bool contentLoaded, const int validationErrorCount)
    {
        if (!contentLoaded)
        {
            return {false, "Installed content could not be loaded."};
        }
        if (validationErrorCount > 0)
        {
            return {false, "Installed content failed validation ("
                + std::to_string(validationErrorCount) + " error(s))."};
        }
        return {true, ""};
    }

    ShellEntryStatus ScenarioPlayability(
        const data::ContentRepository& content, const data::ScenarioDefinition& scenario)
    {
        if (scenario.startRegionId.empty())
        {
            return {false, "No starting region authored."};
        }
        const auto* region = content.FindRegionById(scenario.startRegionId);
        if (region == nullptr)
        {
            return {false, "Missing starting region."};
        }
        if (!scenario.startNodeId.empty())
        {
            const bool nodeExists = [&] {
                for (const auto& node : region->nodes)
                {
                    if (node.locationId == scenario.startNodeId)
                    {
                        return true;
                    }
                }
                return false;
            }();
            if (!nodeExists)
            {
                return {false, "Missing starting node."};
            }
        }
        else if (region->arrivalNodeId.empty())
        {
            return {false, "Starting region has no arrival node."};
        }
        return {true, ""};
    }

    ShellEntryStatus CampaignPlayability(
        const data::ContentRepository& content, const data::CampaignDefinition& campaign)
    {
        if (campaign.startScenarioId.empty())
        {
            return {false, "No starting scenario authored."};
        }
        const auto* scenario = content.FindScenarioById(campaign.startScenarioId);
        if (scenario == nullptr)
        {
            return {false, "Missing starting scenario."};
        }
        const ShellEntryStatus start = ScenarioPlayability(content, *scenario);
        if (!start.playable)
        {
            return {false, "Starting scenario is unplayable: " + start.reason};
        }
        return {true, ""};
    }

    std::vector<const data::ScenarioDefinition*> StandaloneScenarios(
        const data::ContentRepository& content)
    {
        std::vector<const data::ScenarioDefinition*> result;
        for (const auto& scenario : content.Scenarios())
        {
            if (scenario.standaloneSelectable)
            {
                result.push_back(&scenario);
            }
        }
        return result;
    }
}

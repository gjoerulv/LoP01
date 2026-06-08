#include "app/mappers/ScenarioResultModelMapper.h"

#include <string>

#include "data/definitions/CampaignDefinition.h"
#include "data/definitions/ScenarioDefinition.h"
#include "gameplay/campaign/CampaignProgressionRules.h"
#include "gameplay/scenario/ScenarioOutcomeRules.h"

namespace app::mappers
{
    namespace
    {
        // Display name for a scenario id, falling back to the id when the
        // scenario is unknown or carries no authored name.
        std::string ScenarioName(const data::ContentRepository& content, const std::string& scenarioId)
        {
            for (const auto& scenario : content.Scenarios())
            {
                if (scenario.id == scenarioId)
                {
                    return scenario.name.empty() ? scenario.id : scenario.name;
                }
            }
            return scenarioId;
        }

        const data::CampaignDefinition* FindCampaign(
            const data::ContentRepository& content, const std::string& campaignId)
        {
            for (const auto& campaign : content.Campaigns())
            {
                if (campaign.id == campaignId)
                {
                    return &campaign;
                }
            }
            return nullptr;
        }
    }

    ashvale::rendering::ScenarioResultModel ScenarioResultModelMapper::Map(
        const data::ContentRepository& content,
        const gameplay::GameSession& session) const
    {
        ashvale::rendering::ScenarioResultModel model;

        const auto& outcome = session.Outcome();
        if (!outcome.has_value())
        {
            return model;
        }

        using gameplay::scenario::ScenarioOutcomeState;
        const bool victory = outcome->state == ScenarioOutcomeState::Victory;
        model.victory = victory;
        model.outcomeLabel = victory ? "Victory!" : "Defeat.";
        model.reason = outcome->reason;

        const std::string currentName = ScenarioName(content, session.CurrentScenarioId());
        if (!currentName.empty())
        {
            model.title = currentName;
        }

        if (session.IsCampaignActive())
        {
            const data::CampaignDefinition* campaign = FindCampaign(content, session.CampaignId());
            if (campaign != nullptr)
            {
                const auto next = gameplay::campaign::ResolveNextScenarioId(
                    *campaign, session.CurrentScenarioId(), outcome->state);
                if (next.has_value())
                {
                    model.nextStepText = "Next: " + ScenarioName(content, *next);
                }
                else
                {
                    model.nextStepText = victory ? "Campaign complete" : "Campaign failed";
                }
            }
            else
            {
                model.nextStepText = "Scenario ended";
            }
        }
        else
        {
            model.nextStepText = "Scenario ended";
        }

        return model;
    }
}

#include "gameplay/campaign/CampaignProgressionRules.h"

namespace gameplay::campaign {

std::optional<std::string> ResolveNextScenarioId(
    const data::CampaignDefinition& campaign,
    const std::string& currentScenarioId,
    scenario::ScenarioOutcomeState outcome)
{
    if (outcome != scenario::ScenarioOutcomeState::Victory) {
        return std::nullopt;
    }

    const data::CampaignScenarioEntry* entry = campaign.FindScenarioEntry(currentScenarioId);
    if (entry == nullptr || entry->nextScenarioIds.empty()) {
        return std::nullopt;
    }

    // Linear progression: first listed next scenario (branching deferred).
    return entry->nextScenarioIds.front();
}

} // namespace gameplay::campaign

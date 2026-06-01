#pragma once

#include <optional>
#include <string>

#include "data/definitions/CampaignDefinition.h"
#include "gameplay/scenario/ScenarioOutcomeRules.h"

namespace gameplay::campaign {

// Pure campaign progression. Raylib-free, content-free beyond the passed
// CampaignDefinition. Deterministic.
//
// Resolves the next scenario id for a campaign given the current scenario and
// its latched outcome:
//   - advance only on Victory (defeat ends the campaign; ongoing never advances);
//   - the current scenario must be a node in the campaign's transition graph;
//   - linear pick: the first entry of nextScenarioIds (branching deferred);
//   - returns nullopt when there is no next scenario (final scenario / no edge),
//     when the outcome is not Victory, or when currentScenarioId is unknown.
[[nodiscard]] std::optional<std::string> ResolveNextScenarioId(
    const data::CampaignDefinition& campaign,
    const std::string& currentScenarioId,
    scenario::ScenarioOutcomeState outcome);

} // namespace gameplay::campaign

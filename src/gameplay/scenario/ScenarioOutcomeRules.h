#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "data/definitions/ScenarioOutcomeDefinition.h"
#include "gameplay/EnemyTeamState.h"
#include "gameplay/events/EventDefinition.h"
#include "gameplay/events/EventEngine.h"

namespace gameplay::scenario {

enum class ScenarioOutcomeState { Ongoing, Victory, Defeat };

struct ScenarioOutcome {
    ScenarioOutcomeState state = ScenarioOutcomeState::Ongoing;
    // For Victory/Defeat: index into the authored victoryConditions /
    // defeatConditions list. Absent when default victory fires (no authored
    // victory conditions, all hostile teams cleared).
    std::optional<std::size_t> matchedConditionIndex;
    // Human-readable reason for the latched outcome. Empty for Ongoing.
    std::string reason;
};

// Read-only inputs for outcome evaluation. The rule never mutates anything.
struct ScenarioOutcomeContext {
    std::string playerColor;
    const std::vector<EnemyTeamState>* enemyTeams = nullptr;
    const events::EventEvaluationContext* conditionContext = nullptr;
};

// A team is "hostile to the player" iff it is active, not the player's own
// team, and not allied with the player. Allied or removed teams do not block
// default victory.
[[nodiscard]] bool IsHostileToPlayer(
    const EnemyTeamState& team,
    const std::string& playerColor);

// Pure evaluation. Defeat wins over victory when both match in the same call.
// Default victory ("no hostile teams remain") fires only when victoryConditions
// is empty and there are no hostile teams left.
[[nodiscard]] ScenarioOutcome EvaluateScenarioOutcome(
    const ScenarioOutcomeContext& ctx,
    const data::ScenarioOutcomeDefinition& outcome);

} // namespace gameplay::scenario

#pragma once

#include <vector>

#include "gameplay/events/EventDefinition.h"

namespace data {

// Single authored file describing scenario win/loss state.
//
// Both lists are OR-structured (one match suffices). Defeat takes priority over
// victory when both match in the same evaluation — see core_loop_rules.md §36.
//
// An empty victoryConditions list activates default victory ("all hostile enemy
// teams defeated/removed"); an empty defeatConditions list means the scenario
// has no authored defeat path.
struct ScenarioOutcomeDefinition {
    std::vector<gameplay::events::EventCondition> victoryConditions;
    std::vector<gameplay::events::EventCondition> defeatConditions;
};

} // namespace data

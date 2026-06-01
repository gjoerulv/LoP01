#pragma once

#include <optional>
#include <string>
#include <vector>

#include "gameplay/events/EventDefinition.h"

namespace data {

// M16-a: thin Scenario content kind. A Scenario selects a starting Region/node
// and optionally its own win/loss conditions. Regions, units, and the World Map
// remain globally loaded in M16 — a Scenario does NOT own a per-scenario content
// partition, hero pool, Region context, banned-skill list, or roster default yet
// (all deferred; see docs/content_schema.md §9 and the M16 plan).
//
// Outcome fallback (preserves M12 exactly):
//   - If the authored JSON carries a "victoryConditions" or "defeatConditions"
//     key, hasInlineOutcome is true and the scenario uses its own inline lists
//     (which may be empty — an empty victory list still drives M12 default-victory
//     semantics inside ScenarioOutcomeRules).
//   - If neither key is present, hasInlineOutcome is false and the scenario uses
//     the global content/scenario_outcome.json definition exactly as M12 does.
struct ScenarioDefinition {
    std::string id;
    std::string name;
    std::string startRegionId;
    std::string startNodeId;             // optional; empty => region arrivalNodeId
    std::optional<int> startGold;        // optional; absent => GameSession default
    bool standaloneSelectable = false;

    bool hasInlineOutcome = false;       // set by loader from JSON key presence
    std::vector<gameplay::events::EventCondition> victoryConditions;
    std::vector<gameplay::events::EventCondition> defeatConditions;
};

} // namespace data

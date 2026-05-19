#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

#include "gameplay/events/EventDefinition.h"

namespace gameplay::events {

enum class EnemyTeamMutationType { Spawn, Remove, ChangeAlliance };

struct EnemyTeamMutation {
    EnemyTeamMutationType type = EnemyTeamMutationType::Spawn;
    std::string teamColor;
    std::string nodeId;       // Spawn only
    std::string allyColor;    // ChangeAlliance only
    bool addAlliance = true;  // ChangeAlliance: true = add, false = remove
};

struct EventEvaluationContext {
    int currentDay = 0;
    std::map<std::string, int> resources;    // "Wood" -> 10
    std::vector<std::string>   heroIds;      // heroes in traveling party
    std::set<std::string>      storyFlags;   // currently-set story flag names
    std::vector<EnemyTeamMutation>* pendingTeamMutations = nullptr;
};

struct ActionResult {
    bool        success = true;
    std::string message;   // player-facing or debug message; empty on clean success
};

// Pure evaluation — never mutates context. Returns true for Unknown (absent condition).
[[nodiscard]] bool EvaluateCondition(
    const EventEvaluationContext& ctx,
    const EventCondition& cond);

// Executes a single action; mutates ctx on success.
ActionResult ExecuteAction(
    EventEvaluationContext& ctx,
    const EventAction& action);

// Executes actions in authored order; non-atomic (no rollback on failure).
std::vector<ActionResult> ExecuteActions(
    EventEvaluationContext& ctx,
    const std::vector<EventAction>& actions);

} // namespace gameplay::events

#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

#include "gameplay/events/EventDefinition.h"

// Forward declarations let EventEngine.h stay free of inventory-system
// dependencies. EventEngine.cpp pulls the real headers in.
namespace gameplay {
    struct ItemStackState;
    struct ArtifactStackState;
}
namespace data {
    struct ItemDefinition;
    struct ArtifactDefinition;
}

namespace gameplay::events {

enum class EnemyTeamMutationType { Spawn, Remove, ChangeAlliance };

struct EnemyTeamMutation {
    EnemyTeamMutationType type = EnemyTeamMutationType::Spawn;
    std::string teamColor;
    std::string nodeId;       // Spawn only
    std::string allyColor;    // ChangeAlliance only
    bool addAlliance = true;  // ChangeAlliance: true = add, false = remove
    // M30, Spawn only: optional authored enemy-group id giving the team its
    // deterministic service-attack strength. Empty leaves any existing value.
    std::string enemyGroupId;
};

struct EventEvaluationContext {
    int currentDay = 0;
    std::map<std::string, int> resources;    // "Wood" -> 10
    std::vector<std::string>   heroIds;      // heroes in traveling party
    std::set<std::string>      storyFlags;   // currently-set story flag names
    std::vector<EnemyTeamMutation>* pendingTeamMutations = nullptr;

    // M13-b inventory action surface. All four pointers must be non-null for
    // give/take/Item/Artifact actions to succeed. The actions mutate the
    // *items / *artifacts vectors directly and look up authored metadata via
    // the *Catalog pointers. If any pointer is null the action fails with an
    // explicit "context unavailable" message — mirroring the enemy-team
    // mutation null-context pattern.
    std::vector<gameplay::ItemStackState>*       items            = nullptr;
    std::vector<gameplay::ArtifactStackState>*   artifacts        = nullptr;
    const std::vector<data::ItemDefinition>*     itemCatalog      = nullptr;
    const std::vector<data::ArtifactDefinition>* artifactCatalog  = nullptr;
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

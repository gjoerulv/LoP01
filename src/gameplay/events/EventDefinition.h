#pragma once

#include <optional>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace gameplay::events {

enum class EventTriggerType {
    Unknown,
    RegionNodeEntry,
    StartOfDay,
    LocationCollision,
    LocationConfirm,
    NeutralEncounterDefeated,
    ServiceUsed,
    ServiceDestroyed,
    QuestCompletion
};

inline EventTriggerType EventTriggerTypeFromString(const std::string& s)
{
    if (s == "regionNodeEntry")           return EventTriggerType::RegionNodeEntry;
    if (s == "startOfDay")                return EventTriggerType::StartOfDay;
    if (s == "locationCollision")         return EventTriggerType::LocationCollision;
    if (s == "locationConfirm")           return EventTriggerType::LocationConfirm;
    if (s == "neutralEncounterDefeated")  return EventTriggerType::NeutralEncounterDefeated;
    if (s == "serviceUsed")               return EventTriggerType::ServiceUsed;
    if (s == "serviceDestroyed")          return EventTriggerType::ServiceDestroyed;
    if (s == "questCompletion")           return EventTriggerType::QuestCompletion;
    return EventTriggerType::Unknown;
}

struct EventTrigger {
    EventTriggerType type = EventTriggerType::Unknown;
    std::string targetId;   // nodeId / serviceId / questId — type-specific
};

struct EventEligibility {
    std::vector<std::string> teamColors;
    std::vector<std::string> teamKinds;
    std::vector<std::string> requiredHeroIds;
};

enum class EventConditionKind { Unknown, Leaf, All, Any, Not };

// Recursive tree node. std::vector<EventCondition> with an incomplete element type
// is valid in C++17+ per [vector.overview] allocator completeness requirements.
struct EventCondition {
    EventConditionKind kind = EventConditionKind::Unknown;
    std::string        leafType;           // Leaf only — e.g. "teamHasResource"
    nlohmann::json     leafArgs;           // Leaf only — type-specific fields; fully typed in M10-b
    std::vector<EventCondition> operands;  // All / Any / Not
};

struct EventAction {
    std::string    type;   // required, non-empty
    nlohmann::json args;   // type-specific fields — fully typed in M10-b
};

struct EventRepeat {
    std::string mode = "once";   // "once" | "always" | "everyNDays"
    int intervalDays = 0;
};

struct EventDefinition {
    std::string              id;
    EventTrigger             trigger;
    EventEligibility         eligibility;
    EventCondition           condition;
    std::optional<int>       priority;
    EventRepeat              repeat;
    std::vector<EventAction> actions;
};

} // namespace gameplay::events

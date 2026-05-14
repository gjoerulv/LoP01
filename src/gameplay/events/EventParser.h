#pragma once

#include <vector>
#include <nlohmann/json.hpp>

#include "data/ContentValidator.h"
#include "gameplay/events/EventDefinition.h"

namespace gameplay::events {

// Parses a single condition JSON node (leaf or composite) into an EventCondition tree.
EventCondition ParseCondition(const nlohmann::json& cond);

// Parses a JSON array of action objects into a vector of EventAction.
std::vector<EventAction> ParseActions(const nlohmann::json& actionsArray);

// Parses a single event JSON object into an EventDefinition.
// Unknown or missing optional fields are silently defaulted.
// Does not validate — call ValidateEventDefinition first if authoring checks are needed.
EventDefinition ParseEventDefinition(const nlohmann::json& doc);

// Validates the structure of a single event JSON object.
// Returns an empty vector when the document passes all checked rules.
// Reuses ValidationMessage / Severity from ContentValidator.h.
std::vector<ValidationMessage> ValidateEventDefinition(const nlohmann::json& doc);

} // namespace gameplay::events

#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include "data/definitions/BattleScenarioDefinition.h"
#include "data/definitions/LocationDefinition.h"
#include "data/definitions/LocationSceneDefinition.h"
#include "data/definitions/LocationServiceDefinition.h"
#include "data/definitions/QuestDefinition.h"
#include "data/definitions/RegionDefinition.h"
#include "data/definitions/UnitDefinition.h"

enum class Severity { Error, Warning, Info };

struct ValidationMessage {
    Severity    severity;
    std::string code;        // e.g. "ROOT_NOT_OBJECT"
    std::string path;        // dot-path to the offending field, e.g. "schemaVersion"
    std::string message;
    std::string suggestion;  // empty string if unused
};

class ContentValidator {
public:
    // Validates the required top-level identity fields of a single content document:
    // schemaVersion (integer), kind (non-empty string), and id (non-empty string).
    // If the root is not a JSON object, returns only ROOT_NOT_OBJECT and no further messages.
    // Returns an empty vector when the document passes all checked rules.
    std::vector<ValidationMessage> ValidateIdentity(const nlohmann::json& doc) const;

    // Validates cross-references across all loaded content collections.
    // Returns Error messages for broken references. Does not gate on identity fields.
    std::vector<ValidationMessage> ValidateReferences(
        const std::vector<data::RegionDefinition>& regions,
        const std::vector<data::LocationDefinition>& locations,
        const std::vector<data::LocationSceneDefinition>& scenes,
        const std::vector<data::UnitDefinition>& units,
        const std::vector<data::BattleScenarioDefinition>& battleScenarios,
        const std::vector<data::LocationServiceDefinition>& services,
        const std::vector<data::QuestDefinition>& quests) const;
};

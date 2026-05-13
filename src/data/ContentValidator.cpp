#include "data/ContentValidator.h"

std::vector<ValidationMessage> ContentValidator::ValidateIdentity(const nlohmann::json& doc) const
{
    std::vector<ValidationMessage> msgs;

    if (!doc.is_object()) {
        msgs.push_back({
            Severity::Error,
            "ROOT_NOT_OBJECT",
            "$",
            "Content document root must be a JSON object.",
            ""
        });
        return msgs;
    }

    // schemaVersion
    if (!doc.contains("schemaVersion")) {
        msgs.push_back({
            Severity::Error,
            "SCHEMA_VERSION_MISSING",
            "schemaVersion",
            "\"schemaVersion\" is required.",
            ""
        });
    } else if (!doc["schemaVersion"].is_number_integer()) {
        msgs.push_back({
            Severity::Error,
            "SCHEMA_VERSION_TYPE",
            "schemaVersion",
            "\"schemaVersion\" must be an integer.",
            ""
        });
    }

    // kind
    if (!doc.contains("kind")) {
        msgs.push_back({
            Severity::Error,
            "KIND_MISSING",
            "kind",
            "\"kind\" is required.",
            ""
        });
    } else if (!doc["kind"].is_string()) {
        msgs.push_back({
            Severity::Error,
            "KIND_TYPE",
            "kind",
            "\"kind\" must be a string.",
            ""
        });
    } else if (doc["kind"].get<std::string>().empty()) {
        msgs.push_back({
            Severity::Error,
            "KIND_EMPTY",
            "kind",
            "\"kind\" must not be empty.",
            ""
        });
    }

    // id
    if (!doc.contains("id")) {
        msgs.push_back({
            Severity::Error,
            "ID_MISSING",
            "id",
            "\"id\" is required.",
            ""
        });
    } else if (!doc["id"].is_string()) {
        msgs.push_back({
            Severity::Error,
            "ID_TYPE",
            "id",
            "\"id\" must be a string.",
            ""
        });
    } else if (doc["id"].get<std::string>().empty()) {
        msgs.push_back({
            Severity::Error,
            "ID_EMPTY",
            "id",
            "\"id\" must not be empty.",
            ""
        });
    }

    return msgs;
}

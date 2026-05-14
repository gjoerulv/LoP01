#include "gameplay/events/EventParser.h"

#include <string>

namespace gameplay::events {

EventCondition ParseCondition(const nlohmann::json& cond)
{
    EventCondition result;

    const bool hasType = cond.contains("type") && cond["type"].is_string();
    const bool hasAll  = cond.contains("all")  && cond["all"].is_array();
    const bool hasAny  = cond.contains("any")  && cond["any"].is_array();
    const bool hasNot  = cond.contains("not")  && cond["not"].is_object();

    if (hasType && !hasAll && !hasAny && !hasNot) {
        result.kind     = EventConditionKind::Leaf;
        result.leafType = cond["type"].get<std::string>();
        result.leafArgs = cond;
    } else if (hasAll) {
        result.kind = EventConditionKind::All;
        for (const auto& child : cond["all"]) {
            result.operands.push_back(ParseCondition(child));
        }
    } else if (hasAny) {
        result.kind = EventConditionKind::Any;
        for (const auto& child : cond["any"]) {
            result.operands.push_back(ParseCondition(child));
        }
    } else if (hasNot) {
        result.kind = EventConditionKind::Not;
        result.operands.push_back(ParseCondition(cond["not"]));
    }
    // else: kind stays Unknown — caught by ValidateEventDefinition

    return result;
}

std::vector<EventAction> ParseActions(const nlohmann::json& actionsArray)
{
    std::vector<EventAction> result;
    if (!actionsArray.is_array()) return result;
    for (const auto& a : actionsArray) {
        if (!a.is_object()) continue;
        EventAction action;
        action.type = a.value("type", "");
        action.args = a;
        result.push_back(std::move(action));
    }
    return result;
}

namespace {

void ValidateConditionNode(const nlohmann::json& cond,
                           const std::string& path,
                           std::vector<ValidationMessage>& msgs)
{
    if (!cond.is_object()) {
        msgs.push_back({Severity::Error, "EVENT_CONDITION_NOT_OBJECT", path,
            "Condition node must be a JSON object.", ""});
        return;
    }

    const bool hasType = cond.contains("type") && cond["type"].is_string();
    const bool hasAll  = cond.contains("all")  && cond["all"].is_array();
    const bool hasAny  = cond.contains("any")  && cond["any"].is_array();
    const bool hasNot  = cond.contains("not")  && cond["not"].is_object();

    const int compositeCount = (hasAll ? 1 : 0) + (hasAny ? 1 : 0) + (hasNot ? 1 : 0);

    if (hasType && compositeCount > 0) {
        msgs.push_back({Severity::Error, "EVENT_CONDITION_AMBIGUOUS", path,
            "Condition node has both a \"type\" leaf field and a composite key (all/any/not). "
            "A condition must be either a leaf or a composite, not both.", ""});
        return;
    }

    if (!hasType && compositeCount == 0) {
        msgs.push_back({Severity::Error, "EVENT_CONDITION_LEAF_TYPE_MISSING", path,
            "Condition node has no \"type\" field and no composite key (all/any/not). "
            "Every condition must be either a typed leaf or a composite.", ""});
        return;
    }

    if (hasAll) {
        for (size_t i = 0; i < cond["all"].size(); ++i) {
            ValidateConditionNode(cond["all"][i], path + ".all[" + std::to_string(i) + "]", msgs);
        }
    } else if (hasAny) {
        for (size_t i = 0; i < cond["any"].size(); ++i) {
            ValidateConditionNode(cond["any"][i], path + ".any[" + std::to_string(i) + "]", msgs);
        }
    } else if (hasNot) {
        ValidateConditionNode(cond["not"], path + ".not", msgs);
    }
}

} // anonymous namespace

EventDefinition ParseEventDefinition(const nlohmann::json& doc)
{
    EventDefinition def;

    if (!doc.is_object())
        return def;

    def.id = doc.value("id", "");

    if (doc.contains("trigger") && doc["trigger"].is_object()) {
        const auto& t = doc["trigger"];
        def.trigger.type     = EventTriggerTypeFromString(t.value("type", ""));
        def.trigger.targetId = t.value("target_id", "");
    }

    if (doc.contains("eligibility") && doc["eligibility"].is_object()) {
        const auto& e = doc["eligibility"];
        if (e.contains("team_colors") && e["team_colors"].is_array()) {
            for (const auto& c : e["team_colors"]) {
                if (c.is_string()) def.eligibility.teamColors.push_back(c.get<std::string>());
            }
        }
        if (e.contains("team_kinds") && e["team_kinds"].is_array()) {
            for (const auto& k : e["team_kinds"]) {
                if (k.is_string()) def.eligibility.teamKinds.push_back(k.get<std::string>());
            }
        }
        if (e.contains("required_hero_ids") && e["required_hero_ids"].is_array()) {
            for (const auto& h : e["required_hero_ids"]) {
                if (h.is_string()) def.eligibility.requiredHeroIds.push_back(h.get<std::string>());
            }
        }
    }

    if (doc.contains("condition") && doc["condition"].is_object()) {
        def.condition = ParseCondition(doc["condition"]);
    }

    if (doc.contains("priority") && doc["priority"].is_number_integer()) {
        def.priority = doc["priority"].get<int>();
    }

    if (doc.contains("repeat") && doc["repeat"].is_object()) {
        const auto& r = doc["repeat"];
        def.repeat.mode         = r.value("mode", "once");
        def.repeat.intervalDays = r.value("interval_days", 0);
    }

    if (doc.contains("actions")) {
        def.actions = ParseActions(doc["actions"]);
    }

    return def;
}

std::vector<ValidationMessage> ValidateEventDefinition(const nlohmann::json& doc)
{
    std::vector<ValidationMessage> msgs;

    if (!doc.is_object()) {
        msgs.push_back({Severity::Error, "EVENT_ROOT_NOT_OBJECT", "$",
            "Event document root must be a JSON object.", ""});
        return msgs;
    }

    // id
    if (!doc.contains("id") || !doc["id"].is_string()) {
        msgs.push_back({Severity::Error, "EVENT_ID_MISSING", "id",
            "Event \"id\" is required and must be a string.", ""});
    } else if (doc["id"].get<std::string>().empty()) {
        msgs.push_back({Severity::Error, "EVENT_ID_EMPTY", "id",
            "Event \"id\" must not be empty.", ""});
    }

    // trigger
    if (!doc.contains("trigger") || !doc["trigger"].is_object()) {
        msgs.push_back({Severity::Error, "EVENT_TRIGGER_MISSING", "trigger",
            "Event \"trigger\" is required and must be an object.", ""});
    } else {
        const auto& t = doc["trigger"];
        const std::string typeStr = t.value("type", "");
        if (EventTriggerTypeFromString(typeStr) == EventTriggerType::Unknown) {
            msgs.push_back({Severity::Error, "EVENT_TRIGGER_TYPE_UNKNOWN", "trigger.type",
                "Event trigger type \"" + typeStr + "\" is not recognised. "
                "Known types: regionNodeEntry, startOfDay, locationCollision, locationConfirm, "
                "neutralEncounterDefeated, serviceUsed, serviceDestroyed, questCompletion.", ""});
        }
    }

    // condition (optional — validated only when present)
    if (doc.contains("condition")) {
        ValidateConditionNode(doc["condition"], "condition", msgs);
    }

    // actions
    if (doc.contains("actions")) {
        if (!doc["actions"].is_array()) {
            msgs.push_back({Severity::Error, "EVENT_ACTIONS_NOT_ARRAY", "actions",
                "Event \"actions\" must be an array.", ""});
        } else {
            for (size_t i = 0; i < doc["actions"].size(); ++i) {
                const auto& a = doc["actions"][i];
                const std::string path = "actions[" + std::to_string(i) + "]";
                if (!a.is_object()) continue;
                if (!a.contains("type") || !a["type"].is_string()) {
                    msgs.push_back({Severity::Error, "EVENT_ACTION_TYPE_MISSING", path + ".type",
                        "Action at index " + std::to_string(i) + " is missing a \"type\" string.", ""});
                } else if (a["type"].get<std::string>().empty()) {
                    msgs.push_back({Severity::Error, "EVENT_ACTION_TYPE_EMPTY", path + ".type",
                        "Action at index " + std::to_string(i) + " has an empty \"type\".", ""});
                }
            }
        }
    }

    return msgs;
}

} // namespace gameplay::events

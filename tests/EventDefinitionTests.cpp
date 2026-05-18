#include <catch2/catch_test_macros.hpp>
#include "gameplay/events/EventParser.h"
#include <nlohmann/json.hpp>

using namespace gameplay::events;

static nlohmann::json ValidEvent()
{
    return nlohmann::json{
        {"id", "evt_test_01"},
        {"trigger", {{"type", "startOfDay"}}},
        {"condition", {{"type", "always"}}},
        {"actions", nlohmann::json::array({{{"type", "showMessage"}, {"text", {{"en", "Hello."}}}}})}
    };
}

// ---------------------------------------------------------------------------
// ValidateEventDefinition — structural rules
// ---------------------------------------------------------------------------

TEST_CASE("ValidateEventDefinition - valid event produces no messages")
{
    auto msgs = ValidateEventDefinition(ValidEvent());
    REQUIRE(msgs.empty());
}

TEST_CASE("ValidateEventDefinition - root array produces EVENT_ROOT_NOT_OBJECT")
{
    auto msgs = ValidateEventDefinition(nlohmann::json::array());
    REQUIRE(msgs.size() == 1);
    REQUIRE(msgs[0].code == "EVENT_ROOT_NOT_OBJECT");
    REQUIRE(msgs[0].severity == Severity::Error);
    REQUIRE(msgs[0].path == "$");
}

TEST_CASE("ValidateEventDefinition - missing id produces EVENT_ID_MISSING")
{
    auto doc = ValidEvent();
    doc.erase("id");
    auto msgs = ValidateEventDefinition(doc);
    REQUIRE(msgs.size() == 1);
    REQUIRE(msgs[0].code == "EVENT_ID_MISSING");
    REQUIRE(msgs[0].severity == Severity::Error);
    REQUIRE(msgs[0].path == "id");
}

TEST_CASE("ValidateEventDefinition - integer id produces EVENT_ID_MISSING")
{
    auto doc = ValidEvent();
    doc["id"] = 42;
    auto msgs = ValidateEventDefinition(doc);
    REQUIRE(msgs.size() == 1);
    REQUIRE(msgs[0].code == "EVENT_ID_MISSING");
}

TEST_CASE("ValidateEventDefinition - empty id produces EVENT_ID_EMPTY")
{
    auto doc = ValidEvent();
    doc["id"] = "";
    auto msgs = ValidateEventDefinition(doc);
    REQUIRE(msgs.size() == 1);
    REQUIRE(msgs[0].code == "EVENT_ID_EMPTY");
    REQUIRE(msgs[0].severity == Severity::Error);
    REQUIRE(msgs[0].path == "id");
}

TEST_CASE("ValidateEventDefinition - missing trigger produces EVENT_TRIGGER_MISSING")
{
    auto doc = ValidEvent();
    doc.erase("trigger");
    auto msgs = ValidateEventDefinition(doc);
    REQUIRE(msgs.size() == 1);
    REQUIRE(msgs[0].code == "EVENT_TRIGGER_MISSING");
    REQUIRE(msgs[0].severity == Severity::Error);
    REQUIRE(msgs[0].path == "trigger");
}

TEST_CASE("ValidateEventDefinition - trigger as string produces EVENT_TRIGGER_MISSING")
{
    auto doc = ValidEvent();
    doc["trigger"] = "startOfDay";
    auto msgs = ValidateEventDefinition(doc);
    REQUIRE(msgs.size() == 1);
    REQUIRE(msgs[0].code == "EVENT_TRIGGER_MISSING");
}

TEST_CASE("ValidateEventDefinition - unknown trigger type produces EVENT_TRIGGER_TYPE_UNKNOWN")
{
    auto doc = ValidEvent();
    doc["trigger"]["type"] = "onMoonrise";
    auto msgs = ValidateEventDefinition(doc);
    REQUIRE(msgs.size() == 1);
    REQUIRE(msgs[0].code == "EVENT_TRIGGER_TYPE_UNKNOWN");
    REQUIRE(msgs[0].severity == Severity::Error);
    REQUIRE(msgs[0].path == "trigger.type");
}

TEST_CASE("ValidateEventDefinition - all 8 known trigger types are valid")
{
    const std::vector<std::string> knownTypes = {
        "regionNodeEntry", "startOfDay", "locationCollision", "locationConfirm",
        "neutralEncounterDefeated", "serviceUsed", "serviceDestroyed", "questCompletion"
    };
    for (const auto& t : knownTypes) {
        auto doc = ValidEvent();
        doc["trigger"]["type"] = t;
        auto msgs = ValidateEventDefinition(doc);
        INFO("Trigger type: " << t);
        const bool hasTypError = std::ranges::any_of(msgs,
            [](const auto& m) { return m.code == "EVENT_TRIGGER_TYPE_UNKNOWN"; });
        REQUIRE_FALSE(hasTypError);
    }
}

TEST_CASE("ValidateEventDefinition - condition with both type and all produces EVENT_CONDITION_AMBIGUOUS")
{
    auto doc = ValidEvent();
    doc["condition"] = nlohmann::json{
        {"type", "always"},
        {"all", nlohmann::json::array({{{"type", "always"}}})}
    };
    auto msgs = ValidateEventDefinition(doc);
    REQUIRE(msgs.size() == 1);
    REQUIRE(msgs[0].code == "EVENT_CONDITION_AMBIGUOUS");
    REQUIRE(msgs[0].severity == Severity::Error);
    REQUIRE(msgs[0].path == "condition");
}

TEST_CASE("ValidateEventDefinition - condition with no type and no composite produces EVENT_CONDITION_LEAF_TYPE_MISSING")
{
    auto doc = ValidEvent();
    doc["condition"] = nlohmann::json{{"resource", "Wood"}};
    auto msgs = ValidateEventDefinition(doc);
    REQUIRE(msgs.size() == 1);
    REQUIRE(msgs[0].code == "EVENT_CONDITION_LEAF_TYPE_MISSING");
    REQUIRE(msgs[0].severity == Severity::Error);
    REQUIRE(msgs[0].path == "condition");
}

TEST_CASE("ValidateEventDefinition - nested invalid condition in all array reports correct path")
{
    auto doc = ValidEvent();
    doc["condition"] = nlohmann::json{
        {"all", nlohmann::json::array({
            {{"type", "always"}},
            {{"resource", "Wood"}}   // missing type and composite key
        })}
    };
    auto msgs = ValidateEventDefinition(doc);
    REQUIRE(msgs.size() == 1);
    REQUIRE(msgs[0].code == "EVENT_CONDITION_LEAF_TYPE_MISSING");
    REQUIRE(msgs[0].path == "condition.all[1]");
}

TEST_CASE("ValidateEventDefinition - action missing type produces EVENT_ACTION_TYPE_MISSING")
{
    auto doc = ValidEvent();
    doc["actions"] = nlohmann::json::array({{{"resource", "Wood"}, {"amount", 5}}});
    auto msgs = ValidateEventDefinition(doc);
    REQUIRE(msgs.size() == 1);
    REQUIRE(msgs[0].code == "EVENT_ACTION_TYPE_MISSING");
    REQUIRE(msgs[0].severity == Severity::Error);
    REQUIRE(msgs[0].path == "actions[0].type");
}

TEST_CASE("ValidateEventDefinition - action with empty type produces EVENT_ACTION_TYPE_EMPTY")
{
    auto doc = ValidEvent();
    doc["actions"] = nlohmann::json::array({{{"type", ""}}});
    auto msgs = ValidateEventDefinition(doc);
    REQUIRE(msgs.size() == 1);
    REQUIRE(msgs[0].code == "EVENT_ACTION_TYPE_EMPTY");
    REQUIRE(msgs[0].severity == Severity::Error);
    REQUIRE(msgs[0].path == "actions[0].type");
}

// ---------------------------------------------------------------------------
// ParseEventDefinition — round-trip and structure
// ---------------------------------------------------------------------------

TEST_CASE("ParseEventDefinition - valid fixture populates id and trigger")
{
    auto def = ParseEventDefinition(ValidEvent());
    REQUIRE(def.id == "evt_test_01");
    REQUIRE(def.trigger.type == EventTriggerType::StartOfDay);
}

TEST_CASE("ParseEventDefinition - all 8 trigger type strings map to correct enum values")
{
    const std::vector<std::pair<std::string, EventTriggerType>> cases = {
        {"regionNodeEntry",          EventTriggerType::RegionNodeEntry},
        {"startOfDay",               EventTriggerType::StartOfDay},
        {"locationCollision",        EventTriggerType::LocationCollision},
        {"locationConfirm",          EventTriggerType::LocationConfirm},
        {"neutralEncounterDefeated", EventTriggerType::NeutralEncounterDefeated},
        {"serviceUsed",              EventTriggerType::ServiceUsed},
        {"serviceDestroyed",         EventTriggerType::ServiceDestroyed},
        {"questCompletion",          EventTriggerType::QuestCompletion},
    };
    for (const auto& [str, expected] : cases) {
        INFO("Trigger string: " << str);
        REQUIRE(EventTriggerTypeFromString(str) == expected);
    }
}

TEST_CASE("ParseEventDefinition - unknown trigger string maps to Unknown")
{
    REQUIRE(EventTriggerTypeFromString("onMoonrise") == EventTriggerType::Unknown);
    REQUIRE(EventTriggerTypeFromString("") == EventTriggerType::Unknown);
}

TEST_CASE("ParseEventDefinition - leaf condition parses to Leaf kind with correct leafType")
{
    auto def = ParseEventDefinition(ValidEvent());
    REQUIRE(def.condition.kind == EventConditionKind::Leaf);
    REQUIRE(def.condition.leafType == "always");
    REQUIRE(def.condition.operands.empty());
}

TEST_CASE("ParseEventDefinition - nested all/any/not condition parses to correct tree")
{
    auto doc = ValidEvent();
    doc["condition"] = nlohmann::json{
        {"all", nlohmann::json::array({
            {{"type", "teamHasHero"}, {"heroId", "hero_jon"}},
            {{"any", nlohmann::json::array({
                {{"type", "teamHasResource"}, {"resource", "Wood"}, {"amount", 10}},
                {{"not", {{"type", "storyFlagSet"}, {"flag", "chapter2"}}}}
            })}}
        })}
    };

    auto def = ParseEventDefinition(doc);
    REQUIRE(def.condition.kind == EventConditionKind::All);
    REQUIRE(def.condition.operands.size() == 2);

    const auto& first = def.condition.operands[0];
    REQUIRE(first.kind == EventConditionKind::Leaf);
    REQUIRE(first.leafType == "teamHasHero");

    const auto& second = def.condition.operands[1];
    REQUIRE(second.kind == EventConditionKind::Any);
    REQUIRE(second.operands.size() == 2);

    const auto& notNode = second.operands[1];
    REQUIRE(notNode.kind == EventConditionKind::Not);
    REQUIRE(notNode.operands.size() == 1);
    REQUIRE(notNode.operands[0].kind == EventConditionKind::Leaf);
    REQUIRE(notNode.operands[0].leafType == "storyFlagSet");
}

TEST_CASE("ParseEventDefinition - actions parse to correct type strings")
{
    auto doc = ValidEvent();
    doc["actions"] = nlohmann::json::array({
        {{"type", "showMessage"}, {"text", {{"en", "Hi"}}}},
        {{"type", "takeResource"}, {"resource", "Gold"}, {"amount", 100}}
    });

    auto def = ParseEventDefinition(doc);
    REQUIRE(def.actions.size() == 2);
    REQUIRE(def.actions[0].type == "showMessage");
    REQUIRE(def.actions[1].type == "takeResource");
}

TEST_CASE("ParseEventDefinition - eligibility fields parse correctly")
{
    auto doc = ValidEvent();
    doc["eligibility"] = nlohmann::json{
        {"team_colors", {"red", "blue"}},
        {"team_kinds", {"human"}},
        {"required_hero_ids", {"hero_jon", "hero_arya"}}
    };

    auto def = ParseEventDefinition(doc);
    REQUIRE(def.eligibility.teamColors == std::vector<std::string>{"red", "blue"});
    REQUIRE(def.eligibility.teamKinds == std::vector<std::string>{"human"});
    REQUIRE(def.eligibility.requiredHeroIds == std::vector<std::string>{"hero_jon", "hero_arya"});
}

TEST_CASE("ParseEventDefinition - priority parses when present")
{
    auto doc = ValidEvent();
    doc["priority"] = 3;
    auto def = ParseEventDefinition(doc);
    REQUIRE(def.priority.has_value());
    REQUIRE(*def.priority == 3);
}

TEST_CASE("ParseEventDefinition - priority absent when not in JSON")
{
    auto def = ParseEventDefinition(ValidEvent());
    REQUIRE_FALSE(def.priority.has_value());
}

TEST_CASE("ParseEventDefinition - repeat fields parse correctly")
{
    auto doc = ValidEvent();
    doc["repeat"] = nlohmann::json{{"mode", "everyNDays"}, {"interval_days", 7}};
    auto def = ParseEventDefinition(doc);
    REQUIRE(def.repeat.mode == "everyNDays");
    REQUIRE(def.repeat.intervalDays == 7);
}

TEST_CASE("ParseEventDefinition - repeat defaults to once when absent")
{
    auto def = ParseEventDefinition(ValidEvent());
    REQUIRE(def.repeat.mode == "once");
    REQUIRE(def.repeat.intervalDays == 0);
}

// ---------------------------------------------------------------------------
// camelCase trigger-specific target fields
// ---------------------------------------------------------------------------

TEST_CASE("ParseEventDefinition - regionNodeEntry with nodeId parses into targetId")
{
    auto doc = ValidEvent();
    doc["trigger"] = nlohmann::json{{"type", "regionNodeEntry"}, {"nodeId", "node_gate"}};
    auto def = ParseEventDefinition(doc);
    REQUIRE(def.trigger.targetId == "node_gate");
}

TEST_CASE("ParseEventDefinition - locationConfirm with objectId parses into targetId")
{
    auto doc = ValidEvent();
    doc["trigger"] = nlohmann::json{{"type", "locationConfirm"}, {"objectId", "obj_chest"}};
    auto def = ParseEventDefinition(doc);
    REQUIRE(def.trigger.targetId == "obj_chest");
}

TEST_CASE("ParseEventDefinition - neutralEncounterDefeated with encounterId parses into targetId")
{
    auto doc = ValidEvent();
    doc["trigger"] = nlohmann::json{{"type", "neutralEncounterDefeated"}, {"encounterId", "enc_wolves"}};
    auto def = ParseEventDefinition(doc);
    REQUIRE(def.trigger.targetId == "enc_wolves");
}

TEST_CASE("ParseEventDefinition - legacy target_id fallback populates targetId")
{
    auto doc = ValidEvent();
    doc["trigger"] = nlohmann::json{{"type", "startOfDay"}, {"target_id", "legacy_value"}};
    auto def = ParseEventDefinition(doc);
    REQUIRE(def.trigger.targetId == "legacy_value");
}

// ---------------------------------------------------------------------------
// camelCase eligibility and repeat fields
// ---------------------------------------------------------------------------

TEST_CASE("ParseEventDefinition - camelCase eligibility fields parse correctly")
{
    auto doc = ValidEvent();
    doc["eligibility"] = nlohmann::json{
        {"teamColors", {"red"}},
        {"teamKinds", {"human"}},
        {"requiredHeroIds", {"hero_jon"}}
    };
    auto def = ParseEventDefinition(doc);
    REQUIRE(def.eligibility.teamColors == std::vector<std::string>{"red"});
    REQUIRE(def.eligibility.teamKinds == std::vector<std::string>{"human"});
    REQUIRE(def.eligibility.requiredHeroIds == std::vector<std::string>{"hero_jon"});
}

TEST_CASE("ParseEventDefinition - camelCase intervalDays in repeat parses correctly")
{
    auto doc = ValidEvent();
    doc["repeat"] = nlohmann::json{{"mode", "everyNDays"}, {"intervalDays", 3}};
    auto def = ParseEventDefinition(doc);
    REQUIRE(def.repeat.intervalDays == 3);
}

// ---------------------------------------------------------------------------
// Unknown condition and action type validation
// ---------------------------------------------------------------------------

TEST_CASE("ValidateEventDefinition - unknown leaf condition type produces EVENT_CONDITION_TYPE_UNKNOWN")
{
    auto doc = ValidEvent();
    doc["condition"] = nlohmann::json{{"type", "onMoonrise"}};
    auto msgs = ValidateEventDefinition(doc);
    REQUIRE(msgs.size() == 1);
    REQUIRE(msgs[0].code == "EVENT_CONDITION_TYPE_UNKNOWN");
    REQUIRE(msgs[0].severity == Severity::Error);
    REQUIRE(msgs[0].path == "condition.type");
}

TEST_CASE("ValidateEventDefinition - all known leaf condition types are valid")
{
    for (const auto& t : {"always", "teamHasResource", "teamHasHero", "storyFlagSet"}) {
        auto doc = ValidEvent();
        doc["condition"] = nlohmann::json{{"type", t}};
        auto msgs = ValidateEventDefinition(doc);
        INFO("Condition type: " << t);
        const bool hasUnknown = std::ranges::any_of(msgs,
            [](const auto& m) { return m.code == "EVENT_CONDITION_TYPE_UNKNOWN"; });
        REQUIRE_FALSE(hasUnknown);
    }
}

TEST_CASE("ValidateEventDefinition - unknown action type produces EVENT_ACTION_TYPE_UNKNOWN")
{
    auto doc = ValidEvent();
    doc["actions"] = nlohmann::json::array({{{"type", "grantWishes"}}});
    auto msgs = ValidateEventDefinition(doc);
    REQUIRE(msgs.size() == 1);
    REQUIRE(msgs[0].code == "EVENT_ACTION_TYPE_UNKNOWN");
    REQUIRE(msgs[0].severity == Severity::Error);
    REQUIRE(msgs[0].path == "actions[0].type");
}

TEST_CASE("ValidateEventDefinition - all known action types are valid")
{
    for (const auto& t : {"showMessage", "giveResource", "takeResource",
                          "setStoryFlag", "clearStoryFlag", "if"}) {
        auto doc = ValidEvent();
        doc["actions"] = nlohmann::json::array({{{"type", t}}});
        auto msgs = ValidateEventDefinition(doc);
        INFO("Action type: " << t);
        const bool hasUnknown = std::ranges::any_of(msgs,
            [](const auto& m) { return m.code == "EVENT_ACTION_TYPE_UNKNOWN"; });
        REQUIRE_FALSE(hasUnknown);
    }
}

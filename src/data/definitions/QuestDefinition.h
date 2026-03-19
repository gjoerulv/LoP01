#pragma once

#include <string>

namespace data {

enum class QuestObjectiveType {
    Unknown,
    BringResource,
    MeetHero,
    ClearCombatNode
};

inline QuestObjectiveType QuestObjectiveTypeFromString(const std::string& value) {
    if (value == "bring_resource") {
        return QuestObjectiveType::BringResource;
    }
    if (value == "meet_hero") {
        return QuestObjectiveType::MeetHero;
    }
    if (value == "clear_combat_node") {
        return QuestObjectiveType::ClearCombatNode;
    }

    return QuestObjectiveType::Unknown;
}

struct QuestDefinition {
    std::string id;
    std::string name;
    std::string description;
    QuestObjectiveType objective = QuestObjectiveType::Unknown;
    std::string target;
};

} // namespace data

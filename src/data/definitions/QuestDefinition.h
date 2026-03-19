#pragma once

#include <string>

namespace data {

enum class QuestObjectiveType {
    Unknown,
    BringResource,
    MeetHero
};

inline QuestObjectiveType QuestObjectiveTypeFromString(const std::string& value) {
    if (value == "bring_resource") {
        return QuestObjectiveType::BringResource;
    }
    if (value == "meet_hero") {
        return QuestObjectiveType::MeetHero;
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

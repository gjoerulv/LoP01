#include "gameplay/quests/QuestState.h"

#include <algorithm>

namespace gameplay::quests {

namespace {

bool IsDestinationStyleObjective(const data::QuestObjectiveType objective) {
    return objective == data::QuestObjectiveType::BringResource ||
        objective == data::QuestObjectiveType::MeetHero;
}

} // namespace

void QuestState::Initialize(const std::vector<data::QuestDefinition>& definitions) {
    quests_.clear();
    quests_.reserve(definitions.size());

    for (const auto& definition : definitions) {
        quests_.push_back(QuestProgress{
            definition.id,
            definition.name,
            definition.description,
            definition.objective,
            definition.target,
            QuestStatus::InProgress
        });
    }
}

std::vector<std::string> QuestState::OnDestinationReached(const std::string& destinationId) {
    std::vector<std::string> completed;

    for (auto& quest : quests_) {
        if (quest.status != QuestStatus::InProgress) {
            continue;
        }

        if (!IsDestinationStyleObjective(quest.objective)) {
            continue;
        }

        if (quest.target != destinationId) {
            continue;
        }

        quest.status = QuestStatus::Completed;
        completed.push_back("Quest completed: " + quest.name);
    }

    return completed;
}

std::vector<std::string> QuestState::OnCombatNodeCleared(const std::string& nodeId) {
    std::vector<std::string> completed;

    for (auto& quest : quests_) {
        if (quest.status != QuestStatus::InProgress) {
            continue;
        }

        if (quest.objective != data::QuestObjectiveType::ClearCombatNode) {
            continue;
        }

        if (quest.target != nodeId) {
            continue;
        }

        quest.status = QuestStatus::Completed;
        completed.push_back("Quest completed: " + quest.name);
    }

    return completed;
}

void QuestState::RestoreCompletedQuestIds(const std::vector<std::string>& completedQuestIds) {
    for (auto& quest : quests_) {
        const bool completed = std::find(
            completedQuestIds.begin(),
            completedQuestIds.end(),
            quest.id) != completedQuestIds.end();

        quest.status = completed ? QuestStatus::Completed : QuestStatus::InProgress;
    }
}

const std::vector<QuestProgress>& QuestState::Quests() const {
    return quests_;
}

std::vector<std::string> QuestState::CompletedQuestIds() const {
    std::vector<std::string> ids;
    ids.reserve(quests_.size());

    for (const auto& quest : quests_) {
        if (quest.status == QuestStatus::Completed) {
            ids.push_back(quest.id);
        }
    }

    return ids;
}

} // namespace gameplay::quests

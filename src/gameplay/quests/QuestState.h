#pragma once

#include <string>
#include <vector>

#include "data/definitions/QuestDefinition.h"

namespace gameplay::quests {

enum class QuestStatus {
    Available,
    InProgress,
    Completed
};

struct QuestProgress {
    std::string id;
    std::string name;
    std::string description;
    data::QuestObjectiveType objective = data::QuestObjectiveType::Unknown;
    std::string target;
    QuestStatus status = QuestStatus::Available;
};

class QuestState {
public:
    void Initialize(const std::vector<data::QuestDefinition>& definitions);
    std::vector<std::string> OnDestinationReached(const std::string& destinationId);
    std::vector<std::string> OnCombatNodeCleared(const std::string& nodeId);
    void RestoreCompletedQuestIds(const std::vector<std::string>& completedQuestIds);

    [[nodiscard]] const std::vector<QuestProgress>& Quests() const;
    [[nodiscard]] std::vector<std::string> CompletedQuestIds() const;

private:
    std::vector<QuestProgress> quests_;
};

} // namespace gameplay::quests

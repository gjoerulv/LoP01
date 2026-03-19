#include "gameplay/GameSession.h"

#include <algorithm>

namespace gameplay {

GameSession::GameSession()
    : mode_(GameMode::Title), gold_(2500), regionId_("ashvale_heartland"), destinationId_("home_base") {}

void GameSession::AdvanceMode() {
    switch (mode_) {
    case GameMode::Title:
        mode_ = GameMode::OpeningSequence;
        break;
    case GameMode::OpeningSequence:
        mode_ = GameMode::OverworldSelection;
        break;
    case GameMode::OverworldSelection:
        mode_ = GameMode::OverworldMode;
        break;
    case GameMode::OverworldMode:
        mode_ = GameMode::LocationMode;
        break;
    case GameMode::LocationMode:
        mode_ = GameMode::BattleMode;
        break;
    case GameMode::BattleMode:
        mode_ = GameMode::OverworldMode;
        break;
    }
}

void GameSession::AddMinutes(const int minutes) {
    clock_.AdvanceMinutes(minutes);
}

bool GameSession::SpendGold(const int amount) {
    if (amount <= 0) {
        return true;
    }
    if (gold_ < amount) {
        return false;
    }

    gold_ -= amount;
    return true;
}

bool GameSession::TrySpendGold(const int amount) {
    if (amount <= 0) {
        return true;
    }

    if (gold_ < amount) {
        return false;
    }

    gold_ -= amount;
    return true;
}

void GameSession::EnterLocationMode(const std::string& locationId) {
    destinationId_ = locationId;
    mode_ = GameMode::LocationMode;
}

void GameSession::EnterOverworldMode() {
    mode_ = GameMode::OverworldMode;
}

void GameSession::ExitLocationMode() {
    EnterOverworldMode();
}

void GameSession::EnterBattleMode() {
    mode_ = GameMode::BattleMode;
}

bool GameSession::IsInLocationMode() const {
    return mode_ == GameMode::LocationMode;
}

void GameSession::SetDestination(const std::string& destinationId) {
    destinationId_ = destinationId;
}

void GameSession::ApplyDoorOpenCost() {
    AddMinutes(1);
}

void GameSession::ApplyDialogueChoiceCost() {
    AddMinutes(1);
}

bool GameSession::ApplyShopCost(const int goldCost) {
    if (!SpendGold(goldCost)) {
        return false;
    }
    AddMinutes(5);
    return true;
}

bool GameSession::ApplyRecruitCost(const int goldCost) {
    if (!SpendGold(goldCost)) {
        return false;
    }
    AddMinutes(10);
    return true;
}

void GameSession::RestToNextDayStart() {
    const int remainingMinutes = core::GameClock::kMinutesPerSliceDay - clock_.MinutesIntoSliceDay();
    clock_.AdvanceMinutes(std::max(1, remainingMinutes));
}

void GameSession::ApplyWakePenalty() {
    gold_ = std::max(0, gold_ - 1000);
    clock_.SetToWakePenaltyStart();
}

void GameSession::MarkCombatNodeCleared(const std::string& nodeId) {
    nodeWorldState_.MarkCombatNodeCleared(nodeId);
}

bool GameSession::IsCombatNodeCleared(const std::string& nodeId) const {
    return nodeWorldState_.IsCombatNodeCleared(nodeId);
}

const std::vector<std::string>& GameSession::ClearedCombatNodeIds() const {
    return nodeWorldState_.ClearedCombatNodeIds();
}

void GameSession::ApplyOverworldCombatVictoryNodeClear(
    const bool alliesWon,
    const bool enemiesWon,
    const GameMode battleReturnMode,
    const std::string& nodeId,
    const bool nodeIsCombatType) {
    if (!alliesWon || enemiesWon) {
        return;
    }

    if (battleReturnMode != GameMode::OverworldMode) {
        return;
    }

    if (!nodeIsCombatType) {
        return;
    }

    MarkCombatNodeCleared(nodeId);
}

void GameSession::InitializeQuestState(const std::vector<data::QuestDefinition>& questDefinitions) {
    questState_.Initialize(questDefinitions);
}

std::vector<std::string> GameSession::NotifyDestinationReached(const std::string& destinationId) {
    return questState_.OnDestinationReached(destinationId);
}

std::vector<std::string> GameSession::NotifyCombatNodeCleared(const std::string& nodeId) {
    return questState_.OnCombatNodeCleared(nodeId);
}

const std::vector<quests::QuestProgress>& GameSession::QuestProgress() const {
    return questState_.Quests();
}

SessionSnapshot GameSession::Snapshot() const {
    return SessionSnapshot{
        mode_,
        clock_.Day(),
        clock_.MinutesIntoSliceDay(),
        clock_.TimeString(),
        gold_,
        regionId_,
        destinationId_
    };
}

core::SaveData GameSession::ToSaveData() const {
    return core::SaveData{
        clock_.Day(),
        clock_.MinutesIntoSliceDay(),
        gold_,
        ToString(mode_),
        regionId_,
        destinationId_,
        questState_.CompletedQuestIds(),
        nodeWorldState_.ClearedCombatNodeIds()
    };
}

void GameSession::ApplySaveData(const core::SaveData& saveData) {
    mode_ = FromString(saveData.mode);
    gold_ = std::max(0, saveData.gold);
    regionId_ = saveData.regionId;
    destinationId_ = saveData.destinationId;

    clock_ = core::GameClock();
    const int daysToAdvance = std::max(0, saveData.day - 1);
    clock_.AdvanceMinutes(daysToAdvance * core::GameClock::kMinutesPerSliceDay + std::max(0, saveData.minutesIntoSliceDay));

    questState_.RestoreCompletedQuestIds(saveData.completedQuestIds);
    nodeWorldState_.RestoreClearedCombatNodeIds(saveData.clearedCombatNodeIds);
}

std::string GameSession::ToString(const GameMode mode) {
    switch (mode) {
    case GameMode::Title:
        return "title";
    case GameMode::OpeningSequence:
        return "opening_sequence";
    case GameMode::OverworldSelection:
        return "overworld_selection";
    case GameMode::OverworldMode:
        return "overworld_mode";
    case GameMode::LocationMode:
        return "location_mode";
    case GameMode::BattleMode:
        return "battle_mode";
    }

    return "title";
}

GameMode GameSession::FromString(const std::string& mode) {
    if (mode == "opening_sequence") {
        return GameMode::OpeningSequence;
    }
    if (mode == "overworld_selection") {
        return GameMode::OverworldSelection;
    }
    if (mode == "overworld_mode") {
        return GameMode::OverworldMode;
    }
    if (mode == "location_mode") {
        return GameMode::LocationMode;
    }
    if (mode == "battle_mode") {
        return GameMode::BattleMode;
    }

    return GameMode::Title;
}

} // namespace gameplay

#pragma once

#include <string>
#include <vector>

#include "core/GameClock.h"
#include "core/SaveGame.h"
#include "data/definitions/QuestDefinition.h"
#include "gameplay/quests/QuestState.h"
#include "gameplay/world/NodeWorldState.h"

namespace gameplay {

enum class GameMode {
    Title,
    OpeningSequence,
    OverworldSelection,
    OverworldMode,
    LocationMode,
    BattleMode
};

struct SessionSnapshot {
    GameMode mode = GameMode::Title;
    int day = 1;
    int minutesIntoSliceDay = 0;
    std::string time;
    int gold = 2500;
    std::string regionId = "ashvale_heartland";
    std::string destinationId = "home_base";
};

class GameSession {
public:
    GameSession();

    void AdvanceMode();
    void AddMinutes(int minutes);
    bool SpendGold(int amount);
    [[nodiscard]] bool TrySpendGold(int amount);

    void EnterLocationMode(const std::string& locationId);
    void EnterOverworldMode();
    void ExitLocationMode();
    void EnterBattleMode();
    [[nodiscard]] bool IsInLocationMode() const;
    void SetDestination(const std::string& destinationId);

    void ApplyDoorOpenCost();
    void ApplyDialogueChoiceCost();
    bool ApplyShopCost(int goldCost);
    bool ApplyRecruitCost(int goldCost);
    void RestToNextDayStart();

    void ApplyWakePenalty();

    void MarkCombatNodeCleared(const std::string& nodeId);
    [[nodiscard]] bool IsCombatNodeCleared(const std::string& nodeId) const;
    [[nodiscard]] const std::vector<std::string>& ClearedCombatNodeIds() const;
    void ApplyOverworldCombatVictoryNodeClear(
        bool alliesWon,
        bool enemiesWon,
        GameMode battleReturnMode,
        const std::string& nodeId,
        bool nodeIsCombatType);

    void InitializeQuestState(const std::vector<data::QuestDefinition>& questDefinitions);
    [[nodiscard]] std::vector<std::string> NotifyDestinationReached(const std::string& destinationId);
    [[nodiscard]] std::vector<std::string> NotifyCombatNodeCleared(const std::string& nodeId);
    [[nodiscard]] const std::vector<quests::QuestProgress>& QuestProgress() const;

    [[nodiscard]] SessionSnapshot Snapshot() const;

    [[nodiscard]] core::SaveData ToSaveData() const;
    void ApplySaveData(const core::SaveData& saveData);

    static std::string ToString(GameMode mode);
    static GameMode FromString(const std::string& mode);

private:
    GameMode mode_;
    core::GameClock clock_;
    int gold_;
    std::string regionId_;
    std::string destinationId_;
    quests::QuestState questState_;
    world::NodeWorldState nodeWorldState_;
};

} // namespace gameplay

#pragma once

#include <string>

#include "core/GameClock.h"
#include "core/SaveGame.h"

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
    void ExitLocationMode();
    [[nodiscard]] bool IsInLocationMode() const;
    void SetDestination(const std::string& destinationId);

    void ApplyDoorOpenCost();
    void ApplyDialogueChoiceCost();
    bool ApplyShopCost(int goldCost);
    bool ApplyRecruitCost(int goldCost);

    void ApplyWakePenalty();

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
};

} // namespace gameplay

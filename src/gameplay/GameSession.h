#pragma once

#include <string>
#include <set>
#include <vector>

#include "core/GameClock.h"
#include "core/SaveGame.h"
#include "data/definitions/LocationServiceDefinition.h"
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

struct OwnedUnitCountState {
    std::string unitId;
    int count = 0;
};

struct RosterStackState {
    std::string stackId;
    std::string unitId;
    int quantity = 0;
};

struct ActiveBattleStackEntry {
    int activeSlotIndex = -1;
    std::string stackId;
    std::string unitId;
    int quantity = 0;
};

struct BattleStackLifeResult {
    std::string stackId;
    int resultingLife = 0;
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
    void RestToNextDayStart();

    [[nodiscard]] int CurrentWeek() const;
    void RefreshWeeklyRecruitStocks(const std::vector<data::LocationServiceDefinition>& services);
    [[nodiscard]] int RemainingRecruitStock(const std::string& serviceId, int defaultStock) const;
    [[nodiscard]] bool TryConsumeRecruitStock(const std::string& serviceId, int defaultStock);

    void RefreshDailyServiceUses(const std::vector<data::LocationServiceDefinition>& services);
    [[nodiscard]] int RemainingDailyServiceUses(const std::string& serviceId, int defaultUsesPerDay) const;
    [[nodiscard]] bool TryConsumeDailyServiceUse(const std::string& serviceId, int defaultUsesPerDay);

    void GrantSameDayTravelPrep(int discountMinutes, int charges);
    [[nodiscard]] bool HasActiveSameDayTravelPrep() const;
    [[nodiscard]] int ActiveSameDayTravelPrepDiscountMinutes() const;
    [[nodiscard]] int PreviewSameDayTravelPrepToTravelMinutes(int baseTravelMinutes) const;
    [[nodiscard]] int ApplySameDayTravelPrepToTravelMinutes(int baseTravelMinutes);

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

    [[nodiscard]] int ActivePartyCapacity() const;
    void SetLeaderCapableUnitIds(std::vector<std::string> unitIds);
    [[nodiscard]] int OwnedUnitCount(const std::string& unitId) const;
    [[nodiscard]] int ActivePartyAllocatedCount(const std::string& unitId) const;
    [[nodiscard]] int ReserveUnitCount(const std::string& unitId) const;

    [[nodiscard]] const std::vector<RosterStackState>& RosterStacks() const;
    [[nodiscard]] const std::vector<std::string>& ActiveSlotStackIds() const;
    [[nodiscard]] const std::vector<std::string>& ReserveSlotStackIds() const;
    [[nodiscard]] int NextStackIdCounter() const;

    [[nodiscard]] const std::vector<OwnedUnitCountState>& OwnedUnitCounts() const;
    [[nodiscard]] const std::vector<std::string>& ActivePartyUnitIds() const;

    [[nodiscard]] bool AddOwnedUnit(const std::string& unitId, int count = 1);
    [[nodiscard]] bool CanAddOwnedUnit(const std::string& unitId, int count = 1) const;
    [[nodiscard]] bool TryRemoveOwnedUnit(const std::string& unitId, int count = 1);
    [[nodiscard]] bool TryAddUnitToActiveParty(const std::string& unitId);
    [[nodiscard]] bool TryMoveReserveStackToActiveSlot(int reserveSlotIndex);
    [[nodiscard]] bool TryRemoveActivePartyUnitAt(int index);
    [[nodiscard]] bool WouldRemovingActivePartyUnitLeaveNoLeader(int index) const;
    [[nodiscard]] bool TryMoveActivePartyUnit(int fromIndex, int toIndex);
    void ClearActiveParty();

    [[nodiscard]] const RosterStackState* FindRosterStackById(const std::string& stackId) const;
    [[nodiscard]] std::vector<ActiveBattleStackEntry> BuildActiveBattleStackEntries() const;
    [[nodiscard]] bool ApplyBattleStackLifeResults(
        const std::vector<BattleStackLifeResult>& results,
        const std::vector<std::string>& expectedStackIds);

    [[nodiscard]] SessionSnapshot Snapshot() const;

    [[nodiscard]] core::SaveData ToSaveData() const;
    void ApplySaveData(const core::SaveData& saveData);

    static std::string ToString(GameMode mode);
    static GameMode FromString(const std::string& mode);

private:
    static constexpr int kActiveSlotCount = 5;
    static constexpr int kReserveSlotCount = 8;

    void NormalizeRosterState();
    void MarkRosterProjectionDirty();
    void RebuildRosterProjectionCache() const;

    [[nodiscard]] std::string GenerateNextStackId();
    [[nodiscard]] RosterStackState* FindStackById(const std::string& stackId);
    [[nodiscard]] const RosterStackState* FindStackById(const std::string& stackId) const;
    [[nodiscard]] std::string FindCompatibleStackIdInSlots(
        const std::vector<std::string>& slotStackIds,
        const std::string& unitId) const;
    [[nodiscard]] int FindFirstEmptySlotIndex(const std::vector<std::string>& slotStackIds) const;
    void RemoveStackIfDepleted(const std::string& stackId);
    [[nodiscard]] bool IsLeaderCapableUnitId(const std::string& unitId) const;
    [[nodiscard]] int ActiveLeaderCapableCountExcludingOrderedIndex(int excludedOrderedIndex) const;
    [[nodiscard]] int ActiveLeaderCapableCount() const;

    GameMode mode_;
    core::GameClock clock_;
    std::vector<core::RecruitServiceState> recruitServiceStates_;
    std::vector<core::DailyServiceState> dailyServiceStates_;
    int travelPrepDiscountMinutes_ = 0;
    int travelPrepRemainingCharges_ = 0;
    int travelPrepGrantedDay_ = 0;
    int gold_;
    std::string regionId_;
    std::string destinationId_;

    std::vector<RosterStackState> rosterStacks_;
    std::vector<std::string> activeSlotStackIds_;
    std::vector<std::string> reserveSlotStackIds_;
    int nextStackIdCounter_ = 1;

    mutable bool rosterProjectionDirty_ = true;
    mutable std::vector<OwnedUnitCountState> ownedUnitCountProjection_;
    mutable std::vector<std::string> activePartyUnitIdProjection_;

    std::set<std::string> leaderCapableUnitIds_;

    int activePartyCapacity_ = kActiveSlotCount;
    quests::QuestState questState_;
    world::NodeWorldState nodeWorldState_;
};

} // namespace gameplay

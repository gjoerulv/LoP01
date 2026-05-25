#pragma once

#include <functional>
#include <optional>
#include <string>
#include <set>
#include <vector>

#include "core/GameClock.h"
#include "core/SaveGame.h"
#include "data/definitions/LocationServiceDefinition.h"
#include "data/definitions/QuestDefinition.h"
#include "data/definitions/RegionDefinition.h"
#include "data/definitions/ScenarioOutcomeDefinition.h"
#include "gameplay/EnemyTeamState.h"
#include "gameplay/events/EventDefinition.h"
#include "gameplay/events/EventEngine.h"
#include "gameplay/quests/QuestState.h"
#include "gameplay/scenario/ScenarioOutcomeRules.h"
#include "gameplay/world/NodeWorldState.h"

namespace gameplay {

enum class GameMode {
    Title,
    OpeningSequence,
    WorldMapMode,
    RegionMode,
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

struct EnemyTeamActionResult {
    std::string teamColor;
    std::string actionType;
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
    void EnterRegionMode();
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
    void ApplyRegionCombatVictoryNodeClear(
        bool alliesWon,
        bool enemiesWon,
        GameMode battleReturnMode,
        const std::string& nodeId,
        bool nodeIsCombatType);

    void InitializeEventDefinitions(std::vector<events::EventDefinition> definitions);
    [[nodiscard]] std::vector<events::ActionResult> NotifyStartOfDay();
    [[nodiscard]] std::vector<events::ActionResult> NotifyRegionNodeEntry(const std::string& nodeId);

    void SetEnemyTeams(std::vector<EnemyTeamState> teams);
    void SetPlayerColor(std::string color);
    [[nodiscard]] const std::string& PlayerColor() const;
    void SetScenarioOutcomeDefinition(data::ScenarioOutcomeDefinition definition);
    // Evaluates outcome against current session state. If non-Ongoing and the
    // session has not yet latched, latches the outcome (and stays latched
    // through save/load). Called automatically at the boundaries documented in
    // implementation_roadmap.md §4: end of FireMatchingEvents (StartOfDay /
    // RegionNodeEntry), end of ProcessEnemyPhase, and end of ClearEnemyTeamByColor.
    // Safe to call repeatedly; a latched outcome never changes.
    const std::optional<scenario::ScenarioOutcome>& CheckAndLatchOutcome();
    [[nodiscard]] const std::optional<scenario::ScenarioOutcome>& Outcome() const;
    [[nodiscard]] bool IsScenarioEnded() const;
    [[nodiscard]] std::vector<EnemyTeamActionResult> ProcessEnemyPhase(
        const std::vector<data::RegionLinkDefinition>& regionLinks);
    [[nodiscard]] const std::vector<EnemyTeamState>& EnemyTeams() const;
    [[nodiscard]] std::vector<std::string> HostileOccupiedNodeIds(
        const std::string& playerColor) const;
    void ClearEnemyTeamByColor(const std::string& teamColor);
    [[nodiscard]] std::string HostileTeamColorAtNode(
        const std::string& nodeId, const std::string& playerColor) const;

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

    // Shared firing loop for trigger-typed events. Called by NotifyStartOfDay and
    // NotifyRegionNodeEntry. `matches` decides which event definitions are eligible
    // (by trigger type and any per-trigger target match). Applies priority sort,
    // one-shot guard, condition evaluation, action execution, story-flag / Gold
    // persistence, and enemy-team mutation apply — mirroring the original
    // NotifyStartOfDay flow.
    [[nodiscard]] std::vector<events::ActionResult> FireMatchingEvents(
        const std::function<bool(const events::EventDefinition&)>& matches);

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

    std::vector<events::EventDefinition> eventDefinitions_;
    std::vector<std::string>             firedEventIds_;
    std::set<std::string>                storyFlags_;

    std::vector<EnemyTeamState> enemyTeams_;
    static const std::vector<std::string> kTeamColorOrder;

    std::string playerColor_ = "Green";
    data::ScenarioOutcomeDefinition scenarioOutcomeDefinition_;
    std::optional<scenario::ScenarioOutcome> latchedOutcome_;
};

} // namespace gameplay

#pragma once

#include <functional>
#include <optional>
#include <string>
#include <set>
#include <vector>

#include "core/GameClock.h"
#include "core/SaveGame.h"
#include "data/definitions/ArtifactDefinition.h"
#include "data/definitions/ItemDefinition.h"
#include "data/definitions/LocationServiceDefinition.h"
#include "data/definitions/QuestDefinition.h"
#include "data/definitions/RegionDefinition.h"
#include "data/definitions/ScenarioOutcomeDefinition.h"
#include "data/definitions/UnitDefinition.h"
#include "gameplay/EnemyTeamState.h"
#include "gameplay/InventoryState.h"
#include "gameplay/events/EventDefinition.h"
#include "gameplay/events/EventEngine.h"
#include "gameplay/quests/QuestState.h"
#include "gameplay/scenario/ScenarioOutcomeRules.h"
#include "gameplay/world/NodeWorldState.h"

#include <map>

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
    // Pre-summed equipped-artifact stat bonuses for hero entries (M13-b).
    // Zero for generic-unit entries. The caller (BattleFactory via
    // PlayerBattleEntry) adds these on top of the unit's authored stats.
    int artifactAttackBonus = 0;
    int artifactDefenseBonus = 0;
    int artifactMagicBonus = 0;
    int artifactResistanceBonus = 0;
};

// ItemStackState and ArtifactStackState live in gameplay/InventoryState.h so
// EventEngine and other layers can depend on the shapes without including
// GameSession.h (avoids circular dependency).

// Hero artifact slots per docs/core_loop_rules.md §22:
//   1 Attack slot + 1 Defense slot + 3 Misc slots.
enum class ArtifactEquipSlot { Attack, Defense, Misc1, Misc2, Misc3 };

inline bool ArtifactEquipSlotFromString(const std::string& value, ArtifactEquipSlot& out) {
    if (value == "Attack")  { out = ArtifactEquipSlot::Attack;  return true; }
    if (value == "Defense") { out = ArtifactEquipSlot::Defense; return true; }
    if (value == "Misc1")   { out = ArtifactEquipSlot::Misc1;   return true; }
    if (value == "Misc2")   { out = ArtifactEquipSlot::Misc2;   return true; }
    if (value == "Misc3")   { out = ArtifactEquipSlot::Misc3;   return true; }
    return false;
}

inline std::string ArtifactEquipSlotToString(ArtifactEquipSlot slot) {
    switch (slot) {
        case ArtifactEquipSlot::Attack:  return "Attack";
        case ArtifactEquipSlot::Defense: return "Defense";
        case ArtifactEquipSlot::Misc1:   return "Misc1";
        case ArtifactEquipSlot::Misc2:   return "Misc2";
        case ArtifactEquipSlot::Misc3:   return "Misc3";
    }
    return "";
}

// HeroEquipmentState holds one hero's equipped artifact ids. Empty string means
// the slot is empty. heroEquipment_ is keyed by hero/unit id (heroes are unique
// so unit id is their identity in M13).
//
// Hero-departure note (M13-b): if a hero leaves the team, their entry in
// heroEquipment_ is retained, so the equipped artifacts are preserved if/when
// the hero returns. No existing removal path in the codebase touches this map.
// Full §28 battle-spoils transfer is deferred to a later milestone.
struct HeroEquipmentState {
    std::string attackArtifactId;
    std::string defenseArtifactId;
    std::string misc1ArtifactId;
    std::string misc2ArtifactId;
    std::string misc3ArtifactId;
};

// Result of TryEquipArtifact / UnequipArtifact, mirroring events::ActionResult
// shape so the failure mode is identical across both surfaces.
struct EquipResult {
    bool success = true;
    std::string message;   // populated on failure; empty on clean success
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

    // Catalogs feed equip/event-action lookups. Sets are copies; callers may
    // re-set after a ContentRepository reload. Empty catalogs disable item /
    // artifact operations (give/take/equip will fail with "unknown id").
    void SetItemCatalog(std::vector<data::ItemDefinition> catalog);
    void SetArtifactCatalog(std::vector<data::ArtifactDefinition> catalog);
    // Unit catalog feeds the daily-starting Energy formula (needs per-unit
    // agility). Mirrors the item/artifact catalog pattern; set by the App at
    // startup. An empty catalog makes ApplyDailyStartingEnergy treat the party
    // as having no resolvable agility (1000 floor).
    void SetUnitCatalog(std::vector<data::UnitDefinition> catalog);

    // Team Energy pool (docs/core_loop_rules.md §6). M14-a: state + daily reset.
    // ApplyDailyStartingEnergy recomputes dailyMaxEnergy_ from the lowest agility
    // across the entire traveling party (active + reserve) using the unit catalog,
    // with leader passive/item bonuses as zero-valued seams, and sets
    // currentEnergy_ to that value. Spend/recover primitives arrive in M14-b.
    void ApplyDailyStartingEnergy();
    [[nodiscard]] int CurrentEnergy() const;
    [[nodiscard]] int MaxEnergy() const;

    // M14-b spend / recover primitives. Negative spend fails loudly (returns
    // false / does nothing) so a bad future caller cannot silently succeed;
    // zero spend is a harmless success. Energy never exceeds dailyMaxEnergy_
    // and never drops below 0.
    [[nodiscard]] bool CanSpendEnergy(int amount) const;
    bool TrySpendEnergy(int amount);
    void RecoverEnergy(int amount);

    [[nodiscard]] const std::vector<ItemStackState>& Items() const;
    [[nodiscard]] const std::vector<ArtifactStackState>& Artifacts() const;
    // Returns equipment for the given hero/unit id; default-constructed (all
    // slots empty) if the hero has never been equipped.
    [[nodiscard]] HeroEquipmentState HeroEquipment(const std::string& heroId) const;

    // Equip moves one artifact stack-of-1 out of the unequipped Artifacts()
    // container into the hero's named slot. Fails explicitly on unknown
    // artifact, slot/allowedSlots mismatch, hero not in traveling roster, or
    // the slot already being occupied. Unequip is the inverse.
    EquipResult TryEquipArtifact(
        const std::string& heroId,
        ArtifactEquipSlot slot,
        const std::string& artifactId);
    EquipResult UnequipArtifact(
        const std::string& heroId,
        ArtifactEquipSlot slot);

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

    // M13-b runtime inventory + equipment state.
    std::vector<ItemStackState>     items_;
    std::vector<ArtifactStackState> artifacts_;          // unequipped only
    std::map<std::string, HeroEquipmentState> heroEquipment_;
    std::vector<data::ItemDefinition>     itemCatalog_;
    std::vector<data::ArtifactDefinition> artifactCatalog_;
    std::vector<data::UnitDefinition>     unitCatalog_;

    // M14-a team Energy pool. dailyMaxEnergy_ is the day's ceiling and reset
    // target; currentEnergy_ is the spendable amount, clamped to [0, max].
    int currentEnergy_ = 0;
    int dailyMaxEnergy_ = 0;

    // Lowest agility across the entire traveling party (active + reserve),
    // resolved through unitCatalog_. Units missing from the catalog are skipped.
    // Returns 0 when no party agility is resolvable (empty party or no catalog).
    [[nodiscard]] int LowestTravelingPartyAgility() const;

    // Single chokepoint for all time advances. Detects a day-boundary crossing
    // and triggers ApplyDailyStartingEnergy() exactly once (a multi-day jump
    // resets to the formula value, not per skipped day, because reset is
    // "set to", not "add"). All three public time-advancing methods route
    // through here so daily Energy reset is correct regardless of which path
    // advanced the clock.
    void AdvanceClock(int minutes);

    [[nodiscard]] const data::ItemDefinition*     FindItemDefinition(const std::string& id) const;
    [[nodiscard]] const data::ArtifactDefinition* FindArtifactDefinition(const std::string& id) const;
    // Sums equipped artifact statBonuses for a given hero across all equipped
    // slots. Writes into the per-stat int outparams. Missing artifacts in the
    // catalog (e.g. catalog not yet set) contribute zero.
    void SumEquippedArtifactBonuses(
        const std::string& heroId,
        int& attackBonus,
        int& defenseBonus,
        int& magicBonus,
        int& resistanceBonus) const;
};

} // namespace gameplay

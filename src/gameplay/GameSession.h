#pragma once

#include <functional>
#include <optional>
#include <string>
#include <set>
#include <unordered_map>
#include <vector>

#include "core/GameClock.h"
#include "core/SaveGame.h"
#include "data/definitions/ArtifactDefinition.h"
#include "data/definitions/CampaignDefinition.h"
#include "data/definitions/ItemDefinition.h"
#include "data/definitions/LocationServiceDefinition.h"
#include "data/definitions/QuestDefinition.h"
#include "data/definitions/RegionDefinition.h"
#include "data/definitions/ScenarioDefinition.h"
#include "data/definitions/ScenarioOutcomeDefinition.h"
#include "data/definitions/UnitDefinition.h"
#include "data/definitions/WorldMapDefinition.h"
#include "gameplay/EnemyTeamState.h"
#include "gameplay/InventoryState.h"
#include "gameplay/ResourceState.h"
#include "gameplay/economy/MineProductionRules.h"
#include "gameplay/economy/StationedProductionRules.h"
#include "gameplay/campaign/CampaignCarryover.h"
#include "gameplay/events/EventDefinition.h"
#include "gameplay/events/EventEngine.h"
#include "gameplay/quests/QuestState.h"
#include "gameplay/scenario/ScenarioOutcomeRules.h"
#include "gameplay/worldmap/WorldMapTravelRules.h"
#include "gameplay/world/NodeWorldState.h"

#include <map>
#include <unordered_map>

namespace gameplay {

enum class GameMode {
    Title,
    OpeningSequence,
    CampaignSelectMode,
    WorldMapMode,
    RegionMode,
    LocationMode,
    BattleMode
};

// M16-b campaign run lifecycle. None = no campaign active (standalone play).
// InProgress = a scenario of the campaign is being played. Completed = the final
// scenario was won. Failed = a scenario was lost (defeat ends the campaign run).
enum class CampaignState { None, InProgress, Completed, Failed };

inline std::string CampaignStateToString(CampaignState state) {
    switch (state) {
        case CampaignState::InProgress: return "in_progress";
        case CampaignState::Completed:  return "completed";
        case CampaignState::Failed:     return "failed";
        case CampaignState::None:       return "";
    }
    return "";
}

inline CampaignState CampaignStateFromString(const std::string& value) {
    if (value == "in_progress") return CampaignState::InProgress;
    if (value == "completed")   return CampaignState::Completed;
    if (value == "failed")      return CampaignState::Failed;
    return CampaignState::None;
}

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
    // M14-c: team Energy pool exposed for render/HUD. currentEnergy and maxEnergy
    // mirror GameSession::CurrentEnergy() / MaxEnergy().
    int energy = 0;
    int maxEnergy = 0;
    // M16-b campaign status for render/HUD. campaignId is empty for standalone play.
    std::string campaignId;
    std::string currentScenarioId;
    CampaignState campaignState = CampaignState::None;
};

class GameSession {
public:
    GameSession();

    void AdvanceMode();
    void AddMinutes(int minutes);
    bool SpendGold(int amount);
    [[nodiscard]] bool TrySpendGold(int amount);

    // M17 team resource pool. ResourceType::Gold delegates to the existing
    // gold_ field / gold APIs (single source of truth); it is never stored in
    // the non-gold container. Non-gold counts never go below zero.
    [[nodiscard]] int ResourceCount(ResourceType type) const;
    // Adds `amount` of `type`. Negative amounts are clamped so the count never
    // drops below zero (use TrySpendResource for guarded spends). Gold routes
    // through gold_.
    void AddResource(ResourceType type, int amount);
    // Spends `amount` of `type` iff at least that much is held. Returns false
    // and leaves state unchanged when insufficient. amount <= 0 is a no-op that
    // returns true. Gold routes through TrySpendGold.
    [[nodiscard]] bool TrySpendResource(ResourceType type, int amount);

    // M17 owned-service runtime state. Read-only accessor; ownership mutation
    // rules arrive in a later milestone.
    [[nodiscard]] const std::vector<core::OwnedServiceSaveState>& OwnedServices() const;
    [[nodiscard]] const core::OwnedServiceSaveState* FindOwnedService(
        const std::string& serviceId) const;

    // M17 Phase 3a: resolve an owned service's normalized stack-backed stationed
	// units to the mine-production passives they contribute, for a producing
	// service of `serviceKind`. The service catalog lives in ContentRepository,
	// so the caller supplies the kind. Pure read — no payout, no resource
	// mutation, no clock advance. Returns the list consumed by
	// ComputeMineDailyOutput.
	[[nodiscard]] std::vector<economy::MineProductionPassive>
	CollectStationedMineProductionPassives(
		const std::string& serviceId,
		data::LocationServiceKind serviceKind) const;

	// M17 Phase 3a: drop stationed refs that are not stack-backed. A valid ref
	// must have a non-empty unitId and stackId, stackId must resolve to a live
	// roster stack, and that stack's unitId must match the ref's unitId.
	// Idempotent. Runs automatically on ApplySaveData; exposed for explicit
	// re-normalization.
	void NormalizeStationedUnits();

    // M17 Phase 4b: effective ownership tier when the player USES a specific
    // trader service. This is the benefit gate to apply at a trader service:
    // returns 0 unless `serviceId` is a known trader-kind service with owned
    // runtime state that the player owns and that is not locked, destroyed, or
    // hostile-occupied; otherwise returns the player's ownership tier for that
    // service's own trader type (per-type, eligible-only, capped at 8). This
    // enforces "benefits apply only when the owning team uses a same-type
    // service it owns." Prefer this over the raw type count below as the public
    // benefit API. Pure read; no transaction.
    [[nodiscard]] int OwnedTraderServiceTierForService(const std::string& serviceId) const;

    // M17 Phase 4: RAW per-type ownership count for the player team (eligible,
    // capped at 8). This is a type-level count only and is NOT the benefit gate
    // for using a specific service — it does not know which service is in use, so
    // it cannot honor "benefits apply only when using a same-type service the
    // player owns." Use OwnedTraderServiceTierForService for that. Kept for
    // tests/diagnostics. Requires the location-service catalog.
    [[nodiscard]] int OwnedTraderServiceTier(data::LocationServiceKind traderKind) const;

    void EnterLocationMode(const std::string& locationId);
    void EnterRegionMode();
    void EnterWorldMapMode();
    void EnterCampaignSelectMode();
    void EnterTitleMode();
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

    // M17 Phase 3b: location-service catalog gives the session read access to
    // service definitions (kind + authored mine outputs + locationId) so the
    // day-boundary mine payout can resolve owned-service ids. Mirrors the catalog
    // pattern; set by the App at startup. An empty catalog makes daily mine
    // payout a no-op.
    void SetLocationServiceCatalog(std::vector<data::LocationServiceDefinition> catalog);

    // M15-b World Map. SetWorldMap seeds the persisted runtime unlocked-region
    // set from the authored `unlocked` flags. SetRegionCatalog gives the session
    // read access to region definitions (to resolve a destination's arrival node
    // at travel time). Both mirror the catalog pattern; set by the App at startup.
    void SetWorldMap(data::WorldMapDefinition worldMap);
    void SetRegionCatalog(std::vector<data::RegionDefinition> catalog);
    [[nodiscard]] const data::WorldMapDefinition& WorldMap() const;
    [[nodiscard]] bool IsRegionUnlocked(const std::string& regionId) const;
    // True iff on the Region layer and the current node is an authored exit node
    // of the current region's World Map entry.
    [[nodiscard]] bool CanOpenWorldMapHere() const;
    // Count of generic units in the traveling party (active + reserve) that would
    // be lost on a World Map departure. Read-only; mirrors the removal helper.
    [[nodiscard]] int GenericTravelingPartyUnitCount() const;

    struct WorldMapTravelResult {
        bool success = false;
        worldmap::WorldMapTravelBlockReason reason = worldmap::WorldMapTravelBlockReason::None;
        int days = 0;
        int genericsDropped = 0;
    };
    // Apply a World Map trip to session state. Re-checks legality (exit-node gate
    // + pure rule); on any illegal case returns a result and mutates nothing.
    // On success: spends 1000 Energy, drops generic traveling-party units (heroes
    // persist), advances the clock to arrive at 11:00 after `days` (which refreshes
    // Energy via the day-rollover chokepoint), and switches the current region to
    // the destination's arrival node. Stays in RegionMode.
    WorldMapTravelResult TravelToRegion(const std::string& destinationRegionId);

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
    // Sets BOTH the global fallback and the currently-active outcome definition.
    // Preserves M12 / standalone behavior: a session with no campaign evaluates
    // exactly this definition. During a campaign, scenario starts re-select the
    // active definition (inline scenario conditions, else this global fallback).
    void SetScenarioOutcomeDefinition(data::ScenarioOutcomeDefinition definition);
    // The definition CheckAndLatchOutcome currently evaluates. Exposed for
    // inspection/tests (Correction 2: active vs global fallback separation).
    [[nodiscard]] const data::ScenarioOutcomeDefinition& ActiveScenarioOutcomeDefinition() const;

    // M16-b campaign catalogs. Stored once with id->index maps for O(1) lookup;
    // no list scans in hot paths (campaign logic runs only at start/transition).
    void SetScenarioCatalog(std::vector<data::ScenarioDefinition> catalog);
    void SetCampaignCatalog(std::vector<data::CampaignDefinition> catalog);
    [[nodiscard]] const std::vector<data::CampaignDefinition>& CampaignCatalog() const;

    // M16-b campaign progression. StartCampaign begins from the EXISTING default
    // startup roster (M16 keeps the default roster model; scenarios define no
    // roster) and enters the campaign's startScenarioId in RegionMode. No-ops if
    // the campaign id is unknown.
    void StartCampaign(const std::string& campaignId);
    // Call after an outcome latches while a campaign is active. Advances on
    // Victory (applying the old scenario's carry-over rule), marks the run Failed
    // on Defeat, Completed when the final scenario is won. One-shot per latch.
    void ResolveCampaignAfterOutcome();
    // Victory worker (also usable directly in tests after a latched Victory):
    // captures + filters carry-over and transitions to the next scenario, or
    // marks the run Completed when there is no next scenario.
    void AdvanceCampaignOnVictory();

    [[nodiscard]] bool IsCampaignActive() const;
    [[nodiscard]] CampaignState GetCampaignState() const;
    [[nodiscard]] const std::string& CampaignId() const;
    [[nodiscard]] const std::string& CurrentScenarioId() const;
    [[nodiscard]] const std::vector<std::string>& CompletedScenarioIds() const;
    [[nodiscard]] const std::set<std::string>& CampaignFlags() const;

    // Evaluates outcome against current session state. If non-Ongoing and the
    // session has not yet latched, latches the outcome (and stays latched
    // through save/load). Called automatically at the boundaries documented in
    // implementation_roadmap.md.00.archived end of FireMatchingEvents (StartOfDay /
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

    // M17 event-context resource bridge. Seeds ctx.resources with every resource
    // (Gold from gold_, non-gold from nonGoldResources_) so event conditions and
    // actions can read/modify any resource by canonical name. ApplyEventResource-
    // Context writes the (possibly mutated) values back, routing Gold to gold_
    // and non-gold to the pool — preserving the single-source-of-truth rule.
    void SeedEventResourceContext(events::EventEvaluationContext& ctx) const;
    void ApplyEventResourceContext(const events::EventEvaluationContext& ctx);

    GameMode mode_;
    core::GameClock clock_;
    std::vector<core::RecruitServiceState> recruitServiceStates_;
    std::vector<core::DailyServiceState> dailyServiceStates_;
    int travelPrepDiscountMinutes_ = 0;
    int travelPrepRemainingCharges_ = 0;
    int travelPrepGrantedDay_ = 0;
    int gold_;
    // M17 non-gold resource pool, indexed by NonGoldResourceIndex(). Gold is
    // never stored here — it lives solely in gold_. Default-zero.
    std::array<int, kNonGoldResourceCount> nonGoldResources_{};
    // M17 owned-service runtime state: Phase 1 stable fields plus Phase 3a
    // stationing. Stationed refs are stack-backed and normalized on load
    // (NormalizeStationedUnits) so they always reference live roster stacks.
    std::vector<core::OwnedServiceSaveState> ownedServices_;
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
    // M16-b: global fallback (from scenario_outcome.json, never mutated) kept
    // separate from the active definition CheckAndLatchOutcome evaluates.
    data::ScenarioOutcomeDefinition globalScenarioOutcomeDefinition_;
    data::ScenarioOutcomeDefinition activeScenarioOutcomeDefinition_;
    std::optional<scenario::ScenarioOutcome> latchedOutcome_;

    // M16-b campaign runtime state + catalogs.
    std::vector<data::ScenarioDefinition> scenarioCatalog_;
    std::vector<data::CampaignDefinition> campaignCatalog_;
    std::unordered_map<std::string, std::size_t> scenarioIndexById_;
    std::unordered_map<std::string, std::size_t> campaignIndexById_;
    std::string campaignId_;
    std::string currentScenarioId_;
    std::vector<std::string> completedScenarioIds_;
    std::set<std::string> campaignFlags_;
    CampaignState campaignState_ = CampaignState::None;
    // Authored seeds captured so a scenario transition can reset scenario-local
    // runtime to a clean baseline without an App callback.
    std::vector<data::QuestDefinition> questDefinitionSeed_;
    std::vector<EnemyTeamState>        enemyTeamSeed_;

    static constexpr int kDefaultStartGold = 2500;

    // The single ordered scenario-start/transition chokepoint (M16-b). carry is
    // nullopt on initial campaign start (baseline roster kept) and present on a
    // victory transition (filtered carry-over applied after reset/seeding).
    void TransitionToScenario(const std::string& scenarioId,
                              std::optional<campaign::CampaignCarrySet> carry);
    [[nodiscard]] const data::ScenarioDefinition* FindScenarioDefinition(const std::string& id) const;
    [[nodiscard]] const data::CampaignDefinition* FindCampaignDefinition(const std::string& id) const;
    [[nodiscard]] std::string PlayerHeroUnitId() const;
    [[nodiscard]] std::set<std::string> ValidUnitIdSet() const;
    [[nodiscard]] campaign::CampaignCarrySet CaptureCarrySet() const;
    void ApplyCarrySet(const campaign::CampaignCarrySet& set);
    void ReseedWorldMapUnlock();
    void SelectActiveOutcomeForScenario(const data::ScenarioDefinition& scenario);

    // M13-b runtime inventory + equipment state.
    std::vector<ItemStackState>     items_;
    std::vector<ArtifactStackState> artifacts_;          // unequipped only
    std::map<std::string, HeroEquipmentState> heroEquipment_;
    std::vector<data::ItemDefinition>     itemCatalog_;
    std::vector<data::ArtifactDefinition> artifactCatalog_;
    std::vector<data::UnitDefinition>     unitCatalog_;
    // M17 Phase 3b: read-only service definitions for day-boundary mine payout.
    std::vector<data::LocationServiceDefinition> locationServiceCatalog_;

    // M15-b World Map state.
    data::WorldMapDefinition              worldMap_;
    std::vector<data::RegionDefinition>   regionCatalog_;   // for arrival-node lookup
    std::set<std::string>                 unlockedRegionIds_;

    // M14-a team Energy pool. dailyMaxEnergy_ is the day's ceiling and reset
    // target; currentEnergy_ is the spendable amount, clamped to [0, max].
    int currentEnergy_ = 0;
    int dailyMaxEnergy_ = 0;

    // Lowest agility across the entire traveling party (active + reserve),
    // resolved through unitCatalog_. Units missing from the catalog are skipped.
    // Returns 0 when no party agility is resolvable (empty party or no catalog).
    [[nodiscard]] int LowestTravelingPartyAgility() const;

    // Current active-party leader resolution (matches battle's AssignLeader:
    // first player-character leader-capable unit, else first leader-capable;
    // leader-capable = Leader/Hero). Nullptr when none. Used to source the
    // leader's LeaderEnergy passive bonus (the Y term in the Energy formula).
    [[nodiscard]] const data::UnitDefinition* CurrentLeaderUnitDefinition() const;
    [[nodiscard]] int LeaderPassiveEnergyBonus() const;

    [[nodiscard]] const data::RegionDefinition* FindRegionDefinition(const std::string& id) const;
    // M17 Phase 3a: unit-definition lookup in the loaded catalog (nullptr if the
    // catalog is unset or has no such id). Used by stationing normalization and
    // passive collection.
    [[nodiscard]] const data::UnitDefinition* FindUnitDefinition(const std::string& id) const;
    // True iff the current node (regardless of mode) is an authored World Map
    // exit node of the current region's entry. The mode check lives in the
    // callers: CanOpenWorldMapHere() gates opening from RegionMode; TravelToRegion
    // additionally accepts WorldMapMode (the screen is opened from the exit node).
    [[nodiscard]] bool IsOnWorldMapExitNode() const;
    // True for Hero/Leader units (which travel between Regions); false for
    // generics (which are dropped on World Map travel). Uses the unit catalog
    // category, falling back to the leader-capable set when the unit is absent.
    [[nodiscard]] bool IsHeroUnit(const std::string& unitId) const;
    // Removes every generic stack from the traveling party (active + reserve),
    // returning the total generic unit count dropped. Heroes are untouched.
    int RemoveGenericTravelingPartyUnits();

    // Single chokepoint for all time advances. Detects a day-boundary crossing
    // and triggers ApplyDailyStartingEnergy() and ApplyDailyMinePayout() exactly
    // once (a multi-day jump fires once, not per skipped day — matching the
    // Energy "set to" reset and avoiding per-day payout multiplication). All
    // public time-advancing methods route through here so daily effects are
    // correct regardless of which path advanced the clock.
    void AdvanceClock(int minutes);

    // M17 Phase 3b: pay owned mine/resource services their daily output to the
    // player team. Fired once per AdvanceClock day-boundary crossing. Builds its
    // service/roster/unit lookups once per pass and reuses them across all owned
    // services. Gold output routes through gold_; non-gold to the resource pool.
    // No-op when the service catalog is unset.
    void ApplyDailyMinePayout();

    // M17 Phase 3b: resolve an owned service's normalized (stack-backed)
    // stationed refs to unit definitions, using caller-built lookup maps so a
    // multi-service payout pass does not rebuild them per service. Re-checks the
    // stack-backing defensively so a stale ref never resolves.
    [[nodiscard]] std::vector<const data::UnitDefinition*> ResolveStationedUnitDefs(
        const core::OwnedServiceSaveState& owned,
        const std::unordered_map<std::string, std::string>& stackUnitById,
        const std::unordered_map<std::string, const data::UnitDefinition*>& defById) const;

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

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
#include "data/definitions/TraderOwnershipCurve.h"
#include "data/definitions/QuestDefinition.h"
#include "data/definitions/RegionDefinition.h"
#include "data/definitions/ScenarioDefinition.h"
#include "data/definitions/ScenarioOutcomeDefinition.h"
#include "data/definitions/EnemyGroupDefinition.h"
#include "data/definitions/UnitDefinition.h"
#include "data/definitions/WorldMapDefinition.h"
#include "gameplay/EnemyTeamState.h"
#include "gameplay/InventoryState.h"
#include "gameplay/ResourceState.h"
#include "gameplay/battle/AutoResolve.h"
#include "gameplay/battle/ThreatPreview.h"
#include "gameplay/economy/MineProductionRules.h"
#include "gameplay/economy/StationedProductionRules.h"
#include "gameplay/economy/TraderOwnershipRules.h"
#include "gameplay/economy/ServiceClaimRules.h"
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
    BattleMode,
    // Transient end-of-scenario result screen. Entered only when an outcome is
    // latched; left on the player's Continue. Never persisted (App refuses to
    // save while in this mode; see GameSession::FromString for the load-side
    // self-heal).
    ScenarioResultMode,
    // M27 transient read-only owned-service overview panel. Opened from Region
    // mode, left on cancel/toggle. Never persisted (App refuses to save while in
    // this mode; FromString self-heals it to RegionMode on the load side).
    OwnedServiceOverviewMode
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

// Result of a Trading Post transaction. `message` carries the failure reason on
// refusal, or a short success note. Mirrors the EquipResult shape.
struct TradeResult {
    bool success = false;
    std::string message;
};

// Read-only snapshot of what a Trading Post offers right now: whether it is
// usable, the player's effective ownership tier for it, the resolved barter
// matrix for that tier, and the Gold-trade price factor. The interaction layer
// uses this to build/preview options; trades still execute through the
// TryTradingPost* APIs (which re-gate atomically), so gate/curve resolution
// lives in exactly one place. `usable == false` means no options are offered.
struct TradingPostOffer {
    bool usable = false;
    int effectiveTier = 0;
    std::vector<economy::ResolvedExchange> barter;
    int priceFactor = 100;
};

struct EnemyTeamActionResult {
    std::string teamColor;
    // "idle" | "moved" | "service_attack" (resolved in-session) |
    // "service_attack_pending_battle" (player party on the node: the App must
    // run the defense battle and report back).
    std::string actionType;
    std::string nodeId;       // attack target; empty for idle/moved
    std::string summaryText;  // bounded player-facing line for service attacks
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
    // Preflight for a grant: true iff `amount` of `type` can be added without
    // overflowing int. False for a negative amount. Uses safe arithmetic
    // (max - amount) rather than forming current + amount first. Callers that
    // spend-then-grant (e.g. trades) must check this before the spend so the
    // whole operation stays atomic.
    [[nodiscard]] bool CanAddResource(ResourceType type, int amount) const;
    // Spends `amount` of `type` iff at least that much is held. Returns false
    // and leaves state unchanged when insufficient. amount <= 0 is a no-op that
    // returns true. Gold routes through TrySpendGold.
    [[nodiscard]] bool TrySpendResource(ResourceType type, int amount);

    // M17 owned-service runtime state. Read-only accessors. The only in-play
    // ownership mutation is M23 claiming via ClaimContestedServicesAtNode.
    [[nodiscard]] const std::vector<core::OwnedServiceSaveState>& OwnedServices() const;
    [[nodiscard]] const core::OwnedServiceSaveState* FindOwnedService(
        const std::string& serviceId) const;

    // M26 general node-entry claim resolver. Claim for the player every eligible
    // ownable service at `nodeId`'s location when the node is not blocked by a
    // hostile guard/occupier. Eligible = ownable kind, not locked, not destroyed,
    // and currently unowned or hostile-owned (player-owned unchanged so the
    // player's own stationed units are never cleared on re-entry; allied-owned not
    // claimed). On claim the service's ownerTeamColor becomes the player color and
    // any inherited (enemy) stationed units are cleared. No-op (returns empty) when
    // `nodeId` is empty or still hostile-occupied by another team. Content
    // definitions are never mutated. Returns the claimed service ids.
    //
    // This is the single claim path used for BOTH peaceful legal node entry and
    // post-battle guarded capture (call after ClearEnemyTeamByColor so the cleared
    // node is no longer hostile-occupied).
    std::vector<std::string> ResolveNodeEntryClaims(const std::string& nodeId);

    // Back-compat alias for ResolveNodeEntryClaims. Retained for existing callers
    // and tests; identical behavior.
    std::vector<std::string> ClaimContestedServicesAtNode(const std::string& nodeId);

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

	// M27 read-only daily-output preview for a mine service: authored base outputs
	// combined with the service's stationed strongest-only mine_production passives
	// via the SAME rules ApplyDailyMinePayout uses (parse base, collect passives,
	// ComputeMineDailyOutput). Returns the production the mine would yield in one
	// day; the payability gate (lock/destroy/hostile occupation) is intentionally
	// NOT applied here — it is surfaced separately as status. For a currently
	// payable owned mine this equals the resource delta a day boundary applies.
	// Empty for a non-mine / unknown service. Pure read; mutates nothing.
	[[nodiscard]] std::vector<economy::MineResourceOutput>
	PreviewMineDailyOutput(const std::string& serviceId) const;

	// M17 Phase 3a: drop stationed refs that are not stack-backed. A valid ref
	// must have a non-empty unitId and stackId, stackId must resolve to a live
	// roster stack, and that stack's unitId must match the ref's unitId.
	// Idempotent. Runs automatically on ApplySaveData; exposed for explicit
	// re-normalization.
	void NormalizeStationedUnits();

    // M25 player-facing stationing. Physical-placement model: a roster stack is in
    // exactly one place at a time — an active slot, a reserve slot, or stationed at
    // one owned service. Stationing moves a slotted stack out of its slot and adds
    // exactly one stationed ref; unstationing returns the SAME stack id to an empty
    // reserve slot. No unit is ever created, destroyed, or merged, so total unit
    // quantity is conserved. Targets are mines only in M25. These methods are the
    // only stationing-mutation surface — App/UI must route through them and never
    // edit stationedUnits or roster slots directly.

    // True iff `stackId` could be stationed at `serviceId` right now: the service
    // is a player-owned, unlocked, undestroyed mine below capacity, and the stack
    // exists, is currently slotted, is not the Player Character, and is not already
    // stationed anywhere. Does not include the active-party leader guard, which
    // only constrains pulling a whole stack out of an active slot. Pure read.
    [[nodiscard]] bool CanStationStackAtService(
        const std::string& serviceId, const std::string& stackId) const;

    // Move a whole slotted stack out of its active/reserve slot and station it at
    // the service, keeping the same stack id. Fails with no state change when
    // ineligible, when the stack is not currently slotted, or when removing it from
    // an active slot would leave the active party with no leader-capable unit.
    [[nodiscard]] bool TryStationStackAtService(
        const std::string& serviceId, const std::string& stackId);

    // Split `quantity` off a generic slotted stack into a NEW stack (fresh id) and
    // station that new stack; the source stack keeps its slot and id with the
    // remaining quantity. Requires 1 <= quantity < source quantity; a quantity
    // equal to the whole stack routes to TryStationStackAtService. Total unit
    // quantity is conserved exactly. Fails with no state change when ineligible.
    [[nodiscard]] bool TryStationSplitAtService(
        const std::string& serviceId, const std::string& stackId, int quantity);

    // Remove a stationed ref and return the SAME stack id to an empty reserve slot.
    // Fails atomically (no state change) when there is no free reserve slot. Never
    // creates, deletes, or merges a stack.
    [[nodiscard]] bool TryUnstationStackFromService(
        const std::string& serviceId, const std::string& stackId);

    // Stack ids currently slotted (active first, then reserve) that are eligible to
    // station at `serviceId` (non-Player-Character, not already stationed), or an
    // empty list when the service itself cannot currently receive a unit. For the
    // bounded stationing interaction's "station" list; the confirm path re-checks
    // full legality (including the active-party leader guard). Pure read.
    [[nodiscard]] std::vector<std::string> EligibleStationingStackIds(
        const std::string& serviceId) const;

    // Gate for OPENING the stationing interaction at a mine: true iff `serviceId`
    // is a player-owned, unlocked, undestroyed mine. Capacity is intentionally not
    // considered — a full mine can still be opened to unstation. Pure read.
    [[nodiscard]] bool CanOpenStationingAtMine(const std::string& serviceId) const;

    // === M28 storage placement (parallel to stationing; distinct concept) ===
    // A stored stack lives slot-less in the roster, referenced by exactly one owned
    // service's storedUnits. Storage cap is 7 (StorageRules::kMaxStoredUnitsPer
    // Service), separate from the mine-stationing cap of 5. Store requires the stack
    // be slotted (auto cross-exclusion with stationing); retrieve mirrors unstation.

    // True iff `stackId` could be stored at `serviceId` now: the service is a
    // player-owned, unlocked, undestroyed Storage below capacity, and the stack
    // exists, is currently slotted, is not the Player Character, and is not already
    // stationed or stored anywhere. Excludes the active-party leader guard (that
    // only constrains pulling from an active slot). Pure read.
    [[nodiscard]] bool CanStoreStackAtService(
        const std::string& serviceId, const std::string& stackId) const;

    // Move a whole slotted stack out of its active/reserve slot and store it at the
    // service, keeping the same stack id. Fails with no state change when ineligible,
    // not slotted, or when pulling from an active slot would leave no leader. Whole-
    // stack only in M28 (no split into storage).
    [[nodiscard]] bool TryStoreStackAtService(
        const std::string& serviceId, const std::string& stackId);

    // Remove a stored ref and return the SAME stack id to an empty reserve slot.
    // Fails atomically when reserve is full. Heals a corrupt slotted+stored double-
    // placement by removing only the stored ref. Never creates, deletes, or merges a
    // stack.
    [[nodiscard]] bool TryRetrieveStackFromService(
        const std::string& serviceId, const std::string& stackId);

    // Stack ids currently slotted that are eligible to store at `serviceId` (non-PC,
    // not already stationed/stored), or empty when the service cannot receive a unit.
    // Pure read; the confirm path re-checks full legality (leader guard).
    [[nodiscard]] std::vector<std::string> EligibleStorageStackIds(
        const std::string& serviceId) const;

    // Gate for OPENING the storage interaction: true iff `serviceId` is a player-
    // owned, unlocked, undestroyed Storage service. Capacity not considered (a full
    // storage can still be opened to retrieve). Pure read.
    [[nodiscard]] bool CanOpenStorageAtService(const std::string& serviceId) const;

    // M28: drop stored refs that are not stack-backed (mirrors NormalizeStationed-
    // Units). Idempotent; runs on ApplySaveData.
    void NormalizeStoredUnits();

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

    // Player-facing Trading Post transactions. Each gates on the service being a
    // usable Trading Post (not locked, destroyed, or hostile-occupied; ownership
    // never bypasses these), resolves the effective ownership tier for the exact
    // service (0 unless player-owned and eligible), then applies the authored or
    // default curve. Resource/Gold changes route through the existing resource
    // APIs (single Gold source of truth) and are atomic: nothing is mutated
    // unless the whole trade succeeds. Barter is non-Gold resource-for-resource;
    // buy/sell exchange a non-Gold resource against Gold.
    [[nodiscard]] TradeResult TryTradingPostBarter(
        const std::string& serviceId, ResourceType from, ResourceType to, int quantity);
    [[nodiscard]] TradeResult TryTradingPostBuyForGold(
        const std::string& serviceId, ResourceType resource, int quantity);
    [[nodiscard]] TradeResult TryTradingPostSellForGold(
        const std::string& serviceId, ResourceType resource, int quantity);

    // Resolves the current offer for a Trading Post service (usability, effective
    // tier, resolved barter matrix, Gold price factor) for display/preview. Pure
    // read; performs no transaction and mutates nothing.
    [[nodiscard]] TradingPostOffer ResolveTradingPostOffer(const std::string& serviceId) const;

    void EnterLocationMode(const std::string& locationId);
    void EnterRegionMode();
    void EnterWorldMapMode();
    void EnterCampaignSelectMode();
    void EnterTitleMode();
    void EnterScenarioResultMode();
    // M27 transient owned-service overview. Enter from Region mode; Exit returns to
    // Region mode. Read-only screen; never persisted (see ToString/FromString).
    void EnterOwnedServiceOverviewMode();
    void ExitOwnedServiceOverviewMode();
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

    // Authored trader ownership curves (per trader type). Mirrors the catalog
    // pattern; set by the App at startup. An empty catalog (or a type with no
    // curve) makes every trade resolve at the built-in defaults.
    void SetTraderCurveCatalog(std::vector<data::TraderOwnershipCurve> catalog);

    // M15-b World Map. SetWorldMap seeds the persisted runtime unlocked-region
    // set from the authored `unlocked` flags. SetRegionCatalog gives the session
    // read access to region definitions (to resolve a destination's arrival node
    // at travel time). Both mirror the catalog pattern; set by the App at startup.
    void SetWorldMap(data::WorldMapDefinition worldMap);
    void SetRegionCatalog(std::vector<data::RegionDefinition> catalog);
    [[nodiscard]] const data::WorldMapDefinition& WorldMap() const;
    [[nodiscard]] bool IsRegionUnlocked(const std::string& regionId) const;

    // M32 Scenario Context. The Region ids that belong to the active Scenario.
    // Empty means the default context of ALL loaded Regions (backward compatible
    // with thin scenarios that author no `regions` list). Derived from the active
    // ScenarioDefinition at scenario start and re-derived on load.
    [[nodiscard]] const std::vector<std::string>& ScenarioRegionIds() const;
    // True iff `regionId` is exposable in the active Scenario: always true when no
    // context is authored (empty set => all Regions), otherwise true only when the
    // Region is in the context list. Read models (World Map) and TravelToRegion
    // gate on this so a Scenario cannot expose unrelated global Regions.
    [[nodiscard]] bool IsRegionInScenarioContext(const std::string& regionId) const;

    // M32 fog/reveal foundation (docs/core_loop_rules.md §15). Reveal is per-Region
    // and HoMM-persistent (a revealed node never becomes unknown). Seeded around the
    // start node (and start-owned-service nodes) at scenario start, extended on legal
    // movement and World Map arrival, and persisted through save/load. Enemy-team and
    // node read models gate visibility on this so unrevealed nodes hide their content.
    [[nodiscard]] bool IsNodeRevealed(const std::string& regionId, const std::string& nodeId) const;
    // Revealed node ids for a Region (order unspecified). Empty when the Region has
    // no revealed nodes yet. Pure read.
    [[nodiscard]] std::vector<std::string> RevealedNodeIds(const std::string& regionId) const;
    // True iff on the Region layer and the current node is an authored exit node
    // of the current region's World Map entry.
    [[nodiscard]] bool CanOpenWorldMapHere() const;
    // One traveling generic stack that would be lost on a confirmed World Map
    // departure (docs/core_loop_rules.md §5). Only slotted (active/reserve)
    // stacks are at risk; stationed and stored stacks are not traveling, stay
    // behind, and survive Region change.
    struct TravelGenericLossEntry {
        std::string stackId;
        std::string unitId;
        int quantity = 0;
    };
    // At-risk stacks for the travel warning, in active-then-reserve slot order.
    // Pure read. The confirmed-travel removal deletes exactly this set, so the
    // loss the player confirms is the loss that happens.
    [[nodiscard]] std::vector<TravelGenericLossEntry> PreviewRegionTravelGenericLosses() const;
    // Total at-risk generic unit count (sum of preview quantities). Read-only.
    [[nodiscard]] int GenericTravelingPartyUnitCount() const;

    struct WorldMapTravelResult {
        bool success = false;
        worldmap::WorldMapTravelBlockReason reason = worldmap::WorldMapTravelBlockReason::None;
        int days = 0;
        int genericsDropped = 0;
    };
    // Apply a World Map trip to session state. Re-checks legality (exit-node gate
    // + pure rule); on any illegal case returns a result and mutates nothing.
    // On success: spends 1000 Energy, drops traveling (slotted) generic stacks
    // (heroes/Player Character travel; stationed and stored stacks stay behind
    // untouched), advances the clock to arrive at 11:00 after `days` (which
    // refreshes Energy via the day-rollover chokepoint), and switches the current
    // region to the destination's arrival node. Stays in RegionMode. This method
    // owns the loss resolution; warning/confirmation before calling it is the
    // caller's (App's) responsibility.
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
    // Standalone (non-campaign) scenario start: clears any campaign run state,
    // then enters `scenarioId` through the same scenario-transition chokepoint
    // StartCampaign uses (scenario-local reset, playerStart economy/services,
    // outcome selection, RegionMode). Like StartCampaign it begins from the
    // existing roster/clock — there is no scenario-authored roster yet. No-ops
    // if the scenario id is unknown. Callers gate selection legality (e.g.
    // standaloneSelectable) — this method only requires a valid id.
    void StartStandaloneScenario(const std::string& scenarioId);
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

    // M33 threat preview. A bounded, read-only danger estimate
    // (docs/core_loop_rules.md §16). `known` is false when there is no visible
    // threat to preview — including unrevealed nodes (M32 fog), so unknown enemies
    // never leak through preview. `band` is a cheap §16 power-ratio estimate (not
    // the full auto-resolve); `enemyColor` is the bounded presence estimate.
    // PURE: these methods mutate nothing.
    struct ThreatPreview {
        bool known = false;
        battle::ThreatBand band = battle::ThreatBand::Unknown;
        std::string enemyColor;
    };
    // Danger of the player's active party engaging a hostile team that occupies
    // `nodeId` in the current Region. Known only when the node is revealed and a
    // hostile team is present.
    [[nodiscard]] ThreatPreview ThreatPreviewForNode(const std::string& nodeId) const;
    // Danger to a player-owned attackable service when a hostile team currently
    // occupies its node (an imminent absent-player defense). Known only when the
    // service node is revealed and a hostile team is present there.
    [[nodiscard]] ThreatPreview ServiceDefensePreview(const std::string& serviceId) const;

    // M30 contested infrastructure. Service attacks are node-level: an attack
    // against a node resolves every eligible player-owned service there together
    // (occupation, payout gating, and claiming are already node-keyed). Eligible
    // = attackable kind (mine / trader / storage), player-owned, not locked, not
    // destroyed. Arrival nodes are never attacked (protected per
    // docs/core_loop_rules.md §8/§17); ProcessEnemyPhase enforces that gate.
    void SetEnemyGroupCatalog(std::vector<data::EnemyGroupDefinition> catalog);

    struct ServiceAttackOutcome {
        bool attacked = false;             // an eligible target existed; the attack was processed
        bool playerBattleRequired = false; // player party stands on the node: the App must run the
                                           // defense battle and report back via
                                           // ApplyServiceDefenseVictory/Defeat. No mutation yet.
        bool defendersFought = false;      // placed defenders auto-resolved a defense
        bool attackerWon = false;
        std::vector<std::string> capturedServiceIds;
        std::string summaryText;           // bounded player-facing line ("" when !attacked)
    };

    // Service ids at `nodeId` that `attackerColor` may legally attack right now.
    // Pure read; empty when the attacker is unknown/inactive/allied to the player.
    [[nodiscard]] std::vector<std::string> AttackableServiceIdsAtNodeFor(
        const std::string& attackerColor, const std::string& nodeId) const;

    // Resolve one enemy service attack against `nodeId`. Player absent:
    // deterministic auto-resolve (ServiceDefenseRules strength comparison) —
    // defenders hold -> the attacker team is defeated/deactivated; attacker wins
    // (or no defenders exist) -> every eligible service is captured, its placed
    // defender stacks are resolved (generics dismissed, heroes Temporarily
    // Unavailable, refs cleared in the same mutation), and the attacker occupies
    // the node. Player party standing on the node: returns playerBattleRequired
    // and mutates nothing — the existing interactive battle surface decides, then
    // the App reports the result through the two methods below.
    ServiceAttackOutcome ResolveServiceAttack(
        const std::string& attackerColor, const std::string& nodeId);

    // Player-involved defense battle results (App battle surface callbacks).
    // Victory: the attacker team is defeated/deactivated; services unchanged.
    // Defeat: eligible services at the node are captured exactly as in the
    // auto-resolve attacker-win path, and the attacker occupies the node. The
    // active party itself follows the existing battle defeat semantics (no
    // roster mutation here).
    void ApplyServiceDefenseVictory(const std::string& attackerColor, const std::string& nodeId);
    void ApplyServiceDefenseDefeat(const std::string& attackerColor, const std::string& nodeId);

    // M30 Temporarily Unavailable heroes (minimal storage-loss pipeline). While
    // listed here a hero owns no roster stack, so it is hidden from active/
    // reserve/stationing/storage automatically. Heroes become returnable after
    // economy::kUnavailableHeroReturnDays and rejoin the reserve at the next day
    // start with a free reserve slot (stand-in for shared-hero-pool re-entry,
    // which is future scope). The Player Character never enters this list.
    [[nodiscard]] const std::vector<core::TemporarilyUnavailableHeroSaveState>&
    UnavailableHeroes() const;

    // M30 bounded service event log (newest last, capped). Read-only surface for
    // overview/log presentation.
    [[nodiscard]] const std::vector<core::ServiceEventLogEntrySaveState>&
    ServiceEventLog() const;

    // M30 minimal destruction/restoration (docs/core_loop_rules.md §20).
    // Destruction is opt-in via the authored `destroyable` flag, requires the
    // player to occupy the node (standing on it in Region mode, no hostile
    // occupier), costs 1000 Energy + 1 hour, and is blocked while the service
    // has placed (stationed/stored) units. Destroying a destroyed service whose
    // restoration is queued cancels the queue (§20), at full cost. Restoration
    // requires standing on the node, spends the authored restore cost (no
    // Energy, no time), queues, and completes at the next day start — before
    // that day's mine payout, so a restored mine pays immediately.
    [[nodiscard]] bool CanDestroyServiceAtCurrentNode(const std::string& serviceId) const;
    bool TryDestroyServiceAtCurrentNode(const std::string& serviceId);
    [[nodiscard]] bool CanQueueServiceRestorationAtCurrentNode(const std::string& serviceId) const;
    bool TryQueueServiceRestorationAtCurrentNode(const std::string& serviceId);
    // First destroyable service authored at the player's current node, if any.
    // Read used by the App's bounded maintenance action.
    [[nodiscard]] const data::LocationServiceDefinition* DestroyableServiceAtCurrentNode() const;

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

    // M25 stationing helpers. Pure reads over runtime state.
    [[nodiscard]] bool IsStackStationedAnywhere(const std::string& stackId) const;
    // M28: true iff `stackId` is stored at any owned service.
    [[nodiscard]] bool IsStackStoredAnywhere(const std::string& stackId) const;
    // True iff `stackId` currently occupies an active or reserve slot.
    [[nodiscard]] bool IsStackSlotted(const std::string& stackId) const;
    [[nodiscard]] bool UnitIsPlayerCharacter(const std::string& unitId) const;
    [[nodiscard]] core::OwnedServiceSaveState* FindOwnedServiceMutable(
        const std::string& serviceId);
    // Ordered index (among occupied active slots) of `stackId`, or -1 if it is not
    // in an active slot. The ordered index is what the leader-guard helpers expect.
    [[nodiscard]] int ActiveOrderedIndexOfStack(const std::string& stackId) const;

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

    // M30 contested-infrastructure runtime state.
    std::vector<data::EnemyGroupDefinition> enemyGroupCatalog_;
    std::vector<core::TemporarilyUnavailableHeroSaveState> unavailableHeroes_;
    std::vector<core::ServiceEventLogEntrySaveState> serviceEventLog_;
    static constexpr int kMaxServiceEventLogEntries = 24;

    // Append one bounded player-facing service event line (day = current day);
    // trims the oldest entries beyond kMaxServiceEventLogEntries.
    void AppendServiceEvent(std::string text);
    // Deterministic attack strength of an authored enemy group (sum of unit
    // defense powers; unknown group/units contribute 0). Retained as the cheap
    // power proxy used by the M33 threat-preview band.
    [[nodiscard]] int EnemyGroupPower(const std::string& enemyGroupId) const;
    // Total defense power of the placed (stationed + stored) stacks across the
    // eligible services at a node. Cheap power proxy for the threat-preview band.
    [[nodiscard]] int NodeDefenderPower(const std::vector<std::string>& serviceIds) const;
    // Cheap power of the player's active battle party (the same UnitDefensePower
    // proxy), used for the Region-travel threat-preview band. Pure read.
    [[nodiscard]] int PlayerActivePartyPower() const;
    // M33 auto-resolve force builders. The attacker force is the authored enemy
    // group's units (one quantity-1 stack each, first hero/leader gets the aura).
    // The defender force is the placed (stationed + stored) stacks across the
    // eligible services (first hero gets the aura). Both map authored stats through
    // the shared data->battle conversion so the auto-resolve sees the same stats
    // the interactive battle would. Pure reads.
    [[nodiscard]] std::vector<battle::AutoResolveUnit> BuildEnemyGroupForce(
        const std::string& enemyGroupId) const;
    [[nodiscard]] std::vector<battle::AutoResolveUnit> BuildNodeDefenderForce(
        const std::vector<std::string>& serviceIds) const;
    // Capture worker shared by the auto-resolve attacker-win path and the
    // player-defeat path: resolves placed defender stacks (generic -> dismissed,
    // hero -> Temporarily Unavailable, Player Character stack never removed),
    // clears the refs in the same mutation, and transfers ownership. Returns the
    // captured service ids.
    std::vector<std::string> ApplyServiceCaptureAtNode(
        const std::string& attackerColor, const std::string& nodeId);
    // Day-start TU hero return: entries past returnDay rejoin the reserve when a
    // slot is free (otherwise retried at later day starts).
    void ApplyDailyHeroReturns();
    // Day-start restoration completion: destroyed services with a queued
    // restoration become intact again (runs before the day's mine payout).
    void ApplyDailyServiceRestorations();
    static constexpr int kServiceDestructionEnergyCost = 1000;
    static constexpr int kServiceDestructionMinutes = 60;
    // Drop TU entries that are not minimally valid (empty unitId, or the Player
    // Character, which must never be unavailable). Runs on load.
    void NormalizeUnavailableHeroes();

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
    // M32: rebuild the active/reserve roster from an authored scenario starting
    // roster (clears the previous roster and stack ids first). Generic entries
    // become quantity stacks; hero entries become quantity-1 stacks. Validation
    // guarantees the Player Character is present exactly once and the active list
    // has a leader; this method does not re-validate. Used only when a scenario
    // authors a roster.
    void ApplyAuthoredStartRoster(
        const std::vector<data::ScenarioStartRosterEntry>& active,
        const std::vector<data::ScenarioStartRosterEntry>& reserve);
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
    // Authored trader ownership curves, looked up by trader kind for transactions.
    std::vector<data::TraderOwnershipCurve> traderCurveCatalog_;

    // M15-b World Map state.
    data::WorldMapDefinition              worldMap_;
    std::vector<data::RegionDefinition>   regionCatalog_;   // for arrival-node lookup
    std::set<std::string>                 unlockedRegionIds_;

    // M32 Scenario Context: Region ids the active Scenario exposes. Empty => all
    // Regions (default). Derived from the active ScenarioDefinition at scenario
    // start (TransitionToScenario) and re-derived in ApplySaveData.
    std::vector<std::string>              scenarioRegionIds_;

    // M32 fog/reveal runtime state: revealed node ids per Region. HoMM-persistent.
    std::map<std::string, std::set<std::string>> revealedNodesByRegion_;
    static constexpr int kRevealRadius = 2;   // nodes revealed ahead on move/seed
    // Reveal the radius-kRevealRadius graph neighborhood of `nodeId` in `regionId`
    // (using the region catalog's links). When the region/node is not resolvable
    // through the catalog, at least the node itself is marked revealed so the
    // current node is always known. Idempotent.
    void RevealAroundNode(const std::string& regionId, const std::string& nodeId);
    // Seed reveal for a fresh scenario start: the current node plus any start-owned
    // service nodes that live in the current Region. Clears nothing on its own;
    // TransitionToScenario clears revealedNodesByRegion_ before calling this.
    void SeedScenarioReveal();

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
    // Location-service definition lookup by id (nullptr if absent).
    [[nodiscard]] const data::LocationServiceDefinition* FindLocationServiceById(
        const std::string& serviceId) const;
    // Authored trader curve for a trader kind (nullptr if none authored, which
    // resolves to built-in defaults).
    [[nodiscard]] const data::TraderOwnershipCurve* FindTraderCurve(
        data::LocationServiceKind kind) const;

    // Resolved gate for a Trading Post transaction. `service` is null when the id
    // is unknown; `isTradingPost` is false when the id resolves to a non-Trading-
    // Post service; `usable` reflects the lock/destruction/occupation gate; and
    // `effectiveTier` is the player's ownership tier for the exact service (0
    // unless player-owned and eligible), computed only when usable.
    struct TradingPostUseGate {
        const data::LocationServiceDefinition* service = nullptr;
        bool isTradingPost = false;
        bool usable = false;
        int effectiveTier = 0;
    };
    [[nodiscard]] TradingPostUseGate GateTradingPostUse(const std::string& serviceId) const;
    // True iff the current node (regardless of mode) is an authored World Map
    // exit node of the current region's entry. The mode check lives in the
    // callers: CanOpenWorldMapHere() gates opening from RegionMode; TravelToRegion
    // additionally accepts WorldMapMode (the screen is opened from the exit node).
    [[nodiscard]] bool IsOnWorldMapExitNode() const;
    // True for Hero/Leader units (which travel between Regions); false for
    // generics (which are dropped on World Map travel). Uses the unit catalog
    // category, falling back to the leader-capable set when the unit is absent.
    [[nodiscard]] bool IsHeroUnit(const std::string& unitId) const;
    // Removes exactly the PreviewRegionTravelGenericLosses() set: traveling
    // (slotted) generic stacks. Heroes, the Player Character, and stationed/
    // stored stacks are never in that set, so placed-unit refs cannot dangle.
    // Returns the total generic unit count dropped.
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

    // Classify an owned-service owner team color relative to the player, reusing
    // the alliance determination used by HostileOccupiedNodeIds. Empty
    // color => Unowned; player color => Player; a color whose active team is
    // allied to the player => AlliedToPlayer; otherwise HostileToPlayer.
    [[nodiscard]] economy::ServiceOwnerRelationship OwnerRelationshipForColor(
        const std::string& ownerColor) const;

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

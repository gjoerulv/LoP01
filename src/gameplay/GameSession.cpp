#include "gameplay/GameSession.h"
#include "gameplay/EnergyRules.h"
#include "gameplay/economy/MinePayoutRules.h"
#include "gameplay/economy/StationingRules.h"
#include "gameplay/economy/StorageRules.h"
#include "gameplay/economy/TraderOwnershipRules.h"
#include "gameplay/economy/TradingPostTransactionRules.h"
#include "gameplay/effects/UnitPassiveEffects.h"
#include "gameplay/campaign/CampaignProgressionRules.h"
#include "gameplay/events/EventEngine.h"
#include "gameplay/events/EventParser.h"
#include "gameplay/region/RegionTravelRules.h"

#include <algorithm>
#include <limits>
#include <map>
#include <set>
#include <unordered_map>

namespace gameplay {


namespace
{
    bool IsValidOwnedEntry(const OwnedUnitCountState& entry) {
        return !entry.unitId.empty() && entry.count > 0;
    }

    bool IsValidStackEntry(const RosterStackState& entry) {
        return !entry.stackId.empty() && !entry.unitId.empty() && entry.quantity > 0;
    }

    int ParseStackIdSuffix(const std::string& stackId) {
        constexpr const char* kPrefix = "stk_";
        constexpr size_t kPrefixSize = 4;

        if (stackId.size() <= kPrefixSize || stackId.rfind(kPrefix, 0) != 0) {
            return -1;
        }

        const std::string suffix = stackId.substr(kPrefixSize);
        if (suffix.empty()) {
            return -1;
        }

        for (const char ch : suffix) {
            if (ch < '0' || ch > '9') {
                return -1;
            }
        }

        try {
            return std::stoi(suffix);
        }
        catch (...) {
            return -1;
        }
    }

    core::RecruitServiceState* FindRecruitServiceState(
        std::vector<core::RecruitServiceState>& states,
        const std::string& serviceId) {
        for (auto& state : states) {
            if (state.serviceId == serviceId) {
                return &state;
            }
        }

        return nullptr;
    }

    const core::RecruitServiceState* FindRecruitServiceState(
        const std::vector<core::RecruitServiceState>& states,
        const std::string& serviceId) {
        for (const auto& state : states) {
            if (state.serviceId == serviceId) {
                return &state;
            }
        }

        return nullptr;
    }

    core::DailyServiceState* FindDailyServiceState(
        std::vector<core::DailyServiceState>& states,
        const std::string& serviceId) {
        for (auto& state : states) {
            if (state.serviceId == serviceId) {
                return &state;
            }
        }

        return nullptr;
    }

    const core::DailyServiceState* FindDailyServiceState(
        const std::vector<core::DailyServiceState>& states,
        const std::string& serviceId) {
        for (const auto& state : states) {
            if (state.serviceId == serviceId) {
                return &state;
            }
        }

        return nullptr;
    }
}


GameSession::GameSession()
    : mode_(GameMode::Title),
    gold_(2500),
    regionId_("ashvale_heartland"),
    destinationId_("home_base"),
    activeSlotStackIds_(kActiveSlotCount, ""),
    reserveSlotStackIds_(kReserveSlotCount, "") {}

void GameSession::AdvanceMode() {
    switch (mode_) {
    case GameMode::Title:
        mode_ = GameMode::OpeningSequence;
        break;
    case GameMode::OpeningSequence:
        // M15-c: opening drops straight into Region mode. The World Map is no
        // longer a front-end splash; it is opened on demand from an exit node.
        mode_ = GameMode::RegionMode;
        break;
    case GameMode::CampaignSelectMode:
        // M16-c: campaign selection is driven by the controller, not AdvanceMode.
        break;
    case GameMode::WorldMapMode:
        mode_ = GameMode::RegionMode;
        break;
    case GameMode::RegionMode:
        mode_ = GameMode::LocationMode;
        break;
    case GameMode::LocationMode:
        mode_ = GameMode::BattleMode;
        break;
    case GameMode::BattleMode:
        mode_ = GameMode::RegionMode;
        break;
    case GameMode::ScenarioResultMode:
        // Driven by App's result-screen Continue handler, not the generic
        // mode-advance path. No-op here so it can never advance by accident.
        break;
    case GameMode::OwnedServiceOverviewMode:
        // Transient read-only panel; left explicitly via ExitOwnedServiceOverview-
        // Mode, never through the generic mode-advance path. No-op.
        break;
    }
}

void GameSession::AdvanceClock(const int minutes) {
    const int dayBefore = clock_.Day();
    clock_.AdvanceMinutes(minutes);
    if (clock_.Day() != dayBefore) {
        // Day boundary crossed: refresh the daily Energy pool, then pay owned
        // mines. A multi-day jump fires each once (not per skipped day): Energy
        // is "set to" the formula value, and mine payout is intentionally one
        // payout per detected crossing (the docs say "at the start of each day"
        // but do not require accumulating skipped days; see Phase 3b notes).
        ApplyDailyStartingEnergy();
        ApplyDailyMinePayout();
    }
}

void GameSession::AddMinutes(const int minutes) {
    AdvanceClock(minutes);
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

int GameSession::ResourceCount(const ResourceType type) const {
    if (IsGoldResource(type)) {
        return gold_;
    }
    return nonGoldResources_[NonGoldResourceIndex(type)];
}

void GameSession::AddResource(const ResourceType type, const int amount) {
    if (IsGoldResource(type)) {
        // Single source of truth: Gold lives in gold_, never the pool.
        gold_ = std::max(0, gold_ + amount);
        return;
    }
    int& slot = nonGoldResources_[NonGoldResourceIndex(type)];
    slot = std::max(0, slot + amount);
}

bool GameSession::CanAddResource(const ResourceType type, const int amount) const {
    if (amount < 0) {
        return false;
    }
    const int current = ResourceCount(type);
    return current <= std::numeric_limits<int>::max() - amount;
}

bool GameSession::TrySpendResource(const ResourceType type, const int amount) {
    if (amount <= 0) {
        return true;
    }
    if (IsGoldResource(type)) {
        return TrySpendGold(amount);
    }
    int& slot = nonGoldResources_[NonGoldResourceIndex(type)];
    if (slot < amount) {
        return false;
    }
    slot -= amount;
    return true;
}

const std::vector<core::OwnedServiceSaveState>& GameSession::OwnedServices() const {
    return ownedServices_;
}

const core::OwnedServiceSaveState* GameSession::FindOwnedService(
    const std::string& serviceId) const {
    for (const auto& owned : ownedServices_) {
        if (owned.serviceId == serviceId) {
            return &owned;
        }
    }
    return nullptr;
}

void GameSession::NormalizeStationedUnits() {
    // M17 stationed-ref invariant: every stationed ref must be STACK-BACKED.
    // A ref is valid iff:
    //   * unitId is non-empty; AND
    //   * stackId is non-empty; AND
    //   * stackId resolves to an existing roster stack; AND
    //   * that roster stack's unitId equals the ref's unitId.
    // Anything else (empty stackId, missing stack, mismatched unitId) is stale
    // and dropped. This binds stationing to live roster units and prevents
    // catalog-only unit injection — a unit must actually be in the roster (as a
    // stack) to station, for both heroes and generics. Passive lookup uses
    // unitId, but only after this validation.
    //
    // Build a stackId -> unitId lookup once per pass so each ref is an O(1)
    // check rather than a linear roster scan (keeps Phase 3b payout cheap).
    std::unordered_map<std::string, std::string> stackUnitById;
    stackUnitById.reserve(rosterStacks_.size());
    for (const auto& stack : rosterStacks_) {
        stackUnitById.emplace(stack.stackId, stack.unitId);
    }

    for (auto& owned : ownedServices_) {
        auto& refs = owned.stationedUnits;
        refs.erase(std::remove_if(refs.begin(), refs.end(),
            [&](const core::StationedUnitSaveState& ref) {
                if (ref.unitId.empty() || ref.stackId.empty()) {
                    return true;  // not stack-backed
                }
                const auto it = stackUnitById.find(ref.stackId);
                return it == stackUnitById.end() || it->second != ref.unitId;
            }),
            refs.end());
    }
}

std::vector<const data::UnitDefinition*> GameSession::ResolveStationedUnitDefs(
    const core::OwnedServiceSaveState& owned,
    const std::unordered_map<std::string, std::string>& stackUnitById,
    const std::unordered_map<std::string, const data::UnitDefinition*>& defById) const {
    std::vector<const data::UnitDefinition*> stationedDefs;
    stationedDefs.reserve(owned.stationedUnits.size());
    for (const auto& ref : owned.stationedUnits) {
        if (ref.unitId.empty() || ref.stackId.empty()) {
            continue;  // not stack-backed
        }
        const auto stackIt = stackUnitById.find(ref.stackId);
        if (stackIt == stackUnitById.end() || stackIt->second != ref.unitId) {
            continue;  // stale/mismatched -> never contributes
        }
        const auto defIt = defById.find(ref.unitId);
        if (defIt != defById.end()) {
            stationedDefs.push_back(defIt->second);
        }
    }
    return stationedDefs;
}

std::vector<economy::MineProductionPassive>
GameSession::CollectStationedMineProductionPassives(
    const std::string& serviceId, const data::LocationServiceKind serviceKind) const {
    const auto* owned = FindOwnedService(serviceId);
    if (owned == nullptr || owned->stationedUnits.empty()) {
        return {};
    }

    // Single-service entry point: build the lookups for this one call. The
    // payout pass uses ResolveStationedUnitDefs directly with shared maps so it
    // does not rebuild these per owned service.
    std::unordered_map<std::string, std::string> stackUnitById;
    stackUnitById.reserve(rosterStacks_.size());
    for (const auto& stack : rosterStacks_) {
        stackUnitById.emplace(stack.stackId, stack.unitId);
    }
    std::unordered_map<std::string, const data::UnitDefinition*> defById;
    defById.reserve(unitCatalog_.size());
    for (const auto& unit : unitCatalog_) {
        defById.emplace(unit.id, &unit);
    }

    const auto stationedDefs = ResolveStationedUnitDefs(*owned, stackUnitById, defById);
    return economy::CollectMineProductionPassives(stationedDefs, serviceKind);
}

std::vector<economy::MineResourceOutput>
GameSession::PreviewMineDailyOutput(const std::string& serviceId) const {
    const data::LocationServiceDefinition* def = FindLocationServiceById(serviceId);
    if (def == nullptr || def->kind != data::LocationServiceKind::Mine) {
        return {};
    }

    // Authored base outputs -> typed, mirroring ApplyDailyMinePayout. Unparseable
    // resource names are skipped defensively (validation guarantees valid names for
    // loaded content; a hand-edited save cannot inject an unknown resource).
    std::vector<economy::MineResourceOutput> base;
    base.reserve(def->mineOutputs.size());
    for (const auto& out : def->mineOutputs) {
        ResourceType resource;
        if (TryResourceTypeFromString(out.resource, resource)) {
            base.push_back(economy::MineResourceOutput{resource, out.amount});
        }
    }

    // Same strongest-only combination payout uses. CollectStationedMineProduction-
    // Passives returns empty when the service is not owned or has no stationed
    // units, so an unowned/empty mine previews its base output.
    const auto passives = CollectStationedMineProductionPassives(serviceId, def->kind);
    return economy::ComputeMineDailyOutput(base, passives);
}

bool GameSession::IsStackStationedAnywhere(const std::string& stackId) const {
    if (stackId.empty()) {
        return false;
    }
    for (const auto& owned : ownedServices_) {
        for (const auto& ref : owned.stationedUnits) {
            if (ref.stackId == stackId) {
                return true;
            }
        }
    }
    return false;
}

bool GameSession::IsStackSlotted(const std::string& stackId) const {
    if (stackId.empty()) {
        return false;
    }
    if (std::find(activeSlotStackIds_.begin(), activeSlotStackIds_.end(), stackId) !=
        activeSlotStackIds_.end()) {
        return true;
    }
    return std::find(reserveSlotStackIds_.begin(), reserveSlotStackIds_.end(), stackId) !=
        reserveSlotStackIds_.end();
}

bool GameSession::UnitIsPlayerCharacter(const std::string& unitId) const {
    if (unitId.empty()) {
        return false;
    }
    for (const auto& unit : unitCatalog_) {
        if (unit.id == unitId) {
            return unit.isPlayerCharacter;
        }
    }
    return false;
}

core::OwnedServiceSaveState* GameSession::FindOwnedServiceMutable(
    const std::string& serviceId) {
    if (serviceId.empty()) {
        return nullptr;
    }
    for (auto& owned : ownedServices_) {
        if (owned.serviceId == serviceId) {
            return &owned;
        }
    }
    return nullptr;
}

int GameSession::ActiveOrderedIndexOfStack(const std::string& stackId) const {
    if (stackId.empty()) {
        return -1;
    }
    int orderedIndex = 0;
    for (const auto& slotStackId : activeSlotStackIds_) {
        if (slotStackId.empty()) {
            continue;
        }
        if (slotStackId == stackId) {
            return orderedIndex;
        }
        ++orderedIndex;
    }
    return -1;
}

bool GameSession::CanStationStackAtService(
    const std::string& serviceId, const std::string& stackId) const {
    const core::OwnedServiceSaveState* owned = FindOwnedService(serviceId);
    if (owned == nullptr) {
        return false;
    }
    const data::LocationServiceDefinition* def = FindLocationServiceById(serviceId);
    if (def == nullptr) {
        return false;
    }
    const bool playerOwned =
        !owned->ownerTeamColor.empty() && owned->ownerTeamColor == playerColor_;
    if (!economy::ServiceCanReceiveStationedUnit(
            def->kind, playerOwned, owned->locked, owned->destroyed,
            static_cast<int>(owned->stationedUnits.size()))) {
        return false;
    }

    const RosterStackState* stack = FindStackById(stackId);
    if (stack == nullptr || stack->quantity <= 0) {
        return false;
    }
    if (!economy::StackIsStationable(
            /*stackExists=*/true,
            UnitIsPlayerCharacter(stack->unitId),
            IsStackStationedAnywhere(stackId))) {
        return false;
    }

    // To be stationed, the stack must currently occupy a slot (active or reserve);
    // a slot-less stack is already stationed or orphaned and cannot be moved again.
    const bool slotted =
        ActiveOrderedIndexOfStack(stackId) >= 0 ||
        std::find(reserveSlotStackIds_.begin(), reserveSlotStackIds_.end(), stackId) !=
            reserveSlotStackIds_.end();
    return slotted;
}

bool GameSession::TryStationStackAtService(
    const std::string& serviceId, const std::string& stackId) {
    if (!CanStationStackAtService(serviceId, stackId)) {
        return false;
    }

    core::OwnedServiceSaveState* owned = FindOwnedServiceMutable(serviceId);
    const RosterStackState* stack = FindStackById(stackId);
    if (owned == nullptr || stack == nullptr) {
        return false;  // defensive; CanStation already verified both exist
    }
    const std::string unitId = stack->unitId;

    // Pulling from an active slot is a removal from the active party and must
    // respect the leader guard. The Player Character is already excluded by
    // CanStationStackAtService.
    const int activeOrdered = ActiveOrderedIndexOfStack(stackId);
    if (activeOrdered >= 0) {
        if (WouldRemovingActivePartyUnitLeaveNoLeader(activeOrdered)) {
            return false;
        }
        for (auto& slot : activeSlotStackIds_) {
            if (slot == stackId) {
                slot.clear();
                break;
            }
        }
    } else {
        bool cleared = false;
        for (auto& slot : reserveSlotStackIds_) {
            if (slot == stackId) {
                slot.clear();
                cleared = true;
                break;
            }
        }
        if (!cleared) {
            return false;  // defensive: not slotted
        }
    }

    owned->stationedUnits.push_back(core::StationedUnitSaveState{unitId, stackId});
    MarkRosterProjectionDirty();
    return true;
}

bool GameSession::TryStationSplitAtService(
    const std::string& serviceId, const std::string& stackId, const int quantity) {
    if (quantity <= 0) {
        return false;
    }
    const RosterStackState* stack = FindStackById(stackId);
    if (stack == nullptr) {
        return false;
    }
    // A whole-stack amount is not a split — route to the move path; a larger amount
    // is invalid.
    if (quantity >= stack->quantity) {
        return quantity == stack->quantity
            ? TryStationStackAtService(serviceId, stackId)
            : false;
    }
    if (!economy::IsLegalSplitQuantity(quantity, stack->quantity)) {
        return false;
    }
    // The source stack stays slotted; we station a NEW stack. Static eligibility
    // (service capacity/ownership, source not PC, source not already stationed,
    // source slotted) is identical to a whole-stack station, so reuse the gate.
    if (!CanStationStackAtService(serviceId, stackId)) {
        return false;
    }

    core::OwnedServiceSaveState* owned = FindOwnedServiceMutable(serviceId);
    RosterStackState* source = FindStackById(stackId);
    if (owned == nullptr || source == nullptr || quantity >= source->quantity) {
        return false;  // defensive
    }

    const std::string unitId = source->unitId;
    source->quantity -= quantity;  // read unitId and decrement before push_back may
    const std::string newStackId = GenerateNextStackId();  // invalidate `source`
    rosterStacks_.push_back(RosterStackState{newStackId, unitId, quantity});
    owned->stationedUnits.push_back(core::StationedUnitSaveState{unitId, newStackId});
    MarkRosterProjectionDirty();
    return true;
}

bool GameSession::TryUnstationStackFromService(
    const std::string& serviceId, const std::string& stackId) {
    if (stackId.empty()) {
        return false;
    }
    core::OwnedServiceSaveState* owned = FindOwnedServiceMutable(serviceId);
    if (owned == nullptr) {
        return false;
    }
    auto& refs = owned->stationedUnits;
    const auto refIt = std::find_if(refs.begin(), refs.end(),
        [&](const core::StationedUnitSaveState& ref) {
            return ref.stackId == stackId;
        });
    if (refIt == refs.end()) {
        return false;
    }

    // Heal the legacy/permissive double-placement case first. NormalizeStationed-
    // Units stays permissive, so old/injected save-data can leave a stack BOTH
    // slotted (active or reserve) AND stationed. Returning it to reserve again
    // would duplicate the stack id across two placements. Instead, remove only the
    // stationed ref and leave the existing slot placement untouched.
    if (IsStackSlotted(stackId)) {
        refs.erase(refIt);
        MarkRosterProjectionDirty();
        return true;
    }

    // From here the stack is slot-less (the normal stationed state). Without a live
    // roster stack there is nothing to return; never recreate a unit (a stale ref
    // should already have been dropped by NormalizeStationedUnits).
    if (FindStackById(stackId) == nullptr) {
        return false;
    }

    // Return the SAME stack id to a free reserve slot — never recreated or merged.
    const int reserveSlot = FindFirstEmptySlotIndex(reserveSlotStackIds_);
    if (reserveSlot < 0) {
        return false;  // atomic fail: no free reserve slot, stack stays stationed
    }
    reserveSlotStackIds_[reserveSlot] = stackId;
    refs.erase(refIt);
    MarkRosterProjectionDirty();
    return true;
}

std::vector<std::string> GameSession::EligibleStationingStackIds(
    const std::string& serviceId) const {
    const core::OwnedServiceSaveState* owned = FindOwnedService(serviceId);
    const data::LocationServiceDefinition* def = FindLocationServiceById(serviceId);
    if (owned == nullptr || def == nullptr) {
        return {};
    }
    const bool playerOwned =
        !owned->ownerTeamColor.empty() && owned->ownerTeamColor == playerColor_;
    if (!economy::ServiceCanReceiveStationedUnit(
            def->kind, playerOwned, owned->locked, owned->destroyed,
            static_cast<int>(owned->stationedUnits.size()))) {
        return {};
    }

    std::vector<std::string> result;
    auto consider = [&](const std::string& slotStackId) {
        if (slotStackId.empty()) {
            return;
        }
        const RosterStackState* stack = FindStackById(slotStackId);
        if (stack == nullptr || stack->quantity <= 0) {
            return;
        }
        if (UnitIsPlayerCharacter(stack->unitId) || IsStackStationedAnywhere(slotStackId)) {
            return;
        }
        result.push_back(slotStackId);
    };
    for (const auto& slotStackId : activeSlotStackIds_) {
        consider(slotStackId);
    }
    for (const auto& slotStackId : reserveSlotStackIds_) {
        consider(slotStackId);
    }
    return result;
}

bool GameSession::CanOpenStationingAtMine(const std::string& serviceId) const {
    const core::OwnedServiceSaveState* owned = FindOwnedService(serviceId);
    const data::LocationServiceDefinition* def = FindLocationServiceById(serviceId);
    if (owned == nullptr || def == nullptr) {
        return false;
    }
    if (def->kind != data::LocationServiceKind::Mine) {
        return false;
    }
    const bool playerOwned =
        !owned->ownerTeamColor.empty() && owned->ownerTeamColor == playerColor_;
    return playerOwned && !owned->locked && !owned->destroyed;
}

// ===========================================================================
// M28 storage placement. A parallel bucket to M25 stationing: a stack lives in
// exactly one place (active slot, reserve slot, stationed at one service, or
// stored at one service). Storage is a DISTINCT concept (cap 7, persistence /
// retrieval) and never merges with mine stationing. Cross-exclusion is automatic
// because both store and station require the stack to be currently slotted, and a
// stored/stationed stack is slot-less.
// ===========================================================================

void GameSession::NormalizeStoredUnits() {
    // Same stack-backed invariant as NormalizeStationedUnits: drop any stored ref
    // that is empty, points to a missing roster stack, or mismatches that stack's
    // unitId. Keeps storage bound to live roster units.
    std::unordered_map<std::string, std::string> stackUnitById;
    stackUnitById.reserve(rosterStacks_.size());
    for (const auto& stack : rosterStacks_) {
        stackUnitById.emplace(stack.stackId, stack.unitId);
    }

    for (auto& owned : ownedServices_) {
        auto& refs = owned.storedUnits;
        refs.erase(std::remove_if(refs.begin(), refs.end(),
            [&](const core::StoredUnitSaveState& ref) {
                if (ref.unitId.empty() || ref.stackId.empty()) {
                    return true;
                }
                const auto it = stackUnitById.find(ref.stackId);
                return it == stackUnitById.end() || it->second != ref.unitId;
            }),
            refs.end());
    }
}

bool GameSession::IsStackStoredAnywhere(const std::string& stackId) const {
    if (stackId.empty()) {
        return false;
    }
    for (const auto& owned : ownedServices_) {
        for (const auto& ref : owned.storedUnits) {
            if (ref.stackId == stackId) {
                return true;
            }
        }
    }
    return false;
}

bool GameSession::CanStoreStackAtService(
    const std::string& serviceId, const std::string& stackId) const {
    const core::OwnedServiceSaveState* owned = FindOwnedService(serviceId);
    if (owned == nullptr) {
        return false;
    }
    const data::LocationServiceDefinition* def = FindLocationServiceById(serviceId);
    if (def == nullptr) {
        return false;
    }
    const bool playerOwned =
        !owned->ownerTeamColor.empty() && owned->ownerTeamColor == playerColor_;
    if (!economy::ServiceCanReceiveStoredUnit(
            def->kind, playerOwned, owned->locked, owned->destroyed,
            static_cast<int>(owned->storedUnits.size()))) {
        return false;
    }

    const RosterStackState* stack = FindStackById(stackId);
    if (stack == nullptr || stack->quantity <= 0) {
        return false;
    }
    // Not the Player Character, and not already placed (stationed or stored) — the
    // explicit cross-exclusion. The slotted check below is the structural one.
    if (!economy::StackIsStorable(
            /*stackExists=*/true,
            UnitIsPlayerCharacter(stack->unitId),
            IsStackStationedAnywhere(stackId) || IsStackStoredAnywhere(stackId))) {
        return false;
    }

    // To be stored, the stack must currently occupy a slot (active or reserve); a
    // slot-less stack is already placed (stationed/stored) or orphaned and cannot be
    // moved again. This is what makes a stationed stack unstorable automatically.
    return IsStackSlotted(stackId);
}

bool GameSession::TryStoreStackAtService(
    const std::string& serviceId, const std::string& stackId) {
    if (!CanStoreStackAtService(serviceId, stackId)) {
        return false;
    }

    core::OwnedServiceSaveState* owned = FindOwnedServiceMutable(serviceId);
    const RosterStackState* stack = FindStackById(stackId);
    if (owned == nullptr || stack == nullptr) {
        return false;  // defensive; CanStore already verified both exist
    }
    const std::string unitId = stack->unitId;

    // Pulling from an active slot is a removal from the active party and must
    // respect the leader guard. The Player Character is already excluded by
    // CanStoreStackAtService.
    const int activeOrdered = ActiveOrderedIndexOfStack(stackId);
    if (activeOrdered >= 0) {
        if (WouldRemovingActivePartyUnitLeaveNoLeader(activeOrdered)) {
            return false;
        }
        for (auto& slot : activeSlotStackIds_) {
            if (slot == stackId) {
                slot.clear();
                break;
            }
        }
    } else {
        bool cleared = false;
        for (auto& slot : reserveSlotStackIds_) {
            if (slot == stackId) {
                slot.clear();
                cleared = true;
                break;
            }
        }
        if (!cleared) {
            return false;  // defensive: not slotted
        }
    }

    owned->storedUnits.push_back(core::StoredUnitSaveState{unitId, stackId});
    MarkRosterProjectionDirty();
    return true;
}

bool GameSession::TryRetrieveStackFromService(
    const std::string& serviceId, const std::string& stackId) {
    if (stackId.empty()) {
        return false;
    }
    core::OwnedServiceSaveState* owned = FindOwnedServiceMutable(serviceId);
    if (owned == nullptr) {
        return false;
    }
    auto& refs = owned->storedUnits;
    const auto refIt = std::find_if(refs.begin(), refs.end(),
        [&](const core::StoredUnitSaveState& ref) {
            return ref.stackId == stackId;
        });
    if (refIt == refs.end()) {
        return false;
    }

    // Heal a corrupt/legacy double-placement: a stack that is BOTH slotted and
    // stored (e.g. injected save-data). Returning it to reserve again would
    // duplicate the stack id. Remove only the stored ref; leave the slot untouched.
    if (IsStackSlotted(stackId)) {
        refs.erase(refIt);
        MarkRosterProjectionDirty();
        return true;
    }

    // Slot-less (normal stored state). Without a live roster stack there is nothing
    // to return; never recreate a unit (NormalizeStoredUnits drops stale refs).
    if (FindStackById(stackId) == nullptr) {
        return false;
    }

    // Return the SAME stack id to a free reserve slot — never recreated or merged.
    const int reserveSlot = FindFirstEmptySlotIndex(reserveSlotStackIds_);
    if (reserveSlot < 0) {
        return false;  // atomic fail: no free reserve slot, stack stays stored
    }
    reserveSlotStackIds_[reserveSlot] = stackId;
    refs.erase(refIt);
    MarkRosterProjectionDirty();
    return true;
}

std::vector<std::string> GameSession::EligibleStorageStackIds(
    const std::string& serviceId) const {
    const core::OwnedServiceSaveState* owned = FindOwnedService(serviceId);
    const data::LocationServiceDefinition* def = FindLocationServiceById(serviceId);
    if (owned == nullptr || def == nullptr) {
        return {};
    }
    const bool playerOwned =
        !owned->ownerTeamColor.empty() && owned->ownerTeamColor == playerColor_;
    if (!economy::ServiceCanReceiveStoredUnit(
            def->kind, playerOwned, owned->locked, owned->destroyed,
            static_cast<int>(owned->storedUnits.size()))) {
        return {};
    }

    std::vector<std::string> result;
    auto consider = [&](const std::string& slotStackId) {
        if (slotStackId.empty()) {
            return;
        }
        const RosterStackState* stack = FindStackById(slotStackId);
        if (stack == nullptr || stack->quantity <= 0) {
            return;
        }
        if (UnitIsPlayerCharacter(stack->unitId) ||
            IsStackStationedAnywhere(slotStackId) || IsStackStoredAnywhere(slotStackId)) {
            return;
        }
        result.push_back(slotStackId);
    };
    for (const auto& slotStackId : activeSlotStackIds_) {
        consider(slotStackId);
    }
    for (const auto& slotStackId : reserveSlotStackIds_) {
        consider(slotStackId);
    }
    return result;
}

bool GameSession::CanOpenStorageAtService(const std::string& serviceId) const {
    const core::OwnedServiceSaveState* owned = FindOwnedService(serviceId);
    const data::LocationServiceDefinition* def = FindLocationServiceById(serviceId);
    if (owned == nullptr || def == nullptr) {
        return false;
    }
    if (def->kind != data::LocationServiceKind::Storage) {
        return false;
    }
    const bool playerOwned =
        !owned->ownerTeamColor.empty() && owned->ownerTeamColor == playerColor_;
    return playerOwned && !owned->locked && !owned->destroyed;
}

void GameSession::ApplyDailyMinePayout() {
    if (ownedServices_.empty() || locationServiceCatalog_.empty()) {
        return;
    }

    // Build all lookups once per payout pass (not per owned service).
    std::unordered_map<std::string, const data::LocationServiceDefinition*> serviceById;
    serviceById.reserve(locationServiceCatalog_.size());
    for (const auto& svc : locationServiceCatalog_) {
        serviceById.emplace(svc.id, &svc);
    }
    std::unordered_map<std::string, std::string> stackUnitById;
    stackUnitById.reserve(rosterStacks_.size());
    for (const auto& stack : rosterStacks_) {
        stackUnitById.emplace(stack.stackId, stack.unitId);
    }
    std::unordered_map<std::string, const data::UnitDefinition*> defById;
    defById.reserve(unitCatalog_.size());
    for (const auto& unit : unitCatalog_) {
        defById.emplace(unit.id, &unit);
    }
    const auto hostileVec = HostileOccupiedNodeIds(playerColor_);
    const std::set<std::string> hostileNodes(hostileVec.begin(), hostileVec.end());

    for (const auto& owned : ownedServices_) {
        const auto svcIt = serviceById.find(owned.serviceId);
        if (svcIt == serviceById.end()) {
            continue;
        }
        const auto* def = svcIt->second;

        const bool hostileOccupied = hostileNodes.count(def->locationId) != 0;
        if (!economy::MineServiceIsPayable(def->kind, !def->mineOutputs.empty(),
                owned.ownerTeamColor, owned.locked, owned.destroyed,
                playerColor_, hostileOccupied)) {
            continue;
        }

        // Authored base outputs -> typed outputs. Validation guarantees valid
        // resource names for loaded content; any that fail to parse are skipped
        // defensively so a hand-edited save cannot inject an unknown resource.
        std::vector<economy::MineResourceOutput> base;
        base.reserve(def->mineOutputs.size());
        for (const auto& out : def->mineOutputs) {
            ResourceType resource;
            if (TryResourceTypeFromString(out.resource, resource)) {
                base.push_back(economy::MineResourceOutput{resource, out.amount});
            }
        }

        const auto stationedDefs = ResolveStationedUnitDefs(owned, stackUnitById, defById);
        const auto passives = economy::CollectMineProductionPassives(stationedDefs, def->kind);
        const auto outputs = economy::ComputeMineDailyOutput(base, passives);

        for (const auto& line : outputs) {
            // Gold routes to gold_ via the Phase 1 delegation; non-gold to pool.
            AddResource(line.resource, line.amount);
        }
    }
}

namespace {

// Builds the per-type tier candidates from owned services, using a prebuilt
// service-id lookup and hostile-node set so there is no nested rescan.
std::vector<economy::OwnedServiceTierCandidate> BuildTraderTierCandidates(
    const std::vector<core::OwnedServiceSaveState>& ownedServices,
    const std::unordered_map<std::string, const data::LocationServiceDefinition*>& serviceById,
    const std::set<std::string>& hostileNodes) {
    std::vector<economy::OwnedServiceTierCandidate> candidates;
    candidates.reserve(ownedServices.size());
    for (const auto& owned : ownedServices) {
        const auto it = serviceById.find(owned.serviceId);
        if (it == serviceById.end()) {
            continue;
        }
        const auto* def = it->second;
        candidates.push_back(economy::OwnedServiceTierCandidate{
            def->kind,
            owned.ownerTeamColor,
            owned.locked,
            owned.destroyed,
            hostileNodes.count(def->locationId) != 0});
    }
    return candidates;
}

} // namespace

int GameSession::OwnedTraderServiceTierForService(const std::string& serviceId) const {
    if (locationServiceCatalog_.empty()) {
        return 0;
    }

    // One service-id lookup, reused for the exact-service gate and the per-type
    // count below (no nested rescans).
    std::unordered_map<std::string, const data::LocationServiceDefinition*> serviceById;
    serviceById.reserve(locationServiceCatalog_.size());
    for (const auto& svc : locationServiceCatalog_) {
        serviceById.emplace(svc.id, &svc);
    }

    const auto defIt = serviceById.find(serviceId);
    if (defIt == serviceById.end()) {
        return 0;  // unknown service id
    }
    const auto* def = defIt->second;
    if (!data::IsTraderServiceKind(def->kind)) {
        return 0;  // not a trader service
    }

    const auto* owned = FindOwnedService(serviceId);
    if (owned == nullptr) {
        return 0;  // no owned-service runtime state for this id
    }
    if (owned->ownerTeamColor.empty() || owned->ownerTeamColor != playerColor_) {
        return 0;  // not owned by the player team
    }
    if (owned->locked || owned->destroyed) {
        return 0;
    }

    const auto hostileVec = HostileOccupiedNodeIds(playerColor_);
    const std::set<std::string> hostileNodes(hostileVec.begin(), hostileVec.end());
    if (hostileNodes.count(def->locationId) != 0) {
        return 0;  // the used service is hostile-occupied
    }

    // The used service is eligible: return the player's tier for its own type.
    const auto candidates = BuildTraderTierCandidates(ownedServices_, serviceById, hostileNodes);
    return economy::CountOwnedServiceTier(candidates, def->kind, playerColor_);
}

int GameSession::OwnedTraderServiceTier(const data::LocationServiceKind traderKind) const {
    if (ownedServices_.empty() || locationServiceCatalog_.empty()) {
        return 0;
    }

    std::unordered_map<std::string, const data::LocationServiceDefinition*> serviceById;
    serviceById.reserve(locationServiceCatalog_.size());
    for (const auto& svc : locationServiceCatalog_) {
        serviceById.emplace(svc.id, &svc);
    }
    const auto hostileVec = HostileOccupiedNodeIds(playerColor_);
    const std::set<std::string> hostileNodes(hostileVec.begin(), hostileVec.end());

    const auto candidates = BuildTraderTierCandidates(ownedServices_, serviceById, hostileNodes);
    return economy::CountOwnedServiceTier(candidates, traderKind, playerColor_);
}

const data::LocationServiceDefinition* GameSession::FindLocationServiceById(
    const std::string& serviceId) const {
    for (const auto& svc : locationServiceCatalog_) {
        if (svc.id == serviceId) {
            return &svc;
        }
    }
    return nullptr;
}

const data::TraderOwnershipCurve* GameSession::FindTraderCurve(
    const data::LocationServiceKind kind) const {
    for (const auto& curve : traderCurveCatalog_) {
        if (curve.kind == kind) {
            return &curve;
        }
    }
    return nullptr;
}

GameSession::TradingPostUseGate GameSession::GateTradingPostUse(
    const std::string& serviceId) const {
    TradingPostUseGate gate;
    gate.service = FindLocationServiceById(serviceId);
    if (gate.service == nullptr ||
        gate.service->kind != data::LocationServiceKind::TradingPost) {
        return gate;  // unknown id or non-Trading-Post service
    }
    gate.isTradingPost = true;

    // Lock/destruction come from owned-service runtime state (absent => not
    // locked/destroyed for an available, unowned post). Hostile occupation is
    // independent of ownership: an enemy holding the node blocks use regardless.
    const auto* owned = FindOwnedService(serviceId);
    const bool locked = owned != nullptr && owned->locked;
    const bool destroyed = owned != nullptr && owned->destroyed;
    const auto hostileVec = HostileOccupiedNodeIds(playerColor_);
    const bool hostileOccupied =
        std::find(hostileVec.begin(), hostileVec.end(), gate.service->locationId) !=
        hostileVec.end();
    gate.usable = economy::TradingPostUsable(locked, destroyed, hostileOccupied);
    if (!gate.usable) {
        return gate;
    }

    // Effective tier: 0 for a usable but unowned/ineligible post; the per-type
    // ownership tier only when the exact service is player-owned and eligible.
    gate.effectiveTier = OwnedTraderServiceTierForService(serviceId);
    return gate;
}

TradeResult GameSession::TryTradingPostBarter(
    const std::string& serviceId, ResourceType from, ResourceType to, int quantity) {
    const auto gate = GateTradingPostUse(serviceId);
    if (gate.service == nullptr) {
        return {false, "Unknown service"};
    }
    if (!gate.isTradingPost) {
        return {false, "Service is not a Trading Post"};
    }
    if (!gate.usable) {
        return {false, "Trading Post is not available"};
    }

    const auto* curve = FindTraderCurve(data::LocationServiceKind::TradingPost);
    const auto matrix = economy::ResolveTradingPostBarter(curve, gate.effectiveTier);
    const auto quote = economy::QuoteBarter(matrix, from, to, quantity);
    if (!quote.valid) {
        return {false, "This barter is not offered here"};
    }
    // Atomic: confirm the grant cannot overflow before spending, then spend
    // (guarded) and grant. Nothing is mutated unless the whole trade succeeds.
    if (!CanAddResource(to, quantity)) {
        return {false, "Trade would exceed resource capacity"};
    }
    if (!TrySpendResource(from, quote.fromCost)) {
        return {false, "Not enough resources to trade"};
    }
    AddResource(to, quantity);
    return {true, "Trade complete"};
}

TradeResult GameSession::TryTradingPostBuyForGold(
    const std::string& serviceId, ResourceType resource, int quantity) {
    const auto gate = GateTradingPostUse(serviceId);
    if (gate.service == nullptr) {
        return {false, "Unknown service"};
    }
    if (!gate.isTradingPost) {
        return {false, "Service is not a Trading Post"};
    }
    if (!gate.usable) {
        return {false, "Trading Post is not available"};
    }

    const auto* curve = FindTraderCurve(data::LocationServiceKind::TradingPost);
    const int priceFactor = economy::ResolvePriceFactor(curve, gate.effectiveTier);
    const auto quote = economy::QuoteBuyResourceForGold(resource, quantity, priceFactor);
    if (!quote.valid) {
        return {false, "This purchase is not offered here"};
    }
    if (!CanAddResource(resource, quantity)) {
        return {false, "Trade would exceed resource capacity"};
    }
    if (!TrySpendGold(quote.goldAmount)) {
        return {false, "Not enough gold to buy"};
    }
    AddResource(resource, quantity);
    return {true, "Trade complete"};
}

TradeResult GameSession::TryTradingPostSellForGold(
    const std::string& serviceId, ResourceType resource, int quantity) {
    const auto gate = GateTradingPostUse(serviceId);
    if (gate.service == nullptr) {
        return {false, "Unknown service"};
    }
    if (!gate.isTradingPost) {
        return {false, "Service is not a Trading Post"};
    }
    if (!gate.usable) {
        return {false, "Trading Post is not available"};
    }

    const auto* curve = FindTraderCurve(data::LocationServiceKind::TradingPost);
    const int priceFactor = economy::ResolvePriceFactor(curve, gate.effectiveTier);
    const auto quote = economy::QuoteSellResourceForGold(resource, quantity, priceFactor);
    if (!quote.valid) {
        return {false, "This sale is not offered here"};
    }
    if (quote.goldAmount <= 0) {
        return {false, "Sale would yield no gold"};  // never trade resources for nothing
    }
    if (!CanAddResource(ResourceType::Gold, quote.goldAmount)) {
        return {false, "Trade would exceed resource capacity"};
    }
    if (!TrySpendResource(resource, quantity)) {
        return {false, "Not enough resources to sell"};
    }
    AddResource(ResourceType::Gold, quote.goldAmount);
    return {true, "Trade complete"};
}

TradingPostOffer GameSession::ResolveTradingPostOffer(const std::string& serviceId) const {
    TradingPostOffer offer;
    const auto gate = GateTradingPostUse(serviceId);
    if (!gate.isTradingPost || !gate.usable) {
        return offer;  // unknown / non-Trading-Post / locked / destroyed / occupied
    }
    offer.usable = true;
    offer.effectiveTier = gate.effectiveTier;
    const auto* curve = FindTraderCurve(data::LocationServiceKind::TradingPost);
    offer.barter = economy::ResolveTradingPostBarter(curve, gate.effectiveTier);
    offer.priceFactor = economy::ResolvePriceFactor(curve, gate.effectiveTier);
    return offer;
}

void GameSession::SeedEventResourceContext(events::EventEvaluationContext& ctx) const {
    ctx.resources[ResourceTypeToString(ResourceType::Gold)] = gold_;
    for (const auto type : kNonGoldResourceTypes) {
        ctx.resources[ResourceTypeToString(type)] =
            nonGoldResources_[NonGoldResourceIndex(type)];
    }
}

void GameSession::ApplyEventResourceContext(const events::EventEvaluationContext& ctx) {
    const auto applyOne = [&](ResourceType type) {
        const auto it = ctx.resources.find(ResourceTypeToString(type));
        if (it == ctx.resources.end()) {
            return;
        }
        const int value = std::max(0, it->second);
        if (IsGoldResource(type)) {
            gold_ = value;  // single source of truth
        } else {
            nonGoldResources_[NonGoldResourceIndex(type)] = value;
        }
    };
    applyOne(ResourceType::Gold);
    for (const auto type : kNonGoldResourceTypes) {
        applyOne(type);
    }
}

void GameSession::EnterLocationMode(const std::string& locationId) {
    destinationId_ = locationId;
    mode_ = GameMode::LocationMode;
}

void GameSession::EnterRegionMode() {
    mode_ = GameMode::RegionMode;
}

void GameSession::EnterWorldMapMode() {
    mode_ = GameMode::WorldMapMode;
}

void GameSession::EnterCampaignSelectMode() {
    mode_ = GameMode::CampaignSelectMode;
}

void GameSession::EnterTitleMode() {
    mode_ = GameMode::Title;
}

void GameSession::EnterScenarioResultMode() {
    mode_ = GameMode::ScenarioResultMode;
}

void GameSession::EnterOwnedServiceOverviewMode() {
    mode_ = GameMode::OwnedServiceOverviewMode;
}

void GameSession::ExitOwnedServiceOverviewMode() {
    mode_ = GameMode::RegionMode;
}

void GameSession::ExitLocationMode() {
    EnterRegionMode();
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

void GameSession::RestToNextDayStart() {
    const int remainingMinutes = core::GameClock::kMinutesPerSliceDay - clock_.MinutesIntoSliceDay();
    AdvanceClock(std::max(1, remainingMinutes));
}

int GameSession::CurrentWeek() const {
    return ((clock_.Day() - 1) / 7) + 1;
}

void GameSession::RefreshWeeklyRecruitStocks(const std::vector<data::LocationServiceDefinition>& services) {
    const int currentWeek = CurrentWeek();

    for (const auto& service : services) {
        if (service.kind != data::LocationServiceKind::Recruit || service.weeklyStock <= 0) {
            continue;
        }

        auto* state = FindRecruitServiceState(recruitServiceStates_, service.id);
        if (state == nullptr) {
            recruitServiceStates_.push_back(core::RecruitServiceState{
                service.id,
                service.weeklyStock,
                currentWeek
                });
            continue;
        }

        if (state->lastRefreshWeek < currentWeek) {
            state->remainingStock = service.weeklyStock;
            state->lastRefreshWeek = currentWeek;
        }
    }
}

int GameSession::RemainingRecruitStock(const std::string& serviceId, const int defaultStock) const {
    const auto* state = FindRecruitServiceState(recruitServiceStates_, serviceId);
    if (state == nullptr) {
        return std::max(0, defaultStock);
    }

    return std::max(0, state->remainingStock);
}

bool GameSession::TryConsumeRecruitStock(const std::string& serviceId, const int defaultStock) {
    auto* state = FindRecruitServiceState(recruitServiceStates_, serviceId);
    if (state == nullptr) {
        recruitServiceStates_.push_back(core::RecruitServiceState{
            serviceId,
            std::max(0, defaultStock),
            CurrentWeek()
            });
        state = &recruitServiceStates_.back();
    }

    if (state->remainingStock <= 0) {
        return false;
    }

    --state->remainingStock;
    return true;
}

void GameSession::RefreshDailyServiceUses(const std::vector<data::LocationServiceDefinition>& services) {
    const int currentDay = clock_.Day();

    if (travelPrepRemainingCharges_ > 0 && travelPrepGrantedDay_ < currentDay) {
        travelPrepRemainingCharges_ = 0;
        travelPrepDiscountMinutes_ = 0;
        travelPrepGrantedDay_ = 0;
    }

    for (const auto& service : services) {
        if (service.dailyUseLimit <= 0) {
            continue;
        }

        auto* state = FindDailyServiceState(dailyServiceStates_, service.id);
        if (state == nullptr) {
            dailyServiceStates_.push_back(core::DailyServiceState{
                service.id,
                service.dailyUseLimit,
                currentDay
                });
            continue;
        }

        if (state->lastRefreshDay < currentDay) {
            state->remainingUsesToday = service.dailyUseLimit;
            state->lastRefreshDay = currentDay;
        }
    }
}

int GameSession::RemainingDailyServiceUses(const std::string& serviceId, const int defaultUsesPerDay) const {
    const auto* state = FindDailyServiceState(dailyServiceStates_, serviceId);
    if (state == nullptr) {
        return std::max(0, defaultUsesPerDay);
    }

    return std::max(0, state->remainingUsesToday);
}

bool GameSession::TryConsumeDailyServiceUse(const std::string& serviceId, const int defaultUsesPerDay) {
    auto* state = FindDailyServiceState(dailyServiceStates_, serviceId);
    if (state == nullptr) {
        dailyServiceStates_.push_back(core::DailyServiceState{
            serviceId,
            std::max(0, defaultUsesPerDay),
            clock_.Day()
            });
        state = &dailyServiceStates_.back();
    }

    if (state->remainingUsesToday <= 0) {
        return false;
    }

    --state->remainingUsesToday;
    return true;
}

void GameSession::GrantSameDayTravelPrep(const int discountMinutes, const int charges) {
    if (discountMinutes <= 0 || charges <= 0) {
        return;
    }

    travelPrepDiscountMinutes_ = std::max(travelPrepDiscountMinutes_, discountMinutes);
    travelPrepRemainingCharges_ += charges;
    travelPrepGrantedDay_ = clock_.Day();
}

bool GameSession::HasActiveSameDayTravelPrep() const {
    return travelPrepRemainingCharges_ > 0 && travelPrepGrantedDay_ == clock_.Day();
}

int GameSession::ActiveSameDayTravelPrepDiscountMinutes() const {
    if (!HasActiveSameDayTravelPrep()) {
        return 0;
    }

    return std::max(0, travelPrepDiscountMinutes_);
}

int GameSession::PreviewSameDayTravelPrepToTravelMinutes(const int baseTravelMinutes) const {
    if (baseTravelMinutes <= 0) {
        return 0;
    }

    if (!HasActiveSameDayTravelPrep()) {
        return baseTravelMinutes;
    }

    return std::max(1, baseTravelMinutes - std::max(0, travelPrepDiscountMinutes_));
}

int GameSession::ApplySameDayTravelPrepToTravelMinutes(const int baseTravelMinutes) {
    if (baseTravelMinutes <= 0) {
        return 0;
    }

    if (!HasActiveSameDayTravelPrep()) {
        return baseTravelMinutes;
    }

    --travelPrepRemainingCharges_;
    return std::max(1, baseTravelMinutes - std::max(0, travelPrepDiscountMinutes_));
}

void GameSession::ApplyWakePenalty() {
    gold_ = std::max(0, gold_ - 1000);
    // Advancing a full slice day rolls the day forward, which refreshes Energy
    // via the chokepoint ("wake next day with fresh Energy"). SetToWakePenaltyStart
    // only repositions the time within the new day; it does not change the day.
    AdvanceClock(core::GameClock::kMinutesPerSliceDay);
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

void GameSession::ApplyRegionCombatVictoryNodeClear(
    const bool alliesWon,
    const bool enemiesWon,
    const GameMode battleReturnMode,
    const std::string& nodeId,
    const bool nodeIsCombatType) {
    if (!alliesWon || enemiesWon) {
        return;
    }

    if (battleReturnMode != GameMode::RegionMode) {
        return;
    }

    if (!nodeIsCombatType) {
        return;
    }

    MarkCombatNodeCleared(nodeId);
}

void GameSession::InitializeEventDefinitions(std::vector<events::EventDefinition> definitions) {
    eventDefinitions_ = std::move(definitions);
}

std::vector<events::ActionResult> GameSession::FireMatchingEvents(
    const std::function<bool(const events::EventDefinition&)>& matches)
{
    std::vector<events::ActionResult> allResults;
    std::vector<events::EnemyTeamMutation> pendingMutations;

    auto sorted = eventDefinitions_;
    std::stable_sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) {
        if (a.priority.has_value() && b.priority.has_value())
            return *a.priority < *b.priority;
        return a.priority.has_value();
    });

    for (const auto& def : sorted) {
        if (!matches(def)) continue;

        const bool alreadyFired = std::ranges::any_of(firedEventIds_,
            [&](const auto& id) { return id == def.id; });
        if (alreadyFired && def.repeat.mode == "once") continue;

        events::EventEvaluationContext ctx;
        const auto snap = Snapshot();
        ctx.currentDay = snap.day;
        SeedEventResourceContext(ctx);
        const auto& partyIds = ActivePartyUnitIds();
        ctx.heroIds = std::vector<std::string>(partyIds.begin(), partyIds.end());
        ctx.storyFlags = storyFlags_;
        ctx.pendingTeamMutations = &pendingMutations;
        // M13-b: wire inventory-action surface. Actions mutate items_ and
        // artifacts_ directly through these pointers; catalogs supply
        // authored metadata for id lookups + consumable / stackCap rules.
        ctx.items           = &items_;
        ctx.artifacts       = &artifacts_;
        ctx.itemCatalog     = &itemCatalog_;
        ctx.artifactCatalog = &artifactCatalog_;

        if (!events::EvaluateCondition(ctx, def.condition)) continue;

        auto results = events::ExecuteActions(ctx, def.actions);
        allResults.insert(allResults.end(), results.begin(), results.end());

        ApplyEventResourceContext(ctx);
        storyFlags_ = ctx.storyFlags;

        if (def.repeat.mode == "once") {
            firedEventIds_.push_back(def.id);
        }
    }

    for (const auto& mut : pendingMutations) {
        EnemyTeamState* match = nullptr;
        for (auto& team : enemyTeams_) {
            if (team.teamColor == mut.teamColor) {
                match = &team;
                break;
            }
        }

        if (match != nullptr) {
            if (mut.type == events::EnemyTeamMutationType::Spawn) {
                match->nodeId = mut.nodeId;
                match->active = true;
            } else if (mut.type == events::EnemyTeamMutationType::Remove) {
                match->active = false;
            } else if (mut.type == events::EnemyTeamMutationType::ChangeAlliance) {
                if (mut.addAlliance) {
                    if (std::ranges::find(match->alliances, mut.allyColor) == match->alliances.end())
                        match->alliances.push_back(mut.allyColor);
                } else {
                    std::erase(match->alliances, mut.allyColor);
                }
            }
        } else if (mut.type == events::EnemyTeamMutationType::Spawn) {
            // No team of this color exists yet: a spawnTeam action creates one so
            // a node can be guarded without any pre-seeded roster of teams. Only
            // Spawn creates; Remove/ChangeAlliance on an unknown color are no-ops.
            // A later spawnTeam of the same color matches above (no duplicates).
            EnemyTeamState created;
            created.teamColor = mut.teamColor;
            created.nodeId = mut.nodeId;
            created.active = true;
            enemyTeams_.push_back(created);
        }
    }

    // Any authored regionNodeEntry / startOfDay event may have spawned/removed
    // teams, changed alliances, or set story flags that now satisfy victory or
    // defeat. Latch before the caller proceeds to the enemy phase.
    CheckAndLatchOutcome();

    return allResults;
}

std::vector<events::ActionResult> GameSession::NotifyStartOfDay() {
    return FireMatchingEvents([](const events::EventDefinition& def) {
        return def.trigger.type == events::EventTriggerType::StartOfDay;
    });
}

std::vector<events::ActionResult> GameSession::NotifyRegionNodeEntry(const std::string& nodeId) {
    return FireMatchingEvents([&nodeId](const events::EventDefinition& def) {
        return def.trigger.type == events::EventTriggerType::RegionNodeEntry
            && def.trigger.targetId == nodeId;
    });
}

void GameSession::InitializeQuestState(const std::vector<data::QuestDefinition>& questDefinitions) {
    questDefinitionSeed_ = questDefinitions;   // M16-b: kept for scenario-transition reset
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

int GameSession::ActivePartyCapacity() const {
    return std::max(0, activePartyCapacity_);
}

void GameSession::SetLeaderCapableUnitIds(std::vector<std::string> unitIds) {
    std::set<std::string> normalized;
    for (auto& unitId : unitIds) {
        if (!unitId.empty()) {
            normalized.insert(unitId);
        }
    }

    leaderCapableUnitIds_ = std::move(normalized);
}

int GameSession::OwnedUnitCount(const std::string& unitId) const {
    if (unitId.empty()) {
        return 0;
    }

    RebuildRosterProjectionCache();
    for (const auto& entry : ownedUnitCountProjection_) {
        if (entry.unitId == unitId) {
            return std::max(0, entry.count);
        }
    }

    return 0;
}

int GameSession::ActivePartyAllocatedCount(const std::string& unitId) const {
    if (unitId.empty()) {
        return 0;
    }

    int allocated = 0;
    for (const auto& stackId : activeSlotStackIds_) {
        const auto* stack = FindStackById(stackId);
        if (stack != nullptr && stack->unitId == unitId) {
            allocated += std::max(0, stack->quantity);
        }
    }

    return allocated;
}

int GameSession::ReserveUnitCount(const std::string& unitId) const {
    if (unitId.empty()) {
        return 0;
    }

    int reserve = 0;
    for (const auto& stackId : reserveSlotStackIds_) {
        const auto* stack = FindStackById(stackId);
        if (stack != nullptr && stack->unitId == unitId) {
            reserve += std::max(0, stack->quantity);
        }
    }

    return reserve;
}

const std::vector<RosterStackState>& GameSession::RosterStacks() const {
    return rosterStacks_;
}

const std::vector<std::string>& GameSession::ActiveSlotStackIds() const {
    return activeSlotStackIds_;
}

const std::vector<std::string>& GameSession::ReserveSlotStackIds() const {
    return reserveSlotStackIds_;
}

int GameSession::NextStackIdCounter() const {
    return std::max(1, nextStackIdCounter_);
}

const std::vector<OwnedUnitCountState>& GameSession::OwnedUnitCounts() const {
    RebuildRosterProjectionCache();
    return ownedUnitCountProjection_;
}

const std::vector<std::string>& GameSession::ActivePartyUnitIds() const {
    RebuildRosterProjectionCache();
    return activePartyUnitIdProjection_;
}

bool GameSession::AddOwnedUnit(const std::string& unitId, const int count) {
    if (!CanAddOwnedUnit(unitId, count)) {
        return false;
    }

    const std::string compatibleReserveStackId = FindCompatibleStackIdInSlots(reserveSlotStackIds_, unitId);
    if (!compatibleReserveStackId.empty()) {
        if (auto* stack = FindStackById(compatibleReserveStackId)) {
            stack->quantity += count;
            MarkRosterProjectionDirty();
            return true;
        }
    }

    const std::string compatibleActiveStackId = FindCompatibleStackIdInSlots(activeSlotStackIds_, unitId);
    if (!compatibleActiveStackId.empty()) {
        if (auto* stack = FindStackById(compatibleActiveStackId)) {
            stack->quantity += count;
            MarkRosterProjectionDirty();
            return true;
        }
    }

    const int emptyReserveSlot = FindFirstEmptySlotIndex(reserveSlotStackIds_);
    if (emptyReserveSlot < 0) {
        return false;
    }

    const std::string stackId = GenerateNextStackId();
    rosterStacks_.push_back(RosterStackState{stackId, unitId, count});
    reserveSlotStackIds_[emptyReserveSlot] = stackId;
    MarkRosterProjectionDirty();
    return true;
}

bool GameSession::CanAddOwnedUnit(const std::string& unitId, const int count) const {
    if (unitId.empty() || count <= 0) {
        return false;
    }

    if (!FindCompatibleStackIdInSlots(reserveSlotStackIds_, unitId).empty()) {
        return true;
    }

    if (!FindCompatibleStackIdInSlots(activeSlotStackIds_, unitId).empty()) {
        return true;
    }

    return FindFirstEmptySlotIndex(reserveSlotStackIds_) >= 0;
}

bool GameSession::TryRemoveOwnedUnit(const std::string& unitId, const int count) {
    if (unitId.empty() || count <= 0) {
        return false;
    }

    const int owned = OwnedUnitCount(unitId);
    const int activeAllocated = ActivePartyAllocatedCount(unitId);
    if (owned < count || owned - count < activeAllocated) {
        return false;
    }

    int reserveQuantity = 0;
    for (const auto& stackId : reserveSlotStackIds_) {
        const auto* stack = FindStackById(stackId);
        if (stack != nullptr && stack->unitId == unitId) {
            reserveQuantity += std::max(0, stack->quantity);
        }
    }

    if (reserveQuantity < count) {
        return false;
    }

    int remainingToRemove = count;
    for (const auto& slotStackId : reserveSlotStackIds_) {
        if (remainingToRemove <= 0) {
            break;
        }

        const std::string stackId = slotStackId;
        auto* stack = FindStackById(stackId);
        if (stack == nullptr || stack->unitId != unitId) {
            continue;
        }

        const int removed = std::min(remainingToRemove, std::max(0, stack->quantity));
        stack->quantity -= removed;
        remainingToRemove -= removed;

        RemoveStackIfDepleted(stackId);
    }

    MarkRosterProjectionDirty();
    return true;
}

bool GameSession::TryAddUnitToActiveParty(const std::string& unitId) {
    if (unitId.empty()) {
        return false;
    }

    int reserveSlotIndex = -1;
    for (int i = 0; i < static_cast<int>(reserveSlotStackIds_.size()); ++i) {
        const auto* stack = FindStackById(reserveSlotStackIds_[i]);
        if (stack != nullptr && stack->unitId == unitId && stack->quantity > 0) {
            reserveSlotIndex = i;
            break;
        }
    }

    if (reserveSlotIndex < 0) {
        return false;
    }

    return TryMoveReserveStackToActiveSlot(reserveSlotIndex);
}

bool GameSession::TryMoveReserveStackToActiveSlot(const int reserveSlotIndex) {
    if (reserveSlotIndex < 0 || reserveSlotIndex >= static_cast<int>(reserveSlotStackIds_.size())) {
        return false;
    }

    const std::string stackId = reserveSlotStackIds_[reserveSlotIndex];
    if (stackId.empty()) {
        return false;
    }

    if (FindStackById(stackId) == nullptr) {
        return false;
    }

    const int activeSlot = FindFirstEmptySlotIndex(activeSlotStackIds_);
    if (activeSlot < 0) {
        return false;
    }

    activeSlotStackIds_[activeSlot] = stackId;
    reserveSlotStackIds_[reserveSlotIndex].clear();
    MarkRosterProjectionDirty();
    return true;
}

bool GameSession::TryRemoveActivePartyUnitAt(const int index) {
    if (index < 0) {
        return false;
    }

    std::vector<int> occupiedActiveSlots;
    occupiedActiveSlots.reserve(activeSlotStackIds_.size());
    for (int i = 0; i < static_cast<int>(activeSlotStackIds_.size()); ++i) {
        if (!activeSlotStackIds_[i].empty()) {
            occupiedActiveSlots.push_back(i);
        }
    }

    if (index >= static_cast<int>(occupiedActiveSlots.size())) {
        return false;
    }

    const int activeSlot = occupiedActiveSlots[index];
    const std::string stackId = activeSlotStackIds_[activeSlot];
    if (stackId.empty()) {
        return false;
    }

    if (!leaderCapableUnitIds_.empty() && ActiveLeaderCapableCountExcludingOrderedIndex(index) <= 0) {
        return false;
    }

    const int reserveSlot = FindFirstEmptySlotIndex(reserveSlotStackIds_);
    if (reserveSlot < 0) {
        return false;
    }

    reserveSlotStackIds_[reserveSlot] = stackId;
    activeSlotStackIds_[activeSlot].clear();
    MarkRosterProjectionDirty();
    return true;
}

bool GameSession::WouldRemovingActivePartyUnitLeaveNoLeader(const int index) const {
    if (index < 0 || leaderCapableUnitIds_.empty()) {
        return false;
    }

    std::vector<int> occupiedActiveSlots;
    occupiedActiveSlots.reserve(activeSlotStackIds_.size());
    for (int i = 0; i < static_cast<int>(activeSlotStackIds_.size()); ++i) {
        if (!activeSlotStackIds_[i].empty()) {
            occupiedActiveSlots.push_back(i);
        }
    }

    if (index >= static_cast<int>(occupiedActiveSlots.size())) {
        return false;
    }

    return ActiveLeaderCapableCountExcludingOrderedIndex(index) <= 0;
}

bool GameSession::TryMoveActivePartyUnit(const int fromIndex, const int toIndex) {
    std::vector<std::string> orderedActive;
    orderedActive.reserve(activeSlotStackIds_.size());
    for (const auto& stackId : activeSlotStackIds_) {
        if (!stackId.empty()) {
            orderedActive.push_back(stackId);
        }
    }

    if (fromIndex < 0 || fromIndex >= static_cast<int>(orderedActive.size())) {
        return false;
    }

    if (toIndex < 0 || toIndex >= static_cast<int>(orderedActive.size())) {
        return false;
    }

    if (fromIndex == toIndex) {
        return true;
    }

    const std::string movedStackId = orderedActive[fromIndex];
    orderedActive.erase(orderedActive.begin() + fromIndex);
    orderedActive.insert(orderedActive.begin() + toIndex, movedStackId);

    std::fill(activeSlotStackIds_.begin(), activeSlotStackIds_.end(), "");
    for (int i = 0; i < static_cast<int>(orderedActive.size()) && i < static_cast<int>(activeSlotStackIds_.size()); ++i) {
        activeSlotStackIds_[i] = orderedActive[i];
    }

    MarkRosterProjectionDirty();
    return true;
}

void GameSession::ClearActiveParty() {
    while (!ActivePartyUnitIds().empty()) {
        if (!TryRemoveActivePartyUnitAt(0)) {
            break;
        }
    }
}

const RosterStackState* GameSession::FindRosterStackById(const std::string& stackId) const {
    return FindStackById(stackId);
}

std::vector<ActiveBattleStackEntry> GameSession::BuildActiveBattleStackEntries() const {
    std::vector<ActiveBattleStackEntry> entries;
    entries.reserve(activeSlotStackIds_.size());

    for (int slotIndex = 0; slotIndex < static_cast<int>(activeSlotStackIds_.size()); ++slotIndex) {
        const auto& stackId = activeSlotStackIds_[slotIndex];
        const auto* stack = FindStackById(stackId);
        if (stack == nullptr || stack->quantity <= 0) {
            continue;
        }

        ActiveBattleStackEntry entry;
        entry.activeSlotIndex = slotIndex;
        entry.stackId         = stack->stackId;
        entry.unitId          = stack->unitId;
        entry.quantity        = stack->quantity;

        // M13-b: pre-sum equipped-artifact statBonus values for hero entries.
        // Generic units cannot equip artifacts, so their entries leave the
        // bonus fields at zero. SumEquippedArtifactBonuses tolerates a missing
        // heroEquipment_ entry by returning all zeros.
        SumEquippedArtifactBonuses(
            stack->unitId,
            entry.artifactAttackBonus,
            entry.artifactDefenseBonus,
            entry.artifactMagicBonus,
            entry.artifactResistanceBonus);

        entries.push_back(std::move(entry));
    }

    return entries;
}

bool GameSession::ApplyBattleStackLifeResults(
    const std::vector<BattleStackLifeResult>& results,
    const std::vector<std::string>& expectedStackIds) {
    std::set<std::string> expectedSet;
    for (const auto& stackId : expectedStackIds) {
        if (stackId.empty()) {
            return false;
        }

        if (!expectedSet.insert(stackId).second) {
            return false;
        }

        if (FindStackById(stackId) == nullptr) {
            return false;
        }
    }

    std::map<std::string, int> resultByStackId;
    for (const auto& result : results) {
        if (result.stackId.empty() || result.resultingLife < 0) {
            return false;
        }

        if (resultByStackId.find(result.stackId) != resultByStackId.end()) {
            return false;
        }

        resultByStackId[result.stackId] = result.resultingLife;
    }

    if (resultByStackId.size() != expectedSet.size()) {
        return false;
    }

    for (const auto& stackId : expectedSet) {
        if (resultByStackId.find(stackId) == resultByStackId.end()) {
            return false;
        }
    }

    for (const auto& [stackId, _] : resultByStackId) {
        if (expectedSet.find(stackId) == expectedSet.end()) {
            return false;
        }
    }

    auto stacksCopy = rosterStacks_;
    auto activeSlotsCopy = activeSlotStackIds_;
    auto reserveSlotsCopy = reserveSlotStackIds_;

    auto findStackInCopy = [&](const std::string& stackId) -> RosterStackState* {
        for (auto& stack : stacksCopy) {
            if (stack.stackId == stackId) {
                return &stack;
            }
        }

        return nullptr;
    };

    auto removeStackInCopy = [&](const std::string& stackId) {
        for (auto& slot : activeSlotsCopy) {
            if (slot == stackId) {
                slot.clear();
            }
        }

        for (auto& slot : reserveSlotsCopy) {
            if (slot == stackId) {
                slot.clear();
            }
        }

        stacksCopy.erase(
            std::remove_if(
                stacksCopy.begin(),
                stacksCopy.end(),
                [&](const RosterStackState& entry) {
                    return entry.stackId == stackId;
                }),
            stacksCopy.end());
    };

    for (const auto& [stackId, resultingLife] : resultByStackId) {
        auto* stack = findStackInCopy(stackId);
        if (stack == nullptr) {
            return false;
        }

        stack->quantity = resultingLife;
    }

    for (const auto& [stackId, resultingLife] : resultByStackId) {
        if (resultingLife == 0) {
            removeStackInCopy(stackId);
        }
    }

    rosterStacks_ = std::move(stacksCopy);
    activeSlotStackIds_ = std::move(activeSlotsCopy);
    reserveSlotStackIds_ = std::move(reserveSlotsCopy);
    NormalizeRosterState();
    return true;
}

SessionSnapshot GameSession::Snapshot() const {
    return SessionSnapshot{
        mode_,
        clock_.Day(),
        clock_.MinutesIntoSliceDay(),
        clock_.TimeString(),
        gold_,
        regionId_,
        destinationId_,
        currentEnergy_,
        dailyMaxEnergy_,
        campaignId_,
        currentScenarioId_,
        campaignState_
    };
}

core::SaveData GameSession::ToSaveData() const {
    return core::SaveData{
        5,
        clock_.Day(),
        clock_.MinutesIntoSliceDay(),
        gold_,
        ToString(mode_),
        regionId_,
        destinationId_,
        questState_.CompletedQuestIds(),
        nodeWorldState_.ClearedCombatNodeIds(),
        recruitServiceStates_,
        dailyServiceStates_,
        travelPrepDiscountMinutes_,
        travelPrepRemainingCharges_,
        travelPrepGrantedDay_,
        true,
        [&]() {
            std::vector<core::RosterStackSaveState> stacks;
            stacks.reserve(rosterStacks_.size());
            for (const auto& stack : rosterStacks_) {
                if (IsValidStackEntry(stack)) {
                    stacks.push_back(core::RosterStackSaveState{stack.stackId, stack.unitId, stack.quantity});
                }
            }
            return stacks;
        }(),
        activeSlotStackIds_,
        reserveSlotStackIds_,
        nextStackIdCounter_,
        {},
        {},
        firedEventIds_,
        {storyFlags_.begin(), storyFlags_.end()},
        [&]() {
            std::vector<core::EnemyTeamSaveState> states;
            states.reserve(enemyTeams_.size());
            for (const auto& team : enemyTeams_) {
                states.push_back({team.teamColor, team.nodeId, team.active,
                    team.energy, team.cooldownExpiresAtMinutes, team.alliances});
            }
            return states;
        }(),
        // Latched scenario outcome. Empty state string means "not latched".
        latchedOutcome_.has_value()
            ? (latchedOutcome_->state == scenario::ScenarioOutcomeState::Victory ? "victory"
               : latchedOutcome_->state == scenario::ScenarioOutcomeState::Defeat ? "defeat"
               : std::string{})
            : std::string{},
        latchedOutcome_.has_value() && latchedOutcome_->matchedConditionIndex.has_value()
            ? static_cast<int>(*latchedOutcome_->matchedConditionIndex)
            : -1,
        latchedOutcome_.has_value() ? latchedOutcome_->reason : std::string{},
        [&]() {
            std::vector<core::ItemSaveState> out;
            out.reserve(items_.size());
            for (const auto& s : items_) {
                out.push_back({s.itemId, s.quantity});
            }
            return out;
        }(),
        [&]() {
            std::vector<core::ArtifactSaveState> out;
            out.reserve(artifacts_.size());
            for (const auto& s : artifacts_) {
                out.push_back({s.artifactId, s.quantity});
            }
            return out;
        }(),
        [&]() {
            std::vector<core::HeroEquipmentSaveState> out;
            out.reserve(heroEquipment_.size());
            for (const auto& [heroId, equip] : heroEquipment_) {
                core::HeroEquipmentSaveState entry;
                entry.heroId             = heroId;
                entry.attackArtifactId   = equip.attackArtifactId;
                entry.defenseArtifactId  = equip.defenseArtifactId;
                entry.misc1ArtifactId    = equip.misc1ArtifactId;
                entry.misc2ArtifactId    = equip.misc2ArtifactId;
                entry.misc3ArtifactId    = equip.misc3ArtifactId;
                out.push_back(std::move(entry));
            }
            return out;
        }(),
        // M14-a team Energy pool.
        currentEnergy_,
        dailyMaxEnergy_,
        // M15-b World Map unlocked-region set.
        std::vector<std::string>(unlockedRegionIds_.begin(), unlockedRegionIds_.end()),
        // M16-b campaign progression.
        campaignId_,
        currentScenarioId_,
        completedScenarioIds_,
        std::vector<std::string>(campaignFlags_.begin(), campaignFlags_.end()),
        CampaignStateToString(campaignState_),
        // M17 resources: persist only non-zero non-gold resources in canonical
        // order. Gold is intentionally never written here (it stays in `gold`).
        [&]() {
            std::vector<core::ResourceSaveState> out;
            for (const auto type : kNonGoldResourceTypes) {
                const int amount = nonGoldResources_[NonGoldResourceIndex(type)];
                if (amount != 0) {
                    out.push_back({ResourceTypeToString(type), amount});
                }
            }
            return out;
        }(),
        // M17 owned services: stable runtime state, persisted as-is.
        ownedServices_
    };
}

void GameSession::ApplySaveData(const core::SaveData& saveData) {
    const GameMode loadedMode = FromString(saveData.mode);
    const int loadedGold = std::max(0, saveData.gold);
    const std::string loadedRegionId = saveData.regionId;
    const std::string loadedDestinationId = saveData.destinationId;

    core::GameClock loadedClock;
    const int daysToAdvance = std::max(0, saveData.day - 1);
    loadedClock.AdvanceMinutes(daysToAdvance * core::GameClock::kMinutesPerSliceDay + std::max(0, saveData.minutesIntoSliceDay));

    quests::QuestState loadedQuestState = questState_;
    loadedQuestState.RestoreCompletedQuestIds(saveData.completedQuestIds);

    world::NodeWorldState loadedNodeWorldState = nodeWorldState_;
    loadedNodeWorldState.RestoreClearedCombatNodeIds(saveData.clearedCombatNodeIds);

    const std::vector<core::RecruitServiceState> loadedRecruitStates = saveData.recruitServiceStates;
    const std::vector<core::DailyServiceState> loadedDailyStates = saveData.dailyServiceStates;
    const int loadedTravelPrepDiscount = std::max(0, saveData.travelPrepDiscountMinutes);
    const int loadedTravelPrepCharges = std::max(0, saveData.travelPrepRemainingCharges);
    const int loadedTravelPrepDay = std::max(0, saveData.travelPrepGrantedDay);

    std::vector<RosterStackState> loadedStacks;
    std::vector<std::string> loadedActiveSlots(kActiveSlotCount, "");
    std::vector<std::string> loadedReserveSlots(kReserveSlotCount, "");
    int loadedNextCounter = 1;

    if (saveData.hasCanonicalRoster) {
        loadedStacks.reserve(saveData.rosterStacks.size());
        for (const auto& stack : saveData.rosterStacks) {
            loadedStacks.push_back(RosterStackState{stack.stackId, stack.unitId, stack.quantity});
        }

        if (saveData.activeSlotStackIds.size() != kActiveSlotCount ||
            saveData.reserveSlotStackIds.size() != kReserveSlotCount) {
            return;
        }

        loadedActiveSlots = saveData.activeSlotStackIds;
        loadedReserveSlots = saveData.reserveSlotStackIds;
        loadedNextCounter = std::max(1, saveData.nextStackIdCounter);

        if (!leaderCapableUnitIds_.empty()) {
            int activeLeaderCapableCount = 0;
            for (const auto& stackId : loadedActiveSlots) {
                if (stackId.empty()) {
                    continue;
                }

                for (const auto& stack : loadedStacks) {
                    if (stack.stackId == stackId && IsLeaderCapableUnitId(stack.unitId)) {
                        ++activeLeaderCapableCount;
                        break;
                    }
                }
            }

            if (activeLeaderCapableCount <= 0) {
                return;
            }
        }
    }
    else {
        std::map<std::string, int> normalizedOwned;
        for (const auto& entry : saveData.ownedUnitCounts) {
            if (entry.unitId.empty() || entry.count <= 0) {
                continue;
            }

            normalizedOwned[entry.unitId] += entry.count;
        }

        int nextSuffix = 1;
        auto allocateStackId = [&]() {
            return std::string("stk_") + std::to_string(nextSuffix++);
        };

        int activeWriteIndex = 0;
        for (const auto& unitId : saveData.activePartyUnitIds) {
            if (activeWriteIndex >= kActiveSlotCount || unitId.empty()) {
                continue;
            }

            auto ownedIt = normalizedOwned.find(unitId);
            if (ownedIt == normalizedOwned.end() || ownedIt->second <= 0) {
                continue;
            }

            const std::string stackId = allocateStackId();
            loadedStacks.push_back(RosterStackState{stackId, unitId, 1});
            loadedActiveSlots[activeWriteIndex] = stackId;
            ++activeWriteIndex;
            --ownedIt->second;
        }

        int reserveWriteIndex = 0;
        for (const auto& [unitId, count] : normalizedOwned) {
            if (count <= 0) {
                continue;
            }

            if (reserveWriteIndex >= kReserveSlotCount) {
                return;
            }

            const std::string stackId = allocateStackId();
            loadedStacks.push_back(RosterStackState{stackId, unitId, count});
            loadedReserveSlots[reserveWriteIndex] = stackId;
            ++reserveWriteIndex;
        }

        loadedNextCounter = std::max(1, nextSuffix);
    }

    mode_ = loadedMode;
    gold_ = loadedGold;
    regionId_ = loadedRegionId;
    destinationId_ = loadedDestinationId;
    clock_ = loadedClock;
    questState_ = loadedQuestState;
    nodeWorldState_ = loadedNodeWorldState;
    recruitServiceStates_ = loadedRecruitStates;
    dailyServiceStates_ = loadedDailyStates;
    travelPrepDiscountMinutes_ = loadedTravelPrepDiscount;
    travelPrepRemainingCharges_ = loadedTravelPrepCharges;
    travelPrepGrantedDay_ = loadedTravelPrepDay;
    rosterStacks_ = std::move(loadedStacks);
    activeSlotStackIds_ = std::move(loadedActiveSlots);
    reserveSlotStackIds_ = std::move(loadedReserveSlots);
    nextStackIdCounter_ = loadedNextCounter;
    firedEventIds_ = saveData.firedEventIds;
    storyFlags_ = {saveData.storyFlags.begin(), saveData.storyFlags.end()};
    for (const auto& saved : saveData.enemyTeams) {
        for (auto& team : enemyTeams_) {
            if (team.teamColor == saved.teamColor) {
                team.nodeId = saved.nodeId;
                team.active = saved.active;
                team.energy = saved.energy;
                team.cooldownExpiresAtMinutes = saved.cooldownExpiresAtMinutes;
                team.alliances = saved.alliances;
                break;
            }
        }
    }
    // Restore the latched scenario outcome. A saved "victory"/"defeat" state must
    // never be re-evaluated away on load; the latch is persistent by design.
    latchedOutcome_.reset();
    if (saveData.scenarioOutcomeState == "victory" || saveData.scenarioOutcomeState == "defeat") {
        scenario::ScenarioOutcome outcome;
        outcome.state = saveData.scenarioOutcomeState == "victory"
            ? scenario::ScenarioOutcomeState::Victory
            : scenario::ScenarioOutcomeState::Defeat;
        if (saveData.scenarioOutcomeMatchedConditionIndex >= 0) {
            outcome.matchedConditionIndex =
                static_cast<std::size_t>(saveData.scenarioOutcomeMatchedConditionIndex);
        }
        outcome.reason = saveData.scenarioOutcomeReason;
        latchedOutcome_ = outcome;
    }

    // M13-b: restore inventory and equipment.
    items_.clear();
    items_.reserve(saveData.items.size());
    for (const auto& s : saveData.items) {
        items_.push_back({s.itemId, s.quantity});
    }
    artifacts_.clear();
    artifacts_.reserve(saveData.artifacts.size());
    for (const auto& s : saveData.artifacts) {
        artifacts_.push_back({s.artifactId, s.quantity});
    }
    heroEquipment_.clear();
    for (const auto& entry : saveData.heroEquipment) {
        HeroEquipmentState equip;
        equip.attackArtifactId  = entry.attackArtifactId;
        equip.defenseArtifactId = entry.defenseArtifactId;
        equip.misc1ArtifactId   = entry.misc1ArtifactId;
        equip.misc2ArtifactId   = entry.misc2ArtifactId;
        equip.misc3ArtifactId   = entry.misc3ArtifactId;
        heroEquipment_[entry.heroId] = std::move(equip);
    }

    // M14-a: restore team Energy. The -1 sentinel marks a legacy save predating
    // Energy; recompute a fresh daily value so it never loads to a 0 pool. The
    // roster is already restored above, so ApplyDailyStartingEnergy can read the
    // traveling party (agility resolves only if the unit catalog is set — the
    // App sets it before loading; bare-session tests get the 1000 floor).
    if (saveData.energy < 0 || saveData.maxEnergy < 0) {
        ApplyDailyStartingEnergy();
    } else {
        dailyMaxEnergy_ = saveData.maxEnergy;
        currentEnergy_ = std::clamp(saveData.energy, 0, saveData.maxEnergy);
    }

    // M15-b: restore World Map unlocked-region set. An absent/empty saved set
    // means a legacy save — keep the authored seed from SetWorldMap rather than
    // wiping unlock state. A non-empty saved set replaces it (every world map
    // keeps the start region unlocked, so non-empty unambiguously marks M15+).
    if (!saveData.unlockedRegionIds.empty()) {
        unlockedRegionIds_ = std::set<std::string>(
            saveData.unlockedRegionIds.begin(), saveData.unlockedRegionIds.end());
    }

    // M16-b: restore campaign progression. Absent/empty fields (legacy saves)
    // load as no campaign. The active outcome definition is re-selected from the
    // restored current scenario (inline if authored, else the global fallback)
    // so a loaded campaign keeps evaluating the right win/loss conditions.
    campaignId_ = saveData.campaignId;
    currentScenarioId_ = saveData.currentScenarioId;
    completedScenarioIds_ = saveData.completedScenarioIds;
    campaignFlags_ = std::set<std::string>(
        saveData.campaignFlags.begin(), saveData.campaignFlags.end());
    campaignState_ = CampaignStateFromString(saveData.campaignState);
    if (const data::ScenarioDefinition* scenario = FindScenarioDefinition(currentScenarioId_)) {
        SelectActiveOutcomeForScenario(*scenario);
    }

    // M17: restore the non-gold resource pool. Reset to zero, then apply saved
    // entries. Unknown/Gold resource names are ignored so a stray "Gold" entry
    // can never create a second gold store. Negative amounts clamp to zero.
    nonGoldResources_.fill(0);
    for (const auto& entry : saveData.resources) {
        ResourceType type;
        if (!TryResourceTypeFromString(entry.resource, type) || IsGoldResource(type)) {
            continue;
        }
        nonGoldResources_[NonGoldResourceIndex(type)] = std::max(0, entry.amount);
    }

    // M17: restore owned-service runtime state (Phase 1 stable fields + Phase 3a
    // stationing + M28 storage). The roster is already restored above, so normalize
    // stationing and storage now to drop any ref that is not stack-backed (the
    // invariant depends only on the roster, not the unit catalog).
    ownedServices_ = saveData.ownedServices;
    NormalizeStationedUnits();
    NormalizeStoredUnits();

    MarkRosterProjectionDirty();
}

bool GameSession::IsLeaderCapableUnitId(const std::string& unitId) const {
    if (unitId.empty()) {
        return false;
    }

    return leaderCapableUnitIds_.find(unitId) != leaderCapableUnitIds_.end();
}

int GameSession::ActiveLeaderCapableCountExcludingOrderedIndex(const int excludedOrderedIndex) const {
    int count = 0;
    int orderedIndex = 0;

    for (const auto& stackId : activeSlotStackIds_) {
        if (stackId.empty()) {
            continue;
        }

        const auto* stack = FindStackById(stackId);
        if (stack == nullptr) {
            continue;
        }

        if (orderedIndex != excludedOrderedIndex && IsLeaderCapableUnitId(stack->unitId)) {
            ++count;
        }

        ++orderedIndex;
    }

    return count;
}

void GameSession::NormalizeRosterState() {
    std::vector<RosterStackState> normalizedStacks;
    normalizedStacks.reserve(rosterStacks_.size());
    for (const auto& stack : rosterStacks_) {
        if (IsValidStackEntry(stack)) {
            normalizedStacks.push_back(stack);
        }
    }
    rosterStacks_ = std::move(normalizedStacks);

    auto normalizeSlots = [&](std::vector<std::string>& slots, std::vector<std::string>& assigned) {
        std::vector<std::string> normalized(slots.size(), "");
        assigned.reserve(assigned.size() + slots.size());

        int write = 0;
        for (const auto& stackId : slots) {
            if (write >= static_cast<int>(normalized.size())) {
                break;
            }

            if (stackId.empty()) {
                continue;
            }

            if (FindStackById(stackId) == nullptr) {
                continue;
            }

            if (std::find(assigned.begin(), assigned.end(), stackId) != assigned.end()) {
                continue;
            }

            normalized[write] = stackId;
            assigned.push_back(stackId);
            ++write;
        }

        slots = std::move(normalized);
    };

    std::vector<std::string> assignedStackIds;
    assignedStackIds.reserve(activeSlotStackIds_.size() + reserveSlotStackIds_.size());
    normalizeSlots(activeSlotStackIds_, assignedStackIds);
    normalizeSlots(reserveSlotStackIds_, assignedStackIds);

    int maxSuffix = 0;
    for (const auto& stack : rosterStacks_) {
        maxSuffix = std::max(maxSuffix, ParseStackIdSuffix(stack.stackId));
    }

    nextStackIdCounter_ = std::max(nextStackIdCounter_, maxSuffix + 1);
    nextStackIdCounter_ = std::max(1, nextStackIdCounter_);

    MarkRosterProjectionDirty();
}

void GameSession::MarkRosterProjectionDirty() {
    rosterProjectionDirty_ = true;
}

void GameSession::RebuildRosterProjectionCache() const {
    if (!rosterProjectionDirty_) {
        return;
    }

    std::map<std::string, int> aggregated;
    for (const auto& stack : rosterStacks_) {
        if (!IsValidStackEntry(stack)) {
            continue;
        }

        aggregated[stack.unitId] += stack.quantity;
    }

    ownedUnitCountProjection_.clear();
    ownedUnitCountProjection_.reserve(aggregated.size());
    for (const auto& [unitId, count] : aggregated) {
        if (IsValidOwnedEntry(OwnedUnitCountState{unitId, count})) {
            ownedUnitCountProjection_.push_back(OwnedUnitCountState{unitId, count});
        }
    }

    activePartyUnitIdProjection_.clear();
    activePartyUnitIdProjection_.reserve(activeSlotStackIds_.size());
    for (const auto& stackId : activeSlotStackIds_) {
        if (const auto* stack = FindStackById(stackId)) {
            activePartyUnitIdProjection_.push_back(stack->unitId);
        }
    }

    rosterProjectionDirty_ = false;
}

std::string GameSession::GenerateNextStackId() {
    const int suffix = std::max(1, nextStackIdCounter_);
    nextStackIdCounter_ = suffix + 1;
    return "stk_" + std::to_string(suffix);
}

RosterStackState* GameSession::FindStackById(const std::string& stackId) {
    if (stackId.empty()) {
        return nullptr;
    }

    for (auto& stack : rosterStacks_) {
        if (stack.stackId == stackId) {
            return &stack;
        }
    }

    return nullptr;
}

const RosterStackState* GameSession::FindStackById(const std::string& stackId) const {
    if (stackId.empty()) {
        return nullptr;
    }

    for (const auto& stack : rosterStacks_) {
        if (stack.stackId == stackId) {
            return &stack;
        }
    }

    return nullptr;
}

std::string GameSession::FindCompatibleStackIdInSlots(
    const std::vector<std::string>& slotStackIds,
    const std::string& unitId) const {
    if (unitId.empty()) {
        return "";
    }

    for (const auto& stackId : slotStackIds) {
        const auto* stack = FindStackById(stackId);
        if (stack != nullptr && stack->unitId == unitId && stack->quantity > 0) {
            return stackId;
        }
    }

    return "";
}

int GameSession::FindFirstEmptySlotIndex(const std::vector<std::string>& slotStackIds) const {
    for (int i = 0; i < static_cast<int>(slotStackIds.size()); ++i) {
        if (slotStackIds[i].empty()) {
            return i;
        }
    }

    return -1;
}

void GameSession::RemoveStackIfDepleted(const std::string& stackId) {
    auto* stack = FindStackById(stackId);
    if (stack == nullptr || stack->quantity > 0) {
        return;
    }

    for (auto& activeSlot : activeSlotStackIds_) {
        if (activeSlot == stackId) {
            activeSlot.clear();
        }
    }

    for (auto& reserveSlot : reserveSlotStackIds_) {
        if (reserveSlot == stackId) {
            reserveSlot.clear();
        }
    }

    rosterStacks_.erase(
        std::remove_if(
            rosterStacks_.begin(),
            rosterStacks_.end(),
            [&](const RosterStackState& entry) {
                return entry.stackId == stackId;
            }),
        rosterStacks_.end());

    MarkRosterProjectionDirty();
}

std::string GameSession::ToString(const GameMode mode) {
    switch (mode) {
    case GameMode::Title:
        return "title";
    case GameMode::OpeningSequence:
        return "opening_sequence";
    case GameMode::CampaignSelectMode:
        return "campaign_select";
    case GameMode::WorldMapMode:
        return "overworld_selection";
    case GameMode::RegionMode:
        return "overworld_mode";
    case GameMode::LocationMode:
        return "location_mode";
    case GameMode::BattleMode:
        return "battle_mode";
    case GameMode::ScenarioResultMode:
        // Distinct, deliberate value so the switch is exhaustive and the
        // transient mode never silently serializes as "title". App never
        // persists this mode; this string is for diagnostics only.
        return "scenario_result";
    case GameMode::OwnedServiceOverviewMode:
        // Diagnostics/logging only — deliberately NOT reversible. App never
        // persists this transient mode; FromString self-heals it to RegionMode.
        return "owned_service_overview";
    }

    return "title";
}

GameMode GameSession::FromString(const std::string& mode) {
    if (mode == "opening_sequence") {
        return GameMode::OpeningSequence;
    }
    if (mode == "campaign_select") {
        return GameMode::CampaignSelectMode;
    }
    if (mode == "overworld_selection") {
        return GameMode::WorldMapMode;
    }
    if (mode == "overworld_mode") {
        return GameMode::RegionMode;
    }
    if (mode == "location_mode") {
        return GameMode::LocationMode;
    }
    if (mode == "battle_mode") {
        return GameMode::BattleMode;
    }
    if (mode == "scenario_result") {
        // Safe self-heal: this transient mode is never persisted by App, but a
        // hand-edited or corrupt save could carry it. Resolve to the stable,
        // playable RegionMode rather than resuming a transient screen — the
        // outcome latch survives load and UpdateRegionMode re-derives the
        // result screen immediately when the scenario is still ended.
        return GameMode::RegionMode;
    }
    if (mode == "owned_service_overview") {
        // Safe self-heal (M27): the transient overview panel is never persisted by
        // App. A hand-edited/corrupt save carrying it resolves to RegionMode — the
        // player can simply reopen the panel. This is deliberately not the inverse
        // of ToString(OwnedServiceOverviewMode).
        return GameMode::RegionMode;
    }

    return GameMode::Title;
}

const std::vector<std::string> GameSession::kTeamColorOrder =
    { "Green", "Red", "Blue", "Yellow", "Purple", "Orange", "Teal", "White" };

void GameSession::SetEnemyTeams(std::vector<EnemyTeamState> teams) {
    enemyTeams_ = std::move(teams);
    enemyTeamSeed_ = enemyTeams_;   // M16-b: authored seed for scenario-transition reset
}

const std::vector<EnemyTeamState>& GameSession::EnemyTeams() const {
    return enemyTeams_;
}

std::vector<EnemyTeamActionResult> GameSession::ProcessEnemyPhase(
    const std::vector<data::RegionLinkDefinition>& regionLinks) {
    std::vector<EnemyTeamActionResult> results;
    for (const auto& color : kTeamColorOrder) {
        for (auto& team : enemyTeams_) {
            if (!team.active || team.teamColor != color) {
                continue;
            }

            if (!team.patrol.enabled || team.nodeId.empty()) {
                results.push_back({ team.teamColor, "idle" });
                continue;
            }

            std::vector<std::string> candidates;
            for (const auto& link : regionLinks) {
                if (link.fromLocationId == team.nodeId) {
                    candidates.push_back(link.toLocationId);
                } else if (link.toLocationId == team.nodeId) {
                    candidates.push_back(link.fromLocationId);
                }
            }
            std::sort(candidates.begin(), candidates.end());
            candidates.erase(std::unique(candidates.begin(), candidates.end()), candidates.end());

            std::vector<std::string> valid;
            for (const auto& c : candidates) {
                const int hops = region::FindHopCount(team.patrol.centerNodeId, c, regionLinks);
                if (hops >= 0 && hops <= team.patrol.radius) {
                    valid.push_back(c);
                }
            }

            if (!valid.empty()) {
                team.nodeId = valid[0];
                results.push_back({ team.teamColor, "moved" });
            } else {
                results.push_back({ team.teamColor, "idle" });
            }
        }
    }
    // M12-b ordering boundary #2: enemy phase may flip outcome state through
    // future mutating actions (current build only does patrol movement, so the
    // typical case is "no change"). The latch call is here regardless so the
    // call site contract holds as enemy-phase behavior expands.
    CheckAndLatchOutcome();
    return results;
}

std::vector<std::string> GameSession::HostileOccupiedNodeIds(
    const std::string& playerColor) const {
    std::set<std::string> seen;
    for (const auto& team : enemyTeams_) {
        if (!team.active || team.nodeId.empty()) {
            continue;
        }
        if (team.teamColor == playerColor) {
            continue;
        }
        if (std::find(team.alliances.begin(), team.alliances.end(), playerColor)
            != team.alliances.end()) {
            continue;
        }
        seen.insert(team.nodeId);
    }
    return std::vector<std::string>(seen.begin(), seen.end());
}

void GameSession::ClearEnemyTeamByColor(const std::string& teamColor) {
    for (auto& team : enemyTeams_) {
        if (team.teamColor == teamColor) {
            team.active = false;
            break;
        }
    }
    // Hostile-contact victory route: clearing the last hostile team should
    // latch default victory immediately after the contact battle resolves.
    CheckAndLatchOutcome();
}

std::string GameSession::HostileTeamColorAtNode(
    const std::string& nodeId, const std::string& playerColor) const {
    for (const auto& team : enemyTeams_) {
        if (!team.active || team.nodeId != nodeId || team.teamColor == playerColor) continue;
        if (std::ranges::find(team.alliances, playerColor) != team.alliances.end()) continue;
        return team.teamColor;
    }
    return "";
}

economy::ServiceOwnerRelationship GameSession::OwnerRelationshipForColor(
    const std::string& ownerColor) const {
    if (ownerColor.empty()) {
        return economy::ServiceOwnerRelationship::Unowned;
    }
    if (ownerColor == playerColor_) {
        return economy::ServiceOwnerRelationship::Player;
    }
    // Allied iff an active team of that color is allied to the player. Mirrors
    // the alliance determination used by HostileOccupiedNodeIds.
    for (const auto& team : enemyTeams_) {
        if (!team.active || team.teamColor != ownerColor) {
            continue;
        }
        if (std::find(team.alliances.begin(), team.alliances.end(), playerColor_)
            != team.alliances.end()) {
            return economy::ServiceOwnerRelationship::AlliedToPlayer;
        }
    }
    return economy::ServiceOwnerRelationship::HostileToPlayer;
}

std::vector<std::string> GameSession::ClaimContestedServicesAtNode(
    const std::string& nodeId) {
    // Back-compat alias: the claim logic lives in ResolveNodeEntryClaims, the
    // single node-entry claim path for both peaceful entry and guarded capture.
    return ResolveNodeEntryClaims(nodeId);
}

std::vector<std::string> GameSession::ResolveNodeEntryClaims(
    const std::string& nodeId) {
    std::vector<std::string> claimed;
    if (nodeId.empty()) {
        return claimed;
    }
    // Still contested by another hostile team => the guards are not fully
    // cleared; claim nothing.
    const auto hostileNodes = HostileOccupiedNodeIds(playerColor_);
    if (std::find(hostileNodes.begin(), hostileNodes.end(), nodeId) != hostileNodes.end()) {
        return claimed;
    }

    for (const auto& svc : locationServiceCatalog_) {
        if (svc.locationId != nodeId) {
            continue;
        }
        const auto* existing = FindOwnedService(svc.id);
        const std::string ownerColor = existing ? existing->ownerTeamColor : std::string{};
        const bool locked = existing != nullptr && existing->locked;
        const bool destroyed = existing != nullptr && existing->destroyed;
        if (!economy::ServiceIsClaimable(
                svc.kind, locked, destroyed, OwnerRelationshipForColor(ownerColor))) {
            continue;
        }
        // Apply the claim: the player becomes the owner and any inherited
        // stationed units are dropped. Content definitions are never mutated.
        bool updated = false;
        for (auto& owned : ownedServices_) {
            if (owned.serviceId == svc.id) {
                owned.ownerTeamColor = playerColor_;
                owned.stationedUnits.clear();
                updated = true;
                break;
            }
        }
        if (!updated) {
            ownedServices_.push_back(core::OwnedServiceSaveState{
                svc.id, playerColor_, false, false, {}});
        }
        claimed.push_back(svc.id);
    }
    return claimed;
}

void GameSession::SetPlayerColor(std::string color) {
    playerColor_ = std::move(color);
}

const std::string& GameSession::PlayerColor() const {
    return playerColor_;
}

void GameSession::SetScenarioOutcomeDefinition(data::ScenarioOutcomeDefinition definition) {
    // Standalone / no-campaign play (and the global fallback for campaigns):
    // populate both. A campaign scenario start may later re-select the active
    // definition via SelectActiveOutcomeForScenario.
    globalScenarioOutcomeDefinition_ = definition;
    activeScenarioOutcomeDefinition_ = std::move(definition);
}

const data::ScenarioOutcomeDefinition& GameSession::ActiveScenarioOutcomeDefinition() const {
    return activeScenarioOutcomeDefinition_;
}

const std::optional<scenario::ScenarioOutcome>& GameSession::CheckAndLatchOutcome() {
    if (latchedOutcome_.has_value()) {
        return latchedOutcome_;
    }

    events::EventEvaluationContext condCtx;
    const auto snap = Snapshot();
    condCtx.currentDay = snap.day;
    SeedEventResourceContext(condCtx);
    const auto& partyIds = ActivePartyUnitIds();
    condCtx.heroIds = std::vector<std::string>(partyIds.begin(), partyIds.end());
    condCtx.storyFlags = storyFlags_;

    scenario::ScenarioOutcomeContext ctx;
    ctx.playerColor = playerColor_;
    ctx.enemyTeams = &enemyTeams_;
    ctx.conditionContext = &condCtx;

    const auto outcome = scenario::EvaluateScenarioOutcome(ctx, activeScenarioOutcomeDefinition_);
    if (outcome.state != scenario::ScenarioOutcomeState::Ongoing) {
        latchedOutcome_ = outcome;
    }
    return latchedOutcome_;
}

const std::optional<scenario::ScenarioOutcome>& GameSession::Outcome() const {
    return latchedOutcome_;
}

bool GameSession::IsScenarioEnded() const {
    return latchedOutcome_.has_value()
        && latchedOutcome_->state != scenario::ScenarioOutcomeState::Ongoing;
}

// ---------------------------------------------------------------------------
// M16-b — Campaign progression.
// ---------------------------------------------------------------------------

void GameSession::SetScenarioCatalog(std::vector<data::ScenarioDefinition> catalog) {
    scenarioCatalog_ = std::move(catalog);
    scenarioIndexById_.clear();
    for (std::size_t i = 0; i < scenarioCatalog_.size(); ++i) {
        scenarioIndexById_[scenarioCatalog_[i].id] = i;
    }
}

void GameSession::SetCampaignCatalog(std::vector<data::CampaignDefinition> catalog) {
    campaignCatalog_ = std::move(catalog);
    campaignIndexById_.clear();
    for (std::size_t i = 0; i < campaignCatalog_.size(); ++i) {
        campaignIndexById_[campaignCatalog_[i].id] = i;
    }
}

const std::vector<data::CampaignDefinition>& GameSession::CampaignCatalog() const {
    return campaignCatalog_;
}

const data::ScenarioDefinition* GameSession::FindScenarioDefinition(const std::string& id) const {
    const auto it = scenarioIndexById_.find(id);
    return it == scenarioIndexById_.end() ? nullptr : &scenarioCatalog_[it->second];
}

const data::CampaignDefinition* GameSession::FindCampaignDefinition(const std::string& id) const {
    const auto it = campaignIndexById_.find(id);
    return it == campaignIndexById_.end() ? nullptr : &campaignCatalog_[it->second];
}

std::string GameSession::PlayerHeroUnitId() const {
    for (const auto& unit : unitCatalog_) {
        if (unit.isPlayerCharacter) {
            return unit.id;
        }
    }
    return {};
}

std::set<std::string> GameSession::ValidUnitIdSet() const {
    std::set<std::string> ids;
    for (const auto& unit : unitCatalog_) {
        ids.insert(unit.id);
    }
    return ids;
}

campaign::CampaignCarrySet GameSession::CaptureCarrySet() const {
    campaign::CampaignCarrySet s;
    s.gold = gold_;
    for (const auto& stack : rosterStacks_) {
        s.rosterStacks.push_back({stack.stackId, stack.unitId, stack.quantity});
    }
    s.activeSlotStackIds = activeSlotStackIds_;
    s.reserveSlotStackIds = reserveSlotStackIds_;
    for (const auto& item : items_) {
        s.items.push_back({item.itemId, item.quantity});
    }
    for (const auto& artifact : artifacts_) {
        s.artifacts.push_back({artifact.artifactId, artifact.quantity});
    }
    for (const auto& [heroId, eq] : heroEquipment_) {
        s.heroEquipment.push_back({heroId, eq.attackArtifactId, eq.defenseArtifactId,
            eq.misc1ArtifactId, eq.misc2ArtifactId, eq.misc3ArtifactId});
    }
    s.storyFlags = storyFlags_;
    return s;
}

void GameSession::ApplyCarrySet(const campaign::CampaignCarrySet& set) {
    gold_ = set.gold;
    rosterStacks_.clear();
    for (const auto& stack : set.rosterStacks) {
        rosterStacks_.push_back({stack.stackId, stack.unitId, stack.quantity});
    }
    // Slot vectors keep their fixed sizes through the carry filter; guard anyway.
    activeSlotStackIds_ = set.activeSlotStackIds;
    reserveSlotStackIds_ = set.reserveSlotStackIds;
    activeSlotStackIds_.resize(kActiveSlotCount, "");
    reserveSlotStackIds_.resize(kReserveSlotCount, "");

    items_.clear();
    for (const auto& item : set.items) {
        items_.push_back({item.itemId, item.quantity});
    }
    artifacts_.clear();
    for (const auto& artifact : set.artifacts) {
        artifacts_.push_back({artifact.artifactId, artifact.quantity});
    }
    heroEquipment_.clear();
    for (const auto& eq : set.heroEquipment) {
        HeroEquipmentState state;
        state.attackArtifactId  = eq.attackArtifactId;
        state.defenseArtifactId = eq.defenseArtifactId;
        state.misc1ArtifactId   = eq.misc1ArtifactId;
        state.misc2ArtifactId   = eq.misc2ArtifactId;
        state.misc3ArtifactId   = eq.misc3ArtifactId;
        heroEquipment_[eq.heroId] = std::move(state);
    }
    // Carried scenario story flags are added on top of the persistent campaign
    // flags already re-seeded into storyFlags_ by TransitionToScenario.
    for (const auto& flag : set.storyFlags) {
        storyFlags_.insert(flag);
    }
    MarkRosterProjectionDirty();
}

void GameSession::ReseedWorldMapUnlock() {
    unlockedRegionIds_.clear();
    for (const auto& entry : worldMap_.entries) {
        if (entry.unlocked) {
            unlockedRegionIds_.insert(entry.id);
        }
    }
}

void GameSession::SelectActiveOutcomeForScenario(const data::ScenarioDefinition& scenario) {
    if (scenario.hasInlineOutcome) {
        data::ScenarioOutcomeDefinition inlineDef;
        inlineDef.victoryConditions = scenario.victoryConditions;
        inlineDef.defeatConditions  = scenario.defeatConditions;
        activeScenarioOutcomeDefinition_ = std::move(inlineDef);
    } else {
        activeScenarioOutcomeDefinition_ = globalScenarioOutcomeDefinition_;
    }
}

void GameSession::TransitionToScenario(const std::string& scenarioId,
                                       std::optional<campaign::CampaignCarrySet> carry) {
    const data::ScenarioDefinition* scenario = FindScenarioDefinition(scenarioId);
    if (scenario == nullptr) {
        return;   // validated content should never hit this; mutate nothing.
    }

    const data::CampaignDefinition* campaign = FindCampaignDefinition(campaignId_);

    // Step 3: record the completed old scenario, only on a real victory-advance.
    if (carry.has_value() && !currentScenarioId_.empty() &&
        std::find(completedScenarioIds_.begin(), completedScenarioIds_.end(),
                  currentScenarioId_) == completedScenarioIds_.end()) {
        completedScenarioIds_.push_back(currentScenarioId_);
    }

    // Step 4: reset scenario-local state. Promote any declared campaign flags
    // currently set into the persistent campaign-flag set BEFORE clearing story
    // flags. Roster is NOT reset here (no scenario-default roster in M16).
    if (campaign != nullptr) {
        for (const auto& declared : campaign->campaignFlags) {
            if (storyFlags_.count(declared) > 0) {
                campaignFlags_.insert(declared);
            }
        }
    }
    latchedOutcome_.reset();
    storyFlags_.clear();
    firedEventIds_.clear();
    enemyTeams_ = enemyTeamSeed_;
    nodeWorldState_ = world::NodeWorldState{};
    questState_.Initialize(questDefinitionSeed_);
    recruitServiceStates_.clear();
    dailyServiceStates_.clear();

    // Step 5 + 6: advance to the next scenario and set its start region/node.
    currentScenarioId_ = scenarioId;
    regionId_ = scenario->startRegionId;
    if (!scenario->startNodeId.empty()) {
        destinationId_ = scenario->startNodeId;
    } else if (const data::RegionDefinition* region = FindRegionDefinition(scenario->startRegionId)) {
        destinationId_ = region->arrivalNodeId;
    } else {
        destinationId_ = "";
    }

    // Step 7: seed scenario defaults (gold, resources, owned services, active
    // outcome, world-map unlock) and re-seed persistent campaign flags so
    // conditions still see them.
    gold_ = scenario->startGold.value_or(kDefaultStartGold);
    // Authored economy start-state, applied deterministically like gold: reset
    // then apply the authored values. Validation guarantees non-Gold/positive
    // resources and existing ownable services; the Gold guard is defensive.
    // Carry-over (Step 8) still overrides these if a rule touches them.
    nonGoldResources_.fill(0);
    for (const auto& res : scenario->startResources) {
        ResourceType type;
        if (TryResourceTypeFromString(res.resource, type) && !IsGoldResource(type)) {
            AddResource(type, res.amount);
        }
    }
    ownedServices_.clear();
    for (const auto& owned : scenario->startOwnedServices) {
        ownedServices_.push_back(core::OwnedServiceSaveState{
            owned.serviceId, playerColor_, owned.locked, owned.destroyed, {}});
    }
    NormalizeStationedUnits();
    SelectActiveOutcomeForScenario(*scenario);
    ReseedWorldMapUnlock();
    for (const auto& flag : campaignFlags_) {
        storyFlags_.insert(flag);
    }

    // Step 8: apply carry-over (transition only). Overwrites the relevant defaults.
    if (carry.has_value()) {
        ApplyCarrySet(*carry);
    }

    // Step 9: recompute Energy LAST, after the final roster is settled.
    ApplyDailyStartingEnergy();

    // Step 10: clear the latch for the new scenario (idempotent with step 4).
    latchedOutcome_.reset();

    // Step 11: enter Region mode.
    mode_ = GameMode::RegionMode;

    MarkRosterProjectionDirty();
}

void GameSession::StartCampaign(const std::string& campaignId) {
    const data::CampaignDefinition* campaign = FindCampaignDefinition(campaignId);
    if (campaign == nullptr) {
        return;
    }
    campaignId_ = campaignId;
    campaignState_ = CampaignState::InProgress;
    completedScenarioIds_.clear();
    campaignFlags_.clear();
    currentScenarioId_.clear();   // so the initial start records no completion
    // Starts from the existing default startup roster (no carry on initial start).
    TransitionToScenario(campaign->startScenarioId, std::nullopt);
}

void GameSession::AdvanceCampaignOnVictory() {
    if (campaignState_ != CampaignState::InProgress || !latchedOutcome_.has_value() ||
        latchedOutcome_->state != scenario::ScenarioOutcomeState::Victory) {
        return;
    }
    const data::CampaignDefinition* campaign = FindCampaignDefinition(campaignId_);
    if (campaign == nullptr) {
        return;
    }

    const auto nextId = campaign::ResolveNextScenarioId(
        *campaign, currentScenarioId_, scenario::ScenarioOutcomeState::Victory);
    if (!nextId.has_value()) {
        campaignState_ = CampaignState::Completed;   // final scenario won
        return;
    }

    // Resolve the OLD scenario entry's carry-over rule (default: carry nothing
    // but the player hero). fallbackGold targets the NEXT scenario's startGold.
    data::CarryOverRule rule;
    if (const data::CampaignScenarioEntry* entry = campaign->FindScenarioEntry(currentScenarioId_);
        entry != nullptr && !entry->carryOverRuleId.empty()) {
        if (const data::CarryOverRule* found = campaign->FindCarryOverRule(entry->carryOverRuleId)) {
            rule = *found;
        }
    }
    int fallbackGold = kDefaultStartGold;
    if (const data::ScenarioDefinition* next = FindScenarioDefinition(*nextId);
        next != nullptr && next->startGold.has_value()) {
        fallbackGold = *next->startGold;
    }

    const campaign::CampaignCarrySet captured = CaptureCarrySet();
    const campaign::CampaignCarrySet carried = campaign::ApplyCarryOver(
        rule, captured, PlayerHeroUnitId(), fallbackGold, ValidUnitIdSet());

    TransitionToScenario(*nextId, carried);
}

void GameSession::ResolveCampaignAfterOutcome() {
    if (campaignState_ != CampaignState::InProgress || !latchedOutcome_.has_value()) {
        return;
    }
    if (latchedOutcome_->state == scenario::ScenarioOutcomeState::Defeat) {
        campaignState_ = CampaignState::Failed;   // defeat ends the campaign run
        return;
    }
    if (latchedOutcome_->state == scenario::ScenarioOutcomeState::Victory) {
        AdvanceCampaignOnVictory();
    }
}

bool GameSession::IsCampaignActive() const {
    return campaignState_ != CampaignState::None;
}

CampaignState GameSession::GetCampaignState() const {
    return campaignState_;
}

const std::string& GameSession::CampaignId() const {
    return campaignId_;
}

const std::string& GameSession::CurrentScenarioId() const {
    return currentScenarioId_;
}

const std::vector<std::string>& GameSession::CompletedScenarioIds() const {
    return completedScenarioIds_;
}

const std::set<std::string>& GameSession::CampaignFlags() const {
    return campaignFlags_;
}

// ---------------------------------------------------------------------------
// M13-b — Inventory, artifact, and equipment runtime API.
// ---------------------------------------------------------------------------

void GameSession::SetItemCatalog(std::vector<data::ItemDefinition> catalog) {
    itemCatalog_ = std::move(catalog);
}

void GameSession::SetArtifactCatalog(std::vector<data::ArtifactDefinition> catalog) {
    artifactCatalog_ = std::move(catalog);
}

void GameSession::SetUnitCatalog(std::vector<data::UnitDefinition> catalog) {
    unitCatalog_ = std::move(catalog);
}

void GameSession::SetLocationServiceCatalog(std::vector<data::LocationServiceDefinition> catalog) {
    locationServiceCatalog_ = std::move(catalog);
}

void GameSession::SetTraderCurveCatalog(std::vector<data::TraderOwnershipCurve> catalog) {
    traderCurveCatalog_ = std::move(catalog);
}

// ---------------------------------------------------------------------------
// M15-b — World Map travel.
// ---------------------------------------------------------------------------

void GameSession::SetWorldMap(data::WorldMapDefinition worldMap) {
    worldMap_ = std::move(worldMap);
    // Seed the persisted runtime unlocked set from authored start state.
    unlockedRegionIds_.clear();
    for (const auto& entry : worldMap_.entries) {
        if (entry.unlocked) {
            unlockedRegionIds_.insert(entry.id);
        }
    }
}

void GameSession::SetRegionCatalog(std::vector<data::RegionDefinition> catalog) {
    regionCatalog_ = std::move(catalog);
}

const data::WorldMapDefinition& GameSession::WorldMap() const {
    return worldMap_;
}

bool GameSession::IsRegionUnlocked(const std::string& regionId) const {
    return unlockedRegionIds_.contains(regionId);
}

const data::RegionDefinition* GameSession::FindRegionDefinition(const std::string& id) const {
    for (const auto& region : regionCatalog_) {
        if (region.id == id) {
            return &region;
        }
    }
    return nullptr;
}

const data::UnitDefinition* GameSession::FindUnitDefinition(const std::string& id) const {
    for (const auto& unit : unitCatalog_) {
        if (unit.id == id) {
            return &unit;
        }
    }
    return nullptr;
}

bool GameSession::IsOnWorldMapExitNode() const {
    const auto* entry = worldMap_.FindEntry(regionId_);
    if (entry == nullptr) {
        return false;
    }
    return worldmap::IsWorldMapExitNode(*entry, destinationId_);
}

bool GameSession::CanOpenWorldMapHere() const {
    // Opening gate: only from the Region layer, standing on an exit node.
    return mode_ == GameMode::RegionMode && IsOnWorldMapExitNode();
}

bool GameSession::IsHeroUnit(const std::string& unitId) const {
    const auto it = std::find_if(unitCatalog_.begin(), unitCatalog_.end(),
        [&](const data::UnitDefinition& def) { return def.id == unitId; });
    if (it != unitCatalog_.end()) {
        return it->category == data::UnitDefinitionCategory::Hero
            || it->category == data::UnitDefinitionCategory::Leader;
    }
    // Not in catalog: fall back to the leader-capable signal (Hero/Leader units
    // are leader-capable). Unknown units default to generic (dropped on travel).
    return IsLeaderCapableUnitId(unitId);
}

std::vector<GameSession::TravelGenericLossEntry>
GameSession::PreviewRegionTravelGenericLosses() const {
    // Travel-loss rule (docs/core_loop_rules.md §5): only the traveling party
    // is at risk, i.e. stacks currently occupying an active or reserve slot.
    // Stationed (M25) and stored (M28) stacks are unslotted, so they are
    // excluded by construction and survive Region change. The Player Character
    // is excluded explicitly as a hard guard even though its Leader category
    // already makes it a hero.
    std::vector<TravelGenericLossEntry> losses;
    auto considerSlots = [&](const std::vector<std::string>& slotStackIds) {
        for (const auto& slotStackId : slotStackIds) {
            if (slotStackId.empty()) {
                continue;
            }
            const auto* stack = FindStackById(slotStackId);
            if (stack == nullptr) {
                continue;
            }
            if (IsHeroUnit(stack->unitId) || UnitIsPlayerCharacter(stack->unitId)) {
                continue;
            }
            // A corrupt double-slotted stack is still one stack: list it once.
            const bool alreadyListed = std::any_of(losses.begin(), losses.end(),
                [&](const TravelGenericLossEntry& entry) {
                    return entry.stackId == stack->stackId;
                });
            if (alreadyListed) {
                continue;
            }
            losses.push_back(TravelGenericLossEntry{
                stack->stackId, stack->unitId, std::max(0, stack->quantity) });
        }
    };
    considerSlots(activeSlotStackIds_);
    considerSlots(reserveSlotStackIds_);
    return losses;
}

int GameSession::GenericTravelingPartyUnitCount() const {
    int total = 0;
    for (const auto& entry : PreviewRegionTravelGenericLosses()) {
        total += entry.quantity;
    }
    return total;
}

int GameSession::RemoveGenericTravelingPartyUnits() {
    // Collect first (don't mutate roster while scanning), then remove exactly
    // the previewed set so the confirmed warning matches the applied loss.
    const auto losses = PreviewRegionTravelGenericLosses();

    int dropped = 0;
    for (const auto& loss : losses) {
        for (auto& activeSlot : activeSlotStackIds_) {
            if (activeSlot == loss.stackId) {
                activeSlot.clear();
            }
        }
        for (auto& reserveSlot : reserveSlotStackIds_) {
            if (reserveSlot == loss.stackId) {
                reserveSlot.clear();
            }
        }
        std::erase_if(rosterStacks_, [&](const RosterStackState& entry) {
            return entry.stackId == loss.stackId;
        });
        dropped += loss.quantity;
    }

    if (!losses.empty()) {
        MarkRosterProjectionDirty();
    }
    return dropped;
}

GameSession::WorldMapTravelResult GameSession::TravelToRegion(
    const std::string& destinationRegionId) {
    using worldmap::WorldMapTravelBlockReason;

    // Exit-node gate: legal from the Region layer OR the World Map screen (which
    // is itself opened from the exit node), and the current node must still be an
    // authored exit node. Battle / Location modes are rejected. NotAtExitNode is a
    // session-only reason; on failure nothing is mutated (no Energy spend, no
    // generic removal, no time advance, no region change).
    const bool onTravelableLayer =
        mode_ == GameMode::RegionMode || mode_ == GameMode::WorldMapMode;
    if (!onTravelableLayer || !IsOnWorldMapExitNode()) {
        return WorldMapTravelResult{ false, WorldMapTravelBlockReason::NotAtExitNode, 0, 0 };
    }

    const std::vector<std::string> unlocked(unlockedRegionIds_.begin(), unlockedRegionIds_.end());
    const auto eval = worldmap::EvaluateWorldMapTravel(
        regionId_,
        destinationRegionId,
        unlocked,
        worldMap_.adjacency,
        clock_.MinutesIntoSliceDay(),
        CurrentEnergy());
    if (!eval.legal) {
        return WorldMapTravelResult{ false, eval.reason, eval.days, 0 };
    }

    // Arrival node is owned by RegionDefinition (validated to exist at content
    // load). Defensive guard in case the region catalog is missing the entry.
    const auto* destRegion = FindRegionDefinition(destinationRegionId);
    if (destRegion == nullptr || destRegion->arrivalNodeId.empty()) {
        return WorldMapTravelResult{ false, WorldMapTravelBlockReason::NoPath, eval.days, 0 };
    }

    // Commit. Order matters: drop generics before advancing the clock so the
    // arrival-day Energy reset reflects the post-departure (heroes-only) party.
    static_cast<void>(TrySpendEnergy(1000));  // gate guaranteed >= 1000 above
    const int genericsDropped = RemoveGenericTravelingPartyUnits();

    const int elevenHundredMinutes = (11 - core::GameClock::kDayStartHour) * 60;
    const int delta = eval.days * core::GameClock::kMinutesPerSliceDay
        + elevenHundredMinutes - clock_.MinutesIntoSliceDay();
    AddMinutes(delta);  // day rollover → ApplyDailyStartingEnergy via the chokepoint

    regionId_ = destinationRegionId;
    destinationId_ = destRegion->arrivalNodeId;
    // Arrival drops back onto the Region layer at the destination's arrival node.
    // Travel can be committed from WorldMapMode, so the mode must be reset
    // explicitly here; otherwise the player would remain on the World Map screen
    // at 11:00 with every destination unavailable.
    mode_ = GameMode::RegionMode;

    return WorldMapTravelResult{ true, WorldMapTravelBlockReason::None, eval.days, genericsDropped };
}

int GameSession::LowestTravelingPartyAgility() const {
    // The traveling party is the active party plus the reserve. Both are
    // referenced by slot-id vectors into rosterStacks_. Resolve each occupied
    // slot to its unit, look up agility in the unit catalog, and take the
    // minimum. Units not present in the catalog are skipped (cannot resolve
    // agility). Returns 0 when nothing is resolvable so the formula floors at
    // the base 1000.
    int lowest = std::numeric_limits<int>::max();
    bool found = false;

    auto considerSlots = [&](const std::vector<std::string>& slotStackIds) {
        for (const auto& stackId : slotStackIds) {
            if (stackId.empty()) {
                continue;
            }
            const auto* stack = FindStackById(stackId);
            if (stack == nullptr || stack->quantity <= 0) {
                continue;
            }
            const auto it = std::find_if(unitCatalog_.begin(), unitCatalog_.end(),
                [&](const data::UnitDefinition& def) { return def.id == stack->unitId; });
            if (it == unitCatalog_.end()) {
                continue;
            }
            lowest = std::min(lowest, it->stats.agility);
            found = true;
        }
    };

    considerSlots(activeSlotStackIds_);
    considerSlots(reserveSlotStackIds_);

    return found ? lowest : 0;
}

const data::UnitDefinition* GameSession::CurrentLeaderUnitDefinition() const {
    // Resolve the active-party leader the same way battle does (BattleFactory
    // AssignLeader): the first player-character leader-capable unit, else the
    // first leader-capable unit, in active-slot order. Leader-capable = Leader or
    // Hero category. Generics are never the leader.
    const data::UnitDefinition* firstCapable = nullptr;
    for (const auto& stackId : activeSlotStackIds_) {
        if (stackId.empty()) {
            continue;
        }
        const auto* stack = FindStackById(stackId);
        if (stack == nullptr || stack->quantity <= 0) {
            continue;
        }
        const auto* def = FindUnitDefinition(stack->unitId);
        if (def == nullptr) {
            continue;
        }
        const bool capable = def->category == data::UnitDefinitionCategory::Leader
                          || def->category == data::UnitDefinitionCategory::Hero;
        if (!capable) {
            continue;
        }
        if (def->isPlayerCharacter) {
            return def;
        }
        if (firstCapable == nullptr) {
            firstCapable = def;
        }
    }
    return firstCapable;
}

int GameSession::LeaderPassiveEnergyBonus() const {
    const data::UnitDefinition* leader = CurrentLeaderUnitDefinition();
    if (leader == nullptr) {
        return 0;
    }
    int bonus = 0;
    for (const auto* effect : gameplay::effects::CollectUnitPassiveEffects(
             *leader, data::PassiveEffectKind::LeaderEnergy)) {
        if (effect->amount > 0) {
            bonus += effect->amount;
        }
    }
    return bonus;
}

void GameSession::ApplyDailyStartingEnergy() {
    // Y = the current leader's summed LeaderEnergy passive effects. Z = leader
    // equipped-item/artifact Energy bonus, still a zero-valued seam (deferred to
    // a future artifact-effect milestone; not faked here).
    const int starting = ComputeDailyStartingEnergy(
        LowestTravelingPartyAgility(),
        LeaderPassiveEnergyBonus(),
        /*leaderItemEnergyBonus=*/0);
    dailyMaxEnergy_ = starting;
    currentEnergy_ = starting;
}

int GameSession::CurrentEnergy() const {
    return currentEnergy_;
}

int GameSession::MaxEnergy() const {
    return dailyMaxEnergy_;
}

bool GameSession::CanSpendEnergy(const int amount) const {
    if (amount == 0) return true;
    if (amount < 0) return false;
    return currentEnergy_ >= amount;
}

bool GameSession::TrySpendEnergy(const int amount) {
    // Negative spend fails loudly (mutates nothing); zero spend is a harmless
    // success; positive spend is all-or-nothing.
    if (amount == 0) return true;
    if (amount < 0) return false;
    if (currentEnergy_ < amount) return false;
    currentEnergy_ -= amount;
    return true;
}

void GameSession::RecoverEnergy(const int amount) {
    if (amount <= 0) return;
    currentEnergy_ = std::min(dailyMaxEnergy_, currentEnergy_ + amount);
}

const std::vector<ItemStackState>& GameSession::Items() const {
    return items_;
}

const std::vector<ArtifactStackState>& GameSession::Artifacts() const {
    return artifacts_;
}

HeroEquipmentState GameSession::HeroEquipment(const std::string& heroId) const {
    const auto it = heroEquipment_.find(heroId);
    if (it == heroEquipment_.end()) {
        return HeroEquipmentState{};
    }
    return it->second;
}

const data::ItemDefinition* GameSession::FindItemDefinition(const std::string& id) const {
    for (const auto& def : itemCatalog_) {
        if (def.id == id) return &def;
    }
    return nullptr;
}

const data::ArtifactDefinition* GameSession::FindArtifactDefinition(const std::string& id) const {
    for (const auto& def : artifactCatalog_) {
        if (def.id == id) return &def;
    }
    return nullptr;
}

namespace {

// Maps an equip-slot to the matching allowed-slot kind. Misc1/Misc2/Misc3 all
// accept the Misc kind from ArtifactDefinition::allowedSlots.
data::ArtifactSlotKind RequiredSlotKindFor(ArtifactEquipSlot slot) {
    switch (slot) {
        case ArtifactEquipSlot::Attack:  return data::ArtifactSlotKind::Attack;
        case ArtifactEquipSlot::Defense: return data::ArtifactSlotKind::Defense;
        case ArtifactEquipSlot::Misc1:
        case ArtifactEquipSlot::Misc2:
        case ArtifactEquipSlot::Misc3:   return data::ArtifactSlotKind::Misc;
    }
    return data::ArtifactSlotKind::Misc;
}

std::string& SlotField(HeroEquipmentState& equipment, ArtifactEquipSlot slot) {
    switch (slot) {
        case ArtifactEquipSlot::Attack:  return equipment.attackArtifactId;
        case ArtifactEquipSlot::Defense: return equipment.defenseArtifactId;
        case ArtifactEquipSlot::Misc1:   return equipment.misc1ArtifactId;
        case ArtifactEquipSlot::Misc2:   return equipment.misc2ArtifactId;
        case ArtifactEquipSlot::Misc3:   return equipment.misc3ArtifactId;
    }
    return equipment.misc3ArtifactId;
}

const std::string& SlotField(const HeroEquipmentState& equipment, ArtifactEquipSlot slot) {
    switch (slot) {
        case ArtifactEquipSlot::Attack:  return equipment.attackArtifactId;
        case ArtifactEquipSlot::Defense: return equipment.defenseArtifactId;
        case ArtifactEquipSlot::Misc1:   return equipment.misc1ArtifactId;
        case ArtifactEquipSlot::Misc2:   return equipment.misc2ArtifactId;
        case ArtifactEquipSlot::Misc3:   return equipment.misc3ArtifactId;
    }
    return equipment.misc3ArtifactId;
}

} // namespace

void GameSession::SumEquippedArtifactBonuses(
    const std::string& heroId,
    int& attackBonus,
    int& defenseBonus,
    int& magicBonus,
    int& resistanceBonus) const
{
    attackBonus = 0;
    defenseBonus = 0;
    magicBonus = 0;
    resistanceBonus = 0;

    const auto it = heroEquipment_.find(heroId);
    if (it == heroEquipment_.end()) return;

    const HeroEquipmentState& equipment = it->second;
    const std::string* slotIds[] = {
        &equipment.attackArtifactId,
        &equipment.defenseArtifactId,
        &equipment.misc1ArtifactId,
        &equipment.misc2ArtifactId,
        &equipment.misc3ArtifactId,
    };

    for (const std::string* idPtr : slotIds) {
        if (idPtr->empty()) continue;
        const auto* def = FindArtifactDefinition(*idPtr);
        if (def == nullptr) continue;
        for (const auto& bonus : def->statBonuses) {
            switch (bonus.stat) {
                case data::ArtifactStatBonusStat::Attack:     attackBonus     += bonus.amount; break;
                case data::ArtifactStatBonusStat::Defense:    defenseBonus    += bonus.amount; break;
                case data::ArtifactStatBonusStat::Magic:      magicBonus      += bonus.amount; break;
                case data::ArtifactStatBonusStat::Resistance: resistanceBonus += bonus.amount; break;
            }
        }
    }
}

EquipResult GameSession::TryEquipArtifact(
    const std::string& heroId,
    ArtifactEquipSlot slot,
    const std::string& artifactId)
{
    if (heroId.empty()) {
        return {false, "TryEquipArtifact: heroId is empty"};
    }
    if (artifactId.empty()) {
        return {false, "TryEquipArtifact: artifactId is empty"};
    }

    // The hero must be present on the traveling team. Heroes are unique and
    // never stack, so OwnedUnitCount > 0 is the simplest presence check.
    if (OwnedUnitCount(heroId) <= 0) {
        return {false, "TryEquipArtifact: hero \"" + heroId + "\" is not on the traveling team"};
    }

    const auto* def = FindArtifactDefinition(artifactId);
    if (def == nullptr) {
        return {false, "TryEquipArtifact: unknown artifact id \"" + artifactId + "\""};
    }

    // Slot must be one of the artifact's allowed slots.
    const data::ArtifactSlotKind required = RequiredSlotKindFor(slot);
    const bool slotMatches = std::any_of(def->allowedSlots.begin(), def->allowedSlots.end(),
        [required](data::ArtifactSlotKind k) { return k == required; });
    if (!slotMatches) {
        return {false, "TryEquipArtifact: artifact \"" + artifactId
            + "\" cannot be equipped in slot " + ArtifactEquipSlotToString(slot)};
    }

    // Slot must not already hold something — caller must unequip first.
    HeroEquipmentState& equipment = heroEquipment_[heroId];
    std::string& slotRef = SlotField(equipment, slot);
    if (!slotRef.empty()) {
        return {false, "TryEquipArtifact: slot " + ArtifactEquipSlotToString(slot)
            + " on hero \"" + heroId + "\" is already occupied by \"" + slotRef + "\""};
    }

    // At least one unequipped copy must be available.
    auto stackIt = std::find_if(artifacts_.begin(), artifacts_.end(),
        [&](const ArtifactStackState& s) { return s.artifactId == artifactId; });
    if (stackIt == artifacts_.end() || stackIt->quantity <= 0) {
        return {false, "TryEquipArtifact: no unequipped copy of artifact \"" + artifactId + "\""};
    }

    // Commit: move exactly one copy into the slot.
    --stackIt->quantity;
    if (stackIt->quantity == 0) {
        artifacts_.erase(stackIt);
    }
    slotRef = artifactId;
    return {true, ""};
}

EquipResult GameSession::UnequipArtifact(
    const std::string& heroId,
    ArtifactEquipSlot slot)
{
    if (heroId.empty()) {
        return {false, "UnequipArtifact: heroId is empty"};
    }

    auto equipIt = heroEquipment_.find(heroId);
    if (equipIt == heroEquipment_.end()) {
        return {false, "UnequipArtifact: hero \"" + heroId + "\" has no equipped artifacts"};
    }

    std::string& slotRef = SlotField(equipIt->second, slot);
    if (slotRef.empty()) {
        return {false, "UnequipArtifact: slot " + ArtifactEquipSlotToString(slot)
            + " on hero \"" + heroId + "\" is empty"};
    }

    const std::string artifactId = slotRef;
    slotRef.clear();

    // Return the artifact to the unequipped inventory, respecting the 999 cap.
    auto stackIt = std::find_if(artifacts_.begin(), artifacts_.end(),
        [&](const ArtifactStackState& s) { return s.artifactId == artifactId; });
    if (stackIt == artifacts_.end()) {
        artifacts_.push_back({artifactId, 1});
    } else if (stackIt->quantity < 999) {
        ++stackIt->quantity;
    } else {
        // Pathological: returning an unequipped copy would exceed 999. This
        // should be unreachable in practice because equipping moved a copy
        // out of the stack, so the cap had room. We still fail explicitly
        // rather than silently dropping the artifact.
        slotRef = artifactId;  // restore the slot to keep state consistent
        return {false, "UnequipArtifact: returning artifact \"" + artifactId
            + "\" to inventory would exceed the 999-copy cap"};
    }

    return {true, ""};
}

} // namespace gameplay

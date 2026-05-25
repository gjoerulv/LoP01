#include "gameplay/GameSession.h"
#include "gameplay/events/EventEngine.h"
#include "gameplay/events/EventParser.h"
#include "gameplay/region/RegionTravelRules.h"

#include <algorithm>
#include <map>
#include <set>

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
        mode_ = GameMode::WorldMapMode;
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

void GameSession::EnterRegionMode() {
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
    clock_.AdvanceMinutes(std::max(1, remainingMinutes));
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
    clock_.AdvanceMinutes(core::GameClock::kMinutesPerSliceDay);
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
        ctx.resources["Gold"] = snap.gold;
        const auto& partyIds = ActivePartyUnitIds();
        ctx.heroIds = std::vector<std::string>(partyIds.begin(), partyIds.end());
        ctx.storyFlags = storyFlags_;
        ctx.pendingTeamMutations = &pendingMutations;

        if (!events::EvaluateCondition(ctx, def.condition)) continue;

        auto results = events::ExecuteActions(ctx, def.actions);
        allResults.insert(allResults.end(), results.begin(), results.end());

        if (ctx.resources.count("Gold")) {
            gold_ = ctx.resources.at("Gold");
        }
        storyFlags_ = ctx.storyFlags;

        if (def.repeat.mode == "once") {
            firedEventIds_.push_back(def.id);
        }
    }

    for (const auto& mut : pendingMutations) {
        for (auto& team : enemyTeams_) {
            if (team.teamColor != mut.teamColor) continue;
            if (mut.type == events::EnemyTeamMutationType::Spawn) {
                team.nodeId = mut.nodeId;
                team.active = true;
            } else if (mut.type == events::EnemyTeamMutationType::Remove) {
                team.active = false;
            } else if (mut.type == events::EnemyTeamMutationType::ChangeAlliance) {
                if (mut.addAlliance) {
                    if (std::ranges::find(team.alliances, mut.allyColor) == team.alliances.end())
                        team.alliances.push_back(mut.allyColor);
                } else {
                    std::erase(team.alliances, mut.allyColor);
                }
            }
            break;
        }
    }

    // M12-b ordering boundary #1: any authored regionNodeEntry / startOfDay event
    // may have removed teams, changed alliances, or set story flags that now
    // satisfy victory/defeat. Latch before the caller proceeds to enemy phase.
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

        entries.push_back(ActiveBattleStackEntry{
            slotIndex,
            stack->stackId,
            stack->unitId,
            stack->quantity
        });
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
        destinationId_
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
        latchedOutcome_.has_value() ? latchedOutcome_->reason : std::string{}
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
    case GameMode::WorldMapMode:
        return "overworld_selection";
    case GameMode::RegionMode:
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

    return GameMode::Title;
}

const std::vector<std::string> GameSession::kTeamColorOrder =
    { "Green", "Red", "Blue", "Yellow", "Purple", "Orange", "Teal", "White" };

void GameSession::SetEnemyTeams(std::vector<EnemyTeamState> teams) {
    enemyTeams_ = std::move(teams);
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

void GameSession::SetPlayerColor(std::string color) {
    playerColor_ = std::move(color);
}

const std::string& GameSession::PlayerColor() const {
    return playerColor_;
}

void GameSession::SetScenarioOutcomeDefinition(data::ScenarioOutcomeDefinition definition) {
    scenarioOutcomeDefinition_ = std::move(definition);
}

const std::optional<scenario::ScenarioOutcome>& GameSession::CheckAndLatchOutcome() {
    if (latchedOutcome_.has_value()) {
        return latchedOutcome_;
    }

    events::EventEvaluationContext condCtx;
    const auto snap = Snapshot();
    condCtx.currentDay = snap.day;
    condCtx.resources["Gold"] = snap.gold;
    const auto& partyIds = ActivePartyUnitIds();
    condCtx.heroIds = std::vector<std::string>(partyIds.begin(), partyIds.end());
    condCtx.storyFlags = storyFlags_;

    scenario::ScenarioOutcomeContext ctx;
    ctx.playerColor = playerColor_;
    ctx.enemyTeams = &enemyTeams_;
    ctx.conditionContext = &condCtx;

    const auto outcome = scenario::EvaluateScenarioOutcome(ctx, scenarioOutcomeDefinition_);
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

} // namespace gameplay

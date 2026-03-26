#include "gameplay/GameSession.h"

#include <algorithm>

namespace gameplay {


namespace
{
    bool IsValidOwnedEntry(const OwnedUnitCountState& entry) {
        return !entry.unitId.empty() && entry.count > 0;
    }

    bool OwnedUnitIdLess(const OwnedUnitCountState& left, const OwnedUnitCountState& right) {
        return left.unitId < right.unitId;
    }

    auto FindOwnedUnitIt(std::vector<OwnedUnitCountState>& owned, const std::string& unitId) {
        return std::lower_bound(
            owned.begin(),
            owned.end(),
            OwnedUnitCountState{unitId, 0},
            OwnedUnitIdLess);
    }

    auto FindOwnedUnitIt(const std::vector<OwnedUnitCountState>& owned, const std::string& unitId) {
        return std::lower_bound(
            owned.begin(),
            owned.end(),
            OwnedUnitCountState{unitId, 0},
            OwnedUnitIdLess);
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

int GameSession::ActivePartyCapacity() const {
    return std::max(0, activePartyCapacity_);
}

int GameSession::OwnedUnitCount(const std::string& unitId) const {
    if (unitId.empty()) {
        return 0;
    }

    const auto it = FindOwnedUnitIt(ownedUnitCounts_, unitId);
    if (it == ownedUnitCounts_.end() || it->unitId != unitId) {
        return 0;
    }

    return std::max(0, it->count);
}

int GameSession::ActivePartyAllocatedCount(const std::string& unitId) const {
    if (unitId.empty()) {
        return 0;
    }

    int allocated = 0;
    for (const auto& activeId : activePartyUnitIds_) {
        if (activeId == unitId) {
            ++allocated;
        }
    }

    return allocated;
}

int GameSession::ReserveUnitCount(const std::string& unitId) const {
    const int owned = OwnedUnitCount(unitId);
    const int allocated = ActivePartyAllocatedCount(unitId);
    return std::max(0, owned - allocated);
}

const std::vector<OwnedUnitCountState>& GameSession::OwnedUnitCounts() const {
    return ownedUnitCounts_;
}

const std::vector<std::string>& GameSession::ActivePartyUnitIds() const {
    return activePartyUnitIds_;
}

bool GameSession::AddOwnedUnit(const std::string& unitId, const int count) {
    if (unitId.empty() || count <= 0) {
        return false;
    }

    auto it = FindOwnedUnitIt(ownedUnitCounts_, unitId);
    if (it != ownedUnitCounts_.end() && it->unitId == unitId) {
        it->count += count;
        return true;
    }

    ownedUnitCounts_.insert(it, OwnedUnitCountState{unitId, count});
    return true;
}

bool GameSession::TryRemoveOwnedUnit(const std::string& unitId, const int count) {
    if (unitId.empty() || count <= 0) {
        return false;
    }

    auto it = FindOwnedUnitIt(ownedUnitCounts_, unitId);
    if (it == ownedUnitCounts_.end() || it->unitId != unitId) {
        return false;
    }

    const int owned = std::max(0, it->count);
    const int activeAllocated = ActivePartyAllocatedCount(unitId);
    if (owned < count || owned - count < activeAllocated) {
        return false;
    }

    it->count -= count;
    if (it->count <= 0) {
        ownedUnitCounts_.erase(it);
    }

    return true;
}

bool GameSession::TryAddUnitToActiveParty(const std::string& unitId) {
    if (unitId.empty()) {
        return false;
    }

    if (static_cast<int>(activePartyUnitIds_.size()) >= ActivePartyCapacity()) {
        return false;
    }

    if (ReserveUnitCount(unitId) <= 0) {
        return false;
    }

    activePartyUnitIds_.push_back(unitId);
    return true;
}

bool GameSession::TryRemoveActivePartyUnitAt(const int index) {
    if (index < 0 || index >= static_cast<int>(activePartyUnitIds_.size())) {
        return false;
    }

    activePartyUnitIds_.erase(activePartyUnitIds_.begin() + index);
    return true;
}

bool GameSession::TryMoveActivePartyUnit(const int fromIndex, const int toIndex) {
    if (fromIndex < 0 || fromIndex >= static_cast<int>(activePartyUnitIds_.size())) {
        return false;
    }

    if (toIndex < 0 || toIndex >= static_cast<int>(activePartyUnitIds_.size())) {
        return false;
    }

    if (fromIndex == toIndex) {
        return true;
    }

    const std::string moved = activePartyUnitIds_[fromIndex];
    activePartyUnitIds_.erase(activePartyUnitIds_.begin() + fromIndex);
    activePartyUnitIds_.insert(activePartyUnitIds_.begin() + toIndex, moved);
    return true;
}

void GameSession::ClearActiveParty() {
    activePartyUnitIds_.clear();
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
    std::vector<core::OwnedUnitCountSaveState> ownedUnitCounts;
    ownedUnitCounts.reserve(ownedUnitCounts_.size());
    for (const auto& entry : ownedUnitCounts_) {
        ownedUnitCounts.push_back(core::OwnedUnitCountSaveState{entry.unitId, entry.count});
    }

    return core::SaveData{
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
        std::move(ownedUnitCounts),
        activePartyUnitIds_
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
    recruitServiceStates_ = saveData.recruitServiceStates;
    dailyServiceStates_ = saveData.dailyServiceStates;
    travelPrepDiscountMinutes_ = std::max(0, saveData.travelPrepDiscountMinutes);
    travelPrepRemainingCharges_ = std::max(0, saveData.travelPrepRemainingCharges);
    travelPrepGrantedDay_ = std::max(0, saveData.travelPrepGrantedDay);

    ownedUnitCounts_.clear();
    ownedUnitCounts_.reserve(saveData.ownedUnitCounts.size());
    for (const auto& entry : saveData.ownedUnitCounts) {
        ownedUnitCounts_.push_back(OwnedUnitCountState{entry.unitId, entry.count});
    }

    activePartyUnitIds_ = saveData.activePartyUnitIds;
    NormalizeRosterState();
}

void GameSession::NormalizeRosterState() {
    std::vector<OwnedUnitCountState> normalizedOwned;
    normalizedOwned.reserve(ownedUnitCounts_.size());

    for (const auto& entry : ownedUnitCounts_) {
        if (!IsValidOwnedEntry(entry)) {
            continue;
        }

        auto it = FindOwnedUnitIt(normalizedOwned, entry.unitId);
        if (it != normalizedOwned.end() && it->unitId == entry.unitId) {
            it->count += entry.count;
        }
        else {
            normalizedOwned.insert(it, entry);
        }
    }

    ownedUnitCounts_ = std::move(normalizedOwned);

    std::vector<std::string> normalizedActive;
    normalizedActive.reserve(activePartyUnitIds_.size());

    for (const auto& unitId : activePartyUnitIds_) {
        if (unitId.empty()) {
            continue;
        }

        if (static_cast<int>(normalizedActive.size()) >= ActivePartyCapacity()) {
            break;
        }

        int alreadyAllocated = 0;
        for (const auto& allocatedId : normalizedActive) {
            if (allocatedId == unitId) {
                ++alreadyAllocated;
            }
        }

        if (alreadyAllocated >= OwnedUnitCount(unitId)) {
            continue;
        }

        normalizedActive.push_back(unitId);
    }

    activePartyUnitIds_ = std::move(normalizedActive);
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

#include "app/MusteringInteraction.h"

#include <algorithm>

#include "gameplay/GameSession.h"

namespace app {

void MusteringInteraction::Open(const gameplay::GameSession& session) {
    active_ = true;
    selectedReserveIndex_ = 0;
    selectedActiveIndex_ = 0;
    SanitizeSelectionIndices(session);
}

MusteringApplyResult MusteringInteraction::ApplyCommand(
    const MusteringCommand command,
    gameplay::GameSession& session) {
    MusteringApplyResult result;
    if (!active_) {
        return result;
    }

    if (command == MusteringCommand::None) {
        return result;
    }

    result.consumed = true;

    if (command == MusteringCommand::Exit) {
        result.shouldExit = true;
        result.statusText = "Exited mustering";
        return result;
    }

    if (command == MusteringCommand::SelectReservePrev || command == MusteringCommand::SelectReserveNext) {
        const auto reserveCandidates = BuildReserveCandidates(session);
        if (reserveCandidates.empty()) {
            selectedReserveIndex_ = -1;
            result.statusText = "No reserve units available";
            return result;
        }

        if (selectedReserveIndex_ < 0) {
            selectedReserveIndex_ = 0;
        }
        else if (command == MusteringCommand::SelectReservePrev) {
            selectedReserveIndex_ = std::max(0, selectedReserveIndex_ - 1);
        }
        else {
            selectedReserveIndex_ = std::min(static_cast<int>(reserveCandidates.size()) - 1, selectedReserveIndex_ + 1);
        }

        return result;
    }

    if (command == MusteringCommand::SelectActivePrev || command == MusteringCommand::SelectActiveNext) {
        const auto& activeParty = session.ActivePartyUnitIds();
        if (activeParty.empty()) {
            selectedActiveIndex_ = -1;
            result.statusText = "Active party is empty";
            return result;
        }

        if (selectedActiveIndex_ < 0) {
            selectedActiveIndex_ = 0;
        }
        else if (command == MusteringCommand::SelectActivePrev) {
            selectedActiveIndex_ = std::max(0, selectedActiveIndex_ - 1);
        }
        else {
            selectedActiveIndex_ = std::min(static_cast<int>(activeParty.size()) - 1, selectedActiveIndex_ + 1);
        }

        return result;
    }

    if (command == MusteringCommand::AddSelectedReserveToActive) {
        if (static_cast<int>(session.ActivePartyUnitIds().size()) >= session.ActivePartyCapacity()) {
            result.statusText = "Active party is full";
            return result;
        }

        const auto reserveCandidates = BuildReserveCandidates(session);
        if (reserveCandidates.empty()) {
            selectedReserveIndex_ = -1;
            result.statusText = "No reserve units available";
            return result;
        }

        const int reserveIndex = std::clamp(selectedReserveIndex_, 0, static_cast<int>(reserveCandidates.size()) - 1);
        const auto& selected = reserveCandidates[reserveIndex];

        if (!session.TryMoveReserveStackToActiveSlot(selected.reserveSlotIndex)) {
            result.statusText = "Could not move selected reserve stack";
            return result;
        }

        result.statusText = "Added " + selected.unitId + " to active party";
        SanitizeSelectionIndices(session);
        return result;
    }

    if (command == MusteringCommand::RemoveSelectedActive) {
        const auto& activeParty = session.ActivePartyUnitIds();
        if (activeParty.empty()) {
            selectedActiveIndex_ = -1;
            result.statusText = "Active party is empty";
            return result;
        }

        const int activeIndex = std::clamp(selectedActiveIndex_, 0, static_cast<int>(activeParty.size()) - 1);
        const std::string removedUnitId = activeParty[activeIndex];
        if (session.WouldRemovingActivePartyUnitLeaveNoLeader(activeIndex)) {
            result.statusText = "Active party must keep at least one leader-capable unit";
            return result;
        }

        if (!session.TryRemoveActivePartyUnitAt(activeIndex)) {
            result.statusText = "No reserve slot available";
            return result;
        }

        result.statusText = "Removed " + removedUnitId + " from active party";
        SanitizeSelectionIndices(session);
        return result;
    }

    return result;
}

void MusteringInteraction::Close() {
    active_ = false;
    selectedReserveIndex_ = -1;
    selectedActiveIndex_ = -1;
}

bool MusteringInteraction::IsActive() const {
    return active_;
}

std::string MusteringInteraction::BuildPromptText(const gameplay::GameSession& session) const {
    if (!active_) {
        return "Muster Party\nE: Open";
    }

    const auto reserveCandidates = BuildReserveCandidates(session);
    const auto& activeParty = session.ActivePartyUnitIds();

    std::string reserveLine;
    if (reserveCandidates.empty()) {
        reserveLine = "Reserve: none (0/0) [Left/Right]";
    }
    else {
        const int reserveIndex = std::clamp(selectedReserveIndex_, 0, static_cast<int>(reserveCandidates.size()) - 1);
        const auto& selectedReserve = reserveCandidates[reserveIndex];
        reserveLine = "Reserve: " + selectedReserve.unitId + " x" + std::to_string(selectedReserve.quantity) +
            " (" + std::to_string(reserveIndex + 1) + "/" + std::to_string(reserveCandidates.size()) + ") [Left/Right]";
    }

    std::string activeLine;
    if (activeParty.empty()) {
        activeLine = "Active: empty (0/0) [Up/Down]";
    }
    else {
        const int activeIndex = std::clamp(selectedActiveIndex_, 0, static_cast<int>(activeParty.size()) - 1);
        activeLine = "Active: " + activeParty[activeIndex] +
            " (" + std::to_string(activeIndex + 1) + "/" + std::to_string(activeParty.size()) + ") [Up/Down]";
    }

    return "Muster Party\n" + reserveLine + "\n" + activeLine + "\n1 Add  2 Remove  E Done";
}

std::vector<MusteringInteraction::ReserveCandidate> MusteringInteraction::BuildReserveCandidates(
    const gameplay::GameSession& session) const {
    std::vector<ReserveCandidate> result;

    const auto& reserveSlots = session.ReserveSlotStackIds();
    for (int slotIndex = 0; slotIndex < static_cast<int>(reserveSlots.size()); ++slotIndex) {
        const auto& stackId = reserveSlots[slotIndex];
        if (stackId.empty()) {
            continue;
        }

        const auto* stack = session.FindRosterStackById(stackId);
        if (stack == nullptr || stack->quantity <= 0) {
            continue;
        }

        result.push_back(ReserveCandidate{slotIndex, stack->stackId, stack->unitId, stack->quantity});
    }

    return result;
}

void MusteringInteraction::SanitizeSelectionIndices(const gameplay::GameSession& session) {
    const auto reserveCandidates = BuildReserveCandidates(session);
    if (reserveCandidates.empty()) {
        selectedReserveIndex_ = -1;
    }
    else {
        selectedReserveIndex_ = std::clamp(selectedReserveIndex_, 0, static_cast<int>(reserveCandidates.size()) - 1);
    }

    const auto& activeParty = session.ActivePartyUnitIds();
    if (activeParty.empty()) {
        selectedActiveIndex_ = -1;
    }
    else {
        selectedActiveIndex_ = std::clamp(selectedActiveIndex_, 0, static_cast<int>(activeParty.size()) - 1);
    }
}

} // namespace app

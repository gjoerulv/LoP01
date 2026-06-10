#include "app/StorageInteraction.h"

#include <algorithm>

#include "data/definitions/LocationServiceDefinition.h"
#include "gameplay/GameSession.h"
#include "gameplay/economy/StorageRules.h"

namespace app {

namespace {

const char* ListLabel(bool store) {
    return store ? "Store" : "Retrieve";
}

std::string CapacityText(const gameplay::GameSession& session, const std::string& serviceId) {
    const auto* owned = session.FindOwnedService(serviceId);
    const int count = owned == nullptr ? 0 : static_cast<int>(owned->storedUnits.size());
    return std::to_string(count) + "/" +
        std::to_string(gameplay::economy::kMaxStoredUnitsPerService);
}

} // namespace

void StorageInteraction::Open(
    const gameplay::GameSession& /*session*/,
    const data::LocationServiceDefinition& service) {
    active_ = true;
    serviceId_ = service.id;
    mode_ = ListMode::Store;
    selectedIndex_ = 0;
    lastResult_.clear();
}

int StorageInteraction::CurrentListSize(const gameplay::GameSession& session) const {
    if (mode_ == ListMode::Store) {
        return static_cast<int>(session.EligibleStorageStackIds(serviceId_).size());
    }
    const auto* owned = session.FindOwnedService(serviceId_);
    return owned == nullptr ? 0 : static_cast<int>(owned->storedUnits.size());
}

std::string StorageInteraction::ConfirmSelection(gameplay::GameSession& session) {
    if (mode_ == ListMode::Store) {
        const auto eligible = session.EligibleStorageStackIds(serviceId_);
        if (eligible.empty()) {
            return "No eligible units to store";
        }
        const int index = std::clamp(selectedIndex_, 0, static_cast<int>(eligible.size()) - 1);
        const std::string stackId = eligible[index];
        const auto* stack = session.FindRosterStackById(stackId);
        const std::string unitId = stack != nullptr ? stack->unitId : stackId;

        const bool ok = session.TryStoreStackAtService(serviceId_, stackId);
        selectedIndex_ = 0;
        if (!ok) {
            return "Cannot store " + unitId + " here";
        }
        return "Stored " + unitId + " (" + CapacityText(session, serviceId_) + ")";
    }

    const auto* owned = session.FindOwnedService(serviceId_);
    const int count = owned == nullptr ? 0 : static_cast<int>(owned->storedUnits.size());
    if (count == 0) {
        return "No units stored here";
    }
    const int index = std::clamp(selectedIndex_, 0, count - 1);
    const std::string stackId = owned->storedUnits[index].stackId;
    const std::string unitId = owned->storedUnits[index].unitId;

    const bool ok = session.TryRetrieveStackFromService(serviceId_, stackId);
    selectedIndex_ = 0;
    if (!ok) {
        return "Cannot retrieve " + unitId + " (reserve full?)";
    }
    return "Retrieved " + unitId + " to reserve";
}

StorageApplyResult StorageInteraction::ApplyCommand(
    const StorageCommand command, gameplay::GameSession& session) {
    StorageApplyResult result;
    if (!active_ || command == StorageCommand::None) {
        return result;
    }
    result.consumed = true;

    switch (command) {
        case StorageCommand::Exit: {
            result.shouldExit = true;
            result.statusText = "Left storage";
            return result;
        }
        case StorageCommand::CycleList: {
            mode_ = mode_ == ListMode::Store ? ListMode::Retrieve : ListMode::Store;
            selectedIndex_ = 0;
            lastResult_.clear();
            result.statusText = std::string("List: ") + ListLabel(mode_ == ListMode::Store);
            return result;
        }
        case StorageCommand::SelectPrev:
        case StorageCommand::SelectNext: {
            const int count = CurrentListSize(session);
            if (count <= 0) {
                return result;
            }
            const int delta = command == StorageCommand::SelectPrev ? -1 : 1;
            selectedIndex_ = std::clamp(selectedIndex_ + delta, 0, count - 1);
            return result;
        }
        case StorageCommand::Confirm: {
            lastResult_ = ConfirmSelection(session);
            result.statusText = lastResult_;
            return result;
        }
        case StorageCommand::None:
            return result;
    }
    return result;
}

void StorageInteraction::Close() {
    active_ = false;
    serviceId_.clear();
    mode_ = ListMode::Store;
    selectedIndex_ = 0;
    lastResult_.clear();
}

bool StorageInteraction::IsActive() const {
    return active_;
}

std::string StorageInteraction::BuildPromptText(const gameplay::GameSession& session) const {
    if (!active_) {
        return "Storage\nE: Open";
    }

    std::string out = "Storage — store/retrieve units (" + CapacityText(session, serviceId_) + ")\n";
    out += std::string("List: ") + ListLabel(mode_ == ListMode::Store) + "\n";

    if (mode_ == ListMode::Store) {
        const auto eligible = session.EligibleStorageStackIds(serviceId_);
        if (eligible.empty()) {
            out += "> (no eligible units)\n";
        } else {
            const int index = std::clamp(selectedIndex_, 0, static_cast<int>(eligible.size()) - 1);
            const std::string stackId = eligible[index];
            const auto* stack = session.FindRosterStackById(stackId);
            const std::string unitId = stack != nullptr ? stack->unitId : stackId;
            const int quantity = stack != nullptr ? stack->quantity : 0;
            out += "> " + unitId + " x" + std::to_string(quantity) + " (" +
                std::to_string(index + 1) + "/" + std::to_string(eligible.size()) +
                ") [Left/Right]\n";
        }
    } else {
        const auto* owned = session.FindOwnedService(serviceId_);
        const int count = owned == nullptr ? 0 : static_cast<int>(owned->storedUnits.size());
        if (count == 0) {
            out += "> (none stored)\n";
        } else {
            const int index = std::clamp(selectedIndex_, 0, count - 1);
            const auto& ref = owned->storedUnits[index];
            const auto* stack = session.FindRosterStackById(ref.stackId);
            const int quantity = stack != nullptr ? stack->quantity : 0;
            out += "> " + ref.unitId + " x" + std::to_string(quantity) + " (" +
                std::to_string(index + 1) + "/" + std::to_string(count) + ") [Left/Right]\n";
        }
    }

    if (!lastResult_.empty()) {
        out += lastResult_ + "\n";
    }
    out += "2 List  1 Confirm  E Done";
    return out;
}

} // namespace app

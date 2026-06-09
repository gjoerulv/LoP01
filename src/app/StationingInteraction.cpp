#include "app/StationingInteraction.h"

#include <algorithm>

#include "data/definitions/LocationServiceDefinition.h"
#include "gameplay/GameSession.h"
#include "gameplay/economy/StationingRules.h"

namespace app {

namespace {

const char* ListLabel(bool station) {
    return station ? "Station" : "Unstation";
}

std::string CapacityText(const gameplay::GameSession& session, const std::string& serviceId) {
    const auto* owned = session.FindOwnedService(serviceId);
    const int count = owned == nullptr ? 0 : static_cast<int>(owned->stationedUnits.size());
    return std::to_string(count) + "/" +
        std::to_string(gameplay::economy::kMaxStationedUnitsPerService);
}

} // namespace

void StationingInteraction::Open(
    const gameplay::GameSession& session,
    const data::LocationServiceDefinition& service) {
    active_ = true;
    serviceId_ = service.id;
    mode_ = ListMode::Station;
    selectedIndex_ = 0;
    stationQuantity_ = std::max(1, SelectedStationStackQuantity(session));
    lastResult_.clear();
}

int StationingInteraction::CurrentListSize(const gameplay::GameSession& session) const {
    if (mode_ == ListMode::Station) {
        return static_cast<int>(session.EligibleStationingStackIds(serviceId_).size());
    }
    const auto* owned = session.FindOwnedService(serviceId_);
    return owned == nullptr ? 0 : static_cast<int>(owned->stationedUnits.size());
}

int StationingInteraction::SelectedStationStackQuantity(const gameplay::GameSession& session) const {
    const auto eligible = session.EligibleStationingStackIds(serviceId_);
    if (eligible.empty()) {
        return 0;
    }
    const int index = std::clamp(selectedIndex_, 0, static_cast<int>(eligible.size()) - 1);
    const auto* stack = session.FindRosterStackById(eligible[index]);
    return stack != nullptr ? stack->quantity : 0;
}

std::string StationingInteraction::ConfirmSelection(gameplay::GameSession& session) {
    if (mode_ == ListMode::Station) {
        const auto eligible = session.EligibleStationingStackIds(serviceId_);
        if (eligible.empty()) {
            return "No eligible units to station";
        }
        const int index = std::clamp(selectedIndex_, 0, static_cast<int>(eligible.size()) - 1);
        const std::string stackId = eligible[index];
        const auto* stack = session.FindRosterStackById(stackId);
        const std::string unitId = stack != nullptr ? stack->unitId : stackId;
        const int quantity = stack != nullptr ? stack->quantity : 0;

        // Default stations the whole stack; reducing the quantity splits a generic.
        const bool splitting = quantity > 1 && stationQuantity_ < quantity;
        const bool ok = splitting
            ? session.TryStationSplitAtService(serviceId_, stackId, stationQuantity_)
            : session.TryStationStackAtService(serviceId_, stackId);

        selectedIndex_ = 0;
        stationQuantity_ = std::max(1, SelectedStationStackQuantity(session));
        if (!ok) {
            return "Cannot station " + unitId + " here";
        }
        return "Stationed " + unitId + " (" + CapacityText(session, serviceId_) + ")";
    }

    const auto* owned = session.FindOwnedService(serviceId_);
    const int count = owned == nullptr ? 0 : static_cast<int>(owned->stationedUnits.size());
    if (count == 0) {
        return "No units stationed here";
    }
    const int index = std::clamp(selectedIndex_, 0, count - 1);
    const std::string stackId = owned->stationedUnits[index].stackId;
    const std::string unitId = owned->stationedUnits[index].unitId;

    const bool ok = session.TryUnstationStackFromService(serviceId_, stackId);
    selectedIndex_ = 0;
    if (!ok) {
        return "Cannot unstation " + unitId + " (reserve full?)";
    }
    return "Unstationed " + unitId + " to reserve";
}

StationingApplyResult StationingInteraction::ApplyCommand(
    const StationingCommand command, gameplay::GameSession& session) {
    StationingApplyResult result;
    if (!active_ || command == StationingCommand::None) {
        return result;
    }
    result.consumed = true;

    switch (command) {
        case StationingCommand::Exit: {
            result.shouldExit = true;
            result.statusText = "Left the mine";
            return result;
        }
        case StationingCommand::CycleList: {
            mode_ = mode_ == ListMode::Station ? ListMode::Unstation : ListMode::Station;
            selectedIndex_ = 0;
            stationQuantity_ = std::max(1, SelectedStationStackQuantity(session));
            lastResult_.clear();
            result.statusText = std::string("List: ") + ListLabel(mode_ == ListMode::Station);
            return result;
        }
        case StationingCommand::SelectPrev:
        case StationingCommand::SelectNext: {
            const int count = CurrentListSize(session);
            if (count <= 0) {
                return result;
            }
            const int delta = command == StationingCommand::SelectPrev ? -1 : 1;
            selectedIndex_ = std::clamp(selectedIndex_ + delta, 0, count - 1);
            // Default to the whole newly-selected stack (Station list only).
            stationQuantity_ = std::max(1, SelectedStationStackQuantity(session));
            return result;
        }
        case StationingCommand::QuantityDown:
        case StationingCommand::QuantityUp: {
            const int delta = command == StationingCommand::QuantityDown ? -1 : 1;
            const int maxQuantity = std::max(1, SelectedStationStackQuantity(session));
            stationQuantity_ = std::clamp(stationQuantity_ + delta, 1, maxQuantity);
            return result;
        }
        case StationingCommand::Confirm: {
            lastResult_ = ConfirmSelection(session);
            result.statusText = lastResult_;
            return result;
        }
        case StationingCommand::None:
            return result;
    }
    return result;
}

void StationingInteraction::Close() {
    active_ = false;
    serviceId_.clear();
    mode_ = ListMode::Station;
    selectedIndex_ = 0;
    stationQuantity_ = 1;
    lastResult_.clear();
}

bool StationingInteraction::IsActive() const {
    return active_;
}

std::string StationingInteraction::BuildPromptText(const gameplay::GameSession& session) const {
    if (!active_) {
        return "Mine\nE: Open";
    }

    std::string out = "Mine — station workers/guards (" + CapacityText(session, serviceId_) + ")\n";
    out += std::string("List: ") + ListLabel(mode_ == ListMode::Station) + "\n";

    if (mode_ == ListMode::Station) {
        const auto eligible = session.EligibleStationingStackIds(serviceId_);
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
            if (quantity > 1) {
                const int q = std::clamp(stationQuantity_, 1, quantity);
                out += "Station qty: " + std::to_string(q) + " of " +
                    std::to_string(quantity) + " [Up/Down]\n";
            }
        }
    } else {
        const auto* owned = session.FindOwnedService(serviceId_);
        const int count = owned == nullptr ? 0 : static_cast<int>(owned->stationedUnits.size());
        if (count == 0) {
            out += "> (none stationed)\n";
        } else {
            const int index = std::clamp(selectedIndex_, 0, count - 1);
            const auto& ref = owned->stationedUnits[index];
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

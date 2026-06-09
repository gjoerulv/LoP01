#pragma once

#include <string>

namespace gameplay {
class GameSession;
}

namespace data {
struct LocationServiceDefinition;
}

namespace app {

enum class StationingCommand {
    None,
    SelectPrev,    // previous entry in the current list
    SelectNext,    // next entry
    QuantityDown,  // decrease the split quantity (min 1), Station list only
    QuantityUp,    // increase the split quantity
    CycleList,     // toggle the Station <-> Unstation list
    Confirm,       // station the selected eligible stack / unstation the selected one
    Exit           // leave the visit
};

struct StationingApplyResult {
    bool consumed = false;
    bool shouldExit = false;
    std::string statusText;
};

// Bounded, text-prompt stationing visit at a player-owned mine. Mirrors
// TradingPostInteraction: the App owns one instance, opens it from the mine
// service dispatch, routes input commands to ApplyCommand while IsActive(), and
// renders BuildPromptText. ALL roster/stationing mutation goes through the
// GameSession Try*/Can* stationing API — this object holds only transient
// selection state and never edits stationedUnits or roster slots directly. The
// eligible/stationed lists are read fresh from the session each command/render
// (bounded N), so mid-visit changes stay consistent.
class StationingInteraction {
public:
    void Open(const gameplay::GameSession& session, const data::LocationServiceDefinition& service);
    StationingApplyResult ApplyCommand(StationingCommand command, gameplay::GameSession& session);
    void Close();
    [[nodiscard]] bool IsActive() const;
    [[nodiscard]] std::string BuildPromptText(const gameplay::GameSession& session) const;

private:
    enum class ListMode { Station, Unstation };

    [[nodiscard]] int CurrentListSize(const gameplay::GameSession& session) const;
    // Quantity (1..full) of the currently selected eligible Station entry, or 0
    // when there is none. Used to default and clamp stationQuantity_.
    [[nodiscard]] int SelectedStationStackQuantity(const gameplay::GameSession& session) const;
    [[nodiscard]] std::string ConfirmSelection(gameplay::GameSession& session);

    bool active_ = false;
    std::string serviceId_;
    ListMode mode_ = ListMode::Station;
    int selectedIndex_ = 0;
    // Station-list only: how many units of the selected stack to station. Defaults
    // to the whole stack (set on selection); reducing it below the stack size
    // splits a generic stack so the remainder stays with the party.
    int stationQuantity_ = 1;
    std::string lastResult_;
};

} // namespace app

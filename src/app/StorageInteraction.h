#pragma once

#include <string>

namespace gameplay {
class GameSession;
}

namespace data {
struct LocationServiceDefinition;
}

namespace app {

enum class StorageCommand {
    None,
    SelectPrev,  // previous entry in the current list
    SelectNext,  // next entry
    CycleList,   // toggle the Store <-> Retrieve list
    Confirm,     // store the selected eligible stack / retrieve the selected one
    Exit         // leave the visit
};

struct StorageApplyResult {
    bool consumed = false;
    bool shouldExit = false;
    std::string statusText;
};

// Bounded, text-prompt storage visit at a player-owned storage service. Mirrors
// StationingInteraction but for the M28 storage bucket (whole-stack only, no
// split/quantity). The App owns one instance, opens it from the storage service
// dispatch, routes commands to ApplyCommand while IsActive(), and renders
// BuildPromptText. ALL roster/storage mutation goes through the GameSession
// Try*/Can* storage API — this object holds only transient selection state and
// never edits storedUnits or roster slots directly.
class StorageInteraction {
public:
    void Open(const gameplay::GameSession& session, const data::LocationServiceDefinition& service);
    StorageApplyResult ApplyCommand(StorageCommand command, gameplay::GameSession& session);
    void Close();
    [[nodiscard]] bool IsActive() const;
    [[nodiscard]] std::string BuildPromptText(const gameplay::GameSession& session) const;

private:
    enum class ListMode { Store, Retrieve };

    [[nodiscard]] int CurrentListSize(const gameplay::GameSession& session) const;
    [[nodiscard]] std::string ConfirmSelection(gameplay::GameSession& session);

    bool active_ = false;
    std::string serviceId_;
    ListMode mode_ = ListMode::Store;
    int selectedIndex_ = 0;
    std::string lastResult_;
};

} // namespace app

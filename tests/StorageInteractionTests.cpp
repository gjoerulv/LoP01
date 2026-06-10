#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

#include "app/StorageInteraction.h"
#include "core/SaveGame.h"
#include "data/definitions/LocationServiceDefinition.h"
#include "data/definitions/UnitDefinition.h"
#include "gameplay/GameSession.h"

namespace {

data::UnitDefinition MakeUnit(const std::string& id,
    data::UnitDefinitionCategory category, bool isPlayerCharacter = false) {
    data::UnitDefinition u;
    u.id = id;
    u.name = id;
    u.category = category;
    u.isPlayerCharacter = isPlayerCharacter;
    return u;
}

data::LocationServiceDefinition MakeStorage(const std::string& id) {
    data::LocationServiceDefinition svc;
    svc.id = id;
    svc.kind = data::LocationServiceKind::Storage;
    return svc;
}

// pc_hero active; kobold and plain in reserve; one player-owned storage.
gameplay::GameSession MakeSession() {
    core::SaveData s;
    s.schemaVersion = 5;
    s.day = 1;
    s.mode = "region_mode";
    s.hasCanonicalRoster = true;
    s.rosterStacks = {
        core::RosterStackSaveState{"stk_1", "pc_hero", 1},
        core::RosterStackSaveState{"stk_2", "kobold", 3},
        core::RosterStackSaveState{"stk_3", "plain", 2},
    };
    s.activeSlotStackIds = {"stk_1", "", "", "", ""};
    s.reserveSlotStackIds = {"stk_2", "stk_3", "", "", "", "", "", ""};
    s.nextStackIdCounter = 4;
    s.ownedServices = {core::OwnedServiceSaveState{"home_storage", "Green", false, false, {}, {}}};

    gameplay::GameSession session;
    session.SetUnitCatalog({
        MakeUnit("pc_hero", data::UnitDefinitionCategory::Hero, /*isPlayerCharacter=*/true),
        MakeUnit("kobold", data::UnitDefinitionCategory::Generic),
        MakeUnit("plain", data::UnitDefinitionCategory::Generic),
    });
    session.SetLocationServiceCatalog({MakeStorage("home_storage")});
    session.ApplySaveData(s);
    return session;
}

int StoredCount(const gameplay::GameSession& session) {
    const auto* owned = session.FindOwnedService("home_storage");
    return owned == nullptr ? -1 : static_cast<int>(owned->storedUnits.size());
}

} // namespace

using app::StorageCommand;
using app::StorageInteraction;

TEST_CASE("StorageInteraction - confirm in the store list stores the selected unit") {
    auto session = MakeSession();
    StorageInteraction interaction;
    interaction.Open(session, MakeStorage("home_storage"));
    REQUIRE(interaction.IsActive());

    const auto result = interaction.ApplyCommand(StorageCommand::Confirm, session);
    REQUIRE(result.consumed);
    REQUIRE(StoredCount(session) == 1);
    REQUIRE(session.FindOwnedService("home_storage")->storedUnits[0].stackId == "stk_2");
}

TEST_CASE("StorageInteraction - cycling to the retrieve list returns a stored unit") {
    auto session = MakeSession();
    REQUIRE(session.TryStoreStackAtService("home_storage", "stk_2"));

    StorageInteraction interaction;
    interaction.Open(session, MakeStorage("home_storage"));
    interaction.ApplyCommand(StorageCommand::CycleList, session);  // -> Retrieve
    const auto result = interaction.ApplyCommand(StorageCommand::Confirm, session);

    REQUIRE(result.consumed);
    REQUIRE(StoredCount(session) == 0);
    REQUIRE(session.FindRosterStackById("stk_2") != nullptr);
    REQUIRE(session.OwnedUnitCount("kobold") == 3);
}

TEST_CASE("StorageInteraction - exit signals close and stops consuming") {
    auto session = MakeSession();
    StorageInteraction interaction;
    interaction.Open(session, MakeStorage("home_storage"));

    const auto result = interaction.ApplyCommand(StorageCommand::Exit, session);
    REQUIRE(result.shouldExit);
    interaction.Close();
    REQUIRE_FALSE(interaction.IsActive());

    const auto ignored = interaction.ApplyCommand(StorageCommand::Confirm, session);
    REQUIRE_FALSE(ignored.consumed);
}

TEST_CASE("StorageInteraction - Close resets an open visit") {
    auto session = MakeSession();
    StorageInteraction interaction;
    interaction.Open(session, MakeStorage("home_storage"));
    REQUIRE(interaction.IsActive());
    interaction.Close();
    REQUIRE_FALSE(interaction.IsActive());
}

TEST_CASE("StorageInteraction - prompt text reflects the active list and capacity") {
    auto session = MakeSession();
    StorageInteraction interaction;
    interaction.Open(session, MakeStorage("home_storage"));

    const std::string storePrompt = interaction.BuildPromptText(session);
    REQUIRE(storePrompt.find("Store") != std::string::npos);
    REQUIRE(storePrompt.find("0/7") != std::string::npos);

    interaction.ApplyCommand(StorageCommand::CycleList, session);
    const std::string retrievePrompt = interaction.BuildPromptText(session);
    REQUIRE(retrievePrompt.find("Retrieve") != std::string::npos);
}

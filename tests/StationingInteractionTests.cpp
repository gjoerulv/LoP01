#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

#include "app/StationingInteraction.h"
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

data::LocationServiceDefinition MakeMine(const std::string& id) {
    data::LocationServiceDefinition svc;
    svc.id = id;
    svc.kind = data::LocationServiceKind::Mine;
    return svc;
}

// pc_hero active; kobold (x3) and plain (x2) in reserve; one player-owned mine.
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
    s.ownedServices = {core::OwnedServiceSaveState{"iron_mine_svc", "Green", false, false, {}}};

    gameplay::GameSession session;
    session.SetUnitCatalog({
        MakeUnit("pc_hero", data::UnitDefinitionCategory::Hero, /*isPlayerCharacter=*/true),
        MakeUnit("kobold", data::UnitDefinitionCategory::Generic),
        MakeUnit("plain", data::UnitDefinitionCategory::Generic),
    });
    session.SetLocationServiceCatalog({MakeMine("iron_mine_svc")});
    session.ApplySaveData(s);
    return session;
}

int StationedCount(const gameplay::GameSession& session) {
    const auto* owned = session.FindOwnedService("iron_mine_svc");
    return owned == nullptr ? -1 : static_cast<int>(owned->stationedUnits.size());
}

} // namespace

using app::StationingCommand;
using app::StationingInteraction;

TEST_CASE("StationingInteraction - confirm in the station list stations the selected unit") {
    auto session = MakeSession();
    StationingInteraction interaction;
    interaction.Open(session, MakeMine("iron_mine_svc"));
    REQUIRE(interaction.IsActive());

    // Default list is Station; first eligible entry is stk_2 (kobold).
    const auto result = interaction.ApplyCommand(StationingCommand::Confirm, session);
    REQUIRE(result.consumed);
    REQUIRE(StationedCount(session) == 1);
    REQUIRE(session.FindOwnedService("iron_mine_svc")->stationedUnits[0].stackId == "stk_2");
}

TEST_CASE("StationingInteraction - cycling to the unstation list returns a stationed unit") {
    auto session = MakeSession();
    REQUIRE(session.TryStationStackAtService("iron_mine_svc", "stk_2"));

    StationingInteraction interaction;
    interaction.Open(session, MakeMine("iron_mine_svc"));
    interaction.ApplyCommand(StationingCommand::CycleList, session);  // -> Unstation
    const auto result = interaction.ApplyCommand(StationingCommand::Confirm, session);

    REQUIRE(result.consumed);
    REQUIRE(StationedCount(session) == 0);
    // Returned to reserve, same id, still owned.
    REQUIRE(session.FindRosterStackById("stk_2") != nullptr);
    REQUIRE(session.OwnedUnitCount("kobold") == 3);
}

TEST_CASE("StationingInteraction - reducing the quantity splits a generic stack") {
    auto session = MakeSession();
    StationingInteraction interaction;
    interaction.Open(session, MakeMine("iron_mine_svc"));

    // kobold (3) selected; default quantity is the whole stack. Reduce to 1.
    interaction.ApplyCommand(StationingCommand::QuantityDown, session);  // 2
    interaction.ApplyCommand(StationingCommand::QuantityDown, session);  // 1
    const auto result = interaction.ApplyCommand(StationingCommand::Confirm, session);
    REQUIRE(result.consumed);

    // A new split stack of 1 was stationed; the source kept 2 in reserve.
    REQUIRE(StationedCount(session) == 1);
    const std::string newId = session.FindOwnedService("iron_mine_svc")->stationedUnits[0].stackId;
    REQUIRE(newId != "stk_2");
    REQUIRE(session.FindRosterStackById("stk_2")->quantity == 2);
    REQUIRE(session.FindRosterStackById(newId)->quantity == 1);
    REQUIRE(session.OwnedUnitCount("kobold") == 3);
}

TEST_CASE("StationingInteraction - the default quantity stations the whole stack") {
    auto session = MakeSession();
    StationingInteraction interaction;
    interaction.Open(session, MakeMine("iron_mine_svc"));

    // No quantity change: kobold (3) moves whole, keeping its stack id.
    interaction.ApplyCommand(StationingCommand::Confirm, session);

    REQUIRE(StationedCount(session) == 1);
    REQUIRE(session.FindOwnedService("iron_mine_svc")->stationedUnits[0].stackId == "stk_2");
    REQUIRE(session.FindRosterStackById("stk_2")->quantity == 3);
}

TEST_CASE("StationingInteraction - exit signals close and stops consuming") {
    auto session = MakeSession();
    StationingInteraction interaction;
    interaction.Open(session, MakeMine("iron_mine_svc"));

    const auto result = interaction.ApplyCommand(StationingCommand::Exit, session);
    REQUIRE(result.shouldExit);
    interaction.Close();
    REQUIRE_FALSE(interaction.IsActive());

    // After close, commands are ignored.
    const auto ignored = interaction.ApplyCommand(StationingCommand::Confirm, session);
    REQUIRE_FALSE(ignored.consumed);
}

TEST_CASE("StationingInteraction - prompt text reflects the active list and capacity") {
    auto session = MakeSession();
    StationingInteraction interaction;
    interaction.Open(session, MakeMine("iron_mine_svc"));

    const std::string stationPrompt = interaction.BuildPromptText(session);
    REQUIRE(stationPrompt.find("Station") != std::string::npos);
    REQUIRE(stationPrompt.find("0/5") != std::string::npos);

    interaction.ApplyCommand(StationingCommand::CycleList, session);
    const std::string unstationPrompt = interaction.BuildPromptText(session);
    REQUIRE(unstationPrompt.find("Unstation") != std::string::npos);
}

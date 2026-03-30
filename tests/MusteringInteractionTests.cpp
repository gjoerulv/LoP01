#include <catch2/catch_test_macros.hpp>

#include "app/MusteringInteraction.h"
#include "gameplay/GameSession.h"

TEST_CASE("Open_InitializesSelectionFromSessionState") {
    gameplay::GameSession session;
    REQUIRE(session.AddOwnedUnit("unit_guard", 1));
    REQUIRE(session.AddOwnedUnit("unit_medic", 1));
    REQUIRE(session.TryAddUnitToActiveParty("unit_guard"));

    app::MusteringInteraction mustering;
    mustering.Open(session);

    REQUIRE(mustering.IsActive());
    const std::string prompt = mustering.BuildPromptText(session);
    REQUIRE(prompt.find("Reserve: unit_medic") != std::string::npos);
    REQUIRE(prompt.find("Active: unit_guard") != std::string::npos);
}

TEST_CASE("AddSelectedReserve_AppendsToActivePartyEnd") {
    gameplay::GameSession session;
    REQUIRE(session.AddOwnedUnit("unit_guard", 1));
    REQUIRE(session.AddOwnedUnit("unit_medic", 1));
    REQUIRE(session.TryAddUnitToActiveParty("unit_guard"));

    app::MusteringInteraction mustering;
    mustering.Open(session);

    mustering.ApplyCommand(app::MusteringCommand::SelectReserveNext, session);
    const auto addResult = mustering.ApplyCommand(app::MusteringCommand::AddSelectedReserveToActive, session);

    REQUIRE(addResult.consumed);
    REQUIRE(addResult.statusText.find("Added unit_medic") != std::string::npos);

    const auto& active = session.ActivePartyUnitIds();
    REQUIRE(active.size() == 2);
    REQUIRE(active[0] == "unit_guard");
    REQUIRE(active[1] == "unit_medic");
}

TEST_CASE("RemoveSelectedActive_RemovesAndShiftsLeft") {
    gameplay::GameSession session;
    REQUIRE(session.AddOwnedUnit("unit_guard", 1));
    REQUIRE(session.AddOwnedUnit("unit_medic", 1));
    REQUIRE(session.AddOwnedUnit("unit_scout", 1));
    REQUIRE(session.TryAddUnitToActiveParty("unit_guard"));
    REQUIRE(session.TryAddUnitToActiveParty("unit_medic"));
    REQUIRE_FALSE(session.TryAddUnitToActiveParty("unit_guard"));

    app::MusteringInteraction mustering;
    mustering.Open(session);

    mustering.ApplyCommand(app::MusteringCommand::SelectActiveNext, session);
    const auto removeResult = mustering.ApplyCommand(app::MusteringCommand::RemoveSelectedActive, session);

    REQUIRE(removeResult.consumed);
    REQUIRE(removeResult.statusText.find("Removed unit_medic") != std::string::npos);

    const auto& active = session.ActivePartyUnitIds();
    REQUIRE(active.size() == 1);
    REQUIRE(active[0] == "unit_guard");

    bool medicFoundInReserve = false;
    for (const auto& stackId : session.ReserveSlotStackIds()) {
        const auto* stack = session.FindRosterStackById(stackId);
        if (stack != nullptr && stack->unitId == "unit_medic") {
            medicFoundInReserve = true;
        }
    }
    REQUIRE(medicFoundInReserve);
}

TEST_CASE("Selection_SanitizesAfterMutationsAndEmptyStates") {
    gameplay::GameSession session;
    REQUIRE(session.AddOwnedUnit("unit_guard", 1));
    REQUIRE(session.TryAddUnitToActiveParty("unit_guard"));

    app::MusteringInteraction mustering;
    mustering.Open(session);

    auto removeResult = mustering.ApplyCommand(app::MusteringCommand::RemoveSelectedActive, session);
    REQUIRE(removeResult.consumed);

    const auto promptAfterRemove = mustering.BuildPromptText(session);
    REQUIRE(promptAfterRemove.find("Active: empty (0/0)") != std::string::npos);

    auto addResult = mustering.ApplyCommand(app::MusteringCommand::AddSelectedReserveToActive, session);
    REQUIRE(addResult.consumed);

    const auto promptAfterAdd = mustering.BuildPromptText(session);
    REQUIRE(promptAfterAdd.find("Active: unit_guard") != std::string::npos);
}

TEST_CASE("ExitCommand_DeactivatesMustering") {
    gameplay::GameSession session;
    REQUIRE(session.AddOwnedUnit("unit_guard", 1));

    app::MusteringInteraction mustering;
    mustering.Open(session);

    const auto exitResult = mustering.ApplyCommand(app::MusteringCommand::Exit, session);
    REQUIRE(exitResult.consumed);
    REQUIRE(exitResult.shouldExit);

    mustering.Close();
    REQUIRE_FALSE(mustering.IsActive());
}

TEST_CASE("AddFails_WhenActivePartyFull") {
    gameplay::GameSession session;
    REQUIRE(session.AddOwnedUnit("unit_guard", 1));
    REQUIRE(session.AddOwnedUnit("unit_medic", 1));
    REQUIRE(session.AddOwnedUnit("unit_scout", 1));
    REQUIRE(session.AddOwnedUnit("unit_lancer", 1));
    REQUIRE(session.TryAddUnitToActiveParty("unit_guard"));
    REQUIRE(session.TryAddUnitToActiveParty("unit_medic"));
    REQUIRE(session.TryAddUnitToActiveParty("unit_scout"));

    app::MusteringInteraction mustering;
    mustering.Open(session);

    const auto result = mustering.ApplyCommand(app::MusteringCommand::AddSelectedReserveToActive, session);
    REQUIRE(result.consumed);
    REQUIRE(result.statusText == "Active party is full");
    REQUIRE(session.ActivePartyUnitIds().size() == 3);
}

TEST_CASE("RemoveFails_WhenReserveFull") {
    gameplay::GameSession session;
    REQUIRE(session.AddOwnedUnit("unit_guard", 1));
    REQUIRE(session.AddOwnedUnit("unit_medic", 1));
    REQUIRE(session.AddOwnedUnit("unit_scout", 1));
    REQUIRE(session.AddOwnedUnit("unit_lancer", 1));
    REQUIRE(session.AddOwnedUnit("unit_arcanist", 1));
    REQUIRE(session.AddOwnedUnit("unit_miner", 1));
    REQUIRE(session.AddOwnedUnit("unit_slingshot", 1));
    REQUIRE(session.AddOwnedUnit("unit_longbow", 1));

    REQUIRE(session.TryAddUnitToActiveParty("unit_guard"));
    REQUIRE(session.AddOwnedUnit("enemy_captain", 1));

    app::MusteringInteraction mustering;
    mustering.Open(session);
    const auto result = mustering.ApplyCommand(app::MusteringCommand::RemoveSelectedActive, session);

    REQUIRE(result.consumed);
    REQUIRE(result.statusText == "No reserve slot available");
    REQUIRE(session.ActivePartyUnitIds().size() == 1);
}

TEST_CASE("AddFails_WhenNoReserveCandidates") {
    gameplay::GameSession session;

    app::MusteringInteraction mustering;
    mustering.Open(session);

    const auto result = mustering.ApplyCommand(app::MusteringCommand::AddSelectedReserveToActive, session);
    REQUIRE(result.consumed);
    REQUIRE(result.statusText == "No reserve units available");
}

TEST_CASE("RemoveFails_WhenActivePartyEmpty") {
    gameplay::GameSession session;
    REQUIRE(session.AddOwnedUnit("unit_guard", 1));

    app::MusteringInteraction mustering;
    mustering.Open(session);

    const auto result = mustering.ApplyCommand(app::MusteringCommand::RemoveSelectedActive, session);
    REQUIRE(result.consumed);
    REQUIRE(result.statusText == "Active party is empty");
}

TEST_CASE("BuildPromptText_ReflectsSelectionAndCounts") {
    gameplay::GameSession session;
    REQUIRE(session.AddOwnedUnit("unit_guard", 1));
    REQUIRE(session.AddOwnedUnit("unit_medic", 1));
    REQUIRE(session.TryAddUnitToActiveParty("unit_guard"));

    app::MusteringInteraction mustering;
    mustering.Open(session);

    std::string prompt = mustering.BuildPromptText(session);
    REQUIRE(prompt.find("Muster Party") != std::string::npos);
    REQUIRE(prompt.find("Reserve: unit_medic x1 (1/1)") != std::string::npos);
    REQUIRE(prompt.find("Active: unit_guard (1/1)") != std::string::npos);
}

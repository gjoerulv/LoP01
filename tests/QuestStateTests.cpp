#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "data/definitions/QuestDefinition.h"
#include "gameplay/quests/QuestState.h"

namespace {
std::vector<data::QuestDefinition> BuildQuestDefinitions() {
    return {
        data::QuestDefinition{"q_restore_well", "Restore the Well", "Collect spare parts", data::QuestObjectiveType::BringResource, "mine_entrance"},
        data::QuestDefinition{"q_secure_bridge", "Secure the Bridge", "Clear the checkpoint", data::QuestObjectiveType::ClearCombatNode, "bridge_checkpoint"}
    };
}
}

TEST_CASE("QuestState initializes quests from typed definitions") {
    gameplay::quests::QuestState state;
    state.Initialize(BuildQuestDefinitions());

    const auto& quests = state.Quests();
    REQUIRE(quests.size() == 2);
    REQUIRE(quests[0].id == "q_restore_well");
    REQUIRE(quests[0].status == gameplay::quests::QuestStatus::InProgress);
    REQUIRE(quests[1].id == "q_secure_bridge");
    REQUIRE(quests[1].status == gameplay::quests::QuestStatus::InProgress);
}

TEST_CASE("QuestState completes matching destination-target quests once") {
    gameplay::quests::QuestState state;
    state.Initialize(BuildQuestDefinitions());

    const auto updates = state.OnDestinationReached("mine_entrance");
    REQUIRE(updates.size() == 1);
    REQUIRE(updates[0] == "Quest completed: Restore the Well");
    REQUIRE(state.Quests()[0].status == gameplay::quests::QuestStatus::Completed);
    REQUIRE(state.Quests()[1].status == gameplay::quests::QuestStatus::InProgress);

    const auto repeated = state.OnDestinationReached("mine_entrance");
    REQUIRE(repeated.empty());
}

TEST_CASE("QuestState completes matching combat-node-clear quests once") {
    gameplay::quests::QuestState state;
    state.Initialize(BuildQuestDefinitions());

    const auto updates = state.OnCombatNodeCleared("bridge_checkpoint");
    REQUIRE(updates.size() == 1);
    REQUIRE(updates[0] == "Quest completed: Secure the Bridge");
    REQUIRE(state.Quests()[1].status == gameplay::quests::QuestStatus::Completed);

    const auto repeated = state.OnCombatNodeCleared("bridge_checkpoint");
    REQUIRE(repeated.empty());
}

TEST_CASE("QuestState ignores non-matching destination targets") {
    gameplay::quests::QuestState state;
    state.Initialize(BuildQuestDefinitions());

    const auto updates = state.OnDestinationReached("old_inn");
    REQUIRE(updates.empty());
    REQUIRE(state.Quests()[0].status == gameplay::quests::QuestStatus::InProgress);
    REQUIRE(state.Quests()[1].status == gameplay::quests::QuestStatus::InProgress);
}

TEST_CASE("QuestState destination trigger does not complete combat-node-clear objectives") {
    gameplay::quests::QuestState state;
    state.Initialize(BuildQuestDefinitions());

    const auto updates = state.OnDestinationReached("bridge_checkpoint");
    REQUIRE(updates.empty());
    REQUIRE(state.Quests()[1].status == gameplay::quests::QuestStatus::InProgress);
}

TEST_CASE("QuestState restores completed quest ids") {
    gameplay::quests::QuestState state;
    state.Initialize(BuildQuestDefinitions());

    state.RestoreCompletedQuestIds({"q_secure_bridge"});

    const auto& quests = state.Quests();
    REQUIRE(quests[0].status == gameplay::quests::QuestStatus::InProgress);
    REQUIRE(quests[1].status == gameplay::quests::QuestStatus::Completed);

    const auto completedIds = state.CompletedQuestIds();
    REQUIRE(completedIds.size() == 1);
    REQUIRE(completedIds[0] == "q_secure_bridge");
}

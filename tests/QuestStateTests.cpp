#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "data/definitions/QuestDefinition.h"
#include "gameplay/quests/QuestState.h"

namespace {
std::vector<data::QuestDefinition> BuildQuestDefinitions() {
    return {
        data::QuestDefinition{"q_restore_well", "Restore the Well", "Collect spare parts", data::QuestObjectiveType::BringResource, "mine_entrance"},
        data::QuestDefinition{"q_find_mira", "Find Mira", "Locate Mira", data::QuestObjectiveType::MeetHero, "bridge_checkpoint"}
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
    REQUIRE(quests[1].id == "q_find_mira");
    REQUIRE(quests[1].status == gameplay::quests::QuestStatus::InProgress);
}

TEST_CASE("QuestState completes matching destination-target quests once") {
    gameplay::quests::QuestState state;
    state.Initialize(BuildQuestDefinitions());

    const auto updates = state.OnDestinationReached("mine_entrance");
    REQUIRE(updates.size() == 1);
    REQUIRE(updates[0] == "Quest completed: Restore the Well");
    REQUIRE(state.Quests()[0].status == gameplay::quests::QuestStatus::Completed);

    const auto repeated = state.OnDestinationReached("mine_entrance");
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

TEST_CASE("QuestState restores completed quest ids") {
    gameplay::quests::QuestState state;
    state.Initialize(BuildQuestDefinitions());

    state.RestoreCompletedQuestIds({"q_find_mira"});

    const auto& quests = state.Quests();
    REQUIRE(quests[0].status == gameplay::quests::QuestStatus::InProgress);
    REQUIRE(quests[1].status == gameplay::quests::QuestStatus::Completed);

    const auto completedIds = state.CompletedQuestIds();
    REQUIRE(completedIds.size() == 1);
    REQUIRE(completedIds[0] == "q_find_mira");
}

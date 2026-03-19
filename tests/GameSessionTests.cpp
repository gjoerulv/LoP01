#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "data/definitions/QuestDefinition.h"
#include "gameplay/GameSession.h"

TEST_CASE("GameSession explicit mode helpers enter expected gameplay modes") {
    gameplay::GameSession session;

    session.EnterLocationMode("town_center");
    REQUIRE(session.Snapshot().mode == gameplay::GameMode::LocationMode);
    REQUIRE(session.Snapshot().destinationId == "town_center");

    session.EnterBattleMode();
    REQUIRE(session.Snapshot().mode == gameplay::GameMode::BattleMode);

    session.EnterOverworldMode();
    REQUIRE(session.Snapshot().mode == gameplay::GameMode::OverworldMode);
}

TEST_CASE("GameSession advance mode keeps front-end flow order") {
    gameplay::GameSession session;

    REQUIRE(session.Snapshot().mode == gameplay::GameMode::Title);

    session.AdvanceMode();
    REQUIRE(session.Snapshot().mode == gameplay::GameMode::OpeningSequence);

    session.AdvanceMode();
    REQUIRE(session.Snapshot().mode == gameplay::GameMode::OverworldSelection);

    session.AdvanceMode();
    REQUIRE(session.Snapshot().mode == gameplay::GameMode::OverworldMode);
}

TEST_CASE("GameSession rest advances to the next day start") {
    gameplay::GameSession session;

    session.AddMinutes(90);
    REQUIRE(session.Snapshot().day == 1);
    REQUIRE(session.Snapshot().time == "07:30");

    session.RestToNextDayStart();

    const auto snapshot = session.Snapshot();
    REQUIRE(snapshot.day == 2);
    REQUIRE(snapshot.time == "06:00");
}

TEST_CASE("GameSession wake penalty sets wake time and reduces gold") {
    gameplay::GameSession session;

    session.ApplyWakePenalty();
    const auto snapshot = session.Snapshot();

    REQUIRE(snapshot.time == "11:00");
    REQUIRE(snapshot.gold == 1500);
}

TEST_CASE("GameSession wake penalty never drops gold below zero") {
    gameplay::GameSession session;

    REQUIRE(session.TrySpendGold(2500));
    REQUIRE(session.Snapshot().gold == 0);

    session.ApplyWakePenalty();
    REQUIRE(session.Snapshot().gold == 0);
}

TEST_CASE("Entering location mode alone does not advance time") {
    gameplay::GameSession session;

    const auto before = session.Snapshot();
    session.EnterLocationMode("town_center");
    const auto after = session.Snapshot();

    REQUIRE(after.day == before.day);
    REQUIRE(after.time == before.time);
}

TEST_CASE("GameSession updates quest progress from destination triggers") {
    gameplay::GameSession session;

    const std::vector<data::QuestDefinition> quests = {
        data::QuestDefinition{"q_restore_well", "Restore the Well", "Collect spare parts", data::QuestObjectiveType::BringResource, "mine_entrance"},
        data::QuestDefinition{"q_find_mira", "Find Mira", "Locate Mira", data::QuestObjectiveType::MeetHero, "bridge_checkpoint"}
    };

    session.InitializeQuestState(quests);
    const auto updates = session.NotifyDestinationReached("bridge_checkpoint");

    REQUIRE(updates.size() == 1);
    REQUIRE(updates.front() == "Quest completed: Find Mira");

    const auto& progress = session.QuestProgress();
    REQUIRE(progress.size() == 2);
    REQUIRE(progress[0].status == gameplay::quests::QuestStatus::InProgress);
    REQUIRE(progress[1].status == gameplay::quests::QuestStatus::Completed);
}

TEST_CASE("Setting destination alone does not auto-complete quests") {
    gameplay::GameSession session;

    const std::vector<data::QuestDefinition> quests = {
        data::QuestDefinition{"q_restore_well", "Restore the Well", "Collect spare parts", data::QuestObjectiveType::BringResource, "mine_entrance"}
    };

    session.InitializeQuestState(quests);
    session.SetDestination("mine_entrance");

    const auto& progress = session.QuestProgress();
    REQUIRE(progress.size() == 1);
    REQUIRE(progress[0].status == gameplay::quests::QuestStatus::InProgress);

    const auto updates = session.NotifyDestinationReached("mine_entrance");
    REQUIRE(updates.size() == 1);
    REQUIRE(progress[0].status == gameplay::quests::QuestStatus::Completed);
}

TEST_CASE("GameSession save and load restores completed quest progression") {
    const std::vector<data::QuestDefinition> quests = {
        data::QuestDefinition{"q_restore_well", "Restore the Well", "Collect spare parts", data::QuestObjectiveType::BringResource, "mine_entrance"},
        data::QuestDefinition{"q_find_mira", "Find Mira", "Locate Mira", data::QuestObjectiveType::MeetHero, "bridge_checkpoint"}
    };

    gameplay::GameSession source;
    source.InitializeQuestState(quests);
    const auto sourceUpdates = source.NotifyDestinationReached("mine_entrance");
    REQUIRE(sourceUpdates.size() == 1);
    source.MarkCombatNodeCleared("bridge_checkpoint");

    const core::SaveData saveData = source.ToSaveData();

    gameplay::GameSession loaded;
    loaded.InitializeQuestState(quests);
    loaded.ApplySaveData(saveData);

    const auto& progress = loaded.QuestProgress();
    REQUIRE(progress.size() == 2);
    REQUIRE(progress[0].status == gameplay::quests::QuestStatus::Completed);
    REQUIRE(progress[1].status == gameplay::quests::QuestStatus::InProgress);
    REQUIRE(loaded.IsCombatNodeCleared("bridge_checkpoint"));
}

TEST_CASE("GameSession clears combat node only on allied overworld combat victory") {
    gameplay::GameSession session;

    session.ApplyOverworldCombatVictoryNodeClear(
        true,
        false,
        gameplay::GameMode::OverworldMode,
        "bridge_checkpoint",
        true);

    REQUIRE(session.IsCombatNodeCleared("bridge_checkpoint"));

    session.ApplyOverworldCombatVictoryNodeClear(
        false,
        true,
        gameplay::GameMode::OverworldMode,
        "orchard_pass",
        true);

    session.ApplyOverworldCombatVictoryNodeClear(
        true,
        false,
        gameplay::GameMode::LocationMode,
        "clocktower_square",
        true);

    REQUIRE_FALSE(session.IsCombatNodeCleared("orchard_pass"));
    REQUIRE_FALSE(session.IsCombatNodeCleared("clocktower_square"));
}

#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "core/GameClock.h"
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
        data::QuestDefinition{"q_secure_bridge", "Secure the Bridge", "Clear the checkpoint", data::QuestObjectiveType::ClearCombatNode, "bridge_checkpoint"}
    };

    session.InitializeQuestState(quests);
    const auto updates = session.NotifyDestinationReached("bridge_checkpoint");

    REQUIRE(updates.empty());

    const auto& progress = session.QuestProgress();
    REQUIRE(progress.size() == 2);
    REQUIRE(progress[0].status == gameplay::quests::QuestStatus::InProgress);
    REQUIRE(progress[1].status == gameplay::quests::QuestStatus::InProgress);
}

TEST_CASE("GameSession updates quest progress from combat-node-clear triggers") {
    gameplay::GameSession session;

    const std::vector<data::QuestDefinition> quests = {
        data::QuestDefinition{"q_restore_well", "Restore the Well", "Collect spare parts", data::QuestObjectiveType::BringResource, "mine_entrance"},
        data::QuestDefinition{"q_secure_bridge", "Secure the Bridge", "Clear the checkpoint", data::QuestObjectiveType::ClearCombatNode, "bridge_checkpoint"}
    };

    session.InitializeQuestState(quests);
    const auto updates = session.NotifyCombatNodeCleared("bridge_checkpoint");

    REQUIRE(updates.size() == 1);
    REQUIRE(updates.front() == "Quest completed: Secure the Bridge");

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
        data::QuestDefinition{"q_secure_bridge", "Secure the Bridge", "Clear the checkpoint", data::QuestObjectiveType::ClearCombatNode, "bridge_checkpoint"}
    };

    gameplay::GameSession source;
    source.InitializeQuestState(quests);
    const auto sourceUpdates = source.NotifyDestinationReached("mine_entrance");
    REQUIRE(sourceUpdates.size() == 1);
    const auto clearUpdates = source.NotifyCombatNodeCleared("bridge_checkpoint");
    REQUIRE(clearUpdates.size() == 1);
    source.MarkCombatNodeCleared("bridge_checkpoint");

    const core::SaveData saveData = source.ToSaveData();

    gameplay::GameSession loaded;
    loaded.InitializeQuestState(quests);
    loaded.ApplySaveData(saveData);

    const auto& progress = loaded.QuestProgress();
    REQUIRE(progress.size() == 2);
    REQUIRE(progress[0].status == gameplay::quests::QuestStatus::Completed);
    REQUIRE(progress[1].status == gameplay::quests::QuestStatus::Completed);
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

TEST_CASE("GameSession recruit stock refreshes weekly and survives save load") {
    gameplay::GameSession session;

    data::LocationServiceDefinition recruitService;
    recruitService.id = "survivor_post_recruitment";
    recruitService.kind = data::LocationServiceKind::Recruit;
    recruitService.weeklyStock = 3;

    session.RefreshWeeklyRecruitStocks({ recruitService });
    REQUIRE(session.RemainingRecruitStock(recruitService.id, recruitService.weeklyStock) == 3);

    REQUIRE(session.TryConsumeRecruitStock(recruitService.id, recruitService.weeklyStock));
    REQUIRE(session.RemainingRecruitStock(recruitService.id, recruitService.weeklyStock) == 2);

    const core::SaveData saveData = session.ToSaveData();

    gameplay::GameSession loaded;
    loaded.ApplySaveData(saveData);
    REQUIRE(loaded.RemainingRecruitStock(recruitService.id, recruitService.weeklyStock) == 2);

    loaded.AddMinutes(core::GameClock::kMinutesPerSliceDay * 7);
    loaded.RefreshWeeklyRecruitStocks({ recruitService });
    REQUIRE(loaded.RemainingRecruitStock(recruitService.id, recruitService.weeklyStock) == 3);
}

TEST_CASE("GameSession daily service usage and same-day travel prep work with daily cadence") {
    gameplay::GameSession session;

    data::LocationServiceDefinition suppliesService;
    suppliesService.id = "supply_cart_market";
    suppliesService.kind = data::LocationServiceKind::Shop;
    suppliesService.dailyUseLimit = 1;

    session.RefreshDailyServiceUses({ suppliesService });
    REQUIRE(session.RemainingDailyServiceUses(suppliesService.id, suppliesService.dailyUseLimit) == 1);
    REQUIRE(session.TryConsumeDailyServiceUse(suppliesService.id, suppliesService.dailyUseLimit));
    REQUIRE(session.RemainingDailyServiceUses(suppliesService.id, suppliesService.dailyUseLimit) == 0);
    REQUIRE_FALSE(session.TryConsumeDailyServiceUse(suppliesService.id, suppliesService.dailyUseLimit));

    session.GrantSameDayTravelPrep(20, 1);
    REQUIRE(session.HasActiveSameDayTravelPrep());
    REQUIRE(session.ActiveSameDayTravelPrepDiscountMinutes() == 20);
    REQUIRE(session.PreviewSameDayTravelPrepToTravelMinutes(40) == 20);
    REQUIRE(session.ApplySameDayTravelPrepToTravelMinutes(40) == 20);
    REQUIRE_FALSE(session.HasActiveSameDayTravelPrep());
    REQUIRE(session.ActiveSameDayTravelPrepDiscountMinutes() == 0);
    REQUIRE(session.ApplySameDayTravelPrepToTravelMinutes(40) == 40);

    session.GrantSameDayTravelPrep(20, 1);
    REQUIRE(session.HasActiveSameDayTravelPrep());
    session.AddMinutes(core::GameClock::kMinutesPerSliceDay);

    session.RefreshDailyServiceUses({ suppliesService });
    REQUIRE(session.RemainingDailyServiceUses(suppliesService.id, suppliesService.dailyUseLimit) == 1);
    REQUIRE_FALSE(session.HasActiveSameDayTravelPrep());
    REQUIRE(session.ActiveSameDayTravelPrepDiscountMinutes() == 0);
    REQUIRE(session.PreviewSameDayTravelPrepToTravelMinutes(40) == 40);
    REQUIRE(session.ApplySameDayTravelPrepToTravelMinutes(40) == 40);
}

TEST_CASE("GameSession owned roster rejects invalid add inputs and keeps canonical order") {
    gameplay::GameSession session;

    REQUIRE_FALSE(session.AddOwnedUnit("", 1));
    REQUIRE_FALSE(session.AddOwnedUnit("unit_guard", 0));
    REQUIRE_FALSE(session.AddOwnedUnit("unit_guard", -2));

    REQUIRE(session.AddOwnedUnit("unit_medic", 1));
    REQUIRE(session.AddOwnedUnit("unit_guard", 2));
    REQUIRE(session.AddOwnedUnit("unit_guard", 1));

    const auto& owned = session.OwnedUnitCounts();
    REQUIRE(owned.size() == 2);
    REQUIRE(owned[0].unitId == "unit_guard");
    REQUIRE(owned[0].count == 3);
    REQUIRE(owned[1].unitId == "unit_medic");
    REQUIRE(owned[1].count == 1);
}

TEST_CASE("GameSession active party requires reserve availability and preserves order") {
    gameplay::GameSession session;

    REQUIRE(session.AddOwnedUnit("unit_guard", 2));
    REQUIRE(session.AddOwnedUnit("unit_medic", 1));

    REQUIRE(session.TryAddUnitToActiveParty("unit_guard"));
    REQUIRE(session.TryAddUnitToActiveParty("unit_medic"));
    REQUIRE(session.TryAddUnitToActiveParty("unit_guard"));
    REQUIRE_FALSE(session.TryAddUnitToActiveParty("unit_guard"));

    const auto& active = session.ActivePartyUnitIds();
    REQUIRE(active.size() == 3);
    REQUIRE(active[0] == "unit_guard");
    REQUIRE(active[1] == "unit_medic");
    REQUIRE(active[2] == "unit_guard");
}

TEST_CASE("GameSession removing owned units fails when active allocations would underflow") {
    gameplay::GameSession session;

    REQUIRE_FALSE(session.TryRemoveOwnedUnit("", 1));
    REQUIRE_FALSE(session.TryRemoveOwnedUnit("unit_guard", 0));
    REQUIRE_FALSE(session.TryRemoveOwnedUnit("unit_guard", -1));

    REQUIRE(session.AddOwnedUnit("unit_guard", 2));
    REQUIRE(session.TryAddUnitToActiveParty("unit_guard"));

    REQUIRE_FALSE(session.TryRemoveOwnedUnit("unit_guard", 2));
    REQUIRE(session.TryRemoveOwnedUnit("unit_guard", 1));
    REQUIRE(session.OwnedUnitCount("unit_guard") == 1);
    REQUIRE(session.ActivePartyAllocatedCount("unit_guard") == 1);

    REQUIRE_FALSE(session.TryRemoveOwnedUnit("unit_guard", 1));
}

TEST_CASE("GameSession active party mutation APIs validate indices and do not break invariants") {
    gameplay::GameSession session;

    REQUIRE(session.AddOwnedUnit("unit_guard", 2));
    REQUIRE(session.AddOwnedUnit("unit_medic", 1));
    REQUIRE(session.TryAddUnitToActiveParty("unit_guard"));
    REQUIRE(session.TryAddUnitToActiveParty("unit_medic"));

    REQUIRE_FALSE(session.TryRemoveActivePartyUnitAt(-1));
    REQUIRE_FALSE(session.TryRemoveActivePartyUnitAt(5));
    REQUIRE(session.TryMoveActivePartyUnit(0, 1));

    const auto& moved = session.ActivePartyUnitIds();
    REQUIRE(moved.size() == 2);
    REQUIRE(moved[0] == "unit_medic");
    REQUIRE(moved[1] == "unit_guard");

    REQUIRE_FALSE(session.TryMoveActivePartyUnit(0, 4));
    REQUIRE(session.TryRemoveActivePartyUnitAt(0));
    REQUIRE(session.ActivePartyUnitIds().size() == 1);
    REQUIRE(session.ActivePartyUnitIds()[0] == "unit_guard");

    session.ClearActiveParty();
    REQUIRE(session.ActivePartyUnitIds().empty());
}

TEST_CASE("GameSession save and load roundtrips roster ownership and active party order") {
    gameplay::GameSession source;

    REQUIRE(source.AddOwnedUnit("unit_medic", 1));
    REQUIRE(source.AddOwnedUnit("unit_guard", 2));
    REQUIRE(source.TryAddUnitToActiveParty("unit_guard"));
    REQUIRE(source.TryAddUnitToActiveParty("unit_medic"));
    REQUIRE(source.TryAddUnitToActiveParty("unit_guard"));

    const core::SaveData saveData = source.ToSaveData();

    gameplay::GameSession loaded;
    loaded.ApplySaveData(saveData);

    const auto& owned = loaded.OwnedUnitCounts();
    REQUIRE(owned.size() == 2);
    REQUIRE(owned[0].unitId == "unit_guard");
    REQUIRE(owned[0].count == 2);
    REQUIRE(owned[1].unitId == "unit_medic");
    REQUIRE(owned[1].count == 1);

    const auto& active = loaded.ActivePartyUnitIds();
    REQUIRE(active.size() == 3);
    REQUIRE(active[0] == "unit_guard");
    REQUIRE(active[1] == "unit_medic");
    REQUIRE(active[2] == "unit_guard");
}

TEST_CASE("GameSession load sanitizes invalid or out-of-sync roster save data") {
    core::SaveData saveData;
    saveData.mode = "overworld_mode";
    saveData.regionId = "ashvale_heartland";
    saveData.destinationId = "home_base";
    saveData.ownedUnitCounts = {
        core::OwnedUnitCountSaveState{"unit_guard", 1},
        core::OwnedUnitCountSaveState{"unit_guard", 2},
        core::OwnedUnitCountSaveState{"", 2},
        core::OwnedUnitCountSaveState{"unit_medic", -1}
    };
    saveData.activePartyUnitIds = {
        "unit_guard",
        "",
        "unit_guard",
        "unit_guard",
        "unit_medic"
    };

    gameplay::GameSession loaded;
    loaded.ApplySaveData(saveData);

    const auto& owned = loaded.OwnedUnitCounts();
    REQUIRE(owned.size() == 1);
    REQUIRE(owned[0].unitId == "unit_guard");
    REQUIRE(owned[0].count == 3);

    const auto& active = loaded.ActivePartyUnitIds();
    REQUIRE(active.size() == 3);
    REQUIRE(active[0] == "unit_guard");
    REQUIRE(active[1] == "unit_guard");
    REQUIRE(active[2] == "unit_guard");
}

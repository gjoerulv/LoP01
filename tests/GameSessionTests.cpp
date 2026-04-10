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

    session.EnterRegionMode();
    REQUIRE(session.Snapshot().mode == gameplay::GameMode::RegionMode);
}

TEST_CASE("GameSession advance mode keeps front-end flow order") {
    gameplay::GameSession session;

    REQUIRE(session.Snapshot().mode == gameplay::GameMode::Title);

    session.AdvanceMode();
    REQUIRE(session.Snapshot().mode == gameplay::GameMode::OpeningSequence);

    session.AdvanceMode();
    REQUIRE(session.Snapshot().mode == gameplay::GameMode::WorldMapMode);

    session.AdvanceMode();
    REQUIRE(session.Snapshot().mode == gameplay::GameMode::RegionMode);
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

TEST_CASE("GameSession wake penalty advances to next day 11:00 and reduces gold") {
    gameplay::GameSession session;
    const auto before = session.Snapshot();

    session.ApplyWakePenalty();
    const auto snapshot = session.Snapshot();

    REQUIRE(snapshot.day == before.day + 1);
    REQUIRE(snapshot.time == "11:00");
    REQUIRE(snapshot.gold == 1500);
}

TEST_CASE("GameSession wake penalty does not change destination (fallback remains app-managed)") {
    gameplay::GameSession session;
    session.SetDestination("bridge_checkpoint");

    session.ApplyWakePenalty();
    const auto snapshot = session.Snapshot();

    REQUIRE(snapshot.destinationId == "bridge_checkpoint");
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

TEST_CASE("GameSession clears combat node only on allied region combat victory") {
    gameplay::GameSession session;

    session.ApplyRegionCombatVictoryNodeClear(
        true,
        false,
        gameplay::GameMode::RegionMode,
        "bridge_checkpoint",
        true);

    REQUIRE(session.IsCombatNodeCleared("bridge_checkpoint"));

    session.ApplyRegionCombatVictoryNodeClear(
        false,
        true,
        gameplay::GameMode::RegionMode,
        "orchard_pass",
        true);

    session.ApplyRegionCombatVictoryNodeClear(
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
    REQUIRE_FALSE(session.TryAddUnitToActiveParty("unit_guard"));
    REQUIRE_FALSE(session.TryAddUnitToActiveParty("unit_guard"));

    const auto& active = session.ActivePartyUnitIds();
    REQUIRE(active.size() == 2);
    REQUIRE(active[0] == "unit_guard");
    REQUIRE(active[1] == "unit_medic");
}

TEST_CASE("GameSession active party capacity is five slots") {
    gameplay::GameSession session;

    REQUIRE(session.ActivePartyCapacity() == 5);

    REQUIRE(session.AddOwnedUnit("unit_guard", 1));
    REQUIRE(session.AddOwnedUnit("unit_medic", 1));
    REQUIRE(session.AddOwnedUnit("unit_scout", 1));
    REQUIRE(session.AddOwnedUnit("unit_lancer", 1));
    REQUIRE(session.AddOwnedUnit("unit_arcanist", 1));
    REQUIRE(session.AddOwnedUnit("unit_miner", 1));

    REQUIRE(session.TryAddUnitToActiveParty("unit_guard"));
    REQUIRE(session.TryAddUnitToActiveParty("unit_medic"));
    REQUIRE(session.TryAddUnitToActiveParty("unit_scout"));
    REQUIRE(session.TryAddUnitToActiveParty("unit_lancer"));
    REQUIRE(session.TryAddUnitToActiveParty("unit_arcanist"));
    REQUIRE_FALSE(session.TryAddUnitToActiveParty("unit_miner"));

    const auto& active = session.ActivePartyUnitIds();
    REQUIRE(active.size() == 5);
    REQUIRE(active[0] == "unit_guard");
    REQUIRE(active[1] == "unit_medic");
    REQUIRE(active[2] == "unit_scout");
    REQUIRE(active[3] == "unit_lancer");
    REQUIRE(active[4] == "unit_arcanist");
}

TEST_CASE("GameSession rejects removing the last leader-capable active unit when leader rules are configured") {
    gameplay::GameSession session;
    session.SetLeaderCapableUnitIds({"hero_player", "hero_mira"});

    REQUIRE(session.AddOwnedUnit("hero_mira", 1));
    REQUIRE(session.AddOwnedUnit("unit_guard", 1));
    REQUIRE(session.TryAddUnitToActiveParty("hero_mira"));
    REQUIRE(session.TryAddUnitToActiveParty("unit_guard"));

    REQUIRE(session.ActivePartyUnitIds().size() == 2);
    REQUIRE(session.ActivePartyUnitIds()[0] == "hero_mira");

    REQUIRE_FALSE(session.TryRemoveActivePartyUnitAt(0));
    REQUIRE(session.ActivePartyUnitIds().size() == 2);
    REQUIRE(session.ActivePartyUnitIds()[0] == "hero_mira");
}

TEST_CASE("GameSession allows removing a leader-capable active unit when another leader-capable remains") {
    gameplay::GameSession session;
    session.SetLeaderCapableUnitIds({"hero_player", "hero_mira"});

    REQUIRE(session.AddOwnedUnit("hero_mira", 1));
    REQUIRE(session.AddOwnedUnit("hero_player", 1));
    REQUIRE(session.TryAddUnitToActiveParty("hero_mira"));
    REQUIRE(session.TryAddUnitToActiveParty("hero_player"));

    REQUIRE(session.TryRemoveActivePartyUnitAt(0));
    REQUIRE(session.ActivePartyUnitIds().size() == 1);
    REQUIRE(session.ActivePartyUnitIds()[0] == "hero_player");
}

TEST_CASE("GameSession ApplySaveData rejects canonical active party with no leader-capable unit when leader rules are configured") {
    gameplay::GameSession session;
    session.SetLeaderCapableUnitIds({"hero_player", "hero_mira"});
    REQUIRE(session.AddOwnedUnit("hero_player", 1));
    REQUIRE(session.TryAddUnitToActiveParty("hero_player"));

    const auto beforeSnapshot = session.Snapshot();
    const auto beforeStacks = session.RosterStacks();
    const auto beforeActiveSlots = session.ActiveSlotStackIds();
    const auto beforeReserveSlots = session.ReserveSlotStackIds();
    const int beforeCounter = session.NextStackIdCounter();

    core::SaveData invalidLeaderless;
    invalidLeaderless.schemaVersion = 3;
    invalidLeaderless.mode = "overworld_mode";
    invalidLeaderless.regionId = "ashvale_heartland";
    invalidLeaderless.destinationId = "home_base";
    invalidLeaderless.hasCanonicalRoster = true;
    invalidLeaderless.rosterStacks = {
        core::RosterStackSaveState{"stk_1", "unit_guard", 1}
    };
    invalidLeaderless.activeSlotStackIds = {"stk_1", "", "", "", ""};
    invalidLeaderless.reserveSlotStackIds = {"", "", "", "", "", "", "", ""};
    invalidLeaderless.nextStackIdCounter = 2;

    session.ApplySaveData(invalidLeaderless);

    REQUIRE(session.Snapshot().mode == beforeSnapshot.mode);
    REQUIRE(session.Snapshot().day == beforeSnapshot.day);
    REQUIRE(session.Snapshot().gold == beforeSnapshot.gold);
    REQUIRE(session.RosterStacks().size() == beforeStacks.size());
    for (size_t i = 0; i < beforeStacks.size(); ++i) {
        REQUIRE(session.RosterStacks()[i].stackId == beforeStacks[i].stackId);
        REQUIRE(session.RosterStacks()[i].unitId == beforeStacks[i].unitId);
        REQUIRE(session.RosterStacks()[i].quantity == beforeStacks[i].quantity);
    }
    REQUIRE(session.ActiveSlotStackIds() == beforeActiveSlots);
    REQUIRE(session.ReserveSlotStackIds() == beforeReserveSlots);
    REQUIRE(session.NextStackIdCounter() == beforeCounter);
}

TEST_CASE("GameSession canonical stack model generates deterministic stack ids") {
    gameplay::GameSession session;

    REQUIRE(session.NextStackIdCounter() == 1);
    REQUIRE(session.AddOwnedUnit("unit_guard", 2));
    REQUIRE(session.RosterStacks().size() == 1);
    REQUIRE(session.RosterStacks()[0].stackId == "stk_1");
    REQUIRE(session.RosterStacks()[0].quantity == 2);
    REQUIRE(session.NextStackIdCounter() == 2);

    REQUIRE(session.AddOwnedUnit("unit_guard", 1));
    REQUIRE(session.RosterStacks().size() == 1);
    REQUIRE(session.RosterStacks()[0].stackId == "stk_1");
    REQUIRE(session.RosterStacks()[0].quantity == 3);
    REQUIRE(session.NextStackIdCounter() == 2);

    REQUIRE(session.AddOwnedUnit("unit_medic", 1));
    REQUIRE(session.RosterStacks().size() == 2);
    REQUIRE(session.RosterStacks()[1].stackId == "stk_2");
    REQUIRE(session.NextStackIdCounter() == 3);
}

TEST_CASE("GameSession canonical active and reserve slot occupancy remains bounded") {
    gameplay::GameSession session;

    REQUIRE(session.AddOwnedUnit("unit_guard", 3));
    REQUIRE(session.AddOwnedUnit("unit_medic", 2));

    REQUIRE(session.TryAddUnitToActiveParty("unit_guard"));
    REQUIRE(session.TryAddUnitToActiveParty("unit_medic"));
    REQUIRE_FALSE(session.TryAddUnitToActiveParty("unit_guard"));

    int occupiedActive = 0;
    for (const auto& stackId : session.ActiveSlotStackIds()) {
        if (!stackId.empty()) {
            ++occupiedActive;
        }
    }
    REQUIRE(occupiedActive == 2);
    REQUIRE(session.ActivePartyUnitIds().size() == 2);

    int occupiedReserve = 0;
    for (const auto& stackId : session.ReserveSlotStackIds()) {
        if (!stackId.empty()) {
            ++occupiedReserve;
        }
    }
    REQUIRE(occupiedReserve >= 0);
    REQUIRE(occupiedReserve <= 8);
}

TEST_CASE("GameSession removing final quantity clears depleted stack and slot immediately") {
    gameplay::GameSession session;

    REQUIRE(session.AddOwnedUnit("unit_guard", 1));
    REQUIRE(session.ReserveSlotStackIds()[0] == "stk_1");

    REQUIRE(session.TryRemoveOwnedUnit("unit_guard", 1));
    REQUIRE(session.OwnedUnitCount("unit_guard") == 0);
    REQUIRE(session.RosterStacks().empty());
    REQUIRE(session.ReserveSlotStackIds()[0].empty());
}

TEST_CASE("GameSession old-shaped projections are derived and remain consistent") {
    gameplay::GameSession session;

    REQUIRE(session.AddOwnedUnit("unit_guard", 2));
    REQUIRE(session.AddOwnedUnit("unit_medic", 1));
    REQUIRE(session.TryAddUnitToActiveParty("unit_guard"));

    const auto& owned = session.OwnedUnitCounts();
    REQUIRE(owned.size() == 2);
    REQUIRE(owned[0].unitId == "unit_guard");
    REQUIRE(owned[0].count == 2);
    REQUIRE(owned[1].unitId == "unit_medic");
    REQUIRE(owned[1].count == 1);

    const auto& active = session.ActivePartyUnitIds();
    REQUIRE(active.size() == 1);
    REQUIRE(active[0] == "unit_guard");
    REQUIRE(session.ActivePartyAllocatedCount("unit_guard") == 2);
    REQUIRE(session.ReserveUnitCount("unit_guard") == 0);
}

TEST_CASE("GameSession Phase2 mustering moves whole stacks without duplicate stack references") {
    gameplay::GameSession session;

    REQUIRE(session.AddOwnedUnit("unit_guard", 2));
    REQUIRE(session.RosterStacks().size() == 1);
    REQUIRE(session.RosterStacks()[0].stackId == "stk_1");
    REQUIRE(session.NextStackIdCounter() == 2);

    REQUIRE(session.TryAddUnitToActiveParty("unit_guard"));
    REQUIRE_FALSE(session.TryAddUnitToActiveParty("unit_guard"));
    REQUIRE(session.ActivePartyUnitIds().size() == 1);
    REQUIRE(session.RosterStacks().size() == 1);
    REQUIRE(session.NextStackIdCounter() == 2);

    int activeRefs = 0;
    for (const auto& stackId : session.ActiveSlotStackIds()) {
        if (stackId == "stk_1") {
            ++activeRefs;
        }
    }
    REQUIRE(activeRefs == 1);

    REQUIRE(session.TryRemoveActivePartyUnitAt(0));
    REQUIRE(session.ActivePartyUnitIds().empty());
    REQUIRE(session.RosterStacks().size() == 1);
    REQUIRE(session.OwnedUnitCount("unit_guard") == 2);
}

TEST_CASE("GameSession removing owned units fails when active allocations would underflow") {
    gameplay::GameSession session;

    REQUIRE_FALSE(session.TryRemoveOwnedUnit("", 1));
    REQUIRE_FALSE(session.TryRemoveOwnedUnit("unit_guard", 0));
    REQUIRE_FALSE(session.TryRemoveOwnedUnit("unit_guard", -1));

    REQUIRE(session.AddOwnedUnit("unit_guard", 2));
    REQUIRE(session.TryAddUnitToActiveParty("unit_guard"));

    REQUIRE_FALSE(session.TryRemoveOwnedUnit("unit_guard", 1));
    REQUIRE_FALSE(session.TryRemoveOwnedUnit("unit_guard", 2));
    REQUIRE(session.OwnedUnitCount("unit_guard") == 2);
    REQUIRE(session.ActivePartyAllocatedCount("unit_guard") == 2);
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
    REQUIRE_FALSE(source.TryAddUnitToActiveParty("unit_guard"));

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
    REQUIRE(active.size() == 2);
    REQUIRE(active[0] == "unit_guard");
    REQUIRE(active[1] == "unit_medic");
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

TEST_CASE("GameSession recruit placement follows reserve then active merge policy") {
    gameplay::GameSession session;

    REQUIRE(session.AddOwnedUnit("unit_guard", 1));
    REQUIRE(session.RosterStacks().size() == 1);
    REQUIRE(session.TryAddUnitToActiveParty("unit_guard"));

    REQUIRE(session.AddOwnedUnit("unit_guard", 1));
    REQUIRE(session.RosterStacks().size() == 1);
    REQUIRE(session.RosterStacks()[0].quantity == 2);

    REQUIRE(session.TryRemoveActivePartyUnitAt(0));
    REQUIRE(session.AddOwnedUnit("unit_guard", 1));
    REQUIRE(session.RosterStacks().size() == 1);
    REQUIRE(session.RosterStacks()[0].quantity == 3);
}

TEST_CASE("GameSession canonical save roundtrip preserves exact stack ids quantities slots and counter") {
    gameplay::GameSession source;
    REQUIRE(source.AddOwnedUnit("unit_guard", 3));
    REQUIRE(source.AddOwnedUnit("unit_medic", 2));
    REQUIRE(source.TryAddUnitToActiveParty("unit_guard"));

    const auto sourceStacks = source.RosterStacks();
    const auto sourceActiveSlots = source.ActiveSlotStackIds();
    const auto sourceReserveSlots = source.ReserveSlotStackIds();
    const int sourceNextCounter = source.NextStackIdCounter();

    const core::SaveData saveData = source.ToSaveData();

    gameplay::GameSession loaded;
    loaded.ApplySaveData(saveData);

    REQUIRE(loaded.RosterStacks().size() == sourceStacks.size());
    for (size_t i = 0; i < sourceStacks.size(); ++i) {
        REQUIRE(loaded.RosterStacks()[i].stackId == sourceStacks[i].stackId);
        REQUIRE(loaded.RosterStacks()[i].unitId == sourceStacks[i].unitId);
        REQUIRE(loaded.RosterStacks()[i].quantity == sourceStacks[i].quantity);
    }

    REQUIRE(loaded.ActiveSlotStackIds() == sourceActiveSlots);
    REQUIRE(loaded.ReserveSlotStackIds() == sourceReserveSlots);
    REQUIRE(loaded.NextStackIdCounter() == sourceNextCounter);
}

TEST_CASE("GameSession ApplySaveData legacy overflow fails without partial mutation") {
    gameplay::GameSession session;
    REQUIRE(session.AddOwnedUnit("unit_guard", 2));
    REQUIRE(session.TryAddUnitToActiveParty("unit_guard"));

    const auto beforeSnapshot = session.Snapshot();
    const auto beforeStacks = session.RosterStacks();
    const auto beforeActiveSlots = session.ActiveSlotStackIds();
    const auto beforeReserveSlots = session.ReserveSlotStackIds();
    const int beforeCounter = session.NextStackIdCounter();

    core::SaveData overflowLegacy;
    overflowLegacy.mode = "overworld_mode";
    overflowLegacy.regionId = "ashvale_heartland";
    overflowLegacy.destinationId = "home_base";
    overflowLegacy.ownedUnitCounts = {
        core::OwnedUnitCountSaveState{"unit_01", 1},
        core::OwnedUnitCountSaveState{"unit_02", 1},
        core::OwnedUnitCountSaveState{"unit_03", 1},
        core::OwnedUnitCountSaveState{"unit_04", 1},
        core::OwnedUnitCountSaveState{"unit_05", 1},
        core::OwnedUnitCountSaveState{"unit_06", 1},
        core::OwnedUnitCountSaveState{"unit_07", 1},
        core::OwnedUnitCountSaveState{"unit_08", 1},
        core::OwnedUnitCountSaveState{"unit_09", 1}
    };

    session.ApplySaveData(overflowLegacy);

    REQUIRE(session.Snapshot().mode == beforeSnapshot.mode);
    REQUIRE(session.Snapshot().day == beforeSnapshot.day);
    REQUIRE(session.Snapshot().gold == beforeSnapshot.gold);
    REQUIRE(session.RosterStacks().size() == beforeStacks.size());
    for (size_t i = 0; i < beforeStacks.size(); ++i) {
        REQUIRE(session.RosterStacks()[i].stackId == beforeStacks[i].stackId);
        REQUIRE(session.RosterStacks()[i].unitId == beforeStacks[i].unitId);
        REQUIRE(session.RosterStacks()[i].quantity == beforeStacks[i].quantity);
    }
    REQUIRE(session.ActiveSlotStackIds() == beforeActiveSlots);
    REQUIRE(session.ReserveSlotStackIds() == beforeReserveSlots);
    REQUIRE(session.NextStackIdCounter() == beforeCounter);
}

TEST_CASE("GameSession BuildActiveBattleStackEntries exports stack identity and quantity") {
    gameplay::GameSession session;
    REQUIRE(session.AddOwnedUnit("unit_guard", 3));
    REQUIRE(session.AddOwnedUnit("unit_medic", 2));
    REQUIRE(session.TryAddUnitToActiveParty("unit_guard"));
    REQUIRE(session.TryAddUnitToActiveParty("unit_medic"));

    const auto entries = session.BuildActiveBattleStackEntries();
    REQUIRE(entries.size() == 2);

    REQUIRE(entries[0].activeSlotIndex == 0);
    REQUIRE(entries[0].unitId == "unit_guard");
    REQUIRE(entries[0].quantity == 3);
    REQUIRE_FALSE(entries[0].stackId.empty());

    REQUIRE(entries[1].activeSlotIndex == 1);
    REQUIRE(entries[1].unitId == "unit_medic");
    REQUIRE(entries[1].quantity == 2);
    REQUIRE_FALSE(entries[1].stackId.empty());
}

TEST_CASE("GameSession ApplyBattleStackLifeResults writes by stackId and removes zero-life stacks") {
    gameplay::GameSession session;
    REQUIRE(session.AddOwnedUnit("unit_guard", 3));
    REQUIRE(session.AddOwnedUnit("unit_medic", 2));
    REQUIRE(session.TryAddUnitToActiveParty("unit_guard"));
    REQUIRE(session.TryAddUnitToActiveParty("unit_medic"));

    const auto entries = session.BuildActiveBattleStackEntries();
    REQUIRE(entries.size() == 2);

    const std::vector<std::string> expectedStackIds = {entries[0].stackId, entries[1].stackId};
    const std::vector<gameplay::BattleStackLifeResult> results = {
        gameplay::BattleStackLifeResult{entries[0].stackId, 1},
        gameplay::BattleStackLifeResult{entries[1].stackId, 0}
    };

    REQUIRE(session.ApplyBattleStackLifeResults(results, expectedStackIds));
    REQUIRE(session.OwnedUnitCount("unit_guard") == 1);
    REQUIRE(session.OwnedUnitCount("unit_medic") == 0);

    bool removedStackStillReferenced = false;
    for (const auto& slotId : session.ActiveSlotStackIds()) {
        if (slotId == entries[1].stackId) {
            removedStackStillReferenced = true;
        }
    }
    for (const auto& slotId : session.ReserveSlotStackIds()) {
        if (slotId == entries[1].stackId) {
            removedStackStillReferenced = true;
        }
    }
    REQUIRE_FALSE(removedStackStillReferenced);
}

TEST_CASE("GameSession ApplyBattleStackLifeResults removes KO non-player hero stack from party state when life is zero") {
    gameplay::GameSession session;
    REQUIRE(session.AddOwnedUnit("hero_mira", 1));
    REQUIRE(session.AddOwnedUnit("unit_guard", 2));
    REQUIRE(session.TryAddUnitToActiveParty("hero_mira"));
    REQUIRE(session.TryAddUnitToActiveParty("unit_guard"));

    const auto entries = session.BuildActiveBattleStackEntries();
    REQUIRE(entries.size() == 2);

    std::string heroStackId;
    for (const auto& entry : entries) {
        if (entry.unitId == "hero_mira") {
            heroStackId = entry.stackId;
        }
    }
    REQUIRE_FALSE(heroStackId.empty());

    const std::vector<std::string> expectedStackIds = {entries[0].stackId, entries[1].stackId};
    const std::vector<gameplay::BattleStackLifeResult> results = {
        gameplay::BattleStackLifeResult{entries[0].stackId, entries[0].unitId == "hero_mira" ? 0 : entries[0].quantity},
        gameplay::BattleStackLifeResult{entries[1].stackId, entries[1].unitId == "hero_mira" ? 0 : entries[1].quantity}
    };

    REQUIRE(session.ApplyBattleStackLifeResults(results, expectedStackIds));

    for (const auto& slotId : session.ActiveSlotStackIds()) {
        REQUIRE(slotId != heroStackId);
    }
    for (const auto& slotId : session.ReserveSlotStackIds()) {
        REQUIRE(slotId != heroStackId);
    }
}

TEST_CASE("GameSession ApplyBattleStackLifeResults invalid payload fails atomically") {
    gameplay::GameSession session;
    REQUIRE(session.AddOwnedUnit("unit_guard", 3));
    REQUIRE(session.AddOwnedUnit("unit_medic", 2));
    REQUIRE(session.TryAddUnitToActiveParty("unit_guard"));
    REQUIRE(session.TryAddUnitToActiveParty("unit_medic"));

    const auto entries = session.BuildActiveBattleStackEntries();
    REQUIRE(entries.size() == 2);

    const std::vector<std::string> expectedStackIds = {entries[0].stackId, entries[1].stackId};
    const int ownedGuardBefore = session.OwnedUnitCount("unit_guard");
    const int ownedMedicBefore = session.OwnedUnitCount("unit_medic");
    const auto activeBefore = session.ActiveSlotStackIds();
    const auto reserveBefore = session.ReserveSlotStackIds();

    const std::vector<gameplay::BattleStackLifeResult> invalidResults = {
        gameplay::BattleStackLifeResult{entries[0].stackId, 1},
        gameplay::BattleStackLifeResult{entries[0].stackId, 0}
    };

    REQUIRE_FALSE(session.ApplyBattleStackLifeResults(invalidResults, expectedStackIds));
    REQUIRE(session.OwnedUnitCount("unit_guard") == ownedGuardBefore);
    REQUIRE(session.OwnedUnitCount("unit_medic") == ownedMedicBefore);
    REQUIRE(session.ActiveSlotStackIds() == activeBefore);
    REQUIRE(session.ReserveSlotStackIds() == reserveBefore);
}

TEST_CASE("GameSession ApplyBattleStackLifeResults rejects missing and non-participating stack ids") {
    gameplay::GameSession session;
    REQUIRE(session.AddOwnedUnit("unit_guard", 2));
    REQUIRE(session.AddOwnedUnit("unit_medic", 1));
    REQUIRE(session.TryAddUnitToActiveParty("unit_guard"));
    REQUIRE(session.TryAddUnitToActiveParty("unit_medic"));

    const auto entries = session.BuildActiveBattleStackEntries();
    REQUIRE(entries.size() == 2);

    const std::vector<std::string> expectedStackIds = {entries[0].stackId, entries[1].stackId};

    const auto activeBefore = session.ActiveSlotStackIds();
    const auto reserveBefore = session.ReserveSlotStackIds();

    const std::vector<gameplay::BattleStackLifeResult> missingResult = {
        gameplay::BattleStackLifeResult{entries[0].stackId, 1}
    };
    REQUIRE_FALSE(session.ApplyBattleStackLifeResults(missingResult, expectedStackIds));
    REQUIRE(session.ActiveSlotStackIds() == activeBefore);
    REQUIRE(session.ReserveSlotStackIds() == reserveBefore);

    const std::vector<gameplay::BattleStackLifeResult> extraResult = {
        gameplay::BattleStackLifeResult{entries[0].stackId, 1},
        gameplay::BattleStackLifeResult{entries[1].stackId, 1},
        gameplay::BattleStackLifeResult{"stk_non_participating", 1}
    };
    REQUIRE_FALSE(session.ApplyBattleStackLifeResults(extraResult, expectedStackIds));
    REQUIRE(session.ActiveSlotStackIds() == activeBefore);
    REQUIRE(session.ReserveSlotStackIds() == reserveBefore);
}

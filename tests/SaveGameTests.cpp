#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

#include "core/SaveGame.h"

TEST_CASE("SaveGameRepository writes and reads save data") {
    const std::filesystem::path testSavePath = "saves/test_slot.json";

    core::SaveGameRepository repository;
    core::SaveData original;
    original.day = 3;
    original.minutesIntoSliceDay = 45;
    original.gold = 1337;
    original.mode = "overworld_mode";
    original.regionId = "ashvale_heartland";
    original.destinationId = "town_center";
    original.completedQuestIds = { "q_restore_well" };
    original.clearedCombatNodeIds = { "bridge_checkpoint" };
    original.recruitServiceStates = {
        core::RecruitServiceState{"survivor_post_recruitment", 2, 1}
    };
    original.dailyServiceStates = {
        core::DailyServiceState{"supply_cart_market", 0, 3}
    };
    original.travelPrepDiscountMinutes = 20;
    original.travelPrepRemainingCharges = 1;
    original.travelPrepGrantedDay = 3;
    original.ownedUnitCounts = {
        core::OwnedUnitCountSaveState{"unit_guard", 3},
        core::OwnedUnitCountSaveState{"unit_medic", 1}
    };
    original.activePartyUnitIds = {"unit_guard", "unit_medic", "unit_guard"};

    REQUIRE(repository.SaveToFile(original, testSavePath.string()));

    const auto loaded = repository.LoadFromFile(testSavePath.string());
    REQUIRE(loaded.has_value());

    REQUIRE(loaded->day == original.day);
    REQUIRE(loaded->minutesIntoSliceDay == original.minutesIntoSliceDay);
    REQUIRE(loaded->gold == original.gold);
    REQUIRE(loaded->mode == original.mode);
    REQUIRE(loaded->regionId == original.regionId);
    REQUIRE(loaded->destinationId == original.destinationId);
    REQUIRE(loaded->completedQuestIds == original.completedQuestIds);
    REQUIRE(loaded->clearedCombatNodeIds == original.clearedCombatNodeIds);

    REQUIRE(loaded->recruitServiceStates.size() == 1);
    REQUIRE(loaded->recruitServiceStates[0].serviceId == "survivor_post_recruitment");
    REQUIRE(loaded->recruitServiceStates[0].remainingStock == 2);
    REQUIRE(loaded->recruitServiceStates[0].lastRefreshWeek == 1);

    REQUIRE(loaded->dailyServiceStates.size() == 1);
    REQUIRE(loaded->dailyServiceStates[0].serviceId == "supply_cart_market");
    REQUIRE(loaded->dailyServiceStates[0].remainingUsesToday == 0);
    REQUIRE(loaded->dailyServiceStates[0].lastRefreshDay == 3);

    REQUIRE(loaded->travelPrepDiscountMinutes == 20);
    REQUIRE(loaded->travelPrepRemainingCharges == 1);
    REQUIRE(loaded->travelPrepGrantedDay == 3);

    REQUIRE(loaded->ownedUnitCounts.size() == 2);
    REQUIRE(loaded->ownedUnitCounts[0].unitId == "unit_guard");
    REQUIRE(loaded->ownedUnitCounts[0].count == 3);
    REQUIRE(loaded->ownedUnitCounts[1].unitId == "unit_medic");
    REQUIRE(loaded->ownedUnitCounts[1].count == 1);

    REQUIRE(loaded->activePartyUnitIds.size() == 3);
    REQUIRE(loaded->activePartyUnitIds[0] == "unit_guard");
    REQUIRE(loaded->activePartyUnitIds[1] == "unit_medic");
    REQUIRE(loaded->activePartyUnitIds[2] == "unit_guard");

    std::filesystem::remove(testSavePath);
}

TEST_CASE("SaveGameRepository skips malformed owned_unit_counts entries without failing load") {
    const std::filesystem::path testSavePath = "saves/test_slot_malformed_owned_units.json";

    std::ofstream output(testSavePath, std::ios::trunc);
    output << R"({
  "day": 2,
  "minutes_into_slice_day": 30,
  "gold": 900,
  "mode": "overworld_mode",
  "region_id": "ashvale_heartland",
  "destination_id": "home_base",
  "completed_quest_ids": [],
  "cleared_combat_node_ids": [],
  "recruit_service_states": [],
  "daily_service_states": [],
  "travel_prep_discount_minutes": 0,
  "travel_prep_remaining_charges": 0,
  "travel_prep_granted_day": 0,
  "owned_unit_counts": [
    {"unit_id":"unit_guard","count":2},
    {"unit_id":"unit_medic"},
    {"count":1},
    "bad",
    {"unit_id":42,"count":1},
    {"unit_id":"unit_scout","count":"3"}
  ],
  "active_party_unit_ids": ["unit_guard"]
})";
    output.close();

    core::SaveGameRepository repository;
    const auto loaded = repository.LoadFromFile(testSavePath.string());
    REQUIRE(loaded.has_value());

    REQUIRE(loaded->ownedUnitCounts.size() == 1);
    REQUIRE(loaded->ownedUnitCounts[0].unitId == "unit_guard");
    REQUIRE(loaded->ownedUnitCounts[0].count == 2);
    REQUIRE(loaded->activePartyUnitIds.size() == 1);
    REQUIRE(loaded->activePartyUnitIds[0] == "unit_guard");

    std::filesystem::remove(testSavePath);
}

TEST_CASE("SaveGameRepository loads old saves without roster fields") {
    const std::filesystem::path testSavePath = "saves/test_slot_old_schema.json";

    std::ofstream output(testSavePath, std::ios::trunc);
    output << R"({
  "day": 2,
  "minutes_into_slice_day": 30,
  "gold": 900,
  "mode": "overworld_mode",
  "region_id": "ashvale_heartland",
  "destination_id": "home_base",
  "completed_quest_ids": [],
  "cleared_combat_node_ids": [],
  "recruit_service_states": [],
  "daily_service_states": [],
  "travel_prep_discount_minutes": 0,
  "travel_prep_remaining_charges": 0,
  "travel_prep_granted_day": 0
})";
    output.close();

    core::SaveGameRepository repository;
    const auto loaded = repository.LoadFromFile(testSavePath.string());
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->ownedUnitCounts.empty());
    REQUIRE(loaded->activePartyUnitIds.empty());

    std::filesystem::remove(testSavePath);
}

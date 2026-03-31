#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

#include "core/SaveGame.h"

TEST_CASE("SaveGameRepository writes and reads save data") {
    const std::filesystem::path testSavePath = "saves/test_slot.json";

    core::SaveGameRepository repository;
    core::SaveData original;
    original.schemaVersion = 3;
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
    original.hasCanonicalRoster = true;
    original.rosterStacks = {
        core::RosterStackSaveState{"stk_1", "unit_guard", 3},
        core::RosterStackSaveState{"stk_2", "unit_medic", 1},
        core::RosterStackSaveState{"stk_3", "unit_scout", 1},
        core::RosterStackSaveState{"stk_4", "unit_lancer", 1},
        core::RosterStackSaveState{"stk_5", "unit_arcanist", 1},
        core::RosterStackSaveState{"stk_6", "unit_miner", 1}
    };
    original.activeSlotStackIds = {"stk_1", "stk_2", "stk_3", "stk_4", "stk_5"};
    original.reserveSlotStackIds = {"stk_6", "", "", "", "", "", "", ""};
    original.nextStackIdCounter = 7;

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

    REQUIRE(loaded->hasCanonicalRoster);
    REQUIRE(loaded->rosterStacks.size() == 6);
    REQUIRE(loaded->rosterStacks[0].stackId == "stk_1");
    REQUIRE(loaded->rosterStacks[0].unitId == "unit_guard");
    REQUIRE(loaded->rosterStacks[0].quantity == 3);
    REQUIRE(loaded->activeSlotStackIds == std::vector<std::string>{"stk_1", "stk_2", "stk_3", "stk_4", "stk_5"});
    REQUIRE(loaded->reserveSlotStackIds == std::vector<std::string>{"stk_6", "", "", "", "", "", "", ""});
    REQUIRE(loaded->nextStackIdCounter == 7);

    REQUIRE(loaded->ownedUnitCounts.empty());
    REQUIRE(loaded->activePartyUnitIds.empty());

    std::filesystem::remove(testSavePath);
}

TEST_CASE("SaveGameRepository legacy upgrade is deterministic") {
    const std::filesystem::path testSavePath = "saves/test_slot_legacy_upgrade.json";

    std::ofstream output(testSavePath, std::ios::trunc);
    output << R"({
  "schema_version": 3,
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
    {"unit_id":"unit_guard","count":3},
    {"unit_id":"unit_medic","count":1}
  ],
  "active_party_unit_ids": ["unit_guard", "unit_medic", "unit_guard"]
})";
    output.close();

    core::SaveGameRepository repository;
    const auto loaded = repository.LoadFromFile(testSavePath.string());
    REQUIRE(loaded.has_value());

    REQUIRE(loaded->hasCanonicalRoster);
    REQUIRE(loaded->rosterStacks.size() == 4);
    REQUIRE(loaded->activeSlotStackIds == std::vector<std::string>{"stk_1", "stk_2", "stk_3", "", ""});
    REQUIRE(loaded->reserveSlotStackIds == std::vector<std::string>{"stk_4", "", "", "", "", "", "", ""});
    REQUIRE(loaded->nextStackIdCounter == 5);

    std::filesystem::remove(testSavePath);
}

TEST_CASE("SaveGameRepository malformed canonical roster fails load") {
    const std::filesystem::path testSavePath = "saves/test_slot_malformed_canonical.json";

    std::ofstream output(testSavePath, std::ios::trunc);
    output << R"({
  "schema_version": 3,
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
  "roster_stacks": [{"stack_id":"stk_1","unit_id":"unit_guard","quantity":2}],
  "active_slot_stack_ids": ["stk_1", "", ""],
  "reserve_slot_stack_ids": ["stk_1", "", "", "", "", "", "", ""]
})";
    output.close();

    core::SaveGameRepository repository;
    const auto loaded = repository.LoadFromFile(testSavePath.string());
    REQUIRE_FALSE(loaded.has_value());

    std::filesystem::remove(testSavePath);
}

TEST_CASE("SaveGameRepository missing or invalid canonical counter is recomputed deterministically") {
    const std::filesystem::path testSavePath = "saves/test_slot_canonical_counter_recompute.json";

    std::ofstream output(testSavePath, std::ios::trunc);
    output << R"({
  "schema_version": 3,
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
  "roster_stacks": [
    {"stack_id":"stk_4","unit_id":"unit_guard","quantity":2},
    {"stack_id":"stk_9","unit_id":"unit_medic","quantity":1}
  ],
  "active_slot_stack_ids": ["stk_4", "", "", "", ""],
  "reserve_slot_stack_ids": ["stk_9", "", "", "", "", "", "", ""],
  "next_stack_id_counter": 0
})";
    output.close();

    core::SaveGameRepository repository;
    const auto loaded = repository.LoadFromFile(testSavePath.string());
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->nextStackIdCounter == 10);

    std::filesystem::remove(testSavePath);
}

TEST_CASE("SaveGameRepository upgrades canonical schema v2 three-slot active arrays to five slots") {
    const std::filesystem::path testSavePath = "saves/test_slot_v2_canonical_upgrade.json";

    std::ofstream output(testSavePath, std::ios::trunc);
    output << R"({
  "schema_version": 2,
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
  "roster_stacks": [
    {"stack_id":"stk_1","unit_id":"unit_guard","quantity":2},
    {"stack_id":"stk_2","unit_id":"unit_medic","quantity":1}
  ],
  "active_slot_stack_ids": ["stk_1", "", "stk_2"],
  "reserve_slot_stack_ids": ["", "", "", "", "", "", "", ""],
  "next_stack_id_counter": 3
})";
    output.close();

    core::SaveGameRepository repository;
    const auto loaded = repository.LoadFromFile(testSavePath.string());
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->schemaVersion == 2);
    REQUIRE(loaded->activeSlotStackIds == std::vector<std::string>{"stk_1", "", "stk_2", "", ""});
    REQUIRE(loaded->reserveSlotStackIds == std::vector<std::string>{"", "", "", "", "", "", "", ""});

    std::filesystem::remove(testSavePath);
}

TEST_CASE("SaveGameRepository legacy upgrade fills active slots up to five before reserve") {
    const std::filesystem::path testSavePath = "saves/test_slot_legacy_five_active_fill.json";

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
    {"unit_id":"unit_guard","count":3},
    {"unit_id":"unit_medic","count":2},
    {"unit_id":"unit_scout","count":1}
  ],
  "active_party_unit_ids": ["unit_guard", "unit_medic", "unit_guard", "unit_medic", "unit_guard", "unit_scout"]
})";
    output.close();

    core::SaveGameRepository repository;
    const auto loaded = repository.LoadFromFile(testSavePath.string());
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->hasCanonicalRoster);
    REQUIRE(loaded->activeSlotStackIds == std::vector<std::string>{"stk_1", "stk_2", "stk_3", "stk_4", "stk_5"});
    REQUIRE(loaded->reserveSlotStackIds == std::vector<std::string>{"stk_6", "", "", "", "", "", "", ""});

    std::filesystem::remove(testSavePath);
}

TEST_CASE("SaveGameRepository legacy reserve overflow fails load") {
    const std::filesystem::path testSavePath = "saves/test_slot_legacy_overflow.json";

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
    {"unit_id":"unit_01","count":1},
    {"unit_id":"unit_02","count":1},
    {"unit_id":"unit_03","count":1},
    {"unit_id":"unit_04","count":1},
    {"unit_id":"unit_05","count":1},
    {"unit_id":"unit_06","count":1},
    {"unit_id":"unit_07","count":1},
    {"unit_id":"unit_08","count":1},
    {"unit_id":"unit_09","count":1}
  ],
  "active_party_unit_ids": []
})";
    output.close();

    core::SaveGameRepository repository;
    const auto loaded = repository.LoadFromFile(testSavePath.string());
    REQUIRE_FALSE(loaded.has_value());

    std::filesystem::remove(testSavePath);
}

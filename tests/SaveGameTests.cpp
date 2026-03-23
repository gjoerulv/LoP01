#include <catch2/catch_test_macros.hpp>

#include <filesystem>

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

    std::filesystem::remove(testSavePath);
}

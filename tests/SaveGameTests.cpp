#include <catch2/catch_test_macros.hpp>

#include <filesystem>

#include "core/SaveGame.h"

TEST_CASE("SaveGameRepository writes and reads save data") {
    const std::filesystem::path testSavePath = "saves/test_slot.json";

    core::SaveGameRepository repository;
    const core::SaveData original{
        3,
        45,
        1337,
        "overworld_mode",
        "ashvale_heartland",
        "town_center"
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

    std::filesystem::remove(testSavePath);
}

#include <catch2/catch_test_macros.hpp>

#include "core/GameClock.h"

TEST_CASE("GameClock starts at day 1 and 06:00") {
    core::GameClock clock;

    REQUIRE(clock.Day() == 1);
    REQUIRE(clock.TimeString() == "06:00");
}

TEST_CASE("GameClock rolls over to next day at 02:00") {
    core::GameClock clock;

    clock.AdvanceMinutes(core::GameClock::kMinutesPerSliceDay);

    REQUIRE(clock.Day() == 2);
    REQUIRE(clock.TimeString() == "06:00");
}

TEST_CASE("Travel time quantization is clamped and rounded up") {
    REQUIRE(core::GameClock::QuantizeTravelMinutes(1) == 15);
    REQUIRE(core::GameClock::QuantizeTravelMinutes(16) == 30);
    REQUIRE(core::GameClock::QuantizeTravelMinutes(240) == 240);
    REQUIRE(core::GameClock::QuantizeTravelMinutes(600) == 240);
}

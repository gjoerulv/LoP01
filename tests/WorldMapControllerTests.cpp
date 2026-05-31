#include <catch2/catch_test_macros.hpp>

#include "app/WorldMapController.h"
#include "app/input/InputState.h"

using app::WorldMapController;
using app::input::InputState;

namespace {
InputState NoInput() { return InputState{}; }
}

TEST_CASE("WorldMapController - selectNext / selectPrev wrap around") {
    WorldMapController controller;

    InputState next = NoInput();
    next.selectNext = true;
    REQUIRE(controller.Update(next, 3, 0).selectedIndex == 1);
    REQUIRE(controller.Update(next, 3, 2).selectedIndex == 0); // wrap

    InputState prev = NoInput();
    prev.selectPrev = true;
    REQUIRE(controller.Update(prev, 3, 0).selectedIndex == 2); // wrap
    REQUIRE(controller.Update(prev, 3, 2).selectedIndex == 1);
}

TEST_CASE("WorldMapController - confirm sets travelConfirmed") {
    WorldMapController controller;
    InputState confirm = NoInput();
    confirm.confirm = true;
    const auto result = controller.Update(confirm, 2, 1);
    REQUIRE(result.travelConfirmed);
    REQUIRE_FALSE(result.cancelled);
    REQUIRE(result.selectedIndex == 1);
}

TEST_CASE("WorldMapController - cancel and re-pressing M both close the screen") {
    WorldMapController controller;

    InputState cancel = NoInput();
    cancel.cancel = true;
    REQUIRE(controller.Update(cancel, 2, 0).cancelled);

    InputState reopen = NoInput();
    reopen.openWorldMap = true;
    REQUIRE(controller.Update(reopen, 2, 0).cancelled);
}

TEST_CASE("WorldMapController - cancel takes precedence over confirm") {
    WorldMapController controller;
    InputState both = NoInput();
    both.cancel = true;
    both.confirm = true;
    const auto result = controller.Update(both, 2, 0);
    REQUIRE(result.cancelled);
    REQUIRE_FALSE(result.travelConfirmed);
}

TEST_CASE("WorldMapController - zero destinations still allows cancel, no confirm") {
    WorldMapController controller;
    InputState confirm = NoInput();
    confirm.confirm = true;
    const auto confirmResult = controller.Update(confirm, 0, 0);
    REQUIRE_FALSE(confirmResult.travelConfirmed);

    InputState cancel = NoInput();
    cancel.cancel = true;
    REQUIRE(controller.Update(cancel, 0, 0).cancelled);
}

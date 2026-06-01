#include <catch2/catch_test_macros.hpp>

#include "app/CampaignController.h"

using app::CampaignController;
using app::input::InputState;

namespace {
InputState SelectNext() { InputState i; i.selectNext = true; return i; }
InputState SelectPrev() { InputState i; i.selectPrev = true; return i; }
InputState Confirm()    { InputState i; i.confirm = true; return i; }
InputState Cancel()     { InputState i; i.cancel = true; return i; }
} // namespace

TEST_CASE("CampaignController: selectNext/prev wrap around the list") {
    CampaignController controller;
    REQUIRE(controller.Update(SelectNext(), 3, 0).selectedIndex == 1);
    REQUIRE(controller.Update(SelectNext(), 3, 2).selectedIndex == 0);   // wrap
    REQUIRE(controller.Update(SelectPrev(), 3, 0).selectedIndex == 2);   // wrap
    REQUIRE(controller.Update(SelectPrev(), 3, 2).selectedIndex == 1);
}

TEST_CASE("CampaignController: confirm commits the current selection") {
    CampaignController controller;
    const auto result = controller.Update(Confirm(), 3, 1);
    REQUIRE(result.confirmed);
    REQUIRE_FALSE(result.cancelled);
    REQUIRE(result.selectedIndex == 1);
}

TEST_CASE("CampaignController: cancel backs out without confirming") {
    CampaignController controller;
    const auto result = controller.Update(Cancel(), 3, 1);
    REQUIRE(result.cancelled);
    REQUIRE_FALSE(result.confirmed);
}

TEST_CASE("CampaignController: empty list honors cancel and never confirms") {
    CampaignController controller;
    REQUIRE_FALSE(controller.Update(Confirm(), 0, 0).confirmed);
    REQUIRE(controller.Update(Cancel(), 0, 0).cancelled);
    REQUIRE(controller.Update(SelectNext(), 0, 0).selectedIndex == 0);
}

TEST_CASE("CampaignController: cancel takes priority over confirm") {
    CampaignController controller;
    InputState both; both.cancel = true; both.confirm = true;
    const auto result = controller.Update(both, 2, 0);
    REQUIRE(result.cancelled);
    REQUIRE_FALSE(result.confirmed);
}

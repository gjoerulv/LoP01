#include <catch2/catch_test_macros.hpp>

#include "Location.h"

TEST_CASE("Location stores id, name and description", "[location]")
{
    Location loc("town_square", "Town Square", "A bustling plaza.");

    REQUIRE(loc.GetId()          == "town_square");
    REQUIRE(loc.GetName()        == "Town Square");
    REQUIRE(loc.GetDescription() == "A bustling plaza.");
}

TEST_CASE("Location exits are registered and queried correctly", "[location]")
{
    Location loc("start", "Start", "Starting room.");
    loc.AddExit("north", "room_b");
    loc.AddExit("east",  "room_c");

    REQUIRE(loc.HasExit("north"));
    REQUIRE(loc.HasExit("east"));
    REQUIRE_FALSE(loc.HasExit("south"));
    REQUIRE_FALSE(loc.HasExit("west"));

    REQUIRE(loc.GetExitDestination("north") == "room_b");
    REQUIRE(loc.GetExitDestination("east")  == "room_c");
}

TEST_CASE("GetExitDestination throws for unknown direction", "[location]")
{
    Location loc("room", "Room", "A plain room.");
    REQUIRE_THROWS_AS(loc.GetExitDestination("south"), std::invalid_argument);
}

TEST_CASE("GetExits returns all registered exits", "[location]")
{
    Location loc("hub", "Hub", "Central hub.");
    loc.AddExit("north", "n");
    loc.AddExit("south", "s");

    const auto& exits = loc.GetExits();
    REQUIRE(exits.size() == 2);
    REQUIRE(exits.at("north") == "n");
    REQUIRE(exits.at("south") == "s");
}

#include <catch2/catch_test_macros.hpp>

#include "Game.h"

#include <string>

// Helper: advance past title screen.
static Game startedGame()
{
    Game g;
    g.ProcessCommand("start");
    return g;
}

// ── Title screen ──────────────────────────────────────────────────────────────

TEST_CASE("Game shows title screen before start", "[game]")
{
    Game g;
    REQUIRE(g.IsRunning());
    const std::string title = g.RenderCurrentScene();
    REQUIRE(title.find("LANDS OF PERIL") != std::string::npos);
}

TEST_CASE("Game ignores unknown commands before start", "[game]")
{
    Game g;
    const std::string resp = g.ProcessCommand("north");
    REQUIRE(resp.find("start") != std::string::npos);
}

// ── Starting location ────────────────────────────────────────────────────────

TEST_CASE("'start' transitions to Town Square", "[game]")
{
    Game g;
    const std::string resp = g.ProcessCommand("start");
    REQUIRE(resp.find("Town Square") != std::string::npos);
}

// ── Navigation ───────────────────────────────────────────────────────────────

TEST_CASE("Player can move north from Town Square to Market District", "[game]")
{
    Game g = startedGame();
    const std::string resp = g.ProcessCommand("north");
    REQUIRE(resp.find("Market District") != std::string::npos);
}

TEST_CASE("Player can move east from Town Square to Blacksmith", "[game]")
{
    Game g = startedGame();
    const std::string resp = g.ProcessCommand("east");
    REQUIRE(resp.find("Blacksmith") != std::string::npos);
}

TEST_CASE("Player can move south from Town Square to South Gate", "[game]")
{
    Game g = startedGame();
    const std::string resp = g.ProcessCommand("south");
    REQUIRE(resp.find("South Gate") != std::string::npos);
}

TEST_CASE("Direction aliases work (n/s/e/w)", "[game]")
{
    Game g = startedGame();
    REQUIRE(g.ProcessCommand("n").find("Market District")  != std::string::npos);
}

TEST_CASE("'go <direction>' alias works", "[game]")
{
    Game g = startedGame();
    REQUIRE(g.ProcessCommand("go north").find("Market District") != std::string::npos);
}

TEST_CASE("Moving into unavailable direction returns error message", "[game]")
{
    Game g = startedGame();
    // Town Square has no west exit.
    const std::string resp = g.ProcessCommand("west");
    REQUIRE(resp.find("can't go that way") != std::string::npos);
}

// ── Look ────────────────────────────────────────────────────────────────────

TEST_CASE("'look' re-renders current location", "[game]")
{
    Game g = startedGame();
    const std::string look = g.ProcessCommand("look");
    REQUIRE(look.find("Town Square") != std::string::npos);
}

TEST_CASE("'l' alias re-renders current location", "[game]")
{
    Game g = startedGame();
    REQUIRE(g.ProcessCommand("l").find("Town Square") != std::string::npos);
}

// ── Multi-step navigation ────────────────────────────────────────────────────

TEST_CASE("Player can navigate to Darkwood Forest via market and tavern", "[game]")
{
    Game g = startedGame();
    g.ProcessCommand("north"); // -> market
    g.ProcessCommand("east");  // -> tavern
    const std::string forest = g.ProcessCommand("north"); // -> darkwood_forest
    REQUIRE(forest.find("Darkwood Forest") != std::string::npos);
}

TEST_CASE("Player can return from Darkwood Forest to Tavern", "[game]")
{
    Game g = startedGame();
    g.ProcessCommand("north"); // -> market
    g.ProcessCommand("east");  // -> tavern
    g.ProcessCommand("north"); // -> darkwood_forest
    const std::string resp = g.ProcessCommand("south"); // -> tavern
    REQUIRE(resp.find("Rusty Flagon") != std::string::npos);
}

// ── Quit ─────────────────────────────────────────────────────────────────────

TEST_CASE("'quit' stops the game loop", "[game]")
{
    Game g = startedGame();
    REQUIRE(g.IsRunning());
    g.ProcessCommand("quit");
    REQUIRE_FALSE(g.IsRunning());
}

TEST_CASE("'q' alias stops the game loop", "[game]")
{
    Game g = startedGame();
    g.ProcessCommand("q");
    REQUIRE_FALSE(g.IsRunning());
}

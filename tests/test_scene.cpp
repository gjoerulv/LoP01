#include <catch2/catch_test_macros.hpp>

#include "Scene.h"
#include "Location.h"

#include <string>

TEST_CASE("Title scene renders correctly", "[scene]")
{
    Scene scene(Scene::Type::Title);
    REQUIRE(scene.GetType() == Scene::Type::Title);
    const std::string output = scene.Render();
    REQUIRE(output.find("LANDS OF PERIL") != std::string::npos);
    REQUIRE(output.find("start") != std::string::npos);
}

TEST_CASE("GameOver scene renders correctly", "[scene]")
{
    Scene scene(Scene::Type::GameOver);
    REQUIRE(scene.GetType() == Scene::Type::GameOver);
    const std::string output = scene.Render();
    REQUIRE(output.find("GAME OVER") != std::string::npos);
}

TEST_CASE("Location scene renders name, description and exits", "[scene]")
{
    Location loc("forest", "Dark Forest", "Tall trees surround you.");
    loc.AddExit("south", "town");

    Scene scene(loc);
    REQUIRE(scene.GetType() == Scene::Type::Location);
    const std::string output = scene.Render();

    REQUIRE(output.find("Dark Forest") != std::string::npos);
    REQUIRE(output.find("Tall trees surround you.") != std::string::npos);
    REQUIRE(output.find("south") != std::string::npos);
}

TEST_CASE("Location scene with no exits shows no-exit message", "[scene]")
{
    Location loc("dead_end", "Dead End", "A dead end.");

    Scene scene(loc);
    const std::string output = scene.Render();
    REQUIRE(output.find("no exits") != std::string::npos);
}

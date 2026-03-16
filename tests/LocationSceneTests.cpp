#include <catch2/catch_test_macros.hpp>

#include "gameplay/GameSession.h"
#include "gameplay/location/LocationScene.h"

TEST_CASE("LocationScene blocks movement on rectangular collision") {
    gameplay::location::LocationScene scene;

    const auto before = scene.Player();
    const bool movedIntoWall = scene.TryMovePlayer(80.0f, -250.0f);

    REQUIRE_FALSE(movedIntoWall);
    REQUIRE(scene.Player().x == before.x);
    REQUIRE(scene.Player().y == before.y);
}

TEST_CASE("NPC interaction exposes dialogue choices") {
    gameplay::location::LocationScene scene;

    REQUIRE(scene.TryMovePlayer(380.0f, -20.0f));
    const auto interaction = scene.Interact();

    REQUIRE(interaction.has_value());
    REQUIRE(interaction->requiresDialogueChoice);
    REQUIRE(scene.HasActiveDialogue());
    REQUIRE(scene.ActiveDialogueChoices().size() >= 2);

    const auto choice = scene.ChooseDialogueOption(0);
    REQUIRE(choice.has_value());
    REQUIRE_FALSE(scene.HasActiveDialogue());
}

TEST_CASE("Location action costs follow minute and gold rules") {
    gameplay::GameSession session;
    session.EnterLocationMode("town_center");

    auto snapshot = session.Snapshot();
    const int initialGold = snapshot.gold;
    const std::string initialTime = snapshot.time;

    session.ApplyDoorOpenCost();
    session.ApplyDialogueChoiceCost();
    const bool shopped = session.ApplyShopCost(50);
    const bool recruited = session.ApplyRecruitCost(120);

    REQUIRE(shopped);
    REQUIRE(recruited);

    snapshot = session.Snapshot();
    REQUIRE(snapshot.gold == initialGold - 170);
    REQUIRE(snapshot.time != initialTime);
}

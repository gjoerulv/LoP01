#include <catch2/catch_test_macros.hpp>

#include "data/definitions/LocationSceneDefinition.h"
#include "gameplay/GameSession.h"
#include "gameplay/location/LocationScene.h"

namespace
{
	data::LocationSceneDefinition MakeTestScene()
	{
		using data::LocationSceneDefinition;
		using data::LocationSceneZoneDefinition;
		using data::LocationSceneZoneType;
		using data::SceneRectDefinition;

		LocationSceneDefinition scene;
		scene.id = "test_scene";
		scene.spawn = SceneRectDefinition{ 84.0f, 520.0f, 24.0f, 24.0f };

		scene.blockingRects = {
			SceneRectDefinition{0.0f, 0.0f, 1280.0f, 40.0f},
			SceneRectDefinition{0.0f, 0.0f, 40.0f, 720.0f},
			SceneRectDefinition{1240.0f, 0.0f, 40.0f, 720.0f},
			SceneRectDefinition{0.0f, 680.0f, 1280.0f, 40.0f},
			SceneRectDefinition{120.0f, 140.0f, 260.0f, 180.0f},
			SceneRectDefinition{520.0f, 140.0f, 260.0f, 180.0f},
			SceneRectDefinition{920.0f, 140.0f, 260.0f, 180.0f}
		};

		scene.zones = {
			LocationSceneZoneDefinition{
				"inn_door",
				LocationSceneZoneType::InnDoor,
				SceneRectDefinition{220.0f, 320.0f, 80.0f, 20.0f},
				"E: Open Inn Door",
				"Opened inn door",
				"",
				1,
				0,
				0,
				1,
				{}
			},
			LocationSceneZoneDefinition{
				"shop_counter",
				LocationSceneZoneType::Shop,
				SceneRectDefinition{620.0f, 320.0f, 80.0f, 20.0f},
				"E: Shop",
				"Bought supplies",
				"Not enough gold for shopping",
				5,
				50,
				0,
				1,
				{}
			},
			LocationSceneZoneDefinition{
				"recruit_board",
				LocationSceneZoneType::Recruit,
				SceneRectDefinition{1020.0f, 320.0f, 80.0f, 20.0f},
				"E: Recruit",
				"Recruit completed",
				"Not enough gold to recruit",
				10,
				120,
				1,
				1,
				{}
			},
			LocationSceneZoneDefinition{
				"npc_old_man",
				LocationSceneZoneType::Npc,
				SceneRectDefinition{460.0f, 500.0f, 40.0f, 40.0f},
				"E: Talk",
				"",
				"",
				0,
				0,
				0,
				1,
				{"Ask about the town", "Ask about the mine"}
			},
			LocationSceneZoneDefinition{
				"npc_guard",
				LocationSceneZoneType::Npc,
				SceneRectDefinition{760.0f, 500.0f, 40.0f, 40.0f},
				"E: Talk",
				"",
				"",
				0,
				0,
				0,
				1,
				{"Ask about patrol routes", "Ask about recruitment"}
			},
			LocationSceneZoneDefinition{
				"town_exit",
				LocationSceneZoneType::Exit,
				SceneRectDefinition{70.0f, 560.0f, 80.0f, 80.0f},
				"E: Exit To Overworld",
				"Exited to overworld",
				"",
				0,
				0,
				0,
				1,
				{}
			}
		};

		return scene;
	}
}

TEST_CASE("LocationScene blocks movement on rectangular collision") {
	gameplay::location::LocationScene scene;
	scene.Reset(MakeTestScene());

	const auto before = scene.Player();
	const bool movedIntoWall = scene.TryMovePlayer(80.0f, -250.0f);

	REQUIRE_FALSE(movedIntoWall);
	REQUIRE(scene.Player().x == before.x);
	REQUIRE(scene.Player().y == before.y);
}

TEST_CASE("NPC interaction exposes dialogue choices") {
	gameplay::location::LocationScene scene;
	scene.Reset(MakeTestScene());

	REQUIRE(scene.TryMovePlayer(380.0f, -20.0f));
	const auto interaction = scene.Interact();

	REQUIRE(interaction.has_value());
	REQUIRE(interaction->requiresDialogueChoice);
	REQUIRE(scene.HasActiveDialogue());
	REQUIRE(scene.ActiveDialogueChoices().size() >= 2);

	const auto choice = scene.ChooseDialogueOption(0);
	REQUIRE(choice.has_value());
	REQUIRE_FALSE(scene.HasActiveDialogue());

	REQUIRE(choice->timeCostMinutes == 1);
	REQUIRE(choice->message == "Ask about the town");
}

TEST_CASE("Location action costs follow minute and gold rules") {
	gameplay::GameSession session;
	session.EnterLocationMode("town_center");

	auto snapshot = session.Snapshot();
	const int initialGold = snapshot.gold;
	const std::string initialTime = snapshot.time;

	session.ApplyDoorOpenCost();
	session.ApplyDialogueChoiceCost();
	
	REQUIRE(session.TrySpendGold(50));
	session.AddMinutes(5);

	REQUIRE(session.TrySpendGold(120));
	session.AddMinutes(10);

	snapshot = session.Snapshot();
	REQUIRE(snapshot.gold == initialGold - 170);
	REQUIRE(snapshot.time != initialTime);
}

TEST_CASE("Inn marker interaction resolves as inn-door action") {
	gameplay::location::LocationScene scene;
	scene.Reset(MakeTestScene());

	REQUIRE(scene.TryMovePlayer(140.0f, -200.0f));
	const auto interaction = scene.Interact();

	REQUIRE(interaction.has_value());
	REQUIRE(interaction->type == gameplay::location::InteractionType::InnDoor);
	REQUIRE_FALSE(interaction->requiresDialogueChoice);
}

TEST_CASE("Shop and recruit interactions are not inn-door actions") {
	gameplay::location::LocationScene scene;
	scene.Reset(MakeTestScene());

	REQUIRE(scene.TryMovePlayer(540.0f, -200.0f));
	const auto shopInteraction = scene.Interact();
	REQUIRE(shopInteraction.has_value());
	REQUIRE(shopInteraction->type == gameplay::location::InteractionType::Shop);
	REQUIRE(shopInteraction->type != gameplay::location::InteractionType::InnDoor);

	scene.Reset(MakeTestScene());
	REQUIRE(scene.TryMovePlayer(940.0f, -200.0f));
	const auto recruitInteraction = scene.Interact();
	REQUIRE(recruitInteraction.has_value());
	REQUIRE(recruitInteraction->type == gameplay::location::InteractionType::Recruit);
	REQUIRE(recruitInteraction->type != gameplay::location::InteractionType::InnDoor);
}
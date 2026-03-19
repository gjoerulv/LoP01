#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

#include "app/mappers/OverworldModelMapper.h"
#include "data/ContentRepository.h"
#include "gameplay/GameSession.h"

namespace {

void WriteTextFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream output(path, std::ios::trunc);
    output << content;
}

std::filesystem::path BuildOverworldMapperTestContent() {
    const std::filesystem::path root = "saves/overworld_mapper_test";
    std::filesystem::create_directories(root);

    WriteTextFile(root / "regions.json", R"({"regions":[{"id":"ashvale_heartland","name":"Ashvale Heartland","unlocked":true,"nodes":[{"location_id":"home_base","x":0,"y":0,"discovered":true,"travel_available":true},{"location_id":"bridge_checkpoint","x":1,"y":0,"discovered":true,"travel_available":true},{"location_id":"clocktower_square","x":2,"y":0,"discovered":true,"travel_available":true}],"links":[["home_base","bridge_checkpoint"],["bridge_checkpoint","clocktower_square"]]}]})");
    WriteTextFile(root / "locations.json", R"({"locations":[{"id":"home_base","name":"Home Base","type":"home","allows_sleep":true,"overworld_destination":true,"scene_id":"town_square_proto"},{"id":"bridge_checkpoint","name":"Bridge Checkpoint","type":"combat","allows_sleep":false,"blocks_transit_until_cleared":true,"overworld_destination":true,"battle_scenario_id":"debug_intro_battle"},{"id":"clocktower_square","name":"Clocktower Square","type":"town","allows_sleep":false,"overworld_destination":true,"scene_id":"town_square_proto"}]})");
    WriteTextFile(root / "location_scenes.json", R"({"location_scenes":[{"id":"town_square_proto","spawn":{"x":0,"y":0,"width":1,"height":1},"blocking_rects":[],"zones":[]}]})");
    WriteTextFile(root / "units.json", R"({"units":[{"id":"hero","name":"Hero","category":"hero","is_player_character":true,"attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":1,"life":1,"position":"front","range":"melee"}]})");
    WriteTextFile(root / "battle_scenarios.json", R"({"battle_scenarios":[{"id":"debug_intro_battle","name":"Debug","seed":7,"allies":[{"unit_id":"hero"}],"enemies":[{"unit_id":"hero"}]}]})");
    WriteTextFile(root / "enemy_groups.json", R"({"enemy_groups":[]})");
    WriteTextFile(root / "quests.json", R"({"quests":[]})");

    return root;
}

} // namespace

TEST_CASE("OverworldModelMapper exposes cleared combat node text") {
    const auto root = BuildOverworldMapperTestContent();

    data::ContentRepository repository;
    REQUIRE(repository.LoadFromDirectory(root));

    gameplay::SessionSnapshot snapshot;
    snapshot.mode = gameplay::GameMode::OverworldMode;
    snapshot.regionId = "ashvale_heartland";
    snapshot.destinationId = "home_base";
    snapshot.minutesIntoSliceDay = 0;

    app::mappers::OverworldModelMapper mapper;
    const auto model = mapper.Map(repository, snapshot, 1, {"bridge_checkpoint"});

    REQUIRE(model.selectedNodeLabel == "Bridge Checkpoint");
    REQUIRE(model.selectedNodeType.find("Node: Cleared") != std::string::npos);
    REQUIRE(model.selectedNodeEnterable.find("Battle: No (Cleared)") != std::string::npos);

    std::filesystem::remove_all(root);
}

TEST_CASE("OverworldModelMapper exposes uncleared blocker travel text") {
    const auto root = BuildOverworldMapperTestContent();

    data::ContentRepository repository;
    REQUIRE(repository.LoadFromDirectory(root));

    gameplay::SessionSnapshot snapshot;
    snapshot.mode = gameplay::GameMode::OverworldMode;
    snapshot.regionId = "ashvale_heartland";
    snapshot.destinationId = "home_base";
    snapshot.minutesIntoSliceDay = 0;

    app::mappers::OverworldModelMapper mapper;
    const auto model = mapper.Map(repository, snapshot, 2, {});

    REQUIRE(model.selectedNodeLabel == "Clocktower Square");
    REQUIRE(model.selectedNodeEnterable.find("blocked by uncleared route blocker") != std::string::npos);

    std::filesystem::remove_all(root);
}

TEST_CASE("OverworldModelMapper exposes past-02:00 blocked reason text") {
    const auto root = BuildOverworldMapperTestContent();

    data::ContentRepository repository;
    REQUIRE(repository.LoadFromDirectory(root));

    gameplay::SessionSnapshot snapshot;
    snapshot.mode = gameplay::GameMode::OverworldMode;
    snapshot.regionId = "ashvale_heartland";
    snapshot.destinationId = "home_base";
    snapshot.minutesIntoSliceDay = 1180;

    app::mappers::OverworldModelMapper mapper;
    const auto model = mapper.Map(repository, snapshot, 2, {"bridge_checkpoint"});

    REQUIRE(model.selectedNodeLabel == "Clocktower Square");
    REQUIRE(model.selectedNodeEnterable.find("arrives past 02:00") != std::string::npos);

    std::filesystem::remove_all(root);
}

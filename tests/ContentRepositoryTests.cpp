#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

#include "data/ContentRepository.h"

namespace {

void WriteTextFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream output(path, std::ios::trunc);
    output << content;
}

} // namespace

TEST_CASE("ContentRepository loads blocks_transit_until_cleared flag") {
    const std::filesystem::path root = "saves/content_repo_test";
    std::filesystem::create_directories(root);

    WriteTextFile(root / "regions.json", R"({"regions":[{"id":"ashvale_heartland","name":"Ashvale Heartland","unlocked":true,"nodes":[{"location_id":"bridge_checkpoint","x":0,"y":0,"discovered":true,"travel_available":true},{"location_id":"orchard_pass","x":1,"y":1,"discovered":true,"travel_available":true}],"links":[["bridge_checkpoint","orchard_pass"]]}]})");
    WriteTextFile(root / "locations.json", R"({"locations":[{"id":"bridge_checkpoint","name":"Bridge Checkpoint","type":"combat","allows_sleep":false,"blocks_transit_until_cleared":true,"overworld_destination":true,"battle_scenario_id":"debug_intro_battle"},{"id":"orchard_pass","name":"Orchard Pass","type":"combat","allows_sleep":false,"overworld_destination":true,"battle_scenario_id":"debug_intro_battle"}]})");
    WriteTextFile(root / "location_scenes.json", R"({"location_scenes":[{"id":"town_square_proto","spawn":{"x":0,"y":0,"width":1,"height":1},"blocking_rects":[],"zones":[]}]})");
    WriteTextFile(root / "units.json", R"({"units":[{"id":"hero","name":"Hero","category":"hero","is_player_character":true,"attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":1,"life":1,"position":"front","range":"melee"}]})");
    WriteTextFile(root / "battle_scenarios.json", R"({"battle_scenarios":[{"id":"debug_intro_battle","name":"Debug","seed":7,"allies":[{"unit_id":"hero"}],"enemies":[{"unit_id":"hero"}]}]})");
    WriteTextFile(root / "enemy_groups.json", R"({"enemy_groups":[]})");
    WriteTextFile(root / "quests.json", R"({"quests":[]})");

    data::ContentRepository repository;
    REQUIRE(repository.LoadFromDirectory(root));

    const auto* blocker = repository.FindLocationById("bridge_checkpoint");
    const auto* nonBlocker = repository.FindLocationById("orchard_pass");

    REQUIRE(blocker != nullptr);
    REQUIRE(nonBlocker != nullptr);
    REQUIRE(blocker->blocksTransitUntilCleared);
    REQUIRE_FALSE(nonBlocker->blocksTransitUntilCleared);

    std::filesystem::remove_all(root);
}

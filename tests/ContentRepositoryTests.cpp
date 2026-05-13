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

    WriteTextFile(root / "regions.json", R"({"schemaVersion":1,"kind":"RegionCollection","id":"regions","regions":[{"id":"ashvale_heartland","name":"Ashvale Heartland","unlocked":true,"nodes":[{"location_id":"bridge_checkpoint","x":0,"y":0,"discovered":true,"travel_available":true},{"location_id":"orchard_pass","x":1,"y":1,"discovered":true,"travel_available":true}],"links":[["bridge_checkpoint","orchard_pass"]]}]})");
    WriteTextFile(root / "locations.json", R"({"schemaVersion":1,"kind":"LocationCollection","id":"locations","locations":[{"id":"bridge_checkpoint","name":"Bridge Checkpoint","type":"combat","allows_sleep":false,"blocks_transit_until_cleared":true,"overworld_destination":true,"battle_scenario_id":"debug_intro_battle"},{"id":"orchard_pass","name":"Orchard Pass","type":"combat","allows_sleep":false,"overworld_destination":true,"battle_scenario_id":"debug_intro_battle"}]})");
    WriteTextFile(root / "location_scenes.json", R"({"schemaVersion":1,"kind":"LocationSceneCollection","id":"location_scenes","location_scenes":[{"id":"town_square_proto","spawn":{"x":0,"y":0,"width":1,"height":1},"blocking_rects":[],"zones":[]}]})");
    WriteTextFile(root / "units.json", R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[{"id":"hero","name":"Hero","category":"hero","is_player_character":true,"attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":1,"life":1,"position":"front","range":"melee"}]})");
    WriteTextFile(root / "battle_scenarios.json", R"({"schemaVersion":1,"kind":"BattleScenarioCollection","id":"battle_scenarios","battle_scenarios":[{"id":"debug_intro_battle","name":"Debug","seed":7,"allies":[{"unit_id":"hero"}],"enemies":[{"unit_id":"hero"}]}]})");
    WriteTextFile(root / "enemy_groups.json", R"({"schemaVersion":1,"kind":"EnemyGroupCollection","id":"enemy_groups","enemy_groups":[]})");
    WriteTextFile(root / "quests.json", R"({"schemaVersion":1,"kind":"QuestCollection","id":"quests","quests":[]})");
    WriteTextFile(root / "location_services.json", R"({"schemaVersion":1,"kind":"LocationServiceCollection","id":"location_services","location_services":[]})");

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

TEST_CASE("ContentRepository fails when recruit service is missing unit_id") {
    const std::filesystem::path root = "saves/content_repo_recruit_missing_unit_id_test";
    std::filesystem::create_directories(root);

    WriteTextFile(root / "regions.json", R"({"schemaVersion":1,"kind":"RegionCollection","id":"regions","regions":[{"id":"ashvale_heartland","name":"Ashvale Heartland","unlocked":true,"nodes":[{"location_id":"survivor_recruit_post","x":0,"y":0,"discovered":true,"travel_available":true}],"links":[]}]})");
    WriteTextFile(root / "locations.json", R"({"schemaVersion":1,"kind":"LocationCollection","id":"locations","locations":[{"id":"survivor_recruit_post","name":"Survivor Recruit Post","type":"recruit","allows_sleep":false,"overworld_destination":true,"scene_id":"recruit_post_proto"}]})");
    WriteTextFile(root / "location_scenes.json", R"({"schemaVersion":1,"kind":"LocationSceneCollection","id":"location_scenes","location_scenes":[{"id":"recruit_post_proto","spawn":{"x":0,"y":0,"width":1,"height":1},"blocking_rects":[],"zones":[{"id":"recruit_board","type":"recruit","area":{"x":0,"y":0,"width":1,"height":1},"prompt_text":"","result_text":"","failure_text":"","time_cost_minutes":0,"gold_cost":0,"recruit_count":0,"dialogue_choice_time_cost_minutes":1,"dialogue_choices":[]}]}]})");
    WriteTextFile(root / "units.json", R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[{"id":"unit_guard","name":"Guard","category":"generic","is_player_character":false,"attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":1,"life":1,"position":"front","range":"melee"}]})");
    WriteTextFile(root / "battle_scenarios.json", R"({"schemaVersion":1,"kind":"BattleScenarioCollection","id":"battle_scenarios","battle_scenarios":[]})");
    WriteTextFile(root / "enemy_groups.json", R"({"schemaVersion":1,"kind":"EnemyGroupCollection","id":"enemy_groups","enemy_groups":[]})");
    WriteTextFile(root / "quests.json", R"({"schemaVersion":1,"kind":"QuestCollection","id":"quests","quests":[]})");
    WriteTextFile(root / "location_services.json", R"({"schemaVersion":1,"kind":"LocationServiceCollection","id":"location_services","location_services":[{"id":"survivor_post_recruitment","location_id":"survivor_recruit_post","zone_id":"recruit_board","kind":"recruit","prompt_text":"","success_text":"Recruited","failure_text":"No stock","gold_cost":120,"time_cost_minutes":10,"weekly_stock":3}]})");

    data::ContentRepository repository;
    REQUIRE_FALSE(repository.LoadFromDirectory(root));

    std::filesystem::remove_all(root);
}

TEST_CASE("ContentRepository loads valid Home Base muster service") {
    const std::filesystem::path root = "saves/content_repo_muster_valid_test";
    std::filesystem::create_directories(root);

    WriteTextFile(root / "regions.json", R"({"schemaVersion":1,"kind":"RegionCollection","id":"regions","regions":[{"id":"ashvale_heartland","name":"Ashvale Heartland","unlocked":true,"nodes":[{"location_id":"home_base","x":0,"y":0,"discovered":true,"travel_available":true}],"links":[]}]})");
    WriteTextFile(root / "locations.json", R"({"schemaVersion":1,"kind":"LocationCollection","id":"locations","locations":[{"id":"home_base","name":"Home Base","type":"home","allows_sleep":true,"overworld_destination":true,"scene_id":"home_base_proto"}]})");
    WriteTextFile(root / "location_scenes.json", R"({"schemaVersion":1,"kind":"LocationSceneCollection","id":"location_scenes","location_scenes":[{"id":"home_base_proto","spawn":{"x":0,"y":0,"width":1,"height":1},"blocking_rects":[],"zones":[{"id":"muster_table","type":"shop","area":{"x":0,"y":0,"width":1,"height":1},"prompt_text":"","result_text":"","failure_text":"","time_cost_minutes":0,"gold_cost":0,"recruit_count":0,"dialogue_choice_time_cost_minutes":1,"dialogue_choices":[]}]}]})");
    WriteTextFile(root / "units.json", R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[{"id":"hero","name":"Hero","category":"hero","is_player_character":true,"attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":1,"life":1,"position":"front","range":"melee"}]})");
    WriteTextFile(root / "battle_scenarios.json", R"({"schemaVersion":1,"kind":"BattleScenarioCollection","id":"battle_scenarios","battle_scenarios":[]})");
    WriteTextFile(root / "enemy_groups.json", R"({"schemaVersion":1,"kind":"EnemyGroupCollection","id":"enemy_groups","enemy_groups":[]})");
    WriteTextFile(root / "quests.json", R"({"schemaVersion":1,"kind":"QuestCollection","id":"quests","quests":[]})");
    WriteTextFile(root / "location_services.json", R"({"schemaVersion":1,"kind":"LocationServiceCollection","id":"location_services","location_services":[{"id":"home_base_mustering","location_id":"home_base","zone_id":"muster_table","kind":"muster","prompt_text":"E: Muster Party","success_text":"","failure_text":"","gold_cost":0,"time_cost_minutes":0}]})");

    data::ContentRepository repository;
    REQUIRE(repository.LoadFromDirectory(root));

    const auto* service = repository.FindLocationService("home_base", "muster_table");
    REQUIRE(service != nullptr);
    REQUIRE(service->kind == data::LocationServiceKind::Muster);

    std::filesystem::remove_all(root);
}

TEST_CASE("ContentRepository fails when muster service references missing zone") {
    const std::filesystem::path root = "saves/content_repo_muster_missing_zone_test";
    std::filesystem::create_directories(root);

    WriteTextFile(root / "regions.json", R"({"schemaVersion":1,"kind":"RegionCollection","id":"regions","regions":[{"id":"ashvale_heartland","name":"Ashvale Heartland","unlocked":true,"nodes":[{"location_id":"home_base","x":0,"y":0,"discovered":true,"travel_available":true}],"links":[]}]})");
    WriteTextFile(root / "locations.json", R"({"schemaVersion":1,"kind":"LocationCollection","id":"locations","locations":[{"id":"home_base","name":"Home Base","type":"home","allows_sleep":true,"overworld_destination":true,"scene_id":"home_base_proto"}]})");
    WriteTextFile(root / "location_scenes.json", R"({"schemaVersion":1,"kind":"LocationSceneCollection","id":"location_scenes","location_scenes":[{"id":"home_base_proto","spawn":{"x":0,"y":0,"width":1,"height":1},"blocking_rects":[],"zones":[{"id":"inn_door","type":"inn_door","area":{"x":0,"y":0,"width":1,"height":1},"prompt_text":"","result_text":"","failure_text":"","time_cost_minutes":0,"gold_cost":0,"recruit_count":0,"dialogue_choice_time_cost_minutes":1,"dialogue_choices":[]}]}]})");
    WriteTextFile(root / "units.json", R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[{"id":"hero","name":"Hero","category":"hero","is_player_character":true,"attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":1,"life":1,"position":"front","range":"melee"}]})");
    WriteTextFile(root / "battle_scenarios.json", R"({"schemaVersion":1,"kind":"BattleScenarioCollection","id":"battle_scenarios","battle_scenarios":[]})");
    WriteTextFile(root / "enemy_groups.json", R"({"schemaVersion":1,"kind":"EnemyGroupCollection","id":"enemy_groups","enemy_groups":[]})");
    WriteTextFile(root / "quests.json", R"({"schemaVersion":1,"kind":"QuestCollection","id":"quests","quests":[]})");
    WriteTextFile(root / "location_services.json", R"({"schemaVersion":1,"kind":"LocationServiceCollection","id":"location_services","location_services":[{"id":"home_base_mustering","location_id":"home_base","zone_id":"muster_table","kind":"muster","prompt_text":"E: Muster Party","success_text":"","failure_text":"","gold_cost":0,"time_cost_minutes":0}]})");

    data::ContentRepository repository;
    REQUIRE_FALSE(repository.LoadFromDirectory(root));

    std::filesystem::remove_all(root);
}

TEST_CASE("ContentRepository fails when recruit service unit_id is unknown") {
    const std::filesystem::path root = "saves/content_repo_recruit_unknown_unit_id_test";
    std::filesystem::create_directories(root);

    WriteTextFile(root / "regions.json", R"({"schemaVersion":1,"kind":"RegionCollection","id":"regions","regions":[{"id":"ashvale_heartland","name":"Ashvale Heartland","unlocked":true,"nodes":[{"location_id":"survivor_recruit_post","x":0,"y":0,"discovered":true,"travel_available":true}],"links":[]}]})");
    WriteTextFile(root / "locations.json", R"({"schemaVersion":1,"kind":"LocationCollection","id":"locations","locations":[{"id":"survivor_recruit_post","name":"Survivor Recruit Post","type":"recruit","allows_sleep":false,"overworld_destination":true,"scene_id":"recruit_post_proto"}]})");
    WriteTextFile(root / "location_scenes.json", R"({"schemaVersion":1,"kind":"LocationSceneCollection","id":"location_scenes","location_scenes":[{"id":"recruit_post_proto","spawn":{"x":0,"y":0,"width":1,"height":1},"blocking_rects":[],"zones":[{"id":"recruit_board","type":"recruit","area":{"x":0,"y":0,"width":1,"height":1},"prompt_text":"","result_text":"","failure_text":"","time_cost_minutes":0,"gold_cost":0,"recruit_count":0,"dialogue_choice_time_cost_minutes":1,"dialogue_choices":[]}]}]})");
    WriteTextFile(root / "units.json", R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[{"id":"unit_guard","name":"Guard","category":"generic","is_player_character":false,"attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":1,"life":1,"position":"front","range":"melee"}]})");
    WriteTextFile(root / "battle_scenarios.json", R"({"schemaVersion":1,"kind":"BattleScenarioCollection","id":"battle_scenarios","battle_scenarios":[]})");
    WriteTextFile(root / "enemy_groups.json", R"({"schemaVersion":1,"kind":"EnemyGroupCollection","id":"enemy_groups","enemy_groups":[]})");
    WriteTextFile(root / "quests.json", R"({"schemaVersion":1,"kind":"QuestCollection","id":"quests","quests":[]})");
    WriteTextFile(root / "location_services.json", R"({"schemaVersion":1,"kind":"LocationServiceCollection","id":"location_services","location_services":[{"id":"survivor_post_recruitment","location_id":"survivor_recruit_post","zone_id":"recruit_board","kind":"recruit","prompt_text":"","success_text":"Recruited","failure_text":"No stock","gold_cost":120,"time_cost_minutes":10,"unit_id":"unit_missing","weekly_stock":3}]})");

    data::ContentRepository repository;
    REQUIRE_FALSE(repository.LoadFromDirectory(root));

    std::filesystem::remove_all(root);
}

TEST_CASE("ContentRepository fails when a location service references a missing scene zone") {
    const std::filesystem::path root = "saves/content_repo_missing_service_zone_test";
    std::filesystem::create_directories(root);

    WriteTextFile(root / "regions.json", R"({
        "schemaVersion":1,"kind":"RegionCollection","id":"regions",
        "regions":[
            {
                "id":"ashvale_heartland",
                "name":"Ashvale Heartland",
                "unlocked":true,
                "nodes":[
                    {
                        "location_id":"home_base",
                        "x":0,
                        "y":0,
                        "discovered":true,
                        "travel_available":true
                    }
                ],
                "links":[]
            }
        ]
    })");

    WriteTextFile(root / "locations.json", R"({
        "schemaVersion":1,"kind":"LocationCollection","id":"locations",
        "locations":[
            {
                "id":"home_base",
                "name":"Home Base",
                "type":"home",
                "allows_sleep":true,
                "overworld_destination":true,
                "scene_id":"home_base_proto"
            }
        ]
    })");

    // Important:
    // The scene exists, but it does NOT contain the zone id "prep_table".
    WriteTextFile(root / "location_scenes.json", R"({
        "schemaVersion":1,"kind":"LocationSceneCollection","id":"location_scenes",
        "location_scenes":[
            {
                "id":"home_base_proto",
                "spawn":{"x":0,"y":0,"width":1,"height":1},
                "blocking_rects":[],
                "zones":[
                    {
                        "id":"inn_door",
                        "type":"inn_door",
                        "area":{"x":0,"y":0,"width":1,"height":1},
                        "prompt_text":"",
                        "result_text":"",
                        "failure_text":"",
                        "time_cost_minutes":0,
                        "gold_cost":0,
                        "recruit_count":0,
                        "dialogue_choice_time_cost_minutes":1,
                        "dialogue_choices":[]
                    }
                ]
            }
        ]
    })");

    WriteTextFile(root / "units.json", R"({
        "schemaVersion":1,"kind":"UnitCollection","id":"units",
        "units":[
            {
                "id":"hero",
                "name":"Hero",
                "category":"hero",
                "is_player_character":true,
                "attack":1,
                "defense":1,
                "magic":1,
                "resistance":1,
                "min_damage":1,
                "max_damage":1,
                "max_hp":10,
                "max_mp":0,
                "agility":1,
                "life":1,
                "position":"front",
                "range":"melee"
            }
        ]
    })");

    WriteTextFile(root / "battle_scenarios.json", R"({"schemaVersion":1,"kind":"BattleScenarioCollection","id":"battle_scenarios","battle_scenarios":[]})");
    WriteTextFile(root / "enemy_groups.json", R"({"schemaVersion":1,"kind":"EnemyGroupCollection","id":"enemy_groups","enemy_groups":[]})");
    WriteTextFile(root / "quests.json", R"({"schemaVersion":1,"kind":"QuestCollection","id":"quests","quests":[]})");

    WriteTextFile(root / "location_services.json", R"({
        "schemaVersion":1,"kind":"LocationServiceCollection","id":"location_services",
        "location_services":[
            {
                "id":"home_base_preparation",
                "location_id":"home_base",
                "zone_id":"prep_table",
                "kind":"shop",
                "prompt_text":"E: Prepare for Travel",
                "success_text":"Prepared for travel",
                "failure_text":"Already prepared",
                "gold_cost":0,
                "time_cost_minutes":0,
                "daily_use_limit":1,
                "travel_prep_discount_minutes":20,
                "travel_prep_charges":1
            }
        ]
    })");

    data::ContentRepository repository;
    REQUIRE_FALSE(repository.LoadFromDirectory(root));

    std::filesystem::remove_all(root);
}

TEST_CASE("ContentRepository fails when a location service references a missing location") {
    const std::filesystem::path root = "saves/content_repo_missing_service_location_test";
    std::filesystem::create_directories(root);

    WriteTextFile(root / "regions.json", R"({
        "schemaVersion":1,"kind":"RegionCollection","id":"regions",
        "regions":[
            {
                "id":"ashvale_heartland",
                "name":"Ashvale Heartland",
                "unlocked":true,
                "nodes":[
                    {
                        "location_id":"home_base",
                        "x":0,
                        "y":0,
                        "discovered":true,
                        "travel_available":true
                    }
                ],
                "links":[]
            }
        ]
    })");

    WriteTextFile(root / "locations.json", R"({
        "schemaVersion":1,"kind":"LocationCollection","id":"locations",
        "locations":[
            {
                "id":"home_base",
                "name":"Home Base",
                "type":"home",
                "allows_sleep":true,
                "overworld_destination":true,
                "scene_id":"home_base_proto"
            }
        ]
    })");

    WriteTextFile(root / "location_scenes.json", R"({
        "schemaVersion":1,"kind":"LocationSceneCollection","id":"location_scenes",
        "location_scenes":[
            {
                "id":"home_base_proto",
                "spawn":{"x":0,"y":0,"width":1,"height":1},
                "blocking_rects":[],
                "zones":[
                    {
                        "id":"prep_table",
                        "type":"shop",
                        "area":{"x":0,"y":0,"width":1,"height":1},
                        "prompt_text":"",
                        "result_text":"",
                        "failure_text":"",
                        "time_cost_minutes":0,
                        "gold_cost":0,
                        "recruit_count":0,
                        "dialogue_choice_time_cost_minutes":1,
                        "dialogue_choices":[]
                    }
                ]
            }
        ]
    })");

    WriteTextFile(root / "units.json", R"({
        "schemaVersion":1,"kind":"UnitCollection","id":"units",
        "units":[
            {
                "id":"hero",
                "name":"Hero",
                "category":"hero",
                "is_player_character":true,
                "attack":1,
                "defense":1,
                "magic":1,
                "resistance":1,
                "min_damage":1,
                "max_damage":1,
                "max_hp":10,
                "max_mp":0,
                "agility":1,
                "life":1,
                "position":"front",
                "range":"melee"
            }
        ]
    })");

    WriteTextFile(root / "battle_scenarios.json", R"({"schemaVersion":1,"kind":"BattleScenarioCollection","id":"battle_scenarios","battle_scenarios":[]})");
    WriteTextFile(root / "enemy_groups.json", R"({"schemaVersion":1,"kind":"EnemyGroupCollection","id":"enemy_groups","enemy_groups":[]})");
    WriteTextFile(root / "quests.json", R"({"schemaVersion":1,"kind":"QuestCollection","id":"quests","quests":[]})");
    WriteTextFile(root / "location_services.json", R"({
        "schemaVersion":1,"kind":"LocationServiceCollection","id":"location_services",
        "location_services":[
            {
                "id":"missing_location_service",
                "location_id":"does_not_exist",
                "zone_id":"prep_table",
                "kind":"shop",
                "prompt_text":"E: Prepare for Travel",
                "success_text":"Prepared for travel",
                "failure_text":"Already prepared",
                "gold_cost":0,
                "time_cost_minutes":0,
                "daily_use_limit":1,
                "travel_prep_discount_minutes":20,
                "travel_prep_charges":1
            }
        ]
    })");

    data::ContentRepository repository;
    REQUIRE_FALSE(repository.LoadFromDirectory(root));

    std::filesystem::remove_all(root);
}

TEST_CASE("ContentRepository fails when a location references a missing scene") {
    const std::filesystem::path root = "saves/content_repo_missing_location_scene_test";
    std::filesystem::create_directories(root);

    WriteTextFile(root / "regions.json", R"({
        "schemaVersion":1,"kind":"RegionCollection","id":"regions",
        "regions":[
            {
                "id":"ashvale_heartland",
                "name":"Ashvale Heartland",
                "unlocked":true,
                "nodes":[
                    {
                        "location_id":"home_base",
                        "x":0,
                        "y":0,
                        "discovered":true,
                        "travel_available":true
                    }
                ],
                "links":[]
            }
        ]
    })");

    WriteTextFile(root / "locations.json", R"({
        "schemaVersion":1,"kind":"LocationCollection","id":"locations",
        "locations":[
            {
                "id":"home_base",
                "name":"Home Base",
                "type":"home",
                "allows_sleep":true,
                "overworld_destination":true,
                "scene_id":"missing_scene_id"
            }
        ]
    })");

    // Important:
    // The location points to "missing_scene_id", but only "home_base_proto" exists.
    WriteTextFile(root / "location_scenes.json", R"({
        "schemaVersion":1,"kind":"LocationSceneCollection","id":"location_scenes",
        "location_scenes":[
            {
                "id":"home_base_proto",
                "spawn":{"x":0,"y":0,"width":1,"height":1},
                "blocking_rects":[],
                "zones":[]
            }
        ]
    })");

    WriteTextFile(root / "units.json", R"({
        "schemaVersion":1,"kind":"UnitCollection","id":"units",
        "units":[
            {
                "id":"hero",
                "name":"Hero",
                "category":"hero",
                "is_player_character":true,
                "attack":1,
                "defense":1,
                "magic":1,
                "resistance":1,
                "min_damage":1,
                "max_damage":1,
                "max_hp":10,
                "max_mp":0,
                "agility":1,
                "life":1,
                "position":"front",
                "range":"melee"
            }
        ]
    })");

    WriteTextFile(root / "battle_scenarios.json", R"({"schemaVersion":1,"kind":"BattleScenarioCollection","id":"battle_scenarios","battle_scenarios":[]})");
    WriteTextFile(root / "enemy_groups.json", R"({"schemaVersion":1,"kind":"EnemyGroupCollection","id":"enemy_groups","enemy_groups":[]})");
    WriteTextFile(root / "quests.json", R"({"schemaVersion":1,"kind":"QuestCollection","id":"quests","quests":[]})");
    WriteTextFile(root / "location_services.json", R"({"schemaVersion":1,"kind":"LocationServiceCollection","id":"location_services","location_services":[]})");

    data::ContentRepository repository;
    REQUIRE_FALSE(repository.LoadFromDirectory(root));

    std::filesystem::remove_all(root);
}

TEST_CASE("ContentRepository surfaces identity error messages without gating load") {
    const std::filesystem::path root = "saves/content_repo_identity_messages_test";
    std::filesystem::create_directories(root);

    // regions.json is missing schemaVersion/kind/id — should surface identity errors
    // but LoadFromDirectory should still return true (identity errors don't gate bool)
    WriteTextFile(root / "regions.json", R"({"regions":[{"id":"ashvale_heartland","name":"Ashvale Heartland","unlocked":true,"nodes":[{"location_id":"home_base","x":0,"y":0,"discovered":true,"travel_available":true}],"links":[]}]})");
    WriteTextFile(root / "locations.json", R"({"schemaVersion":1,"kind":"LocationCollection","id":"locations","locations":[{"id":"home_base","name":"Home Base","type":"home","allows_sleep":true,"overworld_destination":true,"scene_id":"home_base_proto"}]})");
    WriteTextFile(root / "location_scenes.json", R"({"schemaVersion":1,"kind":"LocationSceneCollection","id":"location_scenes","location_scenes":[{"id":"home_base_proto","spawn":{"x":0,"y":0,"width":1,"height":1},"blocking_rects":[],"zones":[]}]})");
    WriteTextFile(root / "units.json", R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[{"id":"hero","name":"Hero","category":"hero","is_player_character":true,"attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":1,"life":1,"position":"front","range":"melee"}]})");
    WriteTextFile(root / "battle_scenarios.json", R"({"schemaVersion":1,"kind":"BattleScenarioCollection","id":"battle_scenarios","battle_scenarios":[]})");
    WriteTextFile(root / "enemy_groups.json", R"({"schemaVersion":1,"kind":"EnemyGroupCollection","id":"enemy_groups","enemy_groups":[]})");
    WriteTextFile(root / "quests.json", R"({"schemaVersion":1,"kind":"QuestCollection","id":"quests","quests":[]})");
    WriteTextFile(root / "location_services.json", R"({"schemaVersion":1,"kind":"LocationServiceCollection","id":"location_services","location_services":[]})");

    data::ContentRepository repository;
    REQUIRE(repository.LoadFromDirectory(root));

    const auto& msgs = repository.ValidationMessages();
    const bool hasIdentityError = std::ranges::any_of(msgs,
        [](const ValidationMessage& m) { return m.code == "SCHEMA_VERSION_MISSING"; });
    REQUIRE(hasIdentityError);

    std::filesystem::remove_all(root);
}

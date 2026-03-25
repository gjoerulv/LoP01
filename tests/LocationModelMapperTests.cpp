#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

#include "app/mappers/LocationModelMapper.h"
#include "core/GameClock.h"
#include "data/ContentRepository.h"
#include "gameplay/GameSession.h"
#include "gameplay/location/LocationScene.h"

namespace {

void WriteTextFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream output(path, std::ios::trunc);
    output << content;
}

std::filesystem::path BuildLocationMapperTestContent() {
    const std::filesystem::path root = "saves/location_mapper_test";
    std::filesystem::create_directories(root);

    WriteTextFile(root / "regions.json", R"({"regions":[{"id":"ashvale_heartland","name":"Ashvale Heartland","unlocked":true,"nodes":[{"location_id":"survivor_recruit_post","x":0,"y":0,"discovered":true,"travel_available":true}],"links":[]}]})");
    WriteTextFile(root / "locations.json", R"({"locations":[{"id":"survivor_recruit_post","name":"Survivor Recruit Post","type":"recruit","allows_sleep":false,"overworld_destination":true,"scene_id":"recruit_post_proto"}]})");
    WriteTextFile(root / "location_scenes.json", R"({"location_scenes":[{"id":"recruit_post_proto","spawn":{"x":100,"y":100,"width":24,"height":24},"blocking_rects":[],"zones":[{"id":"recruit_board","type":"recruit","area":{"x":110,"y":110,"width":80,"height":40},"prompt_text":"","result_text":"","failure_text":"","time_cost_minutes":0,"gold_cost":0,"recruit_count":0,"dialogue_choice_time_cost_minutes":1,"dialogue_choices":[]}]}]})");
    WriteTextFile(root / "units.json", R"({"units":[{"id":"unit_guard","name":"Town Guard","category":"generic","is_player_character":false,"attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":1,"life":1,"position":"front","range":"melee"}]})");
    WriteTextFile(root / "battle_scenarios.json", R"({"battle_scenarios":[{"id":"debug_intro_battle","name":"Debug","seed":7,"allies":[{"unit_id":"unit_guard"}],"enemies":[{"unit_id":"unit_guard"}]}]})");
    WriteTextFile(root / "enemy_groups.json", R"({"enemy_groups":[]})");
    WriteTextFile(root / "quests.json", R"({"quests":[]})");
    WriteTextFile(root / "location_services.json", R"({"location_services":[{"id":"survivor_post_recruitment","location_id":"survivor_recruit_post","zone_id":"recruit_board","kind":"recruit","prompt_text":"","success_text":"Recruited a Town Guard","failure_text":"No Town Guards available this week","gold_cost":120,"time_cost_minutes":10,"unit_id":"unit_guard","unit_display_name":"Town Guard","weekly_stock":3}]})");

    return root;
}

} // namespace

TEST_CASE("Location recruit prompt reflects weekly restock without requiring recruit interaction") {
    const auto root = BuildLocationMapperTestContent();

    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));

    gameplay::GameSession session;
    session.EnterLocationMode("survivor_recruit_post");

    const auto* sceneDefinition = content.FindLocationSceneById("recruit_post_proto");
    REQUIRE(sceneDefinition != nullptr);

    gameplay::location::LocationScene scene;
    scene.Reset(*sceneDefinition);

    const auto* service = content.FindLocationService("survivor_recruit_post", "recruit_board");
    REQUIRE(service != nullptr);

    session.RefreshWeeklyRecruitStocks(content.LocationServices());
    REQUIRE(session.TryConsumeRecruitStock(service->id, service->weeklyStock));
    REQUIRE(session.TryConsumeRecruitStock(service->id, service->weeklyStock));
    REQUIRE(session.TryConsumeRecruitStock(service->id, service->weeklyStock));
    REQUIRE(session.RemainingRecruitStock(service->id, service->weeklyStock) == 0);

    app::mappers::LocationModelMapper mapper;
    auto model = mapper.Map(content, session, session.Snapshot(), scene, "");
    REQUIRE(model.interactPrompt.find("Stock: 0/3") != std::string::npos);
    REQUIRE_FALSE(model.interactPromptUsable);

    session.AddMinutes(core::GameClock::kMinutesPerSliceDay * 7);

    model = mapper.Map(content, session, session.Snapshot(), scene, "");
    REQUIRE(model.interactPrompt.find("Stock: 0/3") != std::string::npos);

    session.RefreshWeeklyRecruitStocks(content.LocationServices());

    model = mapper.Map(content, session, session.Snapshot(), scene, "");
    REQUIRE(model.interactPrompt.find("Stock: 3/3") != std::string::npos);
    REQUIRE(model.interactPromptUsable);

    REQUIRE(session.TryConsumeRecruitStock(service->id, service->weeklyStock));
    model = mapper.Map(content, session, session.Snapshot(), scene, "");
    REQUIRE(model.interactPrompt.find("Stock: 2/3") != std::string::npos);

    std::filesystem::remove_all(root);
}

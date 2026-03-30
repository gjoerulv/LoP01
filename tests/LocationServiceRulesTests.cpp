#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

#include "core/GameClock.h"
#include "data/ContentRepository.h"
#include "data/definitions/LocationServiceDefinition.h"
#include "gameplay/GameSession.h"
#include "gameplay/location/LocationServiceRules.h"

namespace {

void WriteTextFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream output(path, std::ios::trunc);
    output << content;
}

std::filesystem::path BuildRecruitRulesTestContent() {
    const std::filesystem::path root = "saves/location_service_rules_recruit_test";
    std::filesystem::create_directories(root);

    WriteTextFile(root / "regions.json", R"({"regions":[{"id":"ashvale_heartland","name":"Ashvale Heartland","unlocked":true,"nodes":[{"location_id":"survivor_recruit_post","x":0,"y":0,"discovered":true,"travel_available":true}],"links":[]}]})");
    WriteTextFile(root / "locations.json", R"({"locations":[{"id":"survivor_recruit_post","name":"Survivor Recruit Post","type":"recruit","allows_sleep":false,"overworld_destination":true,"scene_id":"recruit_post_proto"}]})");
    WriteTextFile(root / "location_scenes.json", R"({"location_scenes":[{"id":"recruit_post_proto","spawn":{"x":0,"y":0,"width":1,"height":1},"blocking_rects":[],"zones":[{"id":"recruit_board","type":"recruit","area":{"x":0,"y":0,"width":1,"height":1},"prompt_text":"","result_text":"","failure_text":"","time_cost_minutes":0,"gold_cost":0,"recruit_count":0,"dialogue_choice_time_cost_minutes":1,"dialogue_choices":[]}]}]})");
    WriteTextFile(root / "units.json", R"({"units":[{"id":"unit_guard","name":"Guard Recruit","category":"generic","is_player_character":false,"attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":1,"life":1,"position":"front","range":"melee"}]})");
    WriteTextFile(root / "battle_scenarios.json", R"({"battle_scenarios":[]})");
    WriteTextFile(root / "enemy_groups.json", R"({"enemy_groups":[]})");
    WriteTextFile(root / "quests.json", R"({"quests":[]})");
    WriteTextFile(root / "location_services.json", R"({"location_services":[{"id":"survivor_post_recruitment","location_id":"survivor_recruit_post","zone_id":"recruit_board","kind":"recruit","prompt_text":"","success_text":"Recruited a Guard","failure_text":"No stock","gold_cost":120,"time_cost_minutes":10,"unit_id":"unit_guard","unit_display_name":"Guard Recruit","weekly_stock":3}]})");

    return root;
}

} // namespace

TEST_CASE("Rest service is recognized by typed service kind") {
    data::LocationServiceDefinition service;
    service.kind = data::LocationServiceKind::Rest;

    REQUIRE(gameplay::location::IsRestService(&service));
    REQUIRE_FALSE(gameplay::location::IsShopService(&service));
    REQUIRE_FALSE(gameplay::location::IsRecruitService(&service));
}

TEST_CASE("Shop service is recognized by typed service kind") {
    data::LocationServiceDefinition service;
    service.kind = data::LocationServiceKind::Shop;

    REQUIRE(gameplay::location::IsShopService(&service));
    REQUIRE_FALSE(gameplay::location::IsRestService(&service));
    REQUIRE_FALSE(gameplay::location::IsRecruitService(&service));
}

TEST_CASE("Recruit service is recognized by typed service kind") {
    data::LocationServiceDefinition service;
    service.kind = data::LocationServiceKind::Recruit;

    REQUIRE(gameplay::location::IsRecruitService(&service));
    REQUIRE_FALSE(gameplay::location::IsRestService(&service));
    REQUIRE_FALSE(gameplay::location::IsShopService(&service));
}

TEST_CASE("Muster service is recognized by typed service kind") {
    data::LocationServiceDefinition service;
    service.kind = data::LocationServiceKind::Muster;

    REQUIRE(gameplay::location::IsMusterService(&service));
    REQUIRE_FALSE(gameplay::location::IsRestService(&service));
    REQUIRE_FALSE(gameplay::location::IsShopService(&service));
    REQUIRE_FALSE(gameplay::location::IsRecruitService(&service));
}

TEST_CASE("Recruit service flow preserves stock/gold/time and adds owned roster unit") {
    const auto root = BuildRecruitRulesTestContent();

    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));

    const auto* service = content.FindLocationService("survivor_recruit_post", "recruit_board");
    REQUIRE(service != nullptr);
    REQUIRE(service->kind == data::LocationServiceKind::Recruit);

    gameplay::GameSession session;
    const auto before = session.Snapshot();
    const int ownedBefore = session.OwnedUnitCount(service->unitId);

    const auto result = gameplay::location::TryApplyRecruitService(session, content, *service);
    REQUIRE(result.success);

    const auto after = session.Snapshot();
    REQUIRE(after.gold == before.gold - service->goldCost);
    REQUIRE(after.minutesIntoSliceDay == before.minutesIntoSliceDay + service->timeCostMinutes);
    REQUIRE(session.RemainingRecruitStock(service->id, service->weeklyStock) == service->weeklyStock - 1);
    REQUIRE(session.OwnedUnitCount(service->unitId) == ownedBefore + 1);

    std::filesystem::remove_all(root);
}
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "data/ContentRepository.h"

namespace {

void WriteTextFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream output(path, std::ios::trunc);
    output << content;
}

// Baseline with an ownable mine + Trading Post and a non-ownable Rest service, so
// scenario start-state owned-service references can be validated cross-file.
void WriteStartStateBaseline(const std::filesystem::path& root) {
    std::filesystem::create_directories(root);
    WriteTextFile(root / "regions.json", R"({"schemaVersion":1,"kind":"RegionCollection","id":"regions","regions":[
        {"id":"alpha","name":"Alpha","unlocked":true,"arrivalNodeId":"alpha_arrival","nodes":[
            {"location_id":"alpha_arrival","x":0,"y":0},
            {"location_id":"mine_loc","x":1,"y":0},
            {"location_id":"tp_loc","x":2,"y":0},
            {"location_id":"rest_loc","x":3,"y":0}],"links":[]}
    ]})");
    WriteTextFile(root / "locations.json", R"({"schemaVersion":1,"kind":"LocationCollection","id":"locations","locations":[
        {"id":"alpha_arrival","name":"Arrival","type":"town","allows_sleep":false,"overworld_destination":true},
        {"id":"mine_loc","name":"Mine","type":"service","allows_sleep":false,"overworld_destination":true,"scene_id":"mine_scene"},
        {"id":"tp_loc","name":"Trading Post","type":"town","allows_sleep":false,"overworld_destination":true,"scene_id":"tp_scene"},
        {"id":"rest_loc","name":"Inn","type":"inn","allows_sleep":true,"overworld_destination":true,"scene_id":"rest_scene"}
    ]})");
    WriteTextFile(root / "location_scenes.json", R"({"schemaVersion":1,"kind":"LocationSceneCollection","id":"location_scenes","location_scenes":[
        {"id":"mine_scene","spawn":{"x":0,"y":0,"width":1,"height":1},"blocking_rects":[],"zones":[{"id":"mine_face","type":"recruit","area":{"x":0,"y":0,"width":1,"height":1},"prompt_text":"","result_text":"","failure_text":"","time_cost_minutes":0,"gold_cost":0,"recruit_count":0,"dialogue_choice_time_cost_minutes":1,"dialogue_choices":[]}]},
        {"id":"tp_scene","spawn":{"x":0,"y":0,"width":1,"height":1},"blocking_rects":[],"zones":[{"id":"counter","type":"recruit","area":{"x":0,"y":0,"width":1,"height":1},"prompt_text":"","result_text":"","failure_text":"","time_cost_minutes":0,"gold_cost":0,"recruit_count":0,"dialogue_choice_time_cost_minutes":1,"dialogue_choices":[]}]},
        {"id":"rest_scene","spawn":{"x":0,"y":0,"width":1,"height":1},"blocking_rects":[],"zones":[{"id":"inn_door","type":"inn_door","area":{"x":0,"y":0,"width":1,"height":1},"prompt_text":"","result_text":"","failure_text":"","time_cost_minutes":0,"gold_cost":0,"recruit_count":0,"dialogue_choice_time_cost_minutes":1,"dialogue_choices":[]}]}
    ]})");
    WriteTextFile(root / "units.json", R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[{"id":"hero","name":"Hero","category":"hero","is_player_character":true,"attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":1,"life":1,"position":"front","range":"melee"}]})");
    WriteTextFile(root / "battle_scenarios.json", R"({"schemaVersion":1,"kind":"BattleScenarioCollection","id":"battle_scenarios","battle_scenarios":[]})");
    WriteTextFile(root / "enemy_groups.json", R"({"schemaVersion":1,"kind":"EnemyGroupCollection","id":"enemy_groups","enemy_groups":[]})");
    WriteTextFile(root / "quests.json", R"({"schemaVersion":1,"kind":"QuestCollection","id":"quests","quests":[]})");
    WriteTextFile(root / "location_services.json", R"({"schemaVersion":1,"kind":"LocationServiceCollection","id":"location_services","location_services":[
        {"id":"alpha_mine","location_id":"mine_loc","zone_id":"mine_face","kind":"mine","mine_outputs":[{"resource":"Stone","amount":2}]},
        {"id":"alpha_tp","location_id":"tp_loc","zone_id":"counter","kind":"trading_post"},
        {"id":"alpha_rest","location_id":"rest_loc","zone_id":"inn_door","kind":"rest","rest_kind":"inn"}
    ]})");
}

bool HasCode(const std::vector<ValidationMessage>& msgs, const std::string& code) {
    return std::any_of(msgs.begin(), msgs.end(),
        [&](const ValidationMessage& m) { return m.code == code; });
}

bool HasError(const std::vector<ValidationMessage>& msgs) {
    return std::any_of(msgs.begin(), msgs.end(),
        [](const ValidationMessage& m) { return m.severity == Severity::Error; });
}

// Writes the baseline + a scenarios.json containing exactly `scenarioObject`, loads
// it, and returns the repository's validation messages. `repo` is left loaded so
// callers can inspect parsed scenarios.
std::vector<ValidationMessage> LoadScenario(
    const std::filesystem::path& root, const std::string& scenarioObject, data::ContentRepository& repo) {
    WriteStartStateBaseline(root);
    WriteTextFile(root / "scenarios.json",
        R"({"schemaVersion":1,"kind":"ScenarioCollection","id":"scenarios","scenarios":[)" +
        scenarioObject + "]}");
    (void)repo.LoadFromDirectory(root);
    return repo.ValidationMessages();
}

} // namespace

TEST_CASE("ScenarioStart: a full valid playerStart parses cleanly") {
    const std::filesystem::path root = "saves/ss_valid";
    data::ContentRepository repo;
    const auto msgs = LoadScenario(root,
        R"({"id":"s1","startRegionId":"alpha","playerStart":{"gold":1500,)"
        R"("resources":[{"resource":"Wood","amount":5},{"resource":"Stone","amount":3}],)"
        R"("ownedServices":[{"serviceId":"alpha_mine"},{"serviceId":"alpha_tp","locked":true}]}})", repo);

    REQUIRE_FALSE(HasError(msgs));
    const auto* s = repo.FindScenarioById("s1");
    REQUIRE(s != nullptr);
    REQUIRE(s->startGold.has_value());
    REQUIRE(*s->startGold == 1500);
    REQUIRE(s->startResources.size() == 2);
    REQUIRE(s->startResources[0].resource == "Wood");
    REQUIRE(s->startResources[0].amount == 5);
    REQUIRE(s->startOwnedServices.size() == 2);
    REQUIRE(s->startOwnedServices[1].serviceId == "alpha_tp");
    REQUIRE(s->startOwnedServices[1].locked);
    std::filesystem::remove_all(root);
}

TEST_CASE("ScenarioStart: playerStart.gold acts as the startGold alias") {
    const std::filesystem::path root = "saves/ss_goldalias";
    data::ContentRepository repo;
    const auto msgs = LoadScenario(root,
        R"({"id":"s1","startRegionId":"alpha","playerStart":{"gold":777}})", repo);
    REQUIRE_FALSE(HasError(msgs));
    const auto* s = repo.FindScenarioById("s1");
    REQUIRE(s != nullptr);
    REQUIRE(s->startGold.has_value());
    REQUIRE(*s->startGold == 777);
    std::filesystem::remove_all(root);
}

TEST_CASE("ScenarioStart: legacy startGold still loads (back-compat)") {
    const std::filesystem::path root = "saves/ss_legacygold";
    data::ContentRepository repo;
    const auto msgs = LoadScenario(root,
        R"({"id":"s1","startRegionId":"alpha","startGold":999})", repo);
    REQUIRE_FALSE(HasError(msgs));
    REQUIRE(*repo.FindScenarioById("s1")->startGold == 999);
    std::filesystem::remove_all(root);
}

TEST_CASE("ScenarioStart: authoring both startGold and playerStart.gold is ambiguous") {
    const std::filesystem::path root = "saves/ss_goldambig";
    data::ContentRepository repo;
    const auto msgs = LoadScenario(root,
        R"({"id":"s1","startRegionId":"alpha","startGold":100,"playerStart":{"gold":200}})", repo);
    REQUIRE(HasCode(msgs, "SCENARIO_START_GOLD_AMBIGUOUS"));
    std::filesystem::remove_all(root);
}

TEST_CASE("ScenarioStart: negative legacy startGold is rejected") {
    const std::filesystem::path root = "saves/ss_neglegacy";
    data::ContentRepository repo;
    const auto msgs = LoadScenario(root,
        R"({"id":"s1","startRegionId":"alpha","startGold":-5})", repo);
    REQUIRE(HasCode(msgs, "SCENARIO_START_GOLD_INVALID"));
    std::filesystem::remove_all(root);
}

TEST_CASE("ScenarioStart: negative and non-integer playerStart.gold are rejected") {
    {
        const std::filesystem::path root = "saves/ss_neggold";
        data::ContentRepository repo;
        REQUIRE(HasCode(LoadScenario(root,
            R"({"id":"s1","startRegionId":"alpha","playerStart":{"gold":-1}})", repo),
            "SCENARIO_START_GOLD_INVALID"));
        std::filesystem::remove_all(root);
    }
    {
        const std::filesystem::path root = "saves/ss_nonintgold";
        data::ContentRepository repo;
        REQUIRE(HasCode(LoadScenario(root,
            R"({"id":"s1","startRegionId":"alpha","playerStart":{"gold":"lots"}})", repo),
            "SCENARIO_START_GOLD_INVALID"));
        std::filesystem::remove_all(root);
    }
}

TEST_CASE("ScenarioStart: malformed playerStart shapes fail loudly") {
    struct Case { std::string name; std::string ps; std::string code; };
    const std::vector<Case> cases = {
        {"legacy_gold_nonint", R"("startGold":"lots")", "SCENARIO_START_GOLD_INVALID"},
        {"ps_obj", R"("playerStart":5)", "SCENARIO_PLAYER_START_TYPE_INVALID"},
        {"res_arr", R"("playerStart":{"resources":5})", "SCENARIO_START_RESOURCES_TYPE_INVALID"},
        {"res_entry", R"("playerStart":{"resources":[5]})", "SCENARIO_START_RESOURCE_ENTRY_TYPE_INVALID"},
        {"res_field", R"("playerStart":{"resources":[{"amount":3}]})", "SCENARIO_START_RESOURCE_FIELD_INVALID"},
        {"res_field_nonstring", R"("playerStart":{"resources":[{"resource":123,"amount":1}]})", "SCENARIO_START_RESOURCE_FIELD_INVALID"},
        {"res_amt_missing", R"("playerStart":{"resources":[{"resource":"Wood"}]})", "SCENARIO_START_RESOURCE_AMOUNT_INVALID"},
        {"res_amt_nonint", R"("playerStart":{"resources":[{"resource":"Wood","amount":"many"}]})", "SCENARIO_START_RESOURCE_AMOUNT_INVALID"},
        {"res_amt_zero", R"("playerStart":{"resources":[{"resource":"Wood","amount":0}]})", "SCENARIO_START_RESOURCE_AMOUNT_INVALID"},
        {"res_amt_negative", R"("playerStart":{"resources":[{"resource":"Wood","amount":-1}]})", "SCENARIO_START_RESOURCE_AMOUNT_INVALID"},
        {"res_unknown", R"("playerStart":{"resources":[{"resource":"Mithril","amount":1}]})", "SCENARIO_START_RESOURCE_INVALID"},
        {"res_gold", R"("playerStart":{"resources":[{"resource":"Gold","amount":1}]})", "SCENARIO_START_RESOURCE_GOLD"},
        {"res_dup", R"("playerStart":{"resources":[{"resource":"Wood","amount":1},{"resource":"Wood","amount":2}]})", "SCENARIO_START_RESOURCE_DUPLICATE"},
        {"svc_arr", R"("playerStart":{"ownedServices":5})", "SCENARIO_OWNED_SERVICES_TYPE_INVALID"},
        {"svc_entry", R"("playerStart":{"ownedServices":[5]})", "SCENARIO_OWNED_SERVICE_ENTRY_TYPE_INVALID"},
        {"svc_field", R"("playerStart":{"ownedServices":[{"locked":true}]})", "SCENARIO_OWNED_SERVICE_FIELD_INVALID"},
        {"svc_field_nonstring", R"("playerStart":{"ownedServices":[{"serviceId":123}]})", "SCENARIO_OWNED_SERVICE_FIELD_INVALID"},
        {"svc_field_empty", R"("playerStart":{"ownedServices":[{"serviceId":""}]})", "SCENARIO_OWNED_SERVICE_FIELD_INVALID"},
        {"svc_flag", R"("playerStart":{"ownedServices":[{"serviceId":"alpha_mine","locked":"yes"}]})", "SCENARIO_OWNED_SERVICE_FLAG_INVALID"},
        {"svc_flag_destroyed", R"("playerStart":{"ownedServices":[{"serviceId":"alpha_mine","destroyed":"yes"}]})", "SCENARIO_OWNED_SERVICE_FLAG_INVALID"},
        {"svc_dup", R"("playerStart":{"ownedServices":[{"serviceId":"alpha_mine"},{"serviceId":"alpha_mine"}]})", "SCENARIO_OWNED_SERVICE_DUPLICATE"},
        {"svc_unknown", R"("playerStart":{"ownedServices":[{"serviceId":"nope"}]})", "SCENARIO_OWNED_SERVICE_UNKNOWN"},
        {"svc_not_ownable", R"("playerStart":{"ownedServices":[{"serviceId":"alpha_rest"}]})", "SCENARIO_OWNED_SERVICE_NOT_OWNABLE"},
    };
    for (const auto& c : cases) {
        const std::filesystem::path root = "saves/ss_bad_" + c.name;
        data::ContentRepository repo;
        const auto msgs = LoadScenario(root,
            R"({"id":"s1","startRegionId":"alpha",)" + c.ps + "}", repo);
        INFO(c.name << " expected " << c.code);
        REQUIRE(HasCode(msgs, c.code));
        std::filesystem::remove_all(root);
    }
}

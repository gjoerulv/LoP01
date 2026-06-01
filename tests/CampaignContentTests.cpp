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

void WriteBaselineContent(const std::filesystem::path& root) {
    std::filesystem::create_directories(root);
    WriteTextFile(root / "regions.json", R"({"schemaVersion":1,"kind":"RegionCollection","id":"regions","regions":[
        {"id":"alpha","name":"Alpha","unlocked":true,"arrivalNodeId":"alpha_arrival","nodes":[
            {"location_id":"alpha_arrival","x":0,"y":0}],"links":[]},
        {"id":"beta","name":"Beta","unlocked":true,"arrivalNodeId":"beta_arrival","nodes":[
            {"location_id":"beta_arrival","x":0,"y":0}],"links":[]}
    ]})");
    WriteTextFile(root / "locations.json", R"({"schemaVersion":1,"kind":"LocationCollection","id":"locations","locations":[
        {"id":"alpha_arrival","name":"Alpha Arrival","type":"town","allows_sleep":false,"overworld_destination":true},
        {"id":"beta_arrival","name":"Beta Arrival","type":"town","allows_sleep":false,"overworld_destination":true}
    ]})");
    WriteTextFile(root / "location_scenes.json", R"({"schemaVersion":1,"kind":"LocationSceneCollection","id":"location_scenes","location_scenes":[]})");
    WriteTextFile(root / "units.json", R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[{"id":"hero","name":"Hero","category":"hero","is_player_character":true,"attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":1,"life":1,"position":"front","range":"melee"}]})");
    WriteTextFile(root / "battle_scenarios.json", R"({"schemaVersion":1,"kind":"BattleScenarioCollection","id":"battle_scenarios","battle_scenarios":[]})");
    WriteTextFile(root / "enemy_groups.json", R"({"schemaVersion":1,"kind":"EnemyGroupCollection","id":"enemy_groups","enemy_groups":[]})");
    WriteTextFile(root / "quests.json", R"({"schemaVersion":1,"kind":"QuestCollection","id":"quests","quests":[]})");
    WriteTextFile(root / "location_services.json", R"({"schemaVersion":1,"kind":"LocationServiceCollection","id":"location_services","location_services":[]})");
    // Two scenarios for campaign references to resolve against.
    WriteTextFile(root / "scenarios.json", R"({"schemaVersion":1,"kind":"ScenarioCollection","id":"scenarios","scenarios":[
        {"id":"s1","startRegionId":"alpha"},
        {"id":"s2","startRegionId":"beta"}
    ]})");
}

bool HasMessageWithCode(const std::vector<ValidationMessage>& msgs, const std::string& code) {
    return std::any_of(msgs.begin(), msgs.end(),
        [&](const ValidationMessage& m) { return m.code == code; });
}

const char* kValidCampaign = R"({"schemaVersion":1,"kind":"CampaignCollection","id":"campaigns","campaigns":[
    {"id":"camp","name":"Camp","description":"d","startScenarioId":"s1",
     "campaignFlags":["flag_done"],
     "carryOverRules":[{"id":"carry","carryGold":true,"carryRoster":true,"carryItems":true,"carryArtifacts":true,"carryStoryFlags":["keep"]}],
     "scenarios":[
        {"scenarioId":"s1","nextScenarioIds":["s2"],"carryOverRuleId":"carry"},
        {"scenarioId":"s2","nextScenarioIds":[],"carryOverRuleId":""}
     ]}
]})";

} // namespace

TEST_CASE("Campaign content: missing campaigns.json yields no campaigns (legal)") {
    const std::filesystem::path root = "saves/campaign_missing_test";
    WriteBaselineContent(root);
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(root));
    REQUIRE(repo.Campaigns().empty());
    std::filesystem::remove_all(root);
}

TEST_CASE("Campaign content: a valid campaign loads its graph and rules") {
    const std::filesystem::path root = "saves/campaign_valid_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "campaigns.json", kValidCampaign);

    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(root));
    const auto* c = repo.FindCampaignById("camp");
    REQUIRE(c != nullptr);
    REQUIRE(c->startScenarioId == "s1");
    REQUIRE(c->scenarios.size() == 2);
    REQUIRE(c->campaignFlags.size() == 1);
    const auto* rule = c->FindCarryOverRule("carry");
    REQUIRE(rule != nullptr);
    REQUIRE(rule->carryRoster);
    REQUIRE(rule->carryStoryFlags.size() == 1);
    const auto* entry = c->FindScenarioEntry("s1");
    REQUIRE(entry != nullptr);
    REQUIRE(entry->nextScenarioIds.size() == 1);
    REQUIRE(entry->nextScenarioIds[0] == "s2");
    std::filesystem::remove_all(root);
}

TEST_CASE("Campaign content: empty id rejected, campaign skipped") {
    const std::filesystem::path root = "saves/campaign_emptyid_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "campaigns.json", R"({"schemaVersion":1,"kind":"CampaignCollection","id":"campaigns","campaigns":[
        {"id":"","startScenarioId":"s1","scenarios":[{"scenarioId":"s1"}]}
    ]})");
    data::ContentRepository repo;
    (void)repo.LoadFromDirectory(root);
    REQUIRE(HasMessageWithCode(repo.ValidationMessages(), "CAMPAIGN_ID_EMPTY"));
    REQUIRE(repo.Campaigns().empty());
    std::filesystem::remove_all(root);
}

TEST_CASE("Campaign content: duplicate id rejected") {
    const std::filesystem::path root = "saves/campaign_dup_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "campaigns.json", R"({"schemaVersion":1,"kind":"CampaignCollection","id":"campaigns","campaigns":[
        {"id":"c","startScenarioId":"s1","scenarios":[{"scenarioId":"s1"}]},
        {"id":"c","startScenarioId":"s2","scenarios":[{"scenarioId":"s2"}]}
    ]})");
    data::ContentRepository repo;
    (void)repo.LoadFromDirectory(root);
    REQUIRE(HasMessageWithCode(repo.ValidationMessages(), "CAMPAIGN_DUPLICATE"));
    REQUIRE(repo.Campaigns().size() == 1);
    std::filesystem::remove_all(root);
}

TEST_CASE("Campaign content: unknown scenario reference rejected") {
    const std::filesystem::path root = "saves/campaign_badscenario_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "campaigns.json", R"({"schemaVersion":1,"kind":"CampaignCollection","id":"campaigns","campaigns":[
        {"id":"c","startScenarioId":"s1","scenarios":[{"scenarioId":"s1","nextScenarioIds":["ghost"]}]}
    ]})");
    data::ContentRepository repo;
    (void)repo.LoadFromDirectory(root);
    REQUIRE(HasMessageWithCode(repo.ValidationMessages(), "CAMPAIGN_NEXT_SCENARIO_UNKNOWN"));
    std::filesystem::remove_all(root);
}

TEST_CASE("Campaign content: unknown carry-over rule reference rejected") {
    const std::filesystem::path root = "saves/campaign_badrule_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "campaigns.json", R"({"schemaVersion":1,"kind":"CampaignCollection","id":"campaigns","campaigns":[
        {"id":"c","startScenarioId":"s1","scenarios":[{"scenarioId":"s1","carryOverRuleId":"nope"}]}
    ]})");
    data::ContentRepository repo;
    (void)repo.LoadFromDirectory(root);
    REQUIRE(HasMessageWithCode(repo.ValidationMessages(), "CAMPAIGN_CARRYOVER_RULE_UNKNOWN"));
    std::filesystem::remove_all(root);
}

TEST_CASE("Campaign content: missing start scenario rejected") {
    const std::filesystem::path root = "saves/campaign_badstart_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "campaigns.json", R"({"schemaVersion":1,"kind":"CampaignCollection","id":"campaigns","campaigns":[
        {"id":"c","startScenarioId":"s_missing","scenarios":[{"scenarioId":"s1"}]}
    ]})");
    data::ContentRepository repo;
    (void)repo.LoadFromDirectory(root);
    REQUIRE(HasMessageWithCode(repo.ValidationMessages(), "CAMPAIGN_START_SCENARIO_UNKNOWN"));
    std::filesystem::remove_all(root);
}

TEST_CASE("Campaign content: reload without campaigns.json clears prior campaigns") {
    const std::filesystem::path root = "saves/campaign_reload_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "campaigns.json", kValidCampaign);
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(root));
    REQUIRE_FALSE(repo.Campaigns().empty());

    std::filesystem::remove(root / "campaigns.json");
    REQUIRE(repo.LoadFromDirectory(root));
    REQUIRE(repo.Campaigns().empty());
    std::filesystem::remove_all(root);
}

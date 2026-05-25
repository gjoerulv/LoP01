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
    WriteTextFile(root / "regions.json", R"({"schemaVersion":1,"kind":"RegionCollection","id":"regions","regions":[{"id":"r","name":"R","unlocked":true,"nodes":[{"location_id":"loc","x":0,"y":0,"discovered":true,"travel_available":true}],"links":[]}]})");
    WriteTextFile(root / "locations.json", R"({"schemaVersion":1,"kind":"LocationCollection","id":"locations","locations":[{"id":"loc","name":"Loc","type":"combat","allows_sleep":false,"overworld_destination":true,"battle_scenario_id":"debug_intro_battle"}]})");
    WriteTextFile(root / "location_scenes.json", R"({"schemaVersion":1,"kind":"LocationSceneCollection","id":"location_scenes","location_scenes":[]})");
    WriteTextFile(root / "units.json", R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[{"id":"hero","name":"Hero","category":"hero","is_player_character":true,"attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":1,"life":1,"position":"front","range":"melee"}]})");
    WriteTextFile(root / "battle_scenarios.json", R"({"schemaVersion":1,"kind":"BattleScenarioCollection","id":"battle_scenarios","battle_scenarios":[{"id":"debug_intro_battle","name":"Debug","seed":7,"allies":[{"unit_id":"hero"}],"enemies":[{"unit_id":"hero"}]}]})");
    WriteTextFile(root / "enemy_groups.json", R"({"schemaVersion":1,"kind":"EnemyGroupCollection","id":"enemy_groups","enemy_groups":[]})");
    WriteTextFile(root / "quests.json", R"({"schemaVersion":1,"kind":"QuestCollection","id":"quests","quests":[]})");
    WriteTextFile(root / "location_services.json", R"({"schemaVersion":1,"kind":"LocationServiceCollection","id":"location_services","location_services":[]})");
}

bool HasMessageWithCode(const std::vector<ValidationMessage>& msgs, const std::string& code) {
    return std::any_of(msgs.begin(), msgs.end(),
        [&](const ValidationMessage& m) { return m.code == code; });
}

} // namespace

TEST_CASE("ContentRepository: missing artifacts.json yields empty artifact list (legal)") {
    const std::filesystem::path root = "saves/artifacts_missing_test";
    WriteBaselineContent(root);

    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(root));
    REQUIRE(repo.Artifacts().empty());

    std::filesystem::remove_all(root);
}

TEST_CASE("ContentRepository: artifacts.json happy path with statBonus loads cleanly") {
    const std::filesystem::path root = "saves/artifacts_happy_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "artifacts.json", R"({
        "schemaVersion": 1,
        "kind": "ArtifactCollection",
        "id": "artifacts",
        "artifacts": [
            {
              "id": "artifact_iron_sword",
              "name": {"en":"Iron Sword"},
              "icon": "icon_iron_sword",
              "allowedSlots": ["Attack"],
              "rarity": "minor",
              "tier": 1,
              "baseValue": 250,
              "combinable": true,
              "effects": [ { "type": "statBonus", "stat": "Attack", "amount": 2 } ]
            },
            {
              "id": "artifact_journeyman_charm",
              "name": {"en":"Journeyman Charm"},
              "icon": "icon_charm",
              "allowedSlots": ["Misc"],
              "rarity": "minor",
              "tier": 1,
              "baseValue": 150,
              "combinable": false,
              "effects": [
                { "type": "statBonus", "stat": "Defense",    "amount": 1 },
                { "type": "statBonus", "stat": "Resistance", "amount": 1 }
              ]
            }
        ]
    })");

    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(root));

    const auto& artifacts = repo.Artifacts();
    REQUIRE(artifacts.size() == 2);

    const auto* iron = repo.FindArtifactById("artifact_iron_sword");
    REQUIRE(iron != nullptr);
    REQUIRE(iron->allowedSlots.size() == 1);
    REQUIRE(iron->allowedSlots[0] == data::ArtifactSlotKind::Attack);
    REQUIRE(iron->combinable);
    REQUIRE(iron->statBonuses.size() == 1);
    REQUIRE(iron->statBonuses[0].stat == data::ArtifactStatBonusStat::Attack);
    REQUIRE(iron->statBonuses[0].amount == 2);

    const auto* charm = repo.FindArtifactById("artifact_journeyman_charm");
    REQUIRE(charm != nullptr);
    REQUIRE_FALSE(charm->combinable);
    REQUIRE(charm->statBonuses.size() == 2);

    REQUIRE(repo.FindArtifactById("artifact_does_not_exist") == nullptr);

    std::filesystem::remove_all(root);
}

TEST_CASE("ContentRepository: artifact with no effects field loads as a valid stat-less artifact") {
    const std::filesystem::path root = "saves/artifacts_no_effects_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "artifacts.json", R"({
        "schemaVersion": 1,
        "kind": "ArtifactCollection",
        "id": "artifacts",
        "artifacts": [
            { "id": "artifact_plain", "name": {"en":"Plain"}, "icon": "x",
              "allowedSlots": ["Misc"], "rarity": "minor", "tier": 1,
              "baseValue": 10, "combinable": false }
        ]
    })");

    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(root));
    REQUIRE(repo.Artifacts().size() == 1);
    REQUIRE(repo.Artifacts()[0].statBonuses.empty());

    std::filesystem::remove_all(root);
}

TEST_CASE("ContentRepository: unknown artifact effect type (specialEffect) is rejected, not silently ignored") {
    const std::filesystem::path root = "saves/artifacts_special_effect_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "artifacts.json", R"({
        "schemaVersion": 1,
        "kind": "ArtifactCollection",
        "id": "artifacts",
        "artifacts": [
            { "id": "artifact_disabler", "name": {"en":"Disabler"}, "icon": "x",
              "allowedSlots": ["Misc"], "rarity": "major", "tier": 3, "baseValue": 1000,
              "combinable": false,
              "effects": [ { "type": "specialEffect", "effect": "DisableAllUsableSkillsInBattleForBothTeams" } ] }
        ]
    })");

    data::ContentRepository repo;
    static_cast<void>(repo.LoadFromDirectory(root));
    REQUIRE(HasMessageWithCode(repo.ValidationMessages(), "ARTIFACT_EFFECT_TYPE_UNSUPPORTED"));
    REQUIRE(repo.Artifacts().empty());

    std::filesystem::remove_all(root);
}

TEST_CASE("ContentRepository: unknown statBonus stat is rejected") {
    const std::filesystem::path root = "saves/artifacts_bad_stat_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "artifacts.json", R"({
        "schemaVersion": 1,
        "kind": "ArtifactCollection",
        "id": "artifacts",
        "artifacts": [
            { "id": "artifact_lucky", "name": {"en":"Lucky"}, "icon": "x",
              "allowedSlots": ["Misc"], "rarity": "minor", "tier": 1, "baseValue": 10,
              "combinable": false,
              "effects": [ { "type": "statBonus", "stat": "Luck", "amount": 1 } ] }
        ]
    })");

    data::ContentRepository repo;
    static_cast<void>(repo.LoadFromDirectory(root));
    REQUIRE(HasMessageWithCode(repo.ValidationMessages(), "ARTIFACT_STAT_BONUS_STAT_UNKNOWN"));
    REQUIRE(repo.Artifacts().empty());

    std::filesystem::remove_all(root);
}

TEST_CASE("ContentRepository: unknown allowedSlots entry is rejected") {
    const std::filesystem::path root = "saves/artifacts_bad_slot_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "artifacts.json", R"({
        "schemaVersion": 1,
        "kind": "ArtifactCollection",
        "id": "artifacts",
        "artifacts": [
            { "id": "artifact_helm", "name": {"en":"Helm"}, "icon": "x",
              "allowedSlots": ["Head"], "rarity": "minor", "tier": 1, "baseValue": 10,
              "combinable": false }
        ]
    })");

    data::ContentRepository repo;
    static_cast<void>(repo.LoadFromDirectory(root));
    REQUIRE(HasMessageWithCode(repo.ValidationMessages(), "ARTIFACT_ALLOWED_SLOT_UNKNOWN"));
    REQUIRE(repo.Artifacts().empty());

    std::filesystem::remove_all(root);
}

TEST_CASE("ContentRepository: missing allowedSlots field is rejected") {
    const std::filesystem::path root = "saves/artifacts_no_slots_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "artifacts.json", R"({
        "schemaVersion": 1,
        "kind": "ArtifactCollection",
        "id": "artifacts",
        "artifacts": [
            { "id": "artifact_floating", "name": {"en":"Floating"}, "icon": "x",
              "rarity": "minor", "tier": 1, "baseValue": 10, "combinable": false }
        ]
    })");

    data::ContentRepository repo;
    static_cast<void>(repo.LoadFromDirectory(root));
    REQUIRE(HasMessageWithCode(repo.ValidationMessages(), "ARTIFACT_ALLOWED_SLOTS_MISSING"));
    REQUIRE(repo.Artifacts().empty());

    std::filesystem::remove_all(root);
}

TEST_CASE("ContentRepository: negative artifact baseValue is rejected") {
    const std::filesystem::path root = "saves/artifacts_negative_value_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "artifacts.json", R"({
        "schemaVersion": 1,
        "kind": "ArtifactCollection",
        "id": "artifacts",
        "artifacts": [
            { "id": "artifact_priced_wrong", "name": {"en":"Bad"}, "icon": "x",
              "allowedSlots": ["Misc"], "rarity": "minor", "tier": 1, "baseValue": -10,
              "combinable": false }
        ]
    })");

    data::ContentRepository repo;
    static_cast<void>(repo.LoadFromDirectory(root));
    REQUIRE(HasMessageWithCode(repo.ValidationMessages(), "ARTIFACT_BASE_VALUE_NEGATIVE"));
    REQUIRE(repo.Artifacts().empty());

    std::filesystem::remove_all(root);
}

TEST_CASE("ContentRepository: statBonus missing amount is rejected") {
    const std::filesystem::path root = "saves/artifacts_missing_amount_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "artifacts.json", R"({
        "schemaVersion": 1,
        "kind": "ArtifactCollection",
        "id": "artifacts",
        "artifacts": [
            { "id": "artifact_blank", "name": {"en":"Blank"}, "icon": "x",
              "allowedSlots": ["Attack"], "rarity": "minor", "tier": 1, "baseValue": 10,
              "combinable": false,
              "effects": [ { "type": "statBonus", "stat": "Attack" } ] }
        ]
    })");

    data::ContentRepository repo;
    static_cast<void>(repo.LoadFromDirectory(root));
    REQUIRE(HasMessageWithCode(repo.ValidationMessages(), "ARTIFACT_STAT_BONUS_AMOUNT_INVALID"));
    REQUIRE(repo.Artifacts().empty());

    std::filesystem::remove_all(root);
}

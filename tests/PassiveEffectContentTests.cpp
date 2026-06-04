#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>

#include "data/ContentRepository.h"
#include "data/definitions/LocationServiceDefinition.h"
#include "data/definitions/UnitDefinition.h"
#include "gameplay/economy/MineProductionRules.h"
#include "gameplay/economy/StationedProductionRules.h"

namespace {

void WriteTextFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream output(path, std::ios::trunc);
    output << content;
}

bool HasErrorMessage(const std::vector<ValidationMessage>& msgs) {
    return std::ranges::any_of(msgs,
        [](const ValidationMessage& m) { return m.severity == Severity::Error; });
}

bool HasCode(const std::vector<ValidationMessage>& msgs, const std::string& code) {
    return std::ranges::any_of(msgs,
        [&](const ValidationMessage& m) { return m.code == code; });
}

// Writes a minimal content directory (empty regions/locations/services) plus the
// caller-supplied units.json body.
void WriteContentWithUnits(const std::filesystem::path& root, const std::string& unitsJson) {
    std::filesystem::create_directories(root);
    WriteTextFile(root / "regions.json", R"({"schemaVersion":1,"kind":"RegionCollection","id":"regions","regions":[]})");
    WriteTextFile(root / "locations.json", R"({"schemaVersion":1,"kind":"LocationCollection","id":"locations","locations":[]})");
    WriteTextFile(root / "location_scenes.json", R"({"schemaVersion":1,"kind":"LocationSceneCollection","id":"location_scenes","location_scenes":[]})");
    WriteTextFile(root / "units.json", unitsJson);
    WriteTextFile(root / "battle_scenarios.json", R"({"schemaVersion":1,"kind":"BattleScenarioCollection","id":"battle_scenarios","battle_scenarios":[]})");
    WriteTextFile(root / "enemy_groups.json", R"({"schemaVersion":1,"kind":"EnemyGroupCollection","id":"enemy_groups","enemy_groups":[]})");
    WriteTextFile(root / "quests.json", R"({"schemaVersion":1,"kind":"QuestCollection","id":"quests","quests":[]})");
    WriteTextFile(root / "location_services.json", R"({"schemaVersion":1,"kind":"LocationServiceCollection","id":"location_services","location_services":[]})");
}

const char* kStatsTail =
    R"("attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":1,"life":1,"position":"front","range":"melee")";

} // namespace

TEST_CASE("PassiveEffect content - legacy mine_production_passive and canonical passive_effects load to the same shape") {
    const std::filesystem::path root = "saves/passive_effect_parity";
    const std::string units = std::string(
        R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[)") +
        R"({"id":"legacy","name":"Legacy","category":"generic","is_player_character":false,)" + kStatsTail +
        R"(,"mine_production_passive":{"resource":"Stone","amount":2}},)" +
        R"({"id":"canon","name":"Canon","category":"generic","is_player_character":false,)" + kStatsTail +
        R"(,"passive_effects":[{"kind":"mine_production","resource":"Stone","amount":2}]}]})";
    WriteContentWithUnits(root, units);

    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));
    REQUIRE_FALSE(HasErrorMessage(content.ValidationMessages()));

    const auto* legacy = content.FindUnitById("legacy");
    const auto* canon = content.FindUnitById("canon");
    REQUIRE(legacy != nullptr);
    REQUIRE(canon != nullptr);

    REQUIRE(legacy->passiveEffects.size() == 1);
    REQUIRE(canon->passiveEffects.size() == 1);

    const auto& a = legacy->passiveEffects[0];
    const auto& b = canon->passiveEffects[0];
    REQUIRE(a.kind == data::PassiveEffectKind::MineProduction);
    REQUIRE(b.kind == data::PassiveEffectKind::MineProduction);
    REQUIRE(a.resource == "Stone");
    REQUIRE(b.resource == "Stone");
    REQUIRE(a.target == "mine");
    REQUIRE(b.target == "mine");
    REQUIRE(a.amount == 2);
    REQUIRE(b.amount == 2);

    std::filesystem::remove_all(root);
}

TEST_CASE("PassiveEffect content - runtime mine-production collection reads the canonical passiveEffects") {
    const std::filesystem::path root = "saves/passive_effect_runtime_read";
    const std::string units = std::string(
        R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[)") +
        R"({"id":"canon","name":"Canon","category":"generic","is_player_character":false,)" + kStatsTail +
        R"(,"passive_effects":[{"kind":"mine_production","resource":"Stone","amount":3}]}]})";
    WriteContentWithUnits(root, units);

    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));

    const auto* unit = content.FindUnitById("canon");
    REQUIRE(unit != nullptr);

    const auto passives = gameplay::economy::CollectMineProductionPassives(
        {unit}, data::LocationServiceKind::Mine);
    REQUIRE(passives.size() == 1);
    REQUIRE(passives[0].resource == gameplay::ResourceType::Stone);
    REQUIRE(passives[0].amount == 3);

    std::filesystem::remove_all(root);
}

TEST_CASE("PassiveEffect content - a leader_energy effect loads with no resource or target") {
    const std::filesystem::path root = "saves/passive_effect_leader_energy";
    const std::string units = std::string(
        R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[)") +
        R"({"id":"captain","name":"Captain","category":"leader","is_player_character":true,)" + kStatsTail +
        R"(,"passive_effects":[{"kind":"leader_energy","amount":50}]}]})";
    WriteContentWithUnits(root, units);

    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));
    REQUIRE_FALSE(HasErrorMessage(content.ValidationMessages()));

    const auto* unit = content.FindUnitById("captain");
    REQUIRE(unit != nullptr);
    REQUIRE(unit->passiveEffects.size() == 1);
    REQUIRE(unit->passiveEffects[0].kind == data::PassiveEffectKind::LeaderEnergy);
    REQUIRE(unit->passiveEffects[0].amount == 50);
    REQUIRE(unit->passiveEffects[0].resource.empty());
    REQUIRE(unit->passiveEffects[0].target.empty());

    std::filesystem::remove_all(root);
}

TEST_CASE("PassiveEffect content - authoring both mine_production_passive and passive_effects is rejected") {
    const std::filesystem::path root = "saves/passive_effect_mixed";
    const std::string units = std::string(
        R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[)") +
        R"({"id":"mixed","name":"Mixed","category":"generic","is_player_character":false,)" + kStatsTail +
        R"(,"mine_production_passive":{"resource":"Stone","amount":2})" +
        R"(,"passive_effects":[{"kind":"mine_production","resource":"Wood","amount":1}]}]})";
    WriteContentWithUnits(root, units);

    data::ContentRepository content;
    REQUIRE_FALSE(content.LoadFromDirectory(root));
    REQUIRE(HasCode(content.ValidationMessages(), "PASSIVE_EFFECT_LEGACY_MIXED_WITH_CANONICAL"));

    std::filesystem::remove_all(root);
}

TEST_CASE("PassiveEffect content - passive_effects authored as an object fails with PASSIVE_EFFECTS_TYPE_INVALID") {
    const std::filesystem::path root = "saves/passive_effects_object";
    const std::string units = std::string(
        R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[)") +
        R"({"id":"bad","name":"Bad","category":"generic","is_player_character":false,)" + kStatsTail +
        R"(,"passive_effects":{}}]})";
    WriteContentWithUnits(root, units);

    data::ContentRepository content;
    REQUIRE_FALSE(content.LoadFromDirectory(root));
    REQUIRE(HasCode(content.ValidationMessages(), "PASSIVE_EFFECTS_TYPE_INVALID"));

    std::filesystem::remove_all(root);
}

TEST_CASE("PassiveEffect content - passive_effects authored as a string fails with PASSIVE_EFFECTS_TYPE_INVALID") {
    const std::filesystem::path root = "saves/passive_effects_string";
    const std::string units = std::string(
        R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[)") +
        R"({"id":"bad","name":"Bad","category":"generic","is_player_character":false,)" + kStatsTail +
        R"(,"passive_effects":"mine"}]})";
    WriteContentWithUnits(root, units);

    data::ContentRepository content;
    REQUIRE_FALSE(content.LoadFromDirectory(root));
    REQUIRE(HasCode(content.ValidationMessages(), "PASSIVE_EFFECTS_TYPE_INVALID"));

    std::filesystem::remove_all(root);
}

TEST_CASE("PassiveEffect content - a non-object passive_effects entry fails with PASSIVE_EFFECT_ENTRY_TYPE_INVALID") {
    const std::filesystem::path root = "saves/passive_effects_bad_entry";
    const std::string units = std::string(
        R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[)") +
        R"({"id":"bad","name":"Bad","category":"generic","is_player_character":false,)" + kStatsTail +
        R"(,"passive_effects":[123]}]})";
    WriteContentWithUnits(root, units);

    data::ContentRepository content;
    REQUIRE_FALSE(content.LoadFromDirectory(root));
    REQUIRE(HasCode(content.ValidationMessages(), "PASSIVE_EFFECT_ENTRY_TYPE_INVALID"));

    std::filesystem::remove_all(root);
}

TEST_CASE("PassiveEffect content - an unknown effect kind fails validation") {
    const std::filesystem::path root = "saves/passive_effect_unknown_kind";
    const std::string units = std::string(
        R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[)") +
        R"({"id":"bad","name":"Bad","category":"generic","is_player_character":false,)" + kStatsTail +
        R"(,"passive_effects":[{"kind":"teleport","amount":1}]}]})";
    WriteContentWithUnits(root, units);

    data::ContentRepository content;
    REQUIRE_FALSE(content.LoadFromDirectory(root));
    REQUIRE(HasCode(content.ValidationMessages(), "PASSIVE_EFFECT_KIND_INVALID"));

    std::filesystem::remove_all(root);
}

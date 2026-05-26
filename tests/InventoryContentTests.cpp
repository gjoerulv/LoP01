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

// Mirrors the baseline content set used by ScenarioOutcomeContentTests so the
// repository's required files (regions, locations, etc.) are all valid while
// the test exercises items.json variations.
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

TEST_CASE("ContentRepository: missing items.json yields empty inventory (legal)") {
    const std::filesystem::path root = "saves/inventory_items_missing_test";
    WriteBaselineContent(root);

    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(root));
    REQUIRE(repo.Items().empty());

    std::filesystem::remove_all(root);
}

TEST_CASE("ContentRepository: items.json happy path with mixed subtypes loads cleanly") {
    const std::filesystem::path root = "saves/inventory_items_happy_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "items.json", R"({
        "schemaVersion": 1,
        "kind": "ItemCollection",
        "id": "items",
        "items": [
            { "id": "item_traveler_ration", "name": {"en":"Traveler Ration"}, "icon": "icon_ration",
              "subtype": "consumable", "stackCap": 1, "baseValue": 25 },
            { "id": "item_ashvale_token",   "name": {"en":"Ashvale Token"},  "icon": "icon_token",
              "subtype": "quest",      "stackCap": 1, "baseValue": 0 },
            { "id": "item_oak_log",         "name": {"en":"Oak Log"},        "icon": "icon_log",
              "subtype": "material",   "stackCap": 999, "baseValue": 5 }
        ]
    })");

    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(root));

    const auto& items = repo.Items();
    REQUIRE(items.size() == 3);
    REQUIRE(items[0].id == "item_traveler_ration");
    REQUIRE(items[0].subtype == data::ItemSubtype::Consumable);
    REQUIRE(items[0].stackCap == 1);
    REQUIRE(items[1].subtype == data::ItemSubtype::Quest);
    REQUIRE(items[2].subtype == data::ItemSubtype::Material);

    REQUIRE(repo.FindItemById("item_ashvale_token") != nullptr);
    REQUIRE(repo.FindItemById("item_does_not_exist") == nullptr);

    std::filesystem::remove_all(root);
}

TEST_CASE("ContentRepository: unknown item subtype produces explicit validation error") {
    const std::filesystem::path root = "saves/inventory_items_bad_subtype_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "items.json", R"({
        "schemaVersion": 1,
        "kind": "ItemCollection",
        "id": "items",
        "items": [
            { "id": "item_weird", "name": {"en":"Weird"}, "icon": "x",
              "subtype": "armor", "stackCap": 1, "baseValue": 0 }
        ]
    })");

    data::ContentRepository repo;
    static_cast<void>(repo.LoadFromDirectory(root));
    REQUIRE(HasMessageWithCode(repo.ValidationMessages(), "ITEM_SUBTYPE_UNKNOWN"));
    REQUIRE(repo.Items().empty());

    std::filesystem::remove_all(root);
}

TEST_CASE("ContentRepository: non-positive item stackCap produces explicit validation error") {
    const std::filesystem::path root = "saves/inventory_items_bad_stack_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "items.json", R"({
        "schemaVersion": 1,
        "kind": "ItemCollection",
        "id": "items",
        "items": [
            { "id": "item_zero_stack", "name": {"en":"Bad"}, "icon": "x",
              "subtype": "material", "stackCap": 0, "baseValue": 0 }
        ]
    })");

    data::ContentRepository repo;
    static_cast<void>(repo.LoadFromDirectory(root));
    REQUIRE(HasMessageWithCode(repo.ValidationMessages(), "ITEM_STACK_CAP_INVALID"));
    REQUIRE(repo.Items().empty());

    std::filesystem::remove_all(root);
}

TEST_CASE("ContentRepository: negative item baseValue produces explicit validation error") {
    const std::filesystem::path root = "saves/inventory_items_negative_value_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "items.json", R"({
        "schemaVersion": 1,
        "kind": "ItemCollection",
        "id": "items",
        "items": [
            { "id": "item_priced_wrong", "name": {"en":"Bad"}, "icon": "x",
              "subtype": "material", "stackCap": 1, "baseValue": -5 }
        ]
    })");

    data::ContentRepository repo;
    static_cast<void>(repo.LoadFromDirectory(root));
    REQUIRE(HasMessageWithCode(repo.ValidationMessages(), "ITEM_BASE_VALUE_NEGATIVE"));
    REQUIRE(repo.Items().empty());

    std::filesystem::remove_all(root);
}

TEST_CASE("ContentRepository: item with effects field is rejected as unsupported in M13") {
    const std::filesystem::path root = "saves/inventory_items_effects_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "items.json", R"({
        "schemaVersion": 1,
        "kind": "ItemCollection",
        "id": "items",
        "items": [
            { "id": "item_potion", "name": {"en":"Potion"}, "icon": "x",
              "subtype": "consumable", "stackCap": 1, "baseValue": 50,
              "effects": [ { "type": "recoverHp", "target": "hero", "amount": 50 } ] }
        ]
    })");

    data::ContentRepository repo;
    static_cast<void>(repo.LoadFromDirectory(root));
    REQUIRE(HasMessageWithCode(repo.ValidationMessages(), "ITEM_EFFECTS_UNSUPPORTED"));
    // The item is rejected — not silently accepted with effects ignored.
    REQUIRE(repo.Items().empty());

    std::filesystem::remove_all(root);
}

TEST_CASE("ContentRepository: duplicate item id produces ITEM_ID_DUPLICATE; first occurrence is kept") {
    const std::filesystem::path root = "saves/inventory_items_duplicate_id_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "items.json", R"({
        "schemaVersion": 1,
        "kind": "ItemCollection",
        "id": "items",
        "items": [
            { "id": "item_dup", "name": {"en":"First"},     "icon": "x",
              "subtype": "material", "stackCap": 10, "baseValue": 1 },
            { "id": "item_dup", "name": {"en":"Duplicate"}, "icon": "x",
              "subtype": "material", "stackCap": 20, "baseValue": 2 }
        ]
    })");

    data::ContentRepository repo;
    static_cast<void>(repo.LoadFromDirectory(root));
    REQUIRE(HasMessageWithCode(repo.ValidationMessages(), "ITEM_ID_DUPLICATE"));

    // The first occurrence stays in the loaded catalog; the duplicate does not
    // erase it. This matches the EVENT_ID_DUPLICATE pattern.
    REQUIRE(repo.Items().size() == 1);
    REQUIRE(repo.Items()[0].id == "item_dup");
    REQUIRE(repo.Items()[0].name == "First");
    REQUIRE(repo.Items()[0].stackCap == 10);

    std::filesystem::remove_all(root);
}

TEST_CASE("ContentRepository: empty item id produces explicit validation error") {
    const std::filesystem::path root = "saves/inventory_items_empty_id_test";
    WriteBaselineContent(root);
    WriteTextFile(root / "items.json", R"({
        "schemaVersion": 1,
        "kind": "ItemCollection",
        "id": "items",
        "items": [
            { "name": {"en":"Nameless"}, "subtype": "material", "stackCap": 1, "baseValue": 0 }
        ]
    })");

    data::ContentRepository repo;
    static_cast<void>(repo.LoadFromDirectory(root));
    REQUIRE(HasMessageWithCode(repo.ValidationMessages(), "ITEM_ID_EMPTY"));
    REQUIRE(repo.Items().empty());

    std::filesystem::remove_all(root);
}

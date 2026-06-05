#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "core/GameClock.h"
#include "core/SaveGame.h"
#include "data/ContentRepository.h"
#include "gameplay/GameSession.h"
#include "gameplay/ResourceState.h"

// End-to-end proof that both passive-effect spine consumers fire from authored
// `passive_effects`: a stationed mine_production unit boosts mine output, and a
// leader_energy passive on the active leader raises daily Energy — both resolved
// at the same day-boundary chokepoint. Uses a self-contained authored slice.

using gameplay::ResourceType;

namespace {

constexpr int kOneDay = core::GameClock::kMinutesPerSliceDay;

void WriteTextFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream output(path, std::ios::trunc);
    output << content;
}

bool HasErrorMessage(const std::vector<ValidationMessage>& msgs) {
    return std::ranges::any_of(msgs,
        [](const ValidationMessage& m) { return m.severity == Severity::Error; });
}

const char* kStatsTail =
    R"("attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":8,"life":1,"position":"front","range":"melee")";

// Writes the mine slice (one owned Stone mine, base output 2) with a
// caller-supplied units.json body.
void WriteMineSliceWithUnits(const std::filesystem::path& root, const std::string& unitsJson) {
    std::filesystem::create_directories(root);

    WriteTextFile(root / "regions.json", R"({"schemaVersion":1,"kind":"RegionCollection","id":"regions","regions":[{"id":"ashvale_heartland","name":"Ashvale Heartland","unlocked":true,"nodes":[{"location_id":"mine_loc","x":0,"y":0,"discovered":true,"travel_available":true}],"links":[]}]})");
    WriteTextFile(root / "locations.json", R"({"schemaVersion":1,"kind":"LocationCollection","id":"locations","locations":[{"id":"mine_loc","name":"Stone Mine","type":"recruit","allows_sleep":false,"overworld_destination":true,"scene_id":"mine_scene"}]})");
    WriteTextFile(root / "location_scenes.json", R"({"schemaVersion":1,"kind":"LocationSceneCollection","id":"location_scenes","location_scenes":[{"id":"mine_scene","spawn":{"x":0,"y":0,"width":1,"height":1},"blocking_rects":[],"zones":[{"id":"mine_face","type":"recruit","area":{"x":0,"y":0,"width":1,"height":1},"prompt_text":"","result_text":"","failure_text":"","time_cost_minutes":0,"gold_cost":0,"recruit_count":0,"dialogue_choice_time_cost_minutes":1,"dialogue_choices":[]}]}]})");
    WriteTextFile(root / "units.json", unitsJson);
    WriteTextFile(root / "battle_scenarios.json", R"({"schemaVersion":1,"kind":"BattleScenarioCollection","id":"battle_scenarios","battle_scenarios":[]})");
    WriteTextFile(root / "enemy_groups.json", R"({"schemaVersion":1,"kind":"EnemyGroupCollection","id":"enemy_groups","enemy_groups":[]})");
    WriteTextFile(root / "quests.json", R"({"schemaVersion":1,"kind":"QuestCollection","id":"quests","quests":[]})");
    WriteTextFile(root / "location_services.json", R"({"schemaVersion":1,"kind":"LocationServiceCollection","id":"location_services","location_services":[{"id":"stone_mine_svc","location_id":"mine_loc","zone_id":"mine_face","kind":"mine","mine_outputs":[{"resource":"Stone","amount":2}]}]})");
}

void WritePassiveEconomyContent(const std::filesystem::path& root) {
    const std::string units = std::string(
        R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[)") +
        R"({"id":"captain","name":"Captain","category":"leader","is_player_character":true,)" + kStatsTail +
        R"(,"passive_effects":[{"kind":"leader_energy","amount":50}]},)" +
        R"({"id":"miner","name":"Miner","category":"generic","is_player_character":false,)" + kStatsTail +
        R"(,"passive_effects":[{"kind":"mine_production","resource":"Stone","amount":1}]}]})";
    WriteMineSliceWithUnits(root, units);
}

} // namespace

TEST_CASE("PassiveEffect end-to-end: authored passive_effects drive both mine output and leader Energy at the day boundary") {
    const std::filesystem::path root = "saves/passive_effect_e2e";
    WritePassiveEconomyContent(root);

    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));
    REQUIRE_FALSE(HasErrorMessage(content.ValidationMessages()));

    gameplay::GameSession session;
    session.SetUnitCatalog(content.Units());
    session.SetLocationServiceCatalog(content.LocationServices());

    core::SaveData save;
    save.schemaVersion = 5;
    save.day = 1;
    save.minutesIntoSliceDay = 0;
    save.gold = 2500;
    save.mode = "region_mode";
    save.regionId = "ashvale_heartland";
    save.destinationId = "mine_loc";
    save.hasCanonicalRoster = true;
    save.rosterStacks = {
        core::RosterStackSaveState{"stk_1", "captain", 1},
        core::RosterStackSaveState{"stk_2", "miner", 1}
    };
    save.activeSlotStackIds = {"stk_1", "", "", "", ""};       // captain is the active leader
    save.reserveSlotStackIds = {"stk_2", "", "", "", "", "", "", ""};
    save.nextStackIdCounter = 3;
    save.ownedServices = {
        core::OwnedServiceSaveState{"stone_mine_svc", "Green", false, false,
            {core::StationedUnitSaveState{"miner", "stk_2"}}}  // mine_production +1 Stone
    };
    session.ApplySaveData(save);

    REQUIRE(session.ResourceCount(ResourceType::Stone) == 0);

    session.AddMinutes(kOneDay);  // single day boundary fires both consumers

    // Mine: 2 base + 1 stationed mine_production passive (strongest-only).
    REQUIRE(session.ResourceCount(ResourceType::Stone) == 3);
    // Energy: 1000 + lowest-agility(8)*100 + leader_energy(50).
    REQUIRE(session.MaxEnergy() == 1850);
    REQUIRE(session.CurrentEnergy() == 1850);

    std::filesystem::remove_all(root);
}

TEST_CASE("PassiveEffect end-to-end: legacy mine_production_passive authoring reaches real payout via canonical conversion") {
    const std::filesystem::path root = "saves/passive_effect_e2e_legacy";
    const std::string units = std::string(
        R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[)") +
        R"({"id":"miner","name":"Miner","category":"generic","is_player_character":false,)" + kStatsTail +
        R"(,"mine_production_passive":{"target":"mine","resource":"Stone","amount":1}}]})";
    WriteMineSliceWithUnits(root, units);

    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));
    REQUIRE_FALSE(HasErrorMessage(content.ValidationMessages()));

    // The legacy key is converted to a canonical MineProduction passiveEffects entry.
    const auto* miner = content.FindUnitById("miner");
    REQUIRE(miner != nullptr);
    REQUIRE(miner->passiveEffects.size() == 1);
    REQUIRE(miner->passiveEffects[0].kind == data::PassiveEffectKind::MineProduction);

    gameplay::GameSession session;
    session.SetUnitCatalog(content.Units());
    session.SetLocationServiceCatalog(content.LocationServices());

    core::SaveData save;
    save.schemaVersion = 5;
    save.day = 1;
    save.minutesIntoSliceDay = 0;
    save.gold = 2500;
    save.mode = "region_mode";
    save.regionId = "ashvale_heartland";
    save.destinationId = "mine_loc";
    save.hasCanonicalRoster = true;
    save.rosterStacks = { core::RosterStackSaveState{"stk_1", "miner", 1} };
    save.activeSlotStackIds = {"stk_1", "", "", "", ""};
    save.reserveSlotStackIds = {"", "", "", "", "", "", "", ""};
    save.nextStackIdCounter = 2;
    save.ownedServices = {
        core::OwnedServiceSaveState{"stone_mine_svc", "Green", false, false,
            {core::StationedUnitSaveState{"miner", "stk_1"}}}
    };
    session.ApplySaveData(save);

    REQUIRE(session.ResourceCount(ResourceType::Stone) == 0);
    session.AddMinutes(kOneDay);

    // Identical to the canonical case: base 2 + stationed passive 1 = 3.
    REQUIRE(session.ResourceCount(ResourceType::Stone) == 3);

    std::filesystem::remove_all(root);
}

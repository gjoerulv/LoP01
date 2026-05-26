#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "data/ContentRepository.h"
#include "gameplay/GameSession.h"
#include "gameplay/battle/BattleFactory.h"
#include "gameplay/events/EventDefinition.h"

namespace {

void WriteTextFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream output(path, std::ios::trunc);
    output << content;
}

// Builds a minimal content directory with one hero (hero_player) and one
// battle scenario where the hero is the only ally. The scenario is used by
// BattleFactory::CreateFromScenario; equipped-artifact stat bonuses are
// supplied by the caller via PlayerBattleEntry.
std::filesystem::path BuildHeroContent(const std::string& folder) {
    const std::filesystem::path root = std::filesystem::path("saves") / folder;
    std::filesystem::create_directories(root);

    WriteTextFile(root / "regions.json",
        R"({"regions":[{"id":"ashvale_heartland","name":"R","unlocked":true,"nodes":[{"location_id":"l","x":0,"y":0,"discovered":true,"travel_available":true}],"links":[]}]})");
    WriteTextFile(root / "locations.json",
        R"({"locations":[{"id":"l","name":"L","type":"combat","allows_sleep":false,"overworld_destination":true,"battle_scenario_id":"battle_test"}]})");
    WriteTextFile(root / "location_scenes.json", R"({"location_scenes":[]})");
    WriteTextFile(root / "units.json",
        R"({"units":[
            {"id":"hero_player","name":"Wanderer","category":"leader","is_player_character":true,
             "attack":6,"defense":5,"magic":4,"resistance":3,
             "min_damage":6,"max_damage":8,"max_hp":36,"max_mp":18,"agility":8,"life":1,
             "position":"leader","range":"long_ranged"},
            {"id":"enemy_dummy","name":"Dummy","category":"generic","is_player_character":false,
             "attack":1,"defense":1,"magic":1,"resistance":1,
             "min_damage":1,"max_damage":1,"max_hp":1,"max_mp":0,"agility":1,"life":1,
             "position":"front","range":"melee"}
        ]})");
    WriteTextFile(root / "battle_scenarios.json",
        R"({"battle_scenarios":[{"id":"battle_test","name":"T","seed":7,
            "allies":[{"unit_id":"hero_player"}],
            "enemies":[{"unit_id":"enemy_dummy"}]}]})");
    WriteTextFile(root / "enemy_groups.json", R"({"enemy_groups":[]})");
    WriteTextFile(root / "quests.json", R"({"quests":[]})");
    WriteTextFile(root / "location_services.json", R"({"location_services":[]})");

    return root;
}

const gameplay::battle::BattleUnit* FindHeroBattleUnit(
    const std::vector<gameplay::battle::BattleUnit>& units,
    const std::string& heroId)
{
    for (const auto& u : units) {
        if (u.id == heroId) return &u;
    }
    return nullptr;
}

gameplay::battle::PlayerBattleEntry MakePlayerEntry(
    const std::string& unitId,
    int attackBonus = 0,
    int defenseBonus = 0,
    int magicBonus = 0,
    int resistanceBonus = 0)
{
    gameplay::battle::PlayerBattleEntry e;
    e.activeSlotIndex = 0;
    e.stackId = "stk_1";
    e.unitId  = unitId;
    e.quantity = 1;
    e.artifactAttackBonus     = attackBonus;
    e.artifactDefenseBonus    = defenseBonus;
    e.artifactMagicBonus      = magicBonus;
    e.artifactResistanceBonus = resistanceBonus;
    return e;
}

} // namespace

TEST_CASE("BattleArtifactStat - zero bonuses produce baseline authored stats") {
    const auto root = BuildHeroContent("battle_artifact_baseline");
    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));

    const auto entry = MakePlayerEntry("hero_player");
    const auto battle = gameplay::battle::BattleFactory::CreateFromScenario(
        content, "battle_test", { entry });
    REQUIRE(battle.has_value());
    const auto* hero = FindHeroBattleUnit(battle->Units(), "hero_player");
    REQUIRE(hero != nullptr);
    REQUIRE(hero->stats.attack     == 6);
    REQUIRE(hero->stats.defense    == 5);
    REQUIRE(hero->stats.magic      == 4);
    REQUIRE(hero->stats.resistance == 3);

    std::filesystem::remove_all(root);
}

TEST_CASE("BattleArtifactStat - PlayerBattleEntry bonuses add to per-battle stats") {
    const auto root = BuildHeroContent("battle_artifact_bonuses");
    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));

    // +2 Attack, +1 Defense, +3 Magic, -1 Resistance (negative is legal — the
    // stat is signed; the validation layer only forbids unsupported effect
    // types, not negative amounts).
    const auto entry = MakePlayerEntry("hero_player", 2, 1, 3, -1);
    const auto battle = gameplay::battle::BattleFactory::CreateFromScenario(
        content, "battle_test", { entry });
    REQUIRE(battle.has_value());
    const auto* hero = FindHeroBattleUnit(battle->Units(), "hero_player");
    REQUIRE(hero != nullptr);
    REQUIRE(hero->stats.attack     == 6 + 2);
    REQUIRE(hero->stats.defense    == 5 + 1);
    REQUIRE(hero->stats.magic      == 4 + 3);
    REQUIRE(hero->stats.resistance == 3 - 1);

    std::filesystem::remove_all(root);
}

TEST_CASE("BattleArtifactStat - equipped artifact flows end-to-end through GameSession into per-battle stats") {
    const auto root = BuildHeroContent("battle_artifact_e2e");
    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));

    // Build a session, recruit the hero into the active party, register an
    // artifact catalog, seed an artifact via authored event, equip it, and
    // construct a battle through the same App-side pipeline.
    gameplay::GameSession session;
    session.SetLeaderCapableUnitIds({"hero_player"});
    REQUIRE(session.AddOwnedUnit("hero_player", 1));
    REQUIRE(session.TryAddUnitToActiveParty("hero_player"));

    data::ArtifactDefinition iron;
    iron.id = "artifact_iron_sword";
    iron.name = "Iron Sword";
    iron.allowedSlots = { data::ArtifactSlotKind::Attack };
    iron.statBonuses  = { { data::ArtifactStatBonusStat::Attack, 4 } };
    session.SetArtifactCatalog({ iron });

    gameplay::events::EventDefinition seed;
    seed.id = "evt_seed";
    seed.trigger.type = gameplay::events::EventTriggerType::StartOfDay;
    seed.repeat.mode = "once";
    {
        gameplay::events::EventAction act;
        act.type = "giveArtifact";
        act.args = nlohmann::json{
            {"type", "giveArtifact"},
            {"artifactId", "artifact_iron_sword"},
            {"amount", 1}
        };
        seed.actions.push_back(act);
    }
    session.InitializeEventDefinitions({seed});
    static_cast<void>(session.NotifyStartOfDay());

    REQUIRE(session.TryEquipArtifact(
        "hero_player", gameplay::ArtifactEquipSlot::Attack, "artifact_iron_sword").success);

    // Mirror App's BattleFactory call site: build ActiveBattleStackEntries,
    // translate to PlayerBattleEntry (copying the new bonus fields), and
    // hand off to the factory.
    const auto activeEntries = session.BuildActiveBattleStackEntries();
    REQUIRE(activeEntries.size() == 1);
    REQUIRE(activeEntries[0].artifactAttackBonus == 4);

    std::vector<gameplay::battle::PlayerBattleEntry> playerEntries;
    for (const auto& e : activeEntries) {
        gameplay::battle::PlayerBattleEntry pe;
        pe.activeSlotIndex = e.activeSlotIndex;
        pe.stackId = e.stackId;
        pe.unitId  = e.unitId;
        pe.quantity = e.quantity;
        pe.artifactAttackBonus     = e.artifactAttackBonus;
        pe.artifactDefenseBonus    = e.artifactDefenseBonus;
        pe.artifactMagicBonus      = e.artifactMagicBonus;
        pe.artifactResistanceBonus = e.artifactResistanceBonus;
        playerEntries.push_back(pe);
    }

    const auto battle = gameplay::battle::BattleFactory::CreateFromScenario(
        content, "battle_test", playerEntries);
    REQUIRE(battle.has_value());
    const auto* hero = FindHeroBattleUnit(battle->Units(), "hero_player");
    REQUIRE(hero != nullptr);
    REQUIRE(hero->stats.attack == 6 + 4); // baseline + statBonus

    std::filesystem::remove_all(root);
}

TEST_CASE("BattleArtifactStat - enemy units never receive bonuses regardless of session equipment") {
    const auto root = BuildHeroContent("battle_artifact_enemies_unaffected");
    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));

    // Bonuses are carried on PlayerBattleEntry only. Enemies built from the
    // scenario's enemies[] list go through the no-bonus default path.
    const auto entry = MakePlayerEntry("hero_player", 99, 99, 99, 99);
    const auto battle = gameplay::battle::BattleFactory::CreateFromScenario(
        content, "battle_test", { entry });
    REQUIRE(battle.has_value());

    const auto* dummy = FindHeroBattleUnit(battle->Units(), "enemy_dummy");
    REQUIRE(dummy != nullptr);
    REQUIRE(dummy->stats.attack     == 1);
    REQUIRE(dummy->stats.defense    == 1);
    REQUIRE(dummy->stats.magic      == 1);
    REQUIRE(dummy->stats.resistance == 1);

    std::filesystem::remove_all(root);
}

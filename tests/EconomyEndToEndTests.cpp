#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "core/GameClock.h"
#include "core/SaveGame.h"
#include "data/ContentRepository.h"
#include "data/definitions/LocationServiceDefinition.h"
#include "gameplay/EnemyTeamState.h"
#include "gameplay/GameSession.h"
#include "gameplay/ResourceState.h"
#include "gameplay/economy/TraderOwnershipRules.h"

// End-to-end proof that the M17 owned-service economy works together when driven
// from authored content: a mine pays its authored output (boosted strongest-only
// by a stack-backed stationed passive) at the day boundary, ownership gates hold,
// and trader ownership tiers resolve per type with an authored Trading Post
// matrix. Uses a self-contained authored slice (temp content dir) rather than
// expanding shipped content.

using data::LocationServiceKind;
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

// Writes the minimal authored economy slice: a stone mine (Stone + Gold output),
// a market, and a trading post (each in its own location/scene/zone), a generic
// unit carrying a +1 Stone mine-production passive, and trader curves (a Trading
// Post tier-1 barter override and a Market tier-1 price factor).
void WriteEconomyContent(const std::filesystem::path& root) {
    std::filesystem::create_directories(root);

    WriteTextFile(root / "regions.json", R"({"schemaVersion":1,"kind":"RegionCollection","id":"regions","regions":[{"id":"ashvale_heartland","name":"Ashvale Heartland","unlocked":true,"nodes":[{"location_id":"mine_loc","x":0,"y":0,"discovered":true,"travel_available":true},{"location_id":"market_loc","x":1,"y":0,"discovered":true,"travel_available":true},{"location_id":"tp_loc","x":2,"y":0,"discovered":true,"travel_available":true}],"links":[]}]})");

    WriteTextFile(root / "locations.json", R"({"schemaVersion":1,"kind":"LocationCollection","id":"locations","locations":[{"id":"mine_loc","name":"Stone Mine","type":"recruit","allows_sleep":false,"overworld_destination":true,"scene_id":"mine_scene"},{"id":"market_loc","name":"Market","type":"recruit","allows_sleep":false,"overworld_destination":true,"scene_id":"market_scene"},{"id":"tp_loc","name":"Trading Post","type":"recruit","allows_sleep":false,"overworld_destination":true,"scene_id":"tp_scene"}]})");

    WriteTextFile(root / "location_scenes.json", R"({"schemaVersion":1,"kind":"LocationSceneCollection","id":"location_scenes","location_scenes":[{"id":"mine_scene","spawn":{"x":0,"y":0,"width":1,"height":1},"blocking_rects":[],"zones":[{"id":"mine_face","type":"recruit","area":{"x":0,"y":0,"width":1,"height":1},"prompt_text":"","result_text":"","failure_text":"","time_cost_minutes":0,"gold_cost":0,"recruit_count":0,"dialogue_choice_time_cost_minutes":1,"dialogue_choices":[]}]},{"id":"market_scene","spawn":{"x":0,"y":0,"width":1,"height":1},"blocking_rects":[],"zones":[{"id":"stall","type":"recruit","area":{"x":0,"y":0,"width":1,"height":1},"prompt_text":"","result_text":"","failure_text":"","time_cost_minutes":0,"gold_cost":0,"recruit_count":0,"dialogue_choice_time_cost_minutes":1,"dialogue_choices":[]}]},{"id":"tp_scene","spawn":{"x":0,"y":0,"width":1,"height":1},"blocking_rects":[],"zones":[{"id":"counter","type":"recruit","area":{"x":0,"y":0,"width":1,"height":1},"prompt_text":"","result_text":"","failure_text":"","time_cost_minutes":0,"gold_cost":0,"recruit_count":0,"dialogue_choice_time_cost_minutes":1,"dialogue_choices":[]}]}]})");

    WriteTextFile(root / "units.json", R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[{"id":"hero","name":"Hero","category":"hero","is_player_character":true,"attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":1,"life":1,"position":"front","range":"melee"},{"id":"kobold","name":"Kobold","category":"generic","is_player_character":false,"attack":1,"defense":1,"magic":1,"resistance":1,"min_damage":1,"max_damage":1,"max_hp":10,"max_mp":0,"agility":1,"life":1,"position":"front","range":"melee","mine_production_passive":{"target":"mine","resource":"Stone","amount":1}}]})");

    WriteTextFile(root / "battle_scenarios.json", R"({"schemaVersion":1,"kind":"BattleScenarioCollection","id":"battle_scenarios","battle_scenarios":[]})");
    WriteTextFile(root / "enemy_groups.json", R"({"schemaVersion":1,"kind":"EnemyGroupCollection","id":"enemy_groups","enemy_groups":[]})");
    WriteTextFile(root / "quests.json", R"({"schemaVersion":1,"kind":"QuestCollection","id":"quests","quests":[]})");

    WriteTextFile(root / "location_services.json", R"({"schemaVersion":1,"kind":"LocationServiceCollection","id":"location_services","location_services":[{"id":"stone_mine_svc","location_id":"mine_loc","zone_id":"mine_face","kind":"mine","mine_outputs":[{"resource":"Stone","amount":2},{"resource":"Gold","amount":1000}]},{"id":"market_svc","location_id":"market_loc","zone_id":"stall","kind":"market"},{"id":"tp_svc","location_id":"tp_loc","zone_id":"counter","kind":"trading_post"}]})");

    WriteTextFile(root / "trader_curves.json", R"({"schemaVersion":1,"kind":"TraderCurveCollection","id":"trader_curves","trader_curves":[{"type":"trading_post","tiers":[{"tier":1,"exchange_matrix":[{"from":"Wood","to":"Stone","cost":8}]}]},{"type":"market","tiers":[{"tier":1,"price_factor":95}]}]})");
}

// Minimal canonical save: hero in active slot, kobold stack in reserve (so the
// mine's stationed ref is stack-backed). Caller fills ownedServices.
core::SaveData MakeEconomySave() {
    core::SaveData s;
    s.schemaVersion = 5;
    s.day = 1;
    s.minutesIntoSliceDay = 0;
    s.gold = 2500;
    s.mode = "region_mode";
    s.regionId = "ashvale_heartland";
    s.destinationId = "mine_loc";
    s.hasCanonicalRoster = true;
    s.rosterStacks = {
        core::RosterStackSaveState{"stk_1", "hero", 1},
        core::RosterStackSaveState{"stk_2", "kobold", 1}
    };
    s.activeSlotStackIds = {"stk_1", "", "", "", ""};
    s.reserveSlotStackIds = {"stk_2", "", "", "", "", "", "", ""};
    s.nextStackIdCounter = 3;
    return s;
}

const data::TraderOwnershipCurve* FindCurve(
    const std::vector<data::TraderOwnershipCurve>& curves, LocationServiceKind kind) {
    for (const auto& c : curves) {
        if (c.kind == kind) {
            return &c;
        }
    }
    return nullptr;
}

} // namespace

TEST_CASE("Economy end-to-end: authored slice loads, validates, and pays a stationed mine with trader tiers") {
    const std::filesystem::path root = "saves/economy_e2e_main";
    WriteEconomyContent(root);

    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));
    REQUIRE_FALSE(HasErrorMessage(content.ValidationMessages()));

    gameplay::GameSession session;
    session.SetUnitCatalog(content.Units());
    session.SetLocationServiceCatalog(content.LocationServices());

    auto save = MakeEconomySave();
    save.ownedServices = {
        core::OwnedServiceSaveState{"stone_mine_svc", "Green", false, false,
            {core::StationedUnitSaveState{"kobold", "stk_2"}}},  // +1 Stone passive
        core::OwnedServiceSaveState{"market_svc", "Green", false, false, {}},
        core::OwnedServiceSaveState{"tp_svc", "Green", false, false, {}}
    };
    session.ApplySaveData(save);

    // Trader tiers resolve per type via the service-specific benefit gate.
    REQUIRE(session.OwnedTraderServiceTierForService("market_svc") == 1);
    REQUIRE(session.OwnedTraderServiceTierForService("tp_svc") == 1);
    REQUIRE(session.OwnedTraderServiceTierForService("unknown_svc") == 0);

    // Day boundary: mine pays authored Stone (2) + strongest stationed passive
    // (+1) into the resource pool, and authored Gold (1000) into the gold store.
    const int goldBefore = session.Snapshot().gold;
    REQUIRE(session.ResourceCount(ResourceType::Stone) == 0);

    session.AddMinutes(kOneDay);

    REQUIRE(session.ResourceCount(ResourceType::Stone) == 3);
    REQUIRE(session.Snapshot().gold == goldBefore + 1000);
    REQUIRE(session.ResourceCount(ResourceType::Gold) == goldBefore + 1000);

    // Authored Trading Post tier-1 matrix resolves from loaded curves; absent
    // curve resolves to the documented default barter rate.
    const auto* tpCurve = FindCurve(content.TraderCurves(), LocationServiceKind::TradingPost);
    REQUIRE(tpCurve != nullptr);
    const auto authored = gameplay::economy::ResolveTradingPostBarter(tpCurve, 1);
    const auto authoredCost = std::ranges::find_if(authored, [](const auto& e) {
        return e.from == ResourceType::Wood && e.to == ResourceType::Stone;
    });
    REQUIRE(authoredCost != authored.end());
    REQUIRE(authoredCost->cost == 8);  // authored override

    const auto fallback = gameplay::economy::ResolveTradingPostBarter(nullptr, 1);
    const auto defaultCost = std::ranges::find_if(fallback, [](const auto& e) {
        return e.from == ResourceType::Wood && e.to == ResourceType::Stone;
    });
    REQUIRE(defaultCost != fallback.end());
    REQUIRE(defaultCost->cost == 10);  // documented default

    std::filesystem::remove_all(root);
}

TEST_CASE("Economy end-to-end: a locked owned mine pays nothing") {
    const std::filesystem::path root = "saves/economy_e2e_locked";
    WriteEconomyContent(root);

    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));

    gameplay::GameSession session;
    session.SetUnitCatalog(content.Units());
    session.SetLocationServiceCatalog(content.LocationServices());

    auto save = MakeEconomySave();
    save.ownedServices = {
        core::OwnedServiceSaveState{"stone_mine_svc", "Green", /*locked=*/true, false, {}}
    };
    session.ApplySaveData(save);

    const int goldBefore = session.Snapshot().gold;
    session.AddMinutes(kOneDay);

    REQUIRE(session.ResourceCount(ResourceType::Stone) == 0);
    REQUIRE(session.Snapshot().gold == goldBefore);

    std::filesystem::remove_all(root);
}

TEST_CASE("Economy end-to-end: a hostile-occupied owned mine pays nothing") {
    const std::filesystem::path root = "saves/economy_e2e_occupied";
    WriteEconomyContent(root);

    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));

    gameplay::GameSession session;
    session.SetUnitCatalog(content.Units());
    session.SetLocationServiceCatalog(content.LocationServices());

    auto save = MakeEconomySave();
    save.ownedServices = {
        core::OwnedServiceSaveState{"stone_mine_svc", "Green", false, false, {}}
    };
    session.ApplySaveData(save);

    gameplay::EnemyTeamState enemy;
    enemy.teamColor = "Red";
    enemy.nodeId = "mine_loc";  // occupies the mine's node
    enemy.active = true;
    session.SetEnemyTeams({enemy});

    const int goldBefore = session.Snapshot().gold;
    session.AddMinutes(kOneDay);

    REQUIRE(session.ResourceCount(ResourceType::Stone) == 0);
    REQUIRE(session.Snapshot().gold == goldBefore);

    std::filesystem::remove_all(root);
}

TEST_CASE("Economy end-to-end: a trader service the player does not own yields tier 0") {
    const std::filesystem::path root = "saves/economy_e2e_unowned_trader";
    WriteEconomyContent(root);

    data::ContentRepository content;
    REQUIRE(content.LoadFromDirectory(root));

    gameplay::GameSession session;
    session.SetUnitCatalog(content.Units());
    session.SetLocationServiceCatalog(content.LocationServices());

    auto save = MakeEconomySave();
    // Player owns one Market, but the USED market is enemy-owned.
    save.ownedServices = {
        core::OwnedServiceSaveState{"market_svc", "Red", false, false, {}}
    };
    session.ApplySaveData(save);

    REQUIRE(session.OwnedTraderServiceTierForService("market_svc") == 0);

    std::filesystem::remove_all(root);
}

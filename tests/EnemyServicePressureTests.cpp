#include <catch2/catch_test_macros.hpp>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <string>
#include <vector>

#include "core/SaveGame.h"
#include "data/definitions/EnemyGroupDefinition.h"
#include "data/definitions/LocationServiceDefinition.h"
#include "data/definitions/RegionDefinition.h"
#include "data/definitions/UnitDefinition.h"
#include "gameplay/GameSession.h"
#include "gameplay/events/EventDefinition.h"

// M30 enemy-side service pressure: ProcessEnemyPhase lets a hostile team attack
// player-owned service nodes (same node or adjacent, deterministic first target
// by node id) through the service-defense resolver. Attacks are constrained to
// the current region, never the protected arrival node, and respect patrol
// radius and alliance boundaries. A player-occupied target yields a pending
// battle result for the App with no session mutation.

namespace {

using data::LocationServiceKind;

data::RegionNodeDefinition Node(const std::string& id) {
    data::RegionNodeDefinition n;
    n.locationId = id;
    return n;
}

data::UnitDefinition MakeUnit(const std::string& id, data::UnitDefinitionCategory category,
    int attack, int defense, int maxHp, bool isPlayerCharacter = false) {
    data::UnitDefinition u;
    u.id = id;
    u.name = id;
    u.category = category;
    u.stats.attack = attack;
    u.stats.defense = defense;
    u.stats.maxHp = maxHp;
    u.stats.agility = 5;
    u.isPlayerCharacter = isPlayerCharacter;
    return u;
}

data::LocationServiceDefinition MakeService(const std::string& id, const std::string& locationId,
    LocationServiceKind kind) {
    data::LocationServiceDefinition svc;
    svc.id = id;
    svc.locationId = locationId;
    svc.kind = kind;
    return svc;
}

std::vector<data::RegionLinkDefinition> HeartlandLinks() {
    return {
        { "home_base", "town_center" },
        { "town_center", "iron_mine" },
        { "town_center", "river_depot" },
    };
}

// Player at home_base; mine at iron_mine with 3x militia stationed (power 60);
// optional second service via tweaks. Red hostile (eg_raiders, 60) placement
// set per test; Yellow allied to Green.
struct Fixture {
    core::SaveData save;
    gameplay::EnemyTeamState red;
    gameplay::EnemyTeamState yellow;
    std::vector<data::LocationServiceDefinition> services;

    Fixture() {
        save.schemaVersion = 5;
        save.day = 1;
        save.mode = "overworld_mode";
        save.regionId = "ashvale_heartland";
        save.destinationId = "home_base";
        save.hasCanonicalRoster = true;
        save.rosterStacks = {
            core::RosterStackSaveState{"stk_pc", "pc_hero", 1},
            core::RosterStackSaveState{"stk_mine", "militia", 3},
        };
        save.activeSlotStackIds = {"stk_pc", "", "", "", ""};
        save.reserveSlotStackIds = {"", "", "", "", "", "", "", ""};
        save.nextStackIdCounter = 5;
        save.ownedServices = {
            core::OwnedServiceSaveState{"steel_mine_svc", "Green", false, false,
                {core::StationedUnitSaveState{"militia", "stk_mine"}}, {}},
        };

        red.teamColor = "Red";
        red.nodeId = "town_center";
        red.enemyGroupId = "eg_raiders";
        yellow.teamColor = "Yellow";
        yellow.nodeId = "river_depot";
        yellow.alliances = {"Green"};
        yellow.enemyGroupId = "eg_raiders";

        services = {
            MakeService("steel_mine_svc", "iron_mine", LocationServiceKind::Mine),
        };
    }

    gameplay::GameSession Build() const {
        gameplay::GameSession session;
        session.SetPlayerColor("Green");
        session.SetUnitCatalog({
            MakeUnit("pc_hero", data::UnitDefinitionCategory::Leader, 20, 20, 20, true),
            MakeUnit("militia", data::UnitDefinitionCategory::Generic, 5, 5, 10),
            MakeUnit("raider", data::UnitDefinitionCategory::Generic, 10, 10, 10),
        });
        session.SetLeaderCapableUnitIds({"pc_hero"});
        session.SetLocationServiceCatalog(services);
        session.SetEnemyGroupCatalog({
            data::EnemyGroupDefinition{"eg_raiders", "Raiders", {"raider", "raider"}},
            data::EnemyGroupDefinition{"eg_strong", "Warband", {"raider", "raider", "raider"}},
        });

        data::RegionDefinition heartland;
        heartland.id = "ashvale_heartland";
        heartland.arrivalNodeId = "home_base";
        heartland.nodes = {
            Node("home_base"), Node("town_center"), Node("iron_mine"), Node("river_depot"),
        };
        session.SetRegionCatalog({heartland});

        session.SetEnemyTeams({red, yellow});
        session.ApplySaveData(save);
        return session;
    }
};

const gameplay::EnemyTeamState* TeamByColor(const gameplay::GameSession& session,
    const std::string& color) {
    for (const auto& team : session.EnemyTeams()) {
        if (team.teamColor == color) {
            return &team;
        }
    }
    return nullptr;
}

const gameplay::EnemyTeamActionResult* ResultFor(
    const std::vector<gameplay::EnemyTeamActionResult>& results, const std::string& color) {
    for (const auto& r : results) {
        if (r.teamColor == color) {
            return &r;
        }
    }
    return nullptr;
}

} // namespace

TEST_CASE("EnemyPressure - adjacent owned mine is attacked; equal defenders hold") {
    Fixture f;  // garrison 60 vs eg_raiders 60: defenders hold
    auto session = f.Build();

    const auto results = session.ProcessEnemyPhase(HeartlandLinks());

    const auto* redResult = ResultFor(results, "Red");
    REQUIRE(redResult != nullptr);
    REQUIRE(redResult->actionType == "service_attack");
    REQUIRE(redResult->nodeId == "iron_mine");
    REQUIRE_FALSE(redResult->summaryText.empty());

    REQUIRE_FALSE(TeamByColor(session, "Red")->active);  // repelled attacker defeated
    REQUIRE(session.FindOwnedService("steel_mine_svc")->ownerTeamColor == "Green");
    REQUIRE(session.FindOwnedService("steel_mine_svc")->stationedUnits.size() == 1);
}

TEST_CASE("EnemyPressure - stronger attacker captures the adjacent mine and occupies it") {
    Fixture f;
    f.red.enemyGroupId = "eg_strong";  // 90 vs 60: attacker wins
    auto session = f.Build();

    const auto results = session.ProcessEnemyPhase(HeartlandLinks());

    const auto* redResult = ResultFor(results, "Red");
    REQUIRE(redResult != nullptr);
    REQUIRE(redResult->actionType == "service_attack");
    REQUIRE(session.FindOwnedService("steel_mine_svc")->ownerTeamColor == "Red");
    REQUIRE(session.FindOwnedService("steel_mine_svc")->stationedUnits.empty());
    REQUIRE(session.FindRosterStackById("stk_mine") == nullptr);
    REQUIRE(TeamByColor(session, "Red")->nodeId == "iron_mine");
}

TEST_CASE("EnemyPressure - a team standing on the service node attacks it directly") {
    Fixture f;
    f.red.nodeId = "iron_mine";
    f.red.enemyGroupId = "eg_strong";
    auto session = f.Build();

    const auto results = session.ProcessEnemyPhase(HeartlandLinks());

    const auto* redResult = ResultFor(results, "Red");
    REQUIRE(redResult != nullptr);
    REQUIRE(redResult->actionType == "service_attack");
    REQUIRE(redResult->nodeId == "iron_mine");
    REQUIRE(session.FindOwnedService("steel_mine_svc")->ownerTeamColor == "Red");
}

TEST_CASE("EnemyPressure - the arrival node is protected from service attacks") {
    Fixture f;
    // The only owned service sits on the protected arrival node.
    f.services = { MakeService("home_mine_svc", "home_base", LocationServiceKind::Mine) };
    f.save.ownedServices = {
        core::OwnedServiceSaveState{"home_mine_svc", "Green", false, false,
            {core::StationedUnitSaveState{"militia", "stk_mine"}}, {}},
    };
    f.save.destinationId = "town_center";  // player elsewhere; Red adjacent to home_base
    f.red.nodeId = "town_center";          // hmm: same node as player is fine for idle check
    auto session = f.Build();

    const auto results = session.ProcessEnemyPhase(HeartlandLinks());

    const auto* redResult = ResultFor(results, "Red");
    REQUIRE(redResult != nullptr);
    REQUIRE(redResult->actionType == "idle");  // no patrol, no legal target
    REQUIRE(session.FindOwnedService("home_mine_svc")->ownerTeamColor == "Green");
}

TEST_CASE("EnemyPressure - patrol radius gates which nodes can be attacked") {
    Fixture f;
    f.red.enemyGroupId = "eg_strong";
    f.red.patrol.enabled = true;
    f.red.patrol.centerNodeId = "home_base";
    f.red.patrol.radius = 0;  // iron_mine is 2 hops from center: out of range
    auto session = f.Build();

    const auto outOfRange = session.ProcessEnemyPhase(HeartlandLinks());
    const auto* redResult = ResultFor(outOfRange, "Red");
    REQUIRE(redResult != nullptr);
    REQUIRE(redResult->actionType != "service_attack");
    REQUIRE(session.FindOwnedService("steel_mine_svc")->ownerTeamColor == "Green");

    // Same layout with the mine inside the patrol radius: the attack happens.
    Fixture g;
    g.red.enemyGroupId = "eg_strong";
    g.red.patrol.enabled = true;
    g.red.patrol.centerNodeId = "town_center";
    g.red.patrol.radius = 1;  // iron_mine is 1 hop from center
    auto inRangeSession = g.Build();

    const auto inRange = inRangeSession.ProcessEnemyPhase(HeartlandLinks());
    REQUIRE(ResultFor(inRange, "Red")->actionType == "service_attack");
    REQUIRE(inRangeSession.FindOwnedService("steel_mine_svc")->ownerTeamColor == "Red");
}

TEST_CASE("EnemyPressure - allied teams never attack player services") {
    Fixture f;
    f.yellow.nodeId = "town_center";  // adjacent to the player mine
    f.red.active = false;
    auto session = f.Build();

    const auto results = session.ProcessEnemyPhase(HeartlandLinks());

    const auto* yellowResult = ResultFor(results, "Yellow");
    REQUIRE(yellowResult != nullptr);
    REQUIRE(yellowResult->actionType == "idle");
    REQUIRE(session.FindOwnedService("steel_mine_svc")->ownerTeamColor == "Green");
}

TEST_CASE("EnemyPressure - attacking the player's node defers to the battle surface") {
    Fixture f;
    f.save.destinationId = "iron_mine";  // player party stands on the mine node
    auto session = f.Build();

    const auto results = session.ProcessEnemyPhase(HeartlandLinks());

    const auto* redResult = ResultFor(results, "Red");
    REQUIRE(redResult != nullptr);
    REQUIRE(redResult->actionType == "service_attack_pending_battle");
    REQUIRE(redResult->nodeId == "iron_mine");
    // Nothing mutated: the App owns the interactive resolution.
    REQUIRE(TeamByColor(session, "Red")->active);
    REQUIRE(TeamByColor(session, "Red")->nodeId == "town_center");
    REQUIRE(session.FindOwnedService("steel_mine_svc")->ownerTeamColor == "Green");
    REQUIRE(session.FindOwnedService("steel_mine_svc")->stationedUnits.size() == 1);
    REQUIRE(session.ServiceEventLog().empty());
}

TEST_CASE("EnemyPressure - teams outside the current region do not attack") {
    Fixture f;
    f.red.nodeId = "vale_market";  // not a heartland node
    auto session = f.Build();

    const auto results = session.ProcessEnemyPhase(HeartlandLinks());

    REQUIRE(ResultFor(results, "Red")->actionType == "idle");
    REQUIRE(session.FindOwnedService("steel_mine_svc")->ownerTeamColor == "Green");
}

TEST_CASE("EnemyPressure - spawnTeam event can author the team's enemy group") {
    gameplay::GameSession session;

    gameplay::events::EventDefinition def;
    def.id = "evt_raiders";
    def.trigger.type = gameplay::events::EventTriggerType::RegionNodeEntry;
    def.trigger.targetId = "node_gate";
    gameplay::events::EventAction action;
    action.type = "spawnTeam";
    action.args = nlohmann::json{
        {"type", "spawnTeam"}, {"teamColor", "Red"},
        {"nodeId", "deep_mine"}, {"enemyGroupId", "eg_raiders"}};
    def.actions = {action};
    def.repeat.mode = "once";
    session.InitializeEventDefinitions({def});

    const auto results = session.NotifyRegionNodeEntry("node_gate");
    REQUIRE(results.size() == 1);
    REQUIRE(results[0].success);

    REQUIRE(session.EnemyTeams().size() == 1);
    REQUIRE(session.EnemyTeams().front().enemyGroupId == "eg_raiders");
    REQUIRE(session.EnemyTeams().front().nodeId == "deep_mine");
}

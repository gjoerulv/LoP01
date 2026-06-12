#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <string>
#include <vector>

#include "core/GameClock.h"
#include "core/SaveGame.h"
#include "data/ContentValidator.h"
#include "data/definitions/LocationServiceDefinition.h"
#include "data/definitions/UnitDefinition.h"
#include "gameplay/GameSession.h"
#include "gameplay/ResourceState.h"

// M30 minimal service destruction/restoration (docs/core_loop_rules.md §20):
// destruction is opt-in via the authored `destroyable` flag, requires standing
// on an unoccupied node, costs 1000 Energy + 1 hour, and is blocked by placed
// units. Restoration spends the authored resource cost, queues, and completes
// at the next day start (before that day's mine payout); destroying again
// before completion cancels the queue.

namespace {

using data::LocationServiceKind;

data::UnitDefinition MakeUnit(const std::string& id, data::UnitDefinitionCategory category,
    bool isPlayerCharacter = false) {
    data::UnitDefinition u;
    u.id = id;
    u.name = id;
    u.category = category;
    u.stats.attack = 5;
    u.stats.defense = 5;
    u.stats.maxHp = 10;
    u.stats.agility = 5;
    u.isPlayerCharacter = isPlayerCharacter;
    return u;
}

data::LocationServiceDefinition MakeCopperMine() {
    data::LocationServiceDefinition svc;
    svc.id = "copper_mine_svc";
    svc.locationId = "copper_mine";
    svc.kind = LocationServiceKind::Mine;
    svc.mineOutputs = {data::MineOutputDefinition{"Stone", 2}};
    svc.destroyable = true;
    svc.restoreCost = {data::MineOutputDefinition{"Wood", 2},
                       data::MineOutputDefinition{"Stone", 1}};
    return svc;
}

// Player standing on copper_mine (owned, intact, destroyable), with Wood/Stone
// to afford one restoration. pc agility 5 -> daily Energy 1500.
core::SaveData MakeBaseSave() {
    core::SaveData s;
    s.schemaVersion = 5;
    s.day = 1;
    s.mode = "overworld_mode";
    s.regionId = "ashvale_heartland";
    s.destinationId = "copper_mine";
    s.hasCanonicalRoster = true;
    s.rosterStacks = {
        core::RosterStackSaveState{"stk_pc", "pc_hero", 1},
        core::RosterStackSaveState{"stk_mil", "militia", 2},
    };
    s.activeSlotStackIds = {"stk_pc", "", "", "", ""};
    s.reserveSlotStackIds = {"stk_mil", "", "", "", "", "", "", ""};
    s.nextStackIdCounter = 5;
    s.resources = {core::ResourceSaveState{"Wood", 5}, core::ResourceSaveState{"Stone", 3}};
    s.ownedServices = {
        core::OwnedServiceSaveState{"copper_mine_svc", "Green", false, false, {}, {}},
    };
    return s;
}

gameplay::GameSession Load(const core::SaveData& save) {
    gameplay::GameSession session;
    session.SetPlayerColor("Green");
    session.SetUnitCatalog({
        MakeUnit("pc_hero", data::UnitDefinitionCategory::Leader, true),
        MakeUnit("militia", data::UnitDefinitionCategory::Generic),
    });
    session.SetLeaderCapableUnitIds({"pc_hero"});
    session.SetLocationServiceCatalog({MakeCopperMine()});
    session.ApplySaveData(save);
    session.ApplyDailyStartingEnergy();
    return session;
}

bool LogContains(const gameplay::GameSession& session, const std::string& fragment) {
    for (const auto& entry : session.ServiceEventLog()) {
        if (entry.text.find(fragment) != std::string::npos) {
            return true;
        }
    }
    return false;
}

} // namespace

TEST_CASE("ServiceDestruction - gates: flag, presence, energy, garrison, occupation") {
    // Not destroyable.
    {
        auto session = Load(MakeBaseSave());
        session.SetLocationServiceCatalog({[] {
            auto svc = MakeCopperMine();
            svc.destroyable = false;
            svc.restoreCost.clear();
            return svc;
        }()});
        REQUIRE_FALSE(session.CanDestroyServiceAtCurrentNode("copper_mine_svc"));
    }
    // Not standing on the node.
    {
        auto save = MakeBaseSave();
        save.destinationId = "home_base";
        auto session = Load(save);
        REQUIRE_FALSE(session.CanDestroyServiceAtCurrentNode("copper_mine_svc"));
        REQUIRE(session.DestroyableServiceAtCurrentNode() == nullptr);
    }
    // Not enough Energy.
    {
        auto session = Load(MakeBaseSave());
        REQUIRE(session.TrySpendEnergy(600));  // 1500 -> 900 < 1000
        REQUIRE_FALSE(session.CanDestroyServiceAtCurrentNode("copper_mine_svc"));
    }
    // Placed units block destruction (no ref may dangle through it).
    {
        auto save = MakeBaseSave();
        save.ownedServices[0].stationedUnits = {
            core::StationedUnitSaveState{"militia", "stk_mil"}};
        save.reserveSlotStackIds[0] = "";
        auto session = Load(save);
        REQUIRE_FALSE(session.CanDestroyServiceAtCurrentNode("copper_mine_svc"));
    }
    // Hostile occupier at the node (guardians must be defeated first).
    {
        auto session = Load(MakeBaseSave());
        gameplay::EnemyTeamState red;
        red.teamColor = "Red";
        red.nodeId = "copper_mine";
        session.SetEnemyTeams({red});
        REQUIRE_FALSE(session.CanDestroyServiceAtCurrentNode("copper_mine_svc"));
    }
    // Not in Region mode.
    {
        auto session = Load(MakeBaseSave());
        session.EnterBattleMode();
        REQUIRE_FALSE(session.CanDestroyServiceAtCurrentNode("copper_mine_svc"));
    }
}

TEST_CASE("ServiceDestruction - destroying costs 1000 Energy + 1 hour and flags the service") {
    auto session = Load(MakeBaseSave());
    const int minutesBefore = session.Snapshot().minutesIntoSliceDay;
    REQUIRE(session.CurrentEnergy() == 1500);
    REQUIRE(session.DestroyableServiceAtCurrentNode() != nullptr);

    REQUIRE(session.TryDestroyServiceAtCurrentNode("copper_mine_svc"));

    REQUIRE(session.CurrentEnergy() == 500);
    REQUIRE(session.Snapshot().minutesIntoSliceDay == minutesBefore + 60);
    const auto* owned = session.FindOwnedService("copper_mine_svc");
    REQUIRE(owned->destroyed);
    REQUIRE_FALSE(owned->restorationQueued);
    REQUIRE(LogContains(session, "destroyed copper_mine_svc"));

    // Already destroyed with nothing queued: no second destruction.
    REQUIRE_FALSE(session.CanDestroyServiceAtCurrentNode("copper_mine_svc"));
    // Destroyed services are not attack targets.
    gameplay::EnemyTeamState red;
    red.teamColor = "Red";
    red.nodeId = "town_center";
    session.SetEnemyTeams({red});
    REQUIRE(session.AttackableServiceIdsAtNodeFor("Red", "copper_mine").empty());
}

TEST_CASE("ServiceDestruction - destroying an unowned destroyable service creates runtime state") {
    auto save = MakeBaseSave();
    save.ownedServices.clear();
    auto session = Load(save);

    REQUIRE(session.TryDestroyServiceAtCurrentNode("copper_mine_svc"));
    const auto* owned = session.FindOwnedService("copper_mine_svc");
    REQUIRE(owned != nullptr);
    REQUIRE(owned->destroyed);
    REQUIRE(owned->ownerTeamColor.empty());
}

TEST_CASE("ServiceDestruction - destroyed mine pays nothing until restored") {
    auto session = Load(MakeBaseSave());
    REQUIRE(session.TryDestroyServiceAtCurrentNode("copper_mine_svc"));
    const int stoneBefore = session.ResourceCount(gameplay::ResourceType::Stone);

    session.AddMinutes(core::GameClock::kMinutesPerSliceDay);  // next day, no queue
    REQUIRE(session.ResourceCount(gameplay::ResourceType::Stone) == stoneBefore);
    REQUIRE(session.FindOwnedService("copper_mine_svc")->destroyed);
}

TEST_CASE("ServiceDestruction - restoration queues, spends the cost, and completes at day start") {
    auto session = Load(MakeBaseSave());
    REQUIRE(session.TryDestroyServiceAtCurrentNode("copper_mine_svc"));

    // Queue gates: affordable cost required.
    REQUIRE(session.CanQueueServiceRestorationAtCurrentNode("copper_mine_svc"));
    REQUIRE(session.TryQueueServiceRestorationAtCurrentNode("copper_mine_svc"));
    REQUIRE(session.ResourceCount(gameplay::ResourceType::Wood) == 3);   // 5 - 2
    REQUIRE(session.ResourceCount(gameplay::ResourceType::Stone) == 2);  // 3 - 1
    REQUIRE(session.FindOwnedService("copper_mine_svc")->restorationQueued);
    REQUIRE(LogContains(session, "queued"));

    // Double-queue refused.
    REQUIRE_FALSE(session.CanQueueServiceRestorationAtCurrentNode("copper_mine_svc"));

    // Completes at the next day start, BEFORE that day's payout: the restored
    // mine pays its base output (Stone +2) the same day.
    session.AddMinutes(core::GameClock::kMinutesPerSliceDay);
    const auto* owned = session.FindOwnedService("copper_mine_svc");
    REQUIRE_FALSE(owned->destroyed);
    REQUIRE_FALSE(owned->restorationQueued);
    REQUIRE(session.ResourceCount(gameplay::ResourceType::Stone) == 4);  // 2 + 2 payout
    REQUIRE(LogContains(session, "restored"));
}

TEST_CASE("ServiceDestruction - restoration cannot be queued on an intact service or unaffordably") {
    auto session = Load(MakeBaseSave());
    REQUIRE_FALSE(session.CanQueueServiceRestorationAtCurrentNode("copper_mine_svc"));

    REQUIRE(session.TryDestroyServiceAtCurrentNode("copper_mine_svc"));
    // Drain Wood below the cost.
    REQUIRE(session.TrySpendResource(gameplay::ResourceType::Wood, 4));  // 5 -> 1 < 2
    REQUIRE_FALSE(session.CanQueueServiceRestorationAtCurrentNode("copper_mine_svc"));
    REQUIRE_FALSE(session.TryQueueServiceRestorationAtCurrentNode("copper_mine_svc"));
}

TEST_CASE("ServiceDestruction - destroying again cancels the queued restoration") {
    auto session = Load(MakeBaseSave());
    REQUIRE(session.TryDestroyServiceAtCurrentNode("copper_mine_svc"));
    REQUIRE(session.TryQueueServiceRestorationAtCurrentNode("copper_mine_svc"));

    // The cancel costs the full destruction price again; refill the Energy pool
    // (without crossing a day boundary, which would complete the restoration).
    session.ApplyDailyStartingEnergy();

    // Destroyed + queued: destruction is legal again and cancels the queue.
    REQUIRE(session.CanDestroyServiceAtCurrentNode("copper_mine_svc"));
    REQUIRE(session.TryDestroyServiceAtCurrentNode("copper_mine_svc"));
    const auto* owned = session.FindOwnedService("copper_mine_svc");
    REQUIRE(owned->destroyed);
    REQUIRE_FALSE(owned->restorationQueued);
    REQUIRE(LogContains(session, "cancelled by destruction"));

    // Next day: still destroyed (the queue was cancelled).
    session.AddMinutes(core::GameClock::kMinutesPerSliceDay);
    REQUIRE(session.FindOwnedService("copper_mine_svc")->destroyed);
}

TEST_CASE("ServiceDestruction - destroyed/queued state survives save/load") {
    auto session = Load(MakeBaseSave());
    REQUIRE(session.TryDestroyServiceAtCurrentNode("copper_mine_svc"));
    REQUIRE(session.TryQueueServiceRestorationAtCurrentNode("copper_mine_svc"));

    auto restored = Load(session.ToSaveData());
    const auto* owned = restored.FindOwnedService("copper_mine_svc");
    REQUIRE(owned->destroyed);
    REQUIRE(owned->restorationQueued);

    // The reloaded queue still completes at the next day start.
    restored.AddMinutes(core::GameClock::kMinutesPerSliceDay);
    REQUIRE_FALSE(restored.FindOwnedService("copper_mine_svc")->destroyed);
}

TEST_CASE("ServiceDestruction - validation enforces the destroyable/restore_cost contract") {
    ContentValidator v;

    auto destroyableNoCost = MakeCopperMine();
    destroyableNoCost.restoreCost.clear();
    auto msgs = v.ValidateReferences({}, {}, {}, {}, {}, {destroyableNoCost}, {});
    REQUIRE(std::ranges::any_of(msgs, [](const auto& m) {
        return m.code == "RESTORE_COST_REQUIRED_FOR_DESTROYABLE";
    }));

    auto costNotDestroyable = MakeCopperMine();
    costNotDestroyable.destroyable = false;
    msgs = v.ValidateReferences({}, {}, {}, {}, {}, {costNotDestroyable}, {});
    REQUIRE(std::ranges::any_of(msgs, [](const auto& m) {
        return m.code == "RESTORE_COST_FOR_NON_DESTROYABLE";
    }));

    auto badCost = MakeCopperMine();
    badCost.restoreCost = {data::MineOutputDefinition{"Cheese", 0}};
    msgs = v.ValidateReferences({}, {}, {}, {}, {}, {badCost}, {});
    REQUIRE(std::ranges::any_of(msgs, [](const auto& m) {
        return m.code == "RESTORE_COST_RESOURCE_INVALID";
    }));
    REQUIRE(std::ranges::any_of(msgs, [](const auto& m) {
        return m.code == "RESTORE_COST_AMOUNT_INVALID";
    }));
}

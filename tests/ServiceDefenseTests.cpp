#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <string>
#include <vector>

#include "core/SaveGame.h"
#include "data/definitions/EnemyGroupDefinition.h"
#include "data/definitions/LocationServiceDefinition.h"
#include "data/definitions/UnitDefinition.h"
#include "gameplay/GameSession.h"
#include "gameplay/economy/ServiceDefenseRules.h"

// M30 service defense / stationed-defender resolution. The at-node attack
// resolver is deterministic (ServiceDefenseRules strength comparison) for the
// absent-player case; the player-present case defers to the interactive battle
// surface via playerBattleRequired. Capture resolves placed defender stacks in
// the same mutation: generics dismissed, heroes Temporarily Unavailable, refs
// cleared (no dangling refs, no duplicates), ownership transferred immediately.

namespace {

using data::LocationServiceKind;

data::UnitDefinition MakeUnit(const std::string& id, data::UnitDefinitionCategory category,
    int attack, int defense, int maxHp, int minDamage, int maxDamage,
    bool isPlayerCharacter = false) {
    data::UnitDefinition u;
    u.id = id;
    u.name = id;
    u.category = category;
    u.stats.attack = attack;
    u.stats.defense = defense;
    u.stats.maxHp = maxHp;
    u.stats.minDamage = minDamage;
    u.stats.maxDamage = maxDamage;
    u.stats.agility = 5;
    u.isPlayerCharacter = isPlayerCharacter;
    return u;
}

// M33: outcomes are now decided by the battle-rule-aligned auto-resolve, so these
// units carry real damage ranges. Tuned so the intended decisions are decisive:
// a 3x militia garrison holds vs 2 raiders but is overrun by 3; a lone hero_b is
// overrun by 2 raiders.
std::vector<data::UnitDefinition> MakeCatalog() {
    return {
        MakeUnit("pc_hero", data::UnitDefinitionCategory::Leader, 20, 20, 24, 6, 9, /*isPC=*/true),
        MakeUnit("hero_b", data::UnitDefinitionCategory::Hero, 8, 6, 16, 3, 5),
        MakeUnit("militia", data::UnitDefinitionCategory::Generic, 6, 5, 12, 2, 4),
        MakeUnit("raider", data::UnitDefinitionCategory::Generic, 10, 6, 12, 3, 5),
    };
}

data::LocationServiceDefinition MakeService(const std::string& id, const std::string& locationId,
    LocationServiceKind kind) {
    data::LocationServiceDefinition svc;
    svc.id = id;
    svc.locationId = locationId;
    svc.kind = kind;
    return svc;
}

std::vector<data::EnemyGroupDefinition> MakeGroups() {
    return {
        // Enemy groups author unit TYPES (each a quantity-1 stack in the auto-
        // resolve). eg_raiders = 2 raiders; eg_weak = 1; eg_strong = 3.
        data::EnemyGroupDefinition{"eg_raiders", "Raiders", {"raider", "raider"}},
        data::EnemyGroupDefinition{"eg_weak", "Scout", {"raider"}},
        data::EnemyGroupDefinition{"eg_strong", "Warband", {"raider", "raider", "raider"}},
    };
}

// pc_hero active; steel_mine (iron_mine node) stationed with a 3x militia
// garrison; river_depot storage stored with 2x militia + hero_b. Player stands on
// home_base. Hold/capture is decided by the M33 auto-resolve.
core::SaveData MakeBaseSave() {
    core::SaveData s;
    s.schemaVersion = 5;
    s.day = 1;
    s.mode = "overworld_mode";
    s.regionId = "ashvale_heartland";
    s.destinationId = "home_base";
    s.hasCanonicalRoster = true;
    s.rosterStacks = {
        core::RosterStackSaveState{"stk_pc", "pc_hero", 1},
        core::RosterStackSaveState{"stk_mine", "militia", 3},
        core::RosterStackSaveState{"stk_store", "militia", 2},
        core::RosterStackSaveState{"stk_hero", "hero_b", 1},
    };
    s.activeSlotStackIds = {"stk_pc", "", "", "", ""};
    s.reserveSlotStackIds = {"", "", "", "", "", "", "", ""};
    s.nextStackIdCounter = 5;
    s.ownedServices = {
        core::OwnedServiceSaveState{"steel_mine_svc", "Green", false, false,
            {core::StationedUnitSaveState{"militia", "stk_mine"}}, {}},
        core::OwnedServiceSaveState{"river_depot_storage", "Green", false, false,
            {}, {core::StoredUnitSaveState{"militia", "stk_store"},
                 core::StoredUnitSaveState{"hero_b", "stk_hero"}}},
    };
    return s;
}

gameplay::GameSession Load(const core::SaveData& save,
    const std::string& redGroupId = "eg_raiders") {
    gameplay::GameSession session;
    session.SetPlayerColor("Green");
    session.SetUnitCatalog(MakeCatalog());
    session.SetLeaderCapableUnitIds({"pc_hero", "hero_b"});
    session.SetLocationServiceCatalog({
        MakeService("steel_mine_svc", "iron_mine", LocationServiceKind::Mine),
        MakeService("river_depot_storage", "river_depot", LocationServiceKind::Storage),
    });
    session.SetEnemyGroupCatalog(MakeGroups());

    gameplay::EnemyTeamState red;
    red.teamColor = "Red";
    red.nodeId = "town_center";
    red.enemyGroupId = redGroupId;
    gameplay::EnemyTeamState yellow;
    yellow.teamColor = "Yellow";
    yellow.nodeId = "old_inn";
    yellow.alliances = {"Green"};
    yellow.enemyGroupId = "eg_raiders";
    session.SetEnemyTeams({red, yellow});

    session.ApplySaveData(save);
    return session;
}

const gameplay::EnemyTeamState* TeamByColor(const gameplay::GameSession& session,
    const std::string& color) {
    for (const auto& team : session.EnemyTeams()) {
        if (team.teamColor == color) {
            return &team;
        }
    }
    return nullptr;
}

bool LogContains(const gameplay::GameSession& session, const std::string& fragment) {
    for (const auto& entry : session.ServiceEventLog()) {
        if (entry.text.find(fragment) != std::string::npos) {
            return true;
        }
    }
    return false;
}

// Every roster stack must be in exactly one place; every placed ref must be
// stack-backed.
void RequireNoOrphans(const gameplay::GameSession& session) {
    auto slotsContain = [](const std::vector<std::string>& slots, const std::string& id) {
        return std::find(slots.begin(), slots.end(), id) != slots.end();
    };
    for (const auto& stack : session.RosterStacks()) {
        int placements = 0;
        if (slotsContain(session.ActiveSlotStackIds(), stack.stackId)) ++placements;
        if (slotsContain(session.ReserveSlotStackIds(), stack.stackId)) ++placements;
        for (const auto& owned : session.OwnedServices()) {
            for (const auto& ref : owned.stationedUnits) if (ref.stackId == stack.stackId) ++placements;
            for (const auto& ref : owned.storedUnits) if (ref.stackId == stack.stackId) ++placements;
        }
        INFO("stack " << stack.stackId << " placements=" << placements);
        REQUIRE(placements == 1);
    }
    for (const auto& owned : session.OwnedServices()) {
        for (const auto& ref : owned.stationedUnits) {
            REQUIRE(session.FindRosterStackById(ref.stackId) != nullptr);
        }
        for (const auto& ref : owned.storedUnits) {
            REQUIRE(session.FindRosterStackById(ref.stackId) != nullptr);
        }
    }
}

} // namespace

TEST_CASE("ServiceDefenseRules - attackable kinds are mine, traders, and storage only") {
    using gameplay::economy::ServiceKindIsAttackable;
    REQUIRE(ServiceKindIsAttackable(LocationServiceKind::Mine));
    REQUIRE(ServiceKindIsAttackable(LocationServiceKind::TradingPost));
    REQUIRE(ServiceKindIsAttackable(LocationServiceKind::Market));
    REQUIRE(ServiceKindIsAttackable(LocationServiceKind::FreelancersGuild));
    REQUIRE(ServiceKindIsAttackable(LocationServiceKind::BlackMarket));
    REQUIRE(ServiceKindIsAttackable(LocationServiceKind::Storage));
    REQUIRE_FALSE(ServiceKindIsAttackable(LocationServiceKind::Rest));
    REQUIRE_FALSE(ServiceKindIsAttackable(LocationServiceKind::Shop));
    REQUIRE_FALSE(ServiceKindIsAttackable(LocationServiceKind::Recruit));
    REQUIRE_FALSE(ServiceKindIsAttackable(LocationServiceKind::Muster));
    REQUIRE_FALSE(ServiceKindIsAttackable(LocationServiceKind::Unknown));
}

TEST_CASE("ServiceDefenseRules - power formula and tie-goes-to-defender outcome") {
    data::UnitStatsDefinition stats;
    stats.attack = 5;
    stats.defense = 5;
    stats.maxHp = 10;
    REQUIRE(gameplay::economy::UnitDefensePower(stats) == 20);
    REQUIRE(gameplay::economy::StackDefensePower(stats, 3) == 60);
    REQUIRE(gameplay::economy::StackDefensePower(stats, 0) == 0);

    REQUIRE(gameplay::economy::DefendersHoldService(60, 60));   // tie -> defender
    REQUIRE(gameplay::economy::DefendersHoldService(61, 60));
    REQUIRE_FALSE(gameplay::economy::DefendersHoldService(59, 60));
    REQUIRE_FALSE(gameplay::economy::DefendersHoldService(0, 0)); // no defenders never "hold"
}

TEST_CASE("ServiceAttack - stationed defenders hold the mine and defeat the attacker") {
    auto session = Load(MakeBaseSave());
    // Auto-resolve: a 3x militia garrison holds against eg_raiders (2 raiders).
    const auto outcome = session.ResolveServiceAttack("Red", "iron_mine");

    REQUIRE(outcome.attacked);
    REQUIRE(outcome.defendersFought);
    REQUIRE_FALSE(outcome.attackerWon);
    REQUIRE_FALSE(outcome.playerBattleRequired);
    REQUIRE(outcome.capturedServiceIds.empty());

    REQUIRE_FALSE(TeamByColor(session, "Red")->active);  // repelled attacker is defeated
    const auto* mine = session.FindOwnedService("steel_mine_svc");
    REQUIRE(mine->ownerTeamColor == "Green");
    REQUIRE(mine->stationedUnits.size() == 1);
    REQUIRE(session.FindRosterStackById("stk_mine") != nullptr);
    REQUIRE(LogContains(session, "defenders held"));
    RequireNoOrphans(session);
}

TEST_CASE("ServiceAttack - attacker overruns weaker mine defenders and captures") {
    auto save = MakeBaseSave();
    // Weaken the garrison to 2x militia: the 2 raiders overrun it under auto-resolve.
    save.rosterStacks[1].quantity = 2;
    auto session = Load(save);

    const auto outcome = session.ResolveServiceAttack("Red", "iron_mine");

    REQUIRE(outcome.attacked);
    REQUIRE(outcome.defendersFought);
    REQUIRE(outcome.attackerWon);
    REQUIRE(outcome.capturedServiceIds == std::vector<std::string>{"steel_mine_svc"});

    const auto* mine = session.FindOwnedService("steel_mine_svc");
    REQUIRE(mine->ownerTeamColor == "Red");               // immediate ownership transfer
    REQUIRE(mine->stationedUnits.empty());                // refs cleared, not inherited
    REQUIRE(session.FindRosterStackById("stk_mine") == nullptr);  // generics dismissed
    REQUIRE(TeamByColor(session, "Red")->nodeId == "iron_mine");  // attacker occupies
    REQUIRE(session.UnavailableHeroes().empty());         // no hero was garrisoned
    REQUIRE(LogContains(session, "captured by Red"));
    RequireNoOrphans(session);
}

TEST_CASE("ServiceAttack - storage loss dismisses generics and makes stored heroes TU") {
    // Empty the stored militia stack so only hero_b defends the depot; a lone
    // hero_b is overrun by eg_raiders (2 raiders) under auto-resolve.
    auto save = MakeBaseSave();
    save.rosterStacks[2].quantity = 0;  // stored militia stack emptied
    auto session = Load(save);

    const auto outcome = session.ResolveServiceAttack("Red", "river_depot");

    REQUIRE(outcome.attacked);
    REQUIRE(outcome.attackerWon);
    const auto* depot = session.FindOwnedService("river_depot_storage");
    REQUIRE(depot->ownerTeamColor == "Red");
    REQUIRE(depot->storedUnits.empty());
    REQUIRE(session.FindRosterStackById("stk_store") == nullptr);
    REQUIRE(session.FindRosterStackById("stk_hero") == nullptr);

    // Stored hero entered the Temporarily Unavailable pipeline (weekly return).
    REQUIRE(session.UnavailableHeroes().size() == 1);
    REQUIRE(session.UnavailableHeroes()[0].unitId == "hero_b");
    REQUIRE(session.UnavailableHeroes()[0].becameUnavailableDay == 1);
    REQUIRE(session.UnavailableHeroes()[0].returnDay
        == 1 + gameplay::economy::kUnavailableHeroReturnDays);
    REQUIRE(LogContains(session, "temporarily unavailable"));
    RequireNoOrphans(session);
}

TEST_CASE("ServiceAttack - undefended service is seized without battle") {
    auto save = MakeBaseSave();
    save.ownedServices[0].stationedUnits.clear();
    save.rosterStacks.erase(save.rosterStacks.begin() + 1);  // remove garrison stack
    auto session = Load(save);

    const auto outcome = session.ResolveServiceAttack("Red", "iron_mine");

    REQUIRE(outcome.attacked);
    REQUIRE_FALSE(outcome.defendersFought);
    REQUIRE(outcome.attackerWon);
    REQUIRE(session.FindOwnedService("steel_mine_svc")->ownerTeamColor == "Red");
    REQUIRE(TeamByColor(session, "Red")->nodeId == "iron_mine");
    REQUIRE(LogContains(session, "unopposed"));
}

TEST_CASE("ServiceAttack - player party on the node defers to the battle surface") {
    auto save = MakeBaseSave();
    save.destinationId = "iron_mine";  // player stands on the mine node
    auto session = Load(save);

    const auto outcome = session.ResolveServiceAttack("Red", "iron_mine");

    REQUIRE(outcome.attacked);
    REQUIRE(outcome.playerBattleRequired);
    REQUIRE_FALSE(outcome.attackerWon);
    // Nothing mutated yet: ownership, refs, roster, and the attacker unchanged.
    REQUIRE(session.FindOwnedService("steel_mine_svc")->ownerTeamColor == "Green");
    REQUIRE(session.FindOwnedService("steel_mine_svc")->stationedUnits.size() == 1);
    REQUIRE(TeamByColor(session, "Red")->active);
    REQUIRE(TeamByColor(session, "Red")->nodeId == "town_center");
    REQUIRE(session.ServiceEventLog().empty());
}

TEST_CASE("ServiceAttack - allied, unknown, and inactive attackers are no-ops") {
    auto session = Load(MakeBaseSave());

    REQUIRE_FALSE(session.ResolveServiceAttack("Yellow", "iron_mine").attacked);  // allied
    REQUIRE_FALSE(session.ResolveServiceAttack("Purple", "iron_mine").attacked);  // unknown
    REQUIRE_FALSE(session.ResolveServiceAttack("Green", "iron_mine").attacked);   // player color

    auto save = MakeBaseSave();
    auto inactiveSession = Load(save);
    inactiveSession.ClearEnemyTeamByColor("Red");
    REQUIRE_FALSE(inactiveSession.ResolveServiceAttack("Red", "iron_mine").attacked);

    // Locked/destroyed services are not eligible targets.
    save = MakeBaseSave();
    save.ownedServices[0].locked = true;
    auto lockedSession = Load(save);
    REQUIRE_FALSE(lockedSession.ResolveServiceAttack("Red", "iron_mine").attacked);
}

TEST_CASE("ServiceAttack - a corrupt Player Character placement is never lost or TU") {
    auto save = MakeBaseSave();
    // Corrupt state: the PC stack is referenced as a stored unit AND slotted.
    save.ownedServices[1].storedUnits = {core::StoredUnitSaveState{"pc_hero", "stk_pc"}};
    // The PC never defends, so the garrison is effectively empty and the attacker
    // captures (proving the PC is still never lost or made TU on capture).
    auto session = Load(save, "eg_strong");

    const auto outcome = session.ResolveServiceAttack("Red", "river_depot");

    REQUIRE(outcome.attacked);
    REQUIRE(outcome.attackerWon);
    // The PC stack survives the capture: never removed, never TU; the corrupt
    // stored ref is cleared with the rest.
    REQUIRE(session.FindRosterStackById("stk_pc") != nullptr);
    REQUIRE(session.UnavailableHeroes().empty());
    REQUIRE(session.FindOwnedService("river_depot_storage")->storedUnits.empty());
    REQUIRE(session.FindOwnedService("river_depot_storage")->ownerTeamColor == "Red");
}

TEST_CASE("ServiceAttack - player-involved defense callbacks resolve both results") {
    // Victory: attacker defeated, services intact.
    auto session = Load(MakeBaseSave());
    session.ApplyServiceDefenseVictory("Red", "iron_mine");
    REQUIRE_FALSE(TeamByColor(session, "Red")->active);
    REQUIRE(session.FindOwnedService("steel_mine_svc")->ownerTeamColor == "Green");
    REQUIRE(LogContains(session, "repelled"));

    // Defeat: capture consequences, attacker occupies the node.
    auto defeatSession = Load(MakeBaseSave());
    defeatSession.ApplyServiceDefenseDefeat("Red", "iron_mine");
    REQUIRE(defeatSession.FindOwnedService("steel_mine_svc")->ownerTeamColor == "Red");
    REQUIRE(defeatSession.FindOwnedService("steel_mine_svc")->stationedUnits.empty());
    REQUIRE(defeatSession.FindRosterStackById("stk_mine") == nullptr);
    REQUIRE(TeamByColor(defeatSession, "Red")->nodeId == "iron_mine");
    RequireNoOrphans(defeatSession);
}

TEST_CASE("ServiceAttack - TU hero returns to a free reserve slot after a week") {
    auto save = MakeBaseSave();
    save.rosterStacks[2].quantity = 0;
    auto session = Load(save);
    REQUIRE(session.ResolveServiceAttack("Red", "river_depot").attackerWon);
    REQUIRE(session.UnavailableHeroes().size() == 1);

    // Day 1 + 7 days -> day 8 >= returnDay 8: the hero rejoins the reserve.
    session.AddMinutes(7 * core::GameClock::kMinutesPerSliceDay);
    REQUIRE(session.UnavailableHeroes().empty());
    bool inReserve = false;
    for (const auto& stackId : session.ReserveSlotStackIds()) {
        if (stackId.empty()) continue;
        const auto* stack = session.FindRosterStackById(stackId);
        if (stack != nullptr && stack->unitId == "hero_b") {
            inReserve = true;
            REQUIRE(stack->quantity == 1);
        }
    }
    REQUIRE(inReserve);
    REQUIRE(LogContains(session, "returned to the reserve"));
    RequireNoOrphans(session);
}

TEST_CASE("ServiceAttack - TU hero waits while the reserve is full and returns later") {
    auto save = MakeBaseSave();
    save.rosterStacks[2].quantity = 0;
    // Fill all 8 reserve slots with filler stacks.
    for (int i = 0; i < 8; ++i) {
        const std::string id = "stk_fill_" + std::to_string(i);
        save.rosterStacks.push_back(core::RosterStackSaveState{id, "militia", 1});
        save.reserveSlotStackIds[i] = id;
    }
    save.nextStackIdCounter = 20;
    auto session = Load(save);
    REQUIRE(session.ResolveServiceAttack("Red", "river_depot").attackerWon);

    session.AddMinutes(7 * core::GameClock::kMinutesPerSliceDay);
    REQUIRE(session.UnavailableHeroes().size() == 1);  // reserve full: still TU

    // Free one slot, then cross the next day boundary: the hero returns.
    REQUIRE(session.TryRemoveOwnedUnit("militia", 1));
    session.AddMinutes(core::GameClock::kMinutesPerSliceDay);
    REQUIRE(session.UnavailableHeroes().empty());
    RequireNoOrphans(session);
}

TEST_CASE("ServiceAttack - TU state, event log, and group ids survive save/load") {
    auto save = MakeBaseSave();
    save.rosterStacks[2].quantity = 0;
    auto session = Load(save);
    REQUIRE(session.ResolveServiceAttack("Red", "river_depot").attackerWon);
    const auto logSize = session.ServiceEventLog().size();
    REQUIRE(logSize >= 2);

    auto restored = Load(session.ToSaveData());
    REQUIRE(restored.UnavailableHeroes().size() == 1);
    REQUIRE(restored.UnavailableHeroes()[0].unitId == "hero_b");
    REQUIRE(restored.UnavailableHeroes()[0].returnDay
        == 1 + gameplay::economy::kUnavailableHeroReturnDays);
    REQUIRE(restored.ServiceEventLog().size() == logSize);
    REQUIRE(restored.FindOwnedService("river_depot_storage")->ownerTeamColor == "Red");
    REQUIRE(TeamByColor(restored, "Red")->enemyGroupId == "eg_raiders");

    // A corrupt PC entry in a hand-edited save is dropped on load.
    auto corrupt = restored.ToSaveData();
    corrupt.unavailableHeroes.push_back({"pc_hero", 1, 8});
    corrupt.unavailableHeroes.push_back({"", 1, 8});
    auto healed = Load(corrupt);
    REQUIRE(healed.UnavailableHeroes().size() == 1);
    REQUIRE(healed.UnavailableHeroes()[0].unitId == "hero_b");
}

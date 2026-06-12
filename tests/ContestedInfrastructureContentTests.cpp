#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include "core/GameClock.h"
#include "core/SaveGame.h"
#include "data/ContentRepository.h"
#include "gameplay/GameSession.h"
#include "gameplay/ResourceState.h"
#include "gameplay/economy/ServiceDefenseRules.h"

// M30 real-content proof loop: the SHIPPED content authors a player-owned
// direct storage depot (river_depot, playerStart), a destroyable copper mine
// with a restore cost, raider events carrying enemy groups, and an enemy team
// that can legally pressure the depot. These tests drive the full contested
// loop — store -> pressure -> defense/loss -> TU -> destroy -> restore —
// against that content with no synthetic definitions.

#ifndef LOP01_PROJECT_ROOT
#define LOP01_PROJECT_ROOT "."
#endif

namespace {

std::filesystem::path RealContentDir() {
    return std::filesystem::path(LOP01_PROJECT_ROOT) / "content";
}

bool HasErrorMessage(const std::vector<ValidationMessage>& msgs) {
    return std::any_of(msgs.begin(), msgs.end(),
        [](const ValidationMessage& m) { return m.severity == Severity::Error; });
}

gameplay::GameSession WireCampaignSession(const data::ContentRepository& content) {
    gameplay::GameSession session;
    session.SetPlayerColor("Green");
    session.SetUnitCatalog(content.Units());
    session.SetLeaderCapableUnitIds({"hero_player"});
    REQUIRE(session.AddOwnedUnit("hero_player", 1));
    REQUIRE(session.TryAddUnitToActiveParty("hero_player"));
    session.SetScenarioOutcomeDefinition(content.ScenarioOutcome());
    session.SetRegionCatalog(content.Regions());
    session.SetWorldMap(content.WorldMap());
    session.SetScenarioCatalog(content.Scenarios());
    session.SetCampaignCatalog(content.Campaigns());
    session.SetLocationServiceCatalog(content.LocationServices());
    session.SetTraderCurveCatalog(content.TraderCurves());
    session.SetEnemyGroupCatalog(content.EnemyGroups());
    session.InitializeEventDefinitions(content.EventDefinitions());
    return session;
}

std::string SlottedStackIdOf(const gameplay::GameSession& session, const std::string& unitId) {
    auto inSlots = [&](const std::vector<std::string>& slots, const std::string& stackId) {
        return std::find(slots.begin(), slots.end(), stackId) != slots.end();
    };
    for (const auto& stack : session.RosterStacks()) {
        if (stack.unitId == unitId &&
            (inSlots(session.ActiveSlotStackIds(), stack.stackId) ||
             inSlots(session.ReserveSlotStackIds(), stack.stackId))) {
            return stack.stackId;
        }
    }
    return {};
}

int GroupPower(const data::ContentRepository& repo, const std::string& groupId) {
    const auto* group = repo.FindEnemyGroupById(groupId);
    REQUIRE(group != nullptr);
    int power = 0;
    for (const auto& unitId : group->unitIds) {
        const auto* unit = repo.FindUnitById(unitId);
        REQUIRE(unit != nullptr);
        power += gameplay::economy::UnitDefensePower(unit->stats);
    }
    return power;
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

const data::LocationServiceDefinition* ServiceById(const data::ContentRepository& repo,
    const std::string& serviceId) {
    for (const auto& svc : repo.LocationServices()) {
        if (svc.id == serviceId) {
            return &svc;
        }
    }
    return nullptr;
}

} // namespace

TEST_CASE("Contested content - shipped content with depot/destroyable mine validates cleanly") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));
    REQUIRE_FALSE(HasErrorMessage(repo.ValidationMessages()));

    const auto* depot = ServiceById(repo, "river_depot_storage");
    REQUIRE(depot != nullptr);
    REQUIRE(depot->kind == data::LocationServiceKind::Storage);

    const auto* copper = ServiceById(repo, "copper_mine_svc");
    REQUIRE(copper != nullptr);
    REQUIRE(copper->destroyable);
    REQUIRE_FALSE(copper->restoreCost.empty());
}

TEST_CASE("Contested content - depot raiders spawn via the authored event with their group") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));
    auto session = WireCampaignSession(repo);
    session.StartCampaign("campaign_ashvale");

    // Visiting the depot fires the authored raid warning.
    static_cast<void>(session.NotifyRegionNodeEntry("river_depot"));

    const auto* blue = TeamByColor(session, "Blue");
    REQUIRE(blue != nullptr);
    REQUIRE(blue->active);
    REQUIRE(blue->nodeId == "bridge_checkpoint");
    REQUIRE(blue->enemyGroupId == "eg_patrol_01");
}

TEST_CASE("Contested content - stored defenders hold the depot when strong enough") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));
    auto session = WireCampaignSession(repo);
    session.StartCampaign("campaign_ashvale");
    session.SetDestination("home_base");  // player away from the depot

    // 2x Town Guard stored as one stack outguns the authored patrol.
    REQUIRE(session.AddOwnedUnit("unit_guard", 2));
    const std::string guardStack = SlottedStackIdOf(session, "unit_guard");
    REQUIRE(session.TryStoreStackAtService("river_depot_storage", guardStack));
    const auto* guard = repo.FindUnitById("unit_guard");
    const int defenderPower = gameplay::economy::StackDefensePower(guard->stats, 2);
    const int attackerPower = GroupPower(repo, "eg_patrol_01");
    REQUIRE(gameplay::economy::DefendersHoldService(defenderPower, attackerPower));

    static_cast<void>(session.NotifyRegionNodeEntry("river_depot"));  // spawn Blue
    const auto* region = repo.FindRegionById("ashvale_heartland");
    const auto results = session.ProcessEnemyPhase(region->links);

    bool attacked = false;
    for (const auto& r : results) {
        if (r.teamColor == "Blue" && r.actionType == "service_attack") {
            attacked = true;
            REQUIRE(r.nodeId == "river_depot");
        }
    }
    REQUIRE(attacked);
    REQUIRE_FALSE(TeamByColor(session, "Blue")->active);  // repelled
    const auto* depot = session.FindOwnedService("river_depot_storage");
    REQUIRE(depot->ownerTeamColor == "Green");
    REQUIRE(depot->storedUnits.size() == 1);              // garrison intact
    REQUIRE(session.FindRosterStackById(guardStack) != nullptr);
    REQUIRE_FALSE(session.ServiceEventLog().empty());
}

TEST_CASE("Contested content - losing the depot dismisses generics and makes Mira TU; she returns") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));
    auto session = WireCampaignSession(repo);
    session.StartCampaign("campaign_ashvale");
    session.SetDestination("home_base");

    // One guard (generic) + Mira (hero) stored. The shipped Collapsed Gate
    // warband (eg_ruin_02) outguns them — this REQUIRE guards the content
    // contract that the loss path is actually reachable with shipped stats.
    REQUIRE(session.AddOwnedUnit("unit_guard", 1));
    const std::string guardStack = SlottedStackIdOf(session, "unit_guard");
    REQUIRE(session.TryStoreStackAtService("river_depot_storage", guardStack));
    REQUIRE(session.AddOwnedUnit("hero_mira", 1));
    const std::string miraStack = SlottedStackIdOf(session, "hero_mira");
    REQUIRE(session.TryStoreStackAtService("river_depot_storage", miraStack));

    const auto* guard = repo.FindUnitById("unit_guard");
    const auto* mira = repo.FindUnitById("hero_mira");
    const int defenderPower = gameplay::economy::StackDefensePower(guard->stats, 1)
        + gameplay::economy::StackDefensePower(mira->stats, 1);
    const int attackerPower = GroupPower(repo, "eg_ruin_02");
    REQUIRE_FALSE(gameplay::economy::DefendersHoldService(defenderPower, attackerPower));

    // Seed the warband adjacent to the depot directly (the authored event
    // proves the spawn path separately).
    gameplay::EnemyTeamState blue;
    blue.teamColor = "Blue";
    blue.nodeId = "bridge_checkpoint";
    blue.enemyGroupId = "eg_ruin_02";
    session.SetEnemyTeams({blue});

    const auto* region = repo.FindRegionById("ashvale_heartland");
    static_cast<void>(session.ProcessEnemyPhase(region->links));

    // Storage lost: ownership transferred, generic dismissed, Mira TU.
    const auto* depot = session.FindOwnedService("river_depot_storage");
    REQUIRE(depot->ownerTeamColor == "Blue");
    REQUIRE(depot->storedUnits.empty());
    REQUIRE(session.FindRosterStackById(guardStack) == nullptr);
    REQUIRE(session.FindRosterStackById(miraStack) == nullptr);
    REQUIRE(session.UnavailableHeroes().size() == 1);
    REQUIRE(session.UnavailableHeroes()[0].unitId == "hero_mira");
    REQUIRE(TeamByColor(session, "Blue")->nodeId == "river_depot");

    // Save/reload preserves the loss, then Mira returns after the weekly wait.
    auto restored = WireCampaignSession(repo);
    restored.ApplySaveData(session.ToSaveData());
    REQUIRE(restored.UnavailableHeroes().size() == 1);
    restored.AddMinutes(
        gameplay::economy::kUnavailableHeroReturnDays * core::GameClock::kMinutesPerSliceDay);
    REQUIRE(restored.UnavailableHeroes().empty());
    REQUIRE_FALSE(SlottedStackIdOf(restored, "hero_mira").empty());
}

TEST_CASE("Contested content - the shipped copper mine can be claimed, destroyed, and restored") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));
    auto session = WireCampaignSession(repo);
    session.StartCampaign("campaign_ashvale");

    // Walk onto the unguarded copper mine: the M26 peaceful claim applies.
    session.SetDestination("copper_mine");
    const auto claimed = session.ResolveNodeEntryClaims("copper_mine");
    REQUIRE(std::find(claimed.begin(), claimed.end(), std::string{"copper_mine_svc"})
        != claimed.end());

    session.ApplyDailyStartingEnergy();
    REQUIRE(session.TryDestroyServiceAtCurrentNode("copper_mine_svc"));
    REQUIRE(session.FindOwnedService("copper_mine_svc")->destroyed);

    // Restore: shipped cost is 2 Wood + 1 Stone; playerStart grants Wood only.
    session.AddResource(gameplay::ResourceType::Stone, 1);
    REQUIRE(session.TryQueueServiceRestorationAtCurrentNode("copper_mine_svc"));
    const int goldBefore = session.Snapshot().gold;

    session.AddMinutes(core::GameClock::kMinutesPerSliceDay);
    const auto* owned = session.FindOwnedService("copper_mine_svc");
    REQUIRE_FALSE(owned->destroyed);
    // Restored before the day's payout: the mine pays its authored Gold output
    // on the restoration day.
    REQUIRE(session.Snapshot().gold == goldBefore + 100);
}

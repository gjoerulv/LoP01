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

// v1 strategic-economy proof against the SHIPPED content/ directory: the authored
// slice now exercises Scenario `playerStart` (starting Gold + resources + an
// initial player-owned Trading Post), authored Trading Post trade (barter + Gold
// trade against an authored ownership-tier curve), the current leader's
// `leader_energy` passive, and a `mine_production` stationed-passive payout boost.
// These are content proofs, not engine logic — they load the real content dir.

#ifndef LOP01_PROJECT_ROOT
#define LOP01_PROJECT_ROOT "."
#endif

using gameplay::ResourceType;

namespace {

constexpr int kOneDay = core::GameClock::kMinutesPerSliceDay;

std::filesystem::path RealContentDir() {
    return std::filesystem::path(LOP01_PROJECT_ROOT) / "content";
}

bool HasErrorMessage(const std::vector<ValidationMessage>& msgs) {
    return std::any_of(msgs.begin(), msgs.end(),
        [](const ValidationMessage& m) { return m.severity == Severity::Error; });
}

// Wire a campaign-capable session from real content, mirroring App startup.
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
    return session;
}

} // namespace

TEST_CASE("v1 proof: the shipped content directory validates without errors") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));
    REQUIRE_FALSE(HasErrorMessage(repo.ValidationMessages()));
}

TEST_CASE("v1 proof: scenario_intro playerStart applies starting economy and owned Trading Post") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));
    auto session = WireCampaignSession(repo);

    session.StartCampaign("campaign_ashvale");

    // Authored playerStart: Gold + a non-Gold resource + an initial player-owned service.
    REQUIRE(session.Snapshot().gold == 1200);
    REQUIRE(session.ResourceCount(ResourceType::Wood) == 50);
    const auto* tp = session.FindOwnedService("home_base_trading_post");
    REQUIRE(tp != nullptr);
    REQUIRE(tp->ownerTeamColor == "Green");
}

TEST_CASE("v1 proof: the owned Trading Post trades against the authored tier-1 curve") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));
    auto session = WireCampaignSession(repo);
    session.StartCampaign("campaign_ashvale");

    // The player owns one Trading Post (via playerStart) => effective tier 1, so
    // trades resolve against the authored tier-1 curve, not built-in defaults.
    // Barter: authored Wood->Stone costs 3 Wood each (default would be 10).
    const auto barter =
        session.TryTradingPostBarter("home_base_trading_post", ResourceType::Wood, ResourceType::Stone, 2);
    REQUIRE(barter.success);
    REQUIRE(session.ResourceCount(ResourceType::Wood) == 44);   // 50 - 3*2
    REQUIRE(session.ResourceCount(ResourceType::Stone) == 2);

    // Gold trade: authored tier-1 price factor 200 halves the default 500 buy cost.
    const auto buy =
        session.TryTradingPostBuyForGold("home_base_trading_post", ResourceType::Wood, 1);
    REQUIRE(buy.success);
    REQUIRE(session.Snapshot().gold == 950);                    // 1200 - 250
    REQUIRE(session.ResourceCount(ResourceType::Wood) == 45);
}

TEST_CASE("v1 proof: the leader's authored leader_energy passive raises daily starting Energy") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));

    gameplay::GameSession session;
    session.SetUnitCatalog(repo.Units());
    session.SetLeaderCapableUnitIds({"hero_player"});
    REQUIRE(session.AddOwnedUnit("hero_player", 1));
    REQUIRE(session.TryAddUnitToActiveParty("hero_player"));
    session.ApplyDailyStartingEnergy();

    // Energy = 1000 + lowestAgility*100 + leaderEnergy. hero_player: agility 8,
    // authored leader_energy 150 => 1000 + 800 + 150.
    REQUIRE(session.MaxEnergy() == 1950);
}

TEST_CASE("v1 proof: a stationed mine_production unit raises the Steel Mine's daily payout") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));

    auto makeSave = [](bool withStationedMiner) {
        core::SaveData s;
        s.schemaVersion = 5;
        s.day = 1;
        s.gold = 0;
        s.mode = "region_mode";
        s.regionId = "ashvale_heartland";
        s.destinationId = "home_base";
        s.hasCanonicalRoster = true;
        s.rosterStacks = {core::RosterStackSaveState{"stk_1", "unit_miner", 1}};
        s.activeSlotStackIds = {"stk_1", "", "", "", ""};
        s.reserveSlotStackIds = {"", "", "", "", "", "", "", ""};
        s.nextStackIdCounter = 2;
        std::vector<core::StationedUnitSaveState> stationed;
        if (withStationedMiner) {
            stationed.push_back(core::StationedUnitSaveState{"unit_miner", "stk_1"});
        }
        s.ownedServices = {
            core::OwnedServiceSaveState{"iron_mine_svc", "Green", false, false, stationed}
        };
        return s;
    };

    // Control: the player-owned mine pays only its authored base Steel output (2).
    {
        gameplay::GameSession session;
        session.SetPlayerColor("Green");
        session.SetUnitCatalog(repo.Units());
        session.SetLocationServiceCatalog(repo.LocationServices());
        session.ApplySaveData(makeSave(/*withStationedMiner=*/false));
        session.AddMinutes(kOneDay);
        REQUIRE(session.ResourceCount(ResourceType::Steel) == 2);
    }

    // With a stationed mine_production miner (+1 Steel, strongest-only): 2 + 1 = 3.
    {
        gameplay::GameSession session;
        session.SetPlayerColor("Green");
        session.SetUnitCatalog(repo.Units());
        session.SetLocationServiceCatalog(repo.LocationServices());
        session.ApplySaveData(makeSave(/*withStationedMiner=*/true));
        const int goldBefore = session.Snapshot().gold;
        session.AddMinutes(kOneDay);
        REQUIRE(session.ResourceCount(ResourceType::Steel) == 3);
        REQUIRE(session.Snapshot().gold == goldBefore + 200);   // authored base Gold output
    }
}

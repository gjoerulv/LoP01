#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

#include "core/GameClock.h"
#include "core/SaveGame.h"
#include "data/definitions/CampaignDefinition.h"
#include "data/definitions/LocationServiceDefinition.h"
#include "data/definitions/ScenarioDefinition.h"
#include "data/definitions/UnitDefinition.h"
#include "gameplay/GameSession.h"
#include "gameplay/ResourceState.h"

using gameplay::GameSession;
using gameplay::ResourceType;
using data::LocationServiceKind;

namespace {

constexpr int kOneDay = core::GameClock::kMinutesPerSliceDay;

data::UnitDefinition MakeHero(const std::string& id) {
    data::UnitDefinition u;
    u.id = id;
    u.name = id;
    u.category = data::UnitDefinitionCategory::Hero;
    u.isPlayerCharacter = true;
    u.stats.agility = 5;
    return u;
}

data::LocationServiceDefinition MakeMine(const std::string& id, const std::string& locationId,
    std::vector<data::MineOutputDefinition> outputs) {
    data::LocationServiceDefinition svc;
    svc.id = id;
    svc.locationId = locationId;
    svc.kind = LocationServiceKind::Mine;
    svc.mineOutputs = std::move(outputs);
    return svc;
}

data::LocationServiceDefinition MakeTradingPost(const std::string& id, const std::string& locationId) {
    data::LocationServiceDefinition svc;
    svc.id = id;
    svc.locationId = locationId;
    svc.kind = LocationServiceKind::TradingPost;
    return svc;
}

data::CampaignDefinition MakeCampaign(const std::string& startScenarioId) {
    data::CampaignDefinition c;
    c.id = "camp";
    c.startScenarioId = startScenarioId;
    c.scenarios = {{startScenarioId, {}, ""}};
    return c;
}

// Wires a session with a single hero in the active party and the given service +
// scenario catalogs, then starts the campaign (which applies the scenario start
// state through TransitionToScenario).
GameSession StartSession(const data::ScenarioDefinition& scenario,
    std::vector<data::LocationServiceDefinition> services) {
    GameSession session;
    session.SetUnitCatalog({MakeHero("hero")});
    session.SetLeaderCapableUnitIds({"hero"});
    REQUIRE(session.AddOwnedUnit("hero", 1));
    REQUIRE(session.TryAddUnitToActiveParty("hero"));
    session.SetLocationServiceCatalog(std::move(services));
    session.SetScenarioCatalog({scenario});
    session.SetCampaignCatalog({MakeCampaign(scenario.id)});
    session.StartCampaign("camp");
    return session;
}

data::ScenarioDefinition MakeScenario(const std::string& id) {
    data::ScenarioDefinition s;
    s.id = id;
    s.startRegionId = "alpha";
    s.startNodeId = "n1";  // avoids needing a region catalog for arrival-node lookup
    return s;
}

} // namespace

TEST_CASE("ScenarioStartApply: authored gold, resources and owned services reach the session") {
    auto scenario = MakeScenario("s1");
    scenario.startGold = 1500;
    scenario.startResources = {{"Wood", 5}, {"Stone", 3}};
    scenario.startOwnedServices = {{"mine_svc", false, false}, {"tp_svc", true, false}};

    auto session = StartSession(scenario, {
        MakeMine("mine_svc", "mine_loc", {{"Stone", 2}}),
        MakeTradingPost("tp_svc", "tp_loc"),
    });

    REQUIRE(session.ResourceCount(ResourceType::Gold) == 1500);
    REQUIRE(session.ResourceCount(ResourceType::Wood) == 5);
    REQUIRE(session.ResourceCount(ResourceType::Stone) == 3);

    REQUIRE(session.OwnedServices().size() == 2);
    const auto* mine = session.FindOwnedService("mine_svc");
    REQUIRE(mine != nullptr);
    REQUIRE(mine->ownerTeamColor == session.PlayerColor());  // owned by the player team
    REQUIRE_FALSE(mine->locked);
    const auto* tp = session.FindOwnedService("tp_svc");
    REQUIRE(tp != nullptr);
    REQUIRE(tp->locked);  // authored initial locked state honored
}

TEST_CASE("ScenarioStartApply: no playerStart yields default economy state") {
    auto session = StartSession(MakeScenario("s1"), {});

    REQUIRE(session.ResourceCount(ResourceType::Gold) == 2500);  // GameSession default
    REQUIRE(session.ResourceCount(ResourceType::Wood) == 0);
    REQUIRE(session.OwnedServices().empty());
}

TEST_CASE("ScenarioStartApply: an authored locked Trading Post is refused by the gate") {
    auto scenario = MakeScenario("s1");
    scenario.startOwnedServices = {{"tp_svc", true, false}};  // locked
    auto session = StartSession(scenario, {MakeTradingPost("tp_svc", "tp_loc")});

    REQUIRE_FALSE(session.ResolveTradingPostOffer("tp_svc").usable);

    auto unlocked = MakeScenario("s2");
    unlocked.startOwnedServices = {{"tp_svc", false, false}};
    auto session2 = StartSession(unlocked, {MakeTradingPost("tp_svc", "tp_loc")});
    REQUIRE(session2.ResolveTradingPostOffer("tp_svc").usable);
}

TEST_CASE("ScenarioStartApply: applied start state round-trips through save/load") {
    auto scenario = MakeScenario("s1");
    scenario.startGold = 1200;
    scenario.startResources = {{"Wood", 7}};
    scenario.startOwnedServices = {{"mine_svc", false, true}};  // destroyed flag set
    auto session = StartSession(scenario, {MakeMine("mine_svc", "mine_loc", {{"Stone", 2}})});

    const core::SaveData save = session.ToSaveData();
    REQUIRE(save.schemaVersion == 5);  // no schema bump

    GameSession reloaded;
    reloaded.ApplySaveData(save);
    REQUIRE(reloaded.ResourceCount(ResourceType::Gold) == 1200);
    REQUIRE(reloaded.ResourceCount(ResourceType::Wood) == 7);
    const auto* mine = reloaded.FindOwnedService("mine_svc");
    REQUIRE(mine != nullptr);
    REQUIRE(mine->destroyed);
}

TEST_CASE("ScenarioStartApply: an authored owned mine pays out at the day boundary") {
    auto scenario = MakeScenario("s1");
    scenario.startOwnedServices = {{"mine_svc", false, false}};
    auto session = StartSession(scenario, {MakeMine("mine_svc", "mine_loc", {{"Stone", 2}})});

    REQUIRE(session.ResourceCount(ResourceType::Stone) == 0);
    session.AddMinutes(kOneDay);  // cross the day boundary -> mine pays
    REQUIRE(session.ResourceCount(ResourceType::Stone) == 2);  // no hand-built SaveData needed
}

TEST_CASE("ScenarioStartApply: an authored owned Trading Post resolves ownership tier 1") {
    auto scenario = MakeScenario("s1");
    scenario.startOwnedServices = {{"tp_svc", false, false}};
    auto session = StartSession(scenario, {MakeTradingPost("tp_svc", "tp_loc")});

    REQUIRE(session.OwnedTraderServiceTierForService("tp_svc") == 1);
    const auto offer = session.ResolveTradingPostOffer("tp_svc");
    REQUIRE(offer.usable);
    REQUIRE(offer.effectiveTier == 1);
}

TEST_CASE("ScenarioStartApply: an unowned Trading Post is usable at tier 0") {
    auto session = StartSession(MakeScenario("s1"), {MakeTradingPost("tp_svc", "tp_loc")});

    const auto offer = session.ResolveTradingPostOffer("tp_svc");
    REQUIRE(offer.usable);
    REQUIRE(offer.effectiveTier == 0);
}

TEST_CASE("ScenarioStartApply: two authored owned Trading Posts feed the per-type tier count") {
    auto scenario = MakeScenario("s1");
    scenario.startOwnedServices = {{"tp_a", false, false}, {"tp_b", false, false}};
    auto session = StartSession(scenario, {
        MakeTradingPost("tp_a", "loc_a"),
        MakeTradingPost("tp_b", "loc_b"),
    });

    REQUIRE(session.OwnedTraderServiceTierForService("tp_a") == 2);
    REQUIRE(session.OwnedTraderServiceTierForService("tp_b") == 2);
    REQUIRE(session.ResolveTradingPostOffer("tp_a").effectiveTier == 2);
}

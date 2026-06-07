#include <catch2/catch_test_macros.hpp>

#include <limits>
#include <string>
#include <vector>

#include "core/SaveGame.h"
#include "data/definitions/LocationServiceDefinition.h"
#include "data/definitions/TraderOwnershipCurve.h"
#include "gameplay/EnemyTeamState.h"
#include "gameplay/GameSession.h"
#include "gameplay/ResourceState.h"

using data::LocationServiceKind;
using gameplay::ResourceType;

namespace {

data::LocationServiceDefinition MakeService(const std::string& id,
    const std::string& locationId, LocationServiceKind kind) {
    data::LocationServiceDefinition svc;
    svc.id = id;
    svc.locationId = locationId;
    svc.kind = kind;
    return svc;
}

core::OwnedServiceSaveState Owned(const std::string& serviceId, const std::string& owner,
    bool locked = false, bool destroyed = false) {
    return core::OwnedServiceSaveState{serviceId, owner, locked, destroyed, {}};
}

data::TraderExchangeEntry Exchange(const std::string& from, const std::string& to, int cost) {
    return data::TraderExchangeEntry{from, to, cost};
}

data::TraderTierEntry Tier(int tier, int priceFactor,
    std::vector<data::TraderExchangeEntry> matrix = {}) {
    data::TraderTierEntry e;
    e.tier = tier;
    e.priceFactor = priceFactor;
    e.exchangeMatrix = std::move(matrix);
    return e;
}

data::TraderOwnershipCurve TradingPostCurve(std::vector<data::TraderTierEntry> tiers) {
    data::TraderOwnershipCurve c;
    c.kind = LocationServiceKind::TradingPost;
    c.rawType = "trading_post";
    c.tiers = std::move(tiers);
    return c;
}

core::SaveData MakeBaseSave() {
    core::SaveData s;
    s.schemaVersion = 5;
    s.day = 1;
    s.minutesIntoSliceDay = 0;
    s.gold = 2500;
    s.mode = "region_mode";
    s.regionId = "ashvale_heartland";
    s.destinationId = "home_base";
    s.hasCanonicalRoster = true;
    s.rosterStacks = { core::RosterStackSaveState{"stk_1", "hero", 1} };
    s.activeSlotStackIds = {"stk_1", "", "", "", ""};
    s.reserveSlotStackIds = {"", "", "", "", "", "", "", ""};
    s.nextStackIdCounter = 2;
    return s;
}

gameplay::GameSession MakeSession(
    std::vector<data::LocationServiceDefinition> serviceCatalog,
    std::vector<core::OwnedServiceSaveState> ownedServices,
    std::vector<data::TraderOwnershipCurve> curves = {}) {
    auto save = MakeBaseSave();
    save.ownedServices = std::move(ownedServices);
    gameplay::GameSession session;
    session.SetLocationServiceCatalog(std::move(serviceCatalog));
    session.SetTraderCurveCatalog(std::move(curves));
    session.ApplySaveData(save);
    return session;
}

} // namespace

// ---------------------------------------------------------------------------
// Default rates at a usable, unowned Trading Post (effective tier 0, no curve).
// ---------------------------------------------------------------------------

TEST_CASE("TradingPostTxn - unowned post buys at the default 5x base value") {
    auto session = MakeSession(
        {MakeService("tp", "loc_a", LocationServiceKind::TradingPost)},
        {Owned("tp", "")});  // unowned but available

    const auto result = session.TryTradingPostBuyForGold("tp", ResourceType::Wood, 1);
    REQUIRE(result.success);
    REQUIRE(session.Snapshot().gold == 2000);  // 2500 - 500
    REQUIRE(session.ResourceCount(ResourceType::Wood) == 1);
}

TEST_CASE("TradingPostTxn - unowned post sells at the default 1/5 base value (Gold via gold_)") {
    auto session = MakeSession(
        {MakeService("tp", "loc_a", LocationServiceKind::TradingPost)},
        {Owned("tp", "")});
    session.AddResource(ResourceType::Wood, 5);

    const auto result = session.TryTradingPostSellForGold("tp", ResourceType::Wood, 1);
    REQUIRE(result.success);
    REQUIRE(session.Snapshot().gold == 2520);  // 2500 + 20
    REQUIRE(session.ResourceCount(ResourceType::Wood) == 4);
}

TEST_CASE("TradingPostTxn - unowned post barters at the default table") {
    auto session = MakeSession(
        {MakeService("tp", "loc_a", LocationServiceKind::TradingPost)},
        {Owned("tp", "")});
    session.AddResource(ResourceType::Wood, 100);

    // Default Wood->Stone (both class tier 0) costs 10 Wood per Stone.
    const auto result = session.TryTradingPostBarter("tp", ResourceType::Wood, ResourceType::Stone, 1);
    REQUIRE(result.success);
    REQUIRE(session.ResourceCount(ResourceType::Wood) == 90);
    REQUIRE(session.ResourceCount(ResourceType::Stone) == 1);
}

// ---------------------------------------------------------------------------
// Rule B: effective-tier-0 resolution.
// ---------------------------------------------------------------------------

TEST_CASE("TradingPostTxn - unowned post with an authored tier 0 uses authored tier 0") {
    auto session = MakeSession(
        {MakeService("tp", "loc_a", LocationServiceKind::TradingPost)},
        {Owned("tp", "")},
        {TradingPostCurve({Tier(0, /*priceFactor=*/200,
            {Exchange("Wood", "Stone", 3)})})});
    session.AddResource(ResourceType::Wood, 100);

    // Barter uses the authored tier-0 matrix (3 Wood per Stone), not the default 10.
    const auto barter = session.TryTradingPostBarter("tp", ResourceType::Wood, ResourceType::Stone, 2);
    REQUIRE(barter.success);
    REQUIRE(session.ResourceCount(ResourceType::Wood) == 94);  // 100 - 3*2
    REQUIRE(session.ResourceCount(ResourceType::Stone) == 2);

    // Gold trade uses the authored tier-0 favorability (200 -> half price).
    const auto buy = session.TryTradingPostBuyForGold("tp", ResourceType::Wood, 1);
    REQUIRE(buy.success);
    REQUIRE(session.Snapshot().gold == 2250);  // 2500 - 250
}

TEST_CASE("TradingPostTxn - unowned post without an authored tier 0 uses built-in defaults") {
    // The curve authors only tier 1; an effective-tier-0 use must not inherit it.
    auto session = MakeSession(
        {MakeService("tp", "loc_a", LocationServiceKind::TradingPost)},
        {Owned("tp", "")},
        {TradingPostCurve({Tier(1, /*priceFactor=*/200,
            {Exchange("Wood", "Stone", 3)})})});

    const auto buy = session.TryTradingPostBuyForGold("tp", ResourceType::Wood, 1);
    REQUIRE(buy.success);
    REQUIRE(session.Snapshot().gold == 2000);  // default 500, not the tier-1 250
}

TEST_CASE("TradingPostTxn - a player-owned eligible post uses its ownership tier") {
    auto session = MakeSession(
        {MakeService("tp", "loc_a", LocationServiceKind::TradingPost)},
        {Owned("tp", "Green")},  // player owns the used post -> effective tier 1
        {TradingPostCurve({Tier(1, /*priceFactor=*/200,
            {Exchange("Wood", "Stone", 3)})})});
    session.AddResource(ResourceType::Wood, 100);

    const auto barter = session.TryTradingPostBarter("tp", ResourceType::Wood, ResourceType::Stone, 2);
    REQUIRE(barter.success);
    REQUIRE(session.ResourceCount(ResourceType::Wood) == 94);  // tier-1 matrix: 3 each
    REQUIRE(session.ResourceCount(ResourceType::Stone) == 2);

    const auto buy = session.TryTradingPostBuyForGold("tp", ResourceType::Wood, 1);
    REQUIRE(buy.success);
    REQUIRE(session.Snapshot().gold == 2250);  // tier-1 favorability -> 250
}

// ---------------------------------------------------------------------------
// Usability gate: ownership never bypasses lock / destruction / occupation.
// ---------------------------------------------------------------------------

TEST_CASE("TradingPostTxn - a locked owned post is refused, not used at tier 0") {
    auto session = MakeSession(
        {MakeService("tp", "loc_a", LocationServiceKind::TradingPost)},
        {Owned("tp", "Green", /*locked=*/true)});

    const auto result = session.TryTradingPostBuyForGold("tp", ResourceType::Wood, 1);
    REQUIRE_FALSE(result.success);
    REQUIRE(session.Snapshot().gold == 2500);
    REQUIRE(session.ResourceCount(ResourceType::Wood) == 0);
}

TEST_CASE("TradingPostTxn - a destroyed owned post is refused") {
    auto session = MakeSession(
        {MakeService("tp", "loc_a", LocationServiceKind::TradingPost)},
        {Owned("tp", "Green", false, /*destroyed=*/true)});

    const auto result = session.TryTradingPostBuyForGold("tp", ResourceType::Wood, 1);
    REQUIRE_FALSE(result.success);
    REQUIRE(session.Snapshot().gold == 2500);
}

TEST_CASE("TradingPostTxn - a hostile-occupied post is refused") {
    auto session = MakeSession(
        {MakeService("tp", "loc_a", LocationServiceKind::TradingPost)},
        {Owned("tp", "Green")});

    gameplay::EnemyTeamState enemy;
    enemy.teamColor = "Red";
    enemy.nodeId = "loc_a";
    enemy.active = true;
    session.SetEnemyTeams({enemy});

    const auto result = session.TryTradingPostBuyForGold("tp", ResourceType::Wood, 1);
    REQUIRE_FALSE(result.success);
    REQUIRE(session.Snapshot().gold == 2500);
}

// ---------------------------------------------------------------------------
// Affordability: refusals leave state untouched (atomic).
// ---------------------------------------------------------------------------

TEST_CASE("TradingPostTxn - buying without enough gold changes nothing") {
    auto session = MakeSession(
        {MakeService("tp", "loc_a", LocationServiceKind::TradingPost)},
        {Owned("tp", "")});

    // 2 Gems cost 5000 gold; the team holds only 2500.
    const auto result = session.TryTradingPostBuyForGold("tp", ResourceType::Gems, 2);
    REQUIRE_FALSE(result.success);
    REQUIRE(session.Snapshot().gold == 2500);
    REQUIRE(session.ResourceCount(ResourceType::Gems) == 0);
}

TEST_CASE("TradingPostTxn - bartering without enough resources changes nothing") {
    auto session = MakeSession(
        {MakeService("tp", "loc_a", LocationServiceKind::TradingPost)},
        {Owned("tp", "")});
    session.AddResource(ResourceType::Wood, 5);  // need 10 for 1 Stone at default

    const auto result = session.TryTradingPostBarter("tp", ResourceType::Wood, ResourceType::Stone, 1);
    REQUIRE_FALSE(result.success);
    REQUIRE(session.ResourceCount(ResourceType::Wood) == 5);
    REQUIRE(session.ResourceCount(ResourceType::Stone) == 0);
}

TEST_CASE("TradingPostTxn - selling without enough resources changes nothing") {
    auto session = MakeSession(
        {MakeService("tp", "loc_a", LocationServiceKind::TradingPost)},
        {Owned("tp", "")});

    const auto result = session.TryTradingPostSellForGold("tp", ResourceType::Wood, 1);
    REQUIRE_FALSE(result.success);
    REQUIRE(session.Snapshot().gold == 2500);
}

// ---------------------------------------------------------------------------
// Service-kind and id gating.
// ---------------------------------------------------------------------------

TEST_CASE("TradingPostTxn - a non-Trading-Post service is refused") {
    auto session = MakeSession(
        {MakeService("mkt", "loc_a", LocationServiceKind::Market)},
        {Owned("mkt", "Green")});

    const auto result = session.TryTradingPostBuyForGold("mkt", ResourceType::Wood, 1);
    REQUIRE_FALSE(result.success);
    REQUIRE(session.Snapshot().gold == 2500);
}

TEST_CASE("TradingPostTxn - an unknown service id is refused") {
    auto session = MakeSession(
        {MakeService("tp", "loc_a", LocationServiceKind::TradingPost)},
        {Owned("tp", "")});

    const auto result = session.TryTradingPostBuyForGold("no_such_id", ResourceType::Wood, 1);
    REQUIRE_FALSE(result.success);
    REQUIRE(session.Snapshot().gold == 2500);
}

// ---------------------------------------------------------------------------
// Grant-overflow safety: a spend must never succeed when the grant would
// overflow int. The receiving count is seeded at INT_MAX.
// ---------------------------------------------------------------------------

TEST_CASE("TradingPostTxn - barter grant overflow leaves both resources unchanged") {
    auto session = MakeSession(
        {MakeService("tp", "loc_a", LocationServiceKind::TradingPost)},
        {Owned("tp", "")});
    session.AddResource(ResourceType::Wood, 100);
    session.AddResource(ResourceType::Stone, std::numeric_limits<int>::max());

    const auto result = session.TryTradingPostBarter("tp", ResourceType::Wood, ResourceType::Stone, 1);
    REQUIRE_FALSE(result.success);
    REQUIRE(session.ResourceCount(ResourceType::Wood) == 100);
    REQUIRE(session.ResourceCount(ResourceType::Stone) == std::numeric_limits<int>::max());
}

TEST_CASE("TradingPostTxn - buy grant overflow leaves gold and resource unchanged") {
    auto session = MakeSession(
        {MakeService("tp", "loc_a", LocationServiceKind::TradingPost)},
        {Owned("tp", "")});
    session.AddResource(ResourceType::Wood, std::numeric_limits<int>::max());

    const auto result = session.TryTradingPostBuyForGold("tp", ResourceType::Wood, 1);
    REQUIRE_FALSE(result.success);
    REQUIRE(session.Snapshot().gold == 2500);
    REQUIRE(session.ResourceCount(ResourceType::Wood) == std::numeric_limits<int>::max());
}

TEST_CASE("TradingPostTxn - sell gold grant overflow leaves resource and gold unchanged") {
    auto session = MakeSession(
        {MakeService("tp", "loc_a", LocationServiceKind::TradingPost)},
        {Owned("tp", "")});
    session.AddResource(ResourceType::Wood, 5);
    session.AddResource(ResourceType::Gold, std::numeric_limits<int>::max() - 2500);  // gold -> INT_MAX

    REQUIRE(session.Snapshot().gold == std::numeric_limits<int>::max());
    const auto result = session.TryTradingPostSellForGold("tp", ResourceType::Wood, 1);
    REQUIRE_FALSE(result.success);
    REQUIRE(session.Snapshot().gold == std::numeric_limits<int>::max());
    REQUIRE(session.ResourceCount(ResourceType::Wood) == 5);
}

TEST_CASE("TradingPostTxn - barter with Gold is refused and mutates nothing") {
    auto session = MakeSession(
        {MakeService("tp", "loc_a", LocationServiceKind::TradingPost)},
        {Owned("tp", "")});
    session.AddResource(ResourceType::Wood, 100);

    const auto fromGold = session.TryTradingPostBarter("tp", ResourceType::Gold, ResourceType::Wood, 1);
    const auto toGold = session.TryTradingPostBarter("tp", ResourceType::Wood, ResourceType::Gold, 1);
    REQUIRE_FALSE(fromGold.success);
    REQUIRE_FALSE(toGold.success);
    REQUIRE(session.Snapshot().gold == 2500);
    REQUIRE(session.ResourceCount(ResourceType::Wood) == 100);
}

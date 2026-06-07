#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

#include "app/TradingPostInteraction.h"
#include "core/SaveGame.h"
#include "data/definitions/LocationServiceDefinition.h"
#include "data/definitions/TraderOwnershipCurve.h"
#include "gameplay/EnemyTeamState.h"
#include "gameplay/GameSession.h"
#include "gameplay/ResourceState.h"

using app::TradingPostCommand;
using app::TradingPostInteraction;
using data::LocationServiceKind;
using gameplay::ResourceType;

namespace {

data::LocationServiceDefinition MakeTradingPost(const std::string& id,
    const std::string& locationId, int timeCostMinutes = 20) {
    data::LocationServiceDefinition svc;
    svc.id = id;
    svc.locationId = locationId;
    svc.kind = LocationServiceKind::TradingPost;
    svc.timeCostMinutes = timeCostMinutes;
    return svc;
}

core::OwnedServiceSaveState Owned(const std::string& serviceId, const std::string& owner,
    bool locked = false, bool destroyed = false) {
    return core::OwnedServiceSaveState{serviceId, owner, locked, destroyed, {}};
}

data::TraderOwnershipCurve TradingPostCurve(int tier, int priceFactor,
    std::vector<data::TraderExchangeEntry> matrix) {
    data::TraderTierEntry entry;
    entry.tier = tier;
    entry.priceFactor = priceFactor;
    entry.exchangeMatrix = std::move(matrix);
    data::TraderOwnershipCurve curve;
    curve.kind = LocationServiceKind::TradingPost;
    curve.rawType = "trading_post";
    curve.tiers = {entry};
    return curve;
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
    s.rosterStacks = {core::RosterStackSaveState{"stk_1", "hero", 1}};
    s.activeSlotStackIds = {"stk_1", "", "", "", ""};
    s.reserveSlotStackIds = {"", "", "", "", "", "", "", ""};
    s.nextStackIdCounter = 2;
    return s;
}

gameplay::GameSession MakeSession(
    std::vector<data::LocationServiceDefinition> serviceCatalog,
    std::vector<core::OwnedServiceSaveState> ownedServices = {},
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
// GameSession::ResolveTradingPostOffer (the display/preview read).
// ---------------------------------------------------------------------------

TEST_CASE("TradingPostOffer - unowned usable post resolves at tier 0 with default barter") {
    const auto service = MakeTradingPost("tp", "loc");
    auto session = MakeSession({service});

    const auto offer = session.ResolveTradingPostOffer("tp");
    REQUIRE(offer.usable);
    REQUIRE(offer.effectiveTier == 0);
    REQUIRE(offer.priceFactor == 100);
    REQUIRE_FALSE(offer.barter.empty());  // built-in default table
}

TEST_CASE("TradingPostOffer - player-owned post resolves at its tier with authored rates") {
    const auto service = MakeTradingPost("tp", "loc");
    auto session = MakeSession({service}, {Owned("tp", "Green")},
        {TradingPostCurve(1, 200, {{"Wood", "Stone", 5}})});

    const auto offer = session.ResolveTradingPostOffer("tp");
    REQUIRE(offer.usable);
    REQUIRE(offer.effectiveTier == 1);
    REQUIRE(offer.priceFactor == 200);
    REQUIRE(offer.barter.size() == 1);
}

TEST_CASE("TradingPostOffer - locked post is not usable and offers nothing") {
    const auto service = MakeTradingPost("tp", "loc");
    auto session = MakeSession({service}, {Owned("tp", "Green", /*locked=*/true)});

    const auto offer = session.ResolveTradingPostOffer("tp");
    REQUIRE_FALSE(offer.usable);
    REQUIRE(offer.barter.empty());
}

// ---------------------------------------------------------------------------
// Interaction lifecycle and navigation.
// ---------------------------------------------------------------------------

TEST_CASE("TradingPostInteraction - Open activates and reflects effective tier") {
    const auto service = MakeTradingPost("tp", "loc");
    auto session = MakeSession({service});

    TradingPostInteraction tp;
    tp.Open(session, service);

    REQUIRE(tp.IsActive());
    const std::string prompt = tp.BuildPromptText(session);
    REQUIRE(prompt.find("Trading Post") != std::string::npos);
    REQUIRE(prompt.find("Mode: Buy") != std::string::npos);
    REQUIRE(prompt.find("Visit cost: 20 min") != std::string::npos);  // shown up front
}

TEST_CASE("TradingPostInteraction - prompt shows the ownership tier when the post is owned") {
    const auto service = MakeTradingPost("tp", "loc");
    auto session = MakeSession({service}, {Owned("tp", "Green")},
        {TradingPostCurve(1, 200, {{"Wood", "Stone", 5}})});

    TradingPostInteraction tp;
    tp.Open(session, service);

    REQUIRE(tp.BuildPromptText(session).find("ownership tier 1") != std::string::npos);
}

TEST_CASE("TradingPostInteraction - prompt flags an unaffordable previewed trade") {
    const auto service = MakeTradingPost("tp", "loc");
    auto session = MakeSession({service});  // 2500 gold, mode Buy, Wood at 500/unit

    TradingPostInteraction tp;
    tp.Open(session, service);
    REQUIRE(tp.BuildPromptText(session).find("[need more]") == std::string::npos);  // qty 1 = 500, affordable

    for (int i = 0; i < 5; ++i) {
        tp.ApplyCommand(TradingPostCommand::QuantityUp, session);  // qty 6 -> 3000 gold
    }
    REQUIRE(tp.BuildPromptText(session).find("[need more]") != std::string::npos);
}

TEST_CASE("TradingPostInteraction - visit cost becomes pending after a successful trade") {
    const auto service = MakeTradingPost("tp", "loc", /*timeCostMinutes=*/20);
    auto session = MakeSession({service});

    TradingPostInteraction tp;
    tp.Open(session, service);
    REQUIRE(tp.BuildPromptText(session).find("after first trade") != std::string::npos);

    tp.ApplyCommand(TradingPostCommand::ConfirmTrade, session);  // buy Wood (success)
    REQUIRE(tp.BuildPromptText(session).find("pending on exit") != std::string::npos);
}

TEST_CASE("TradingPostInteraction - CycleMode rotates Buy -> Sell -> Barter -> Buy") {
    const auto service = MakeTradingPost("tp", "loc");
    auto session = MakeSession({service});

    TradingPostInteraction tp;
    tp.Open(session, service);

    REQUIRE(tp.ApplyCommand(TradingPostCommand::CycleMode, session).statusText.find("Sell") != std::string::npos);
    REQUIRE(tp.ApplyCommand(TradingPostCommand::CycleMode, session).statusText.find("Barter") != std::string::npos);
    REQUIRE(tp.ApplyCommand(TradingPostCommand::CycleMode, session).statusText.find("Buy") != std::string::npos);
}

TEST_CASE("TradingPostInteraction - quantity clamps at 1 and increments") {
    const auto service = MakeTradingPost("tp", "loc");
    auto session = MakeSession({service});

    TradingPostInteraction tp;
    tp.Open(session, service);

    tp.ApplyCommand(TradingPostCommand::QuantityDown, session);  // already 1, stays 1
    REQUIRE(tp.BuildPromptText(session).find("Qty: 1") != std::string::npos);
    tp.ApplyCommand(TradingPostCommand::QuantityUp, session);
    tp.ApplyCommand(TradingPostCommand::QuantityUp, session);
    REQUIRE(tp.BuildPromptText(session).find("Qty: 3") != std::string::npos);
}

TEST_CASE("TradingPostInteraction - quantity clamps at the upper cap of 999") {
    const auto service = MakeTradingPost("tp", "loc");
    auto session = MakeSession({service});

    TradingPostInteraction tp;
    tp.Open(session, service);

    for (int i = 0; i < 1005; ++i) {
        tp.ApplyCommand(TradingPostCommand::QuantityUp, session);
    }
    REQUIRE(tp.BuildPromptText(session).find("Qty: 999") != std::string::npos);
}

// ---------------------------------------------------------------------------
// Trades execute through the M19 APIs (Gold via gold_, resources atomic).
// ---------------------------------------------------------------------------

TEST_CASE("TradingPostInteraction - Buy spends Gold and grants the resource") {
    const auto service = MakeTradingPost("tp", "loc");
    auto session = MakeSession({service});

    TradingPostInteraction tp;
    tp.Open(session, service);  // mode Buy, index 0 = Wood

    const auto result = tp.ApplyCommand(TradingPostCommand::ConfirmTrade, session);
    REQUIRE(result.statusText.find("Trade complete") != std::string::npos);
    REQUIRE(session.Snapshot().gold == 2000);  // default 5x base = 500
    REQUIRE(session.ResourceCount(ResourceType::Wood) == 1);
}

TEST_CASE("TradingPostInteraction - Sell spends the resource and grants Gold") {
    const auto service = MakeTradingPost("tp", "loc");
    auto session = MakeSession({service});
    session.AddResource(ResourceType::Wood, 5);

    TradingPostInteraction tp;
    tp.Open(session, service);
    tp.ApplyCommand(TradingPostCommand::CycleMode, session);  // -> Sell

    const auto result = tp.ApplyCommand(TradingPostCommand::ConfirmTrade, session);
    REQUIRE(result.statusText.find("Trade complete") != std::string::npos);
    REQUIRE(session.Snapshot().gold == 2520);  // default 1/5 base = 20
    REQUIRE(session.ResourceCount(ResourceType::Wood) == 4);
}

TEST_CASE("TradingPostInteraction - Barter uses the resolved matrix entry") {
    const auto service = MakeTradingPost("tp", "loc");
    auto session = MakeSession({service}, {Owned("tp", "Green")},
        {TradingPostCurve(1, 100, {{"Wood", "Stone", 5}})});
    session.AddResource(ResourceType::Wood, 100);

    TradingPostInteraction tp;
    tp.Open(session, service);
    tp.ApplyCommand(TradingPostCommand::CycleMode, session);   // -> Sell
    tp.ApplyCommand(TradingPostCommand::CycleMode, session);   // -> Barter
    tp.ApplyCommand(TradingPostCommand::QuantityUp, session);  // qty 2

    const auto result = tp.ApplyCommand(TradingPostCommand::ConfirmTrade, session);
    REQUIRE(result.statusText.find("Trade complete") != std::string::npos);
    REQUIRE(session.ResourceCount(ResourceType::Wood) == 90);  // 5 per Stone * 2
    REQUIRE(session.ResourceCount(ResourceType::Stone) == 2);
}

TEST_CASE("TradingPostInteraction - a failed trade reports the reason and changes nothing") {
    const auto service = MakeTradingPost("tp", "loc");
    auto session = MakeSession({service});

    TradingPostInteraction tp;
    tp.Open(session, service);
    tp.ApplyCommand(TradingPostCommand::CycleMode, session);  // -> Sell (holding 0 Wood)

    const auto result = tp.ApplyCommand(TradingPostCommand::ConfirmTrade, session);
    REQUIRE(result.statusText.find("Not enough resources") != std::string::npos);
    REQUIRE(session.Snapshot().gold == 2500);
    REQUIRE(session.ResourceCount(ResourceType::Wood) == 0);
}

// ---------------------------------------------------------------------------
// Per-visit time cost (Decision 59): once on exit, only if a trade succeeded.
// ---------------------------------------------------------------------------

TEST_CASE("TradingPostInteraction - exit after a successful trade costs the visit time once") {
    const auto service = MakeTradingPost("tp", "loc", /*timeCostMinutes=*/20);
    auto session = MakeSession({service});

    TradingPostInteraction tp;
    tp.Open(session, service);
    tp.ApplyCommand(TradingPostCommand::ConfirmTrade, session);  // buy Wood (success)
    tp.ApplyCommand(TradingPostCommand::ConfirmTrade, session);  // buy Wood again (success)

    const auto exit = tp.ApplyCommand(TradingPostCommand::Exit, session);
    REQUIRE(exit.shouldExit);
    REQUIRE(exit.statusText.find("20 min") != std::string::npos);
    REQUIRE(session.Snapshot().minutesIntoSliceDay == 20);  // charged once, not per trade
}

TEST_CASE("TradingPostInteraction - exit with no successful trade costs no time") {
    const auto service = MakeTradingPost("tp", "loc", /*timeCostMinutes=*/20);
    auto session = MakeSession({service});

    TradingPostInteraction tp;
    tp.Open(session, service);
    tp.ApplyCommand(TradingPostCommand::CycleMode, session);      // -> Sell
    tp.ApplyCommand(TradingPostCommand::ConfirmTrade, session);   // fails (no Wood)
    tp.ApplyCommand(TradingPostCommand::SelectNext, session);     // navigation only

    const auto exit = tp.ApplyCommand(TradingPostCommand::Exit, session);
    REQUIRE(exit.shouldExit);
    REQUIRE(session.Snapshot().minutesIntoSliceDay == 0);
}

// ---------------------------------------------------------------------------
// Gate: a non-usable post offers no barter and refuses confirms (defense in
// depth; the App also refuses to open one).
// ---------------------------------------------------------------------------

TEST_CASE("TradingPostInteraction - barter at a locked post is refused and changes nothing") {
    const auto service = MakeTradingPost("tp", "loc");
    auto session = MakeSession({service}, {Owned("tp", "Green", /*locked=*/true)},
        {TradingPostCurve(1, 100, {{"Wood", "Stone", 5}})});
    session.AddResource(ResourceType::Wood, 100);

    TradingPostInteraction tp;
    tp.Open(session, service);
    tp.ApplyCommand(TradingPostCommand::CycleMode, session);  // -> Sell
    tp.ApplyCommand(TradingPostCommand::CycleMode, session);  // -> Barter (no entries: locked)

    const auto result = tp.ApplyCommand(TradingPostCommand::ConfirmTrade, session);
    REQUIRE_FALSE(result.statusText.empty());
    REQUIRE(session.ResourceCount(ResourceType::Wood) == 100);
    REQUIRE(session.ResourceCount(ResourceType::Stone) == 0);

    const auto exit = tp.ApplyCommand(TradingPostCommand::Exit, session);
    REQUIRE(session.Snapshot().minutesIntoSliceDay == 0);  // nothing succeeded
}

TEST_CASE("TradingPostInteraction - a locked post shows unavailable text, not a buy quote") {
    const auto service = MakeTradingPost("tp", "loc");
    auto session = MakeSession({service}, {Owned("tp", "Green", /*locked=*/true)});

    TradingPostInteraction tp;
    tp.Open(session, service);  // default Buy mode

    const std::string prompt = tp.BuildPromptText(session);
    REQUIRE(prompt.find("not available") != std::string::npos);
    REQUIRE(prompt.find("Buy 1") == std::string::npos);  // no fake quote line
}

TEST_CASE("TradingPostInteraction - confirm on a locked post is refused and charges no time") {
    const auto service = MakeTradingPost("tp", "loc", /*timeCostMinutes=*/20);
    auto session = MakeSession({service}, {Owned("tp", "Green", /*locked=*/true)});

    TradingPostInteraction tp;
    tp.Open(session, service);  // default Buy mode

    const auto result = tp.ApplyCommand(TradingPostCommand::ConfirmTrade, session);
    REQUIRE(result.statusText.find("not available") != std::string::npos);
    REQUIRE(session.Snapshot().gold == 2500);
    REQUIRE(session.ResourceCount(ResourceType::Wood) == 0);

    const auto exit = tp.ApplyCommand(TradingPostCommand::Exit, session);
    REQUIRE(session.Snapshot().minutesIntoSliceDay == 0);  // no successful visit
}

TEST_CASE("TradingPostInteraction - a destroyed post offers no trades") {
    const auto service = MakeTradingPost("tp", "loc");
    auto session = MakeSession({service}, {Owned("tp", "Green", false, /*destroyed=*/true)});

    TradingPostInteraction tp;
    tp.Open(session, service);

    const auto result = tp.ApplyCommand(TradingPostCommand::ConfirmTrade, session);
    REQUIRE(result.statusText.find("not available") != std::string::npos);
    REQUIRE(session.Snapshot().gold == 2500);
}

TEST_CASE("TradingPostInteraction - a hostile-occupied post offers no trades") {
    const auto service = MakeTradingPost("tp", "loc");
    auto session = MakeSession({service}, {Owned("tp", "Green")});

    gameplay::EnemyTeamState enemy;
    enemy.teamColor = "Red";
    enemy.nodeId = "loc";  // occupies the Trading Post's node
    enemy.active = true;
    session.SetEnemyTeams({enemy});

    TradingPostInteraction tp;
    tp.Open(session, service);

    REQUIRE(tp.BuildPromptText(session).find("not available") != std::string::npos);
    const auto result = tp.ApplyCommand(TradingPostCommand::ConfirmTrade, session);
    REQUIRE(result.statusText.find("not available") != std::string::npos);
    REQUIRE(session.Snapshot().gold == 2500);
}

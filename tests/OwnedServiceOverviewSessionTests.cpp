#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

#include "core/GameClock.h"
#include "core/SaveGame.h"
#include "data/definitions/LocationServiceDefinition.h"
#include "data/definitions/UnitDefinition.h"
#include "gameplay/GameSession.h"
#include "gameplay/ResourceState.h"

// M27 Slice 1 — GameSession seam: the transient owned-service overview mode's
// persistence policy (NOT a round-trip) and the read-only PreviewMineDailyOutput,
// which must combine base + strongest-only stationed mine_production exactly like
// ApplyDailyMinePayout.

using data::LocationServiceKind;
using gameplay::GameMode;
using gameplay::ResourceType;

namespace {

constexpr int kOneDay = core::GameClock::kMinutesPerSliceDay;

data::UnitDefinition MakeUnit(const std::string& id) {
    data::UnitDefinition u;
    u.id = id;
    u.name = id;
    u.category = data::UnitDefinitionCategory::Generic;
    return u;
}

data::UnitDefinition MakeMiner(const std::string& id, const std::string& resource, int amount) {
    auto u = MakeUnit(id);
    u.passiveEffects.push_back(data::UnitPassiveEffect{
        data::PassiveEffectKind::MineProduction, resource, "mine", amount});
    return u;
}

data::LocationServiceDefinition MakeMine(const std::string& id, const std::string& locationId,
    std::vector<data::MineOutputDefinition> outputs) {
    data::LocationServiceDefinition d;
    d.id = id;
    d.locationId = locationId;
    d.zoneId = "z";
    d.kind = LocationServiceKind::Mine;
    d.mineOutputs = std::move(outputs);
    return d;
}

data::LocationServiceDefinition MakeTradingPost(const std::string& id, const std::string& locationId) {
    data::LocationServiceDefinition d;
    d.id = id;
    d.locationId = locationId;
    d.zoneId = "z";
    d.kind = LocationServiceKind::TradingPost;
    return d;
}

// stk_1 stonewright(+2 Stone) active, stk_2 kobold(+1 Stone) reserve, stk_3 plain reserve.
core::SaveData BaseSave() {
    core::SaveData s;
    s.schemaVersion = 5;
    s.day = 1;
    s.gold = 100;
    s.mode = "overworld_mode";
    s.regionId = "r1";
    s.destinationId = "mine_node";
    s.hasCanonicalRoster = true;
    s.rosterStacks = {
        core::RosterStackSaveState{"stk_1", "stonewright", 1},
        core::RosterStackSaveState{"stk_2", "kobold", 1},
        core::RosterStackSaveState{"stk_3", "plain", 1},
    };
    s.activeSlotStackIds = {"stk_1", "", "", "", ""};
    s.reserveSlotStackIds = {"stk_2", "stk_3", "", "", "", "", "", ""};
    s.nextStackIdCounter = 4;
    return s;
}

gameplay::GameSession MakeSession(
    std::vector<data::LocationServiceDefinition> services,
    std::vector<core::OwnedServiceSaveState> ownedServices,
    core::SaveData save = BaseSave()) {
    save.ownedServices = std::move(ownedServices);
    gameplay::GameSession session;
    session.SetPlayerColor("Green");
    session.SetUnitCatalog({
        MakeMiner("stonewright", "Stone", 2),
        MakeMiner("kobold", "Stone", 1),
        MakeUnit("plain"),
    });
    session.SetLocationServiceCatalog(std::move(services));
    session.ApplySaveData(save);
    return session;
}

core::OwnedServiceSaveState Owned(const std::string& id, const std::string& owner,
    bool locked, bool destroyed, std::vector<core::StationedUnitSaveState> stationed = {}) {
    return core::OwnedServiceSaveState{id, owner, locked, destroyed, std::move(stationed)};
}

int AmountOf(const std::vector<gameplay::economy::MineResourceOutput>& outputs, ResourceType r) {
    for (const auto& o : outputs) {
        if (o.resource == r) return o.amount;
    }
    return 0;
}

} // namespace

TEST_CASE("Overview mode - ToString is diagnostic only and FromString self-heals to Region") {
    REQUIRE(gameplay::GameSession::ToString(GameMode::OwnedServiceOverviewMode) ==
            "owned_service_overview");
    REQUIRE(gameplay::GameSession::FromString("owned_service_overview") == GameMode::RegionMode);
    // Deliberately NOT a round-trip: ToString -> FromString resolves to Region.
    REQUIRE(gameplay::GameSession::FromString(
                gameplay::GameSession::ToString(GameMode::OwnedServiceOverviewMode)) ==
            GameMode::RegionMode);
}

TEST_CASE("Overview mode - enter/exit toggles between the panel and Region") {
    auto session = MakeSession({MakeMine("mine_svc", "mine_node", {{"Stone", 2}})},
                               {Owned("mine_svc", "Green", false, false)});
    REQUIRE(session.Snapshot().mode == GameMode::RegionMode);
    session.EnterOwnedServiceOverviewMode();
    REQUIRE(session.Snapshot().mode == GameMode::OwnedServiceOverviewMode);
    session.ExitOwnedServiceOverviewMode();
    REQUIRE(session.Snapshot().mode == GameMode::RegionMode);
}

TEST_CASE("Overview mode - a save carrying the overview mode loads as Region, no schema bump") {
    auto save = BaseSave();
    save.mode = "owned_service_overview";  // as if hand-edited / corrupt
    save.ownedServices = {Owned("mine_svc", "Green", false, false)};

    auto session = MakeSession({MakeMine("mine_svc", "mine_node", {{"Stone", 2}})}, {}, save);

    REQUIRE(session.Snapshot().mode == GameMode::RegionMode);  // self-healed
    REQUIRE(session.ToSaveData().schemaVersion == 5);          // no bump
}

TEST_CASE("Overview preview - base only when nothing is stationed") {
    auto session = MakeSession({MakeMine("mine_svc", "mine_node", {{"Stone", 2}, {"Gold", 1000}})},
                               {Owned("mine_svc", "Green", false, false)});

    const auto preview = session.PreviewMineDailyOutput("mine_svc");
    REQUIRE(AmountOf(preview, ResourceType::Stone) == 2);
    REQUIRE(AmountOf(preview, ResourceType::Gold) == 1000);
}

TEST_CASE("Overview preview - equals the daily payout delta for a payable stationed mine") {
    auto session = MakeSession(
        {MakeMine("mine_svc", "mine_node", {{"Stone", 2}, {"Gold", 1000}})},
        {Owned("mine_svc", "Green", false, false,
            {core::StationedUnitSaveState{"stonewright", "stk_1"}})});  // +2 Stone

    const auto preview = session.PreviewMineDailyOutput("mine_svc");
    REQUIRE(AmountOf(preview, ResourceType::Stone) == 4);  // 2 base + 2 strongest
    REQUIRE(AmountOf(preview, ResourceType::Gold) == 1000);

    const int goldBefore = session.Snapshot().gold;
    const int stoneBefore = session.ResourceCount(ResourceType::Stone);
    session.AddMinutes(kOneDay);  // one daily payout

    REQUIRE(session.ResourceCount(ResourceType::Stone) - stoneBefore ==
            AmountOf(preview, ResourceType::Stone));
    REQUIRE(session.Snapshot().gold - goldBefore == AmountOf(preview, ResourceType::Gold));
}

TEST_CASE("Overview preview - strongest-only boost does not stack") {
    auto session = MakeSession(
        {MakeMine("mine_svc", "mine_node", {{"Stone", 2}})},
        {Owned("mine_svc", "Green", false, false,
            {
                core::StationedUnitSaveState{"stonewright", "stk_1"},  // +2
                core::StationedUnitSaveState{"kobold", "stk_2"}        // +1
            })});

    const auto preview = session.PreviewMineDailyOutput("mine_svc");
    REQUIRE(AmountOf(preview, ResourceType::Stone) == 4);  // 2 + strongest 2, not +2+1
}

TEST_CASE("Overview preview - shows production potential even while locked (status is separate)") {
    // Deliberately ungated: the preview reports what the mine produces; lock/destroy/
    // hostile occupation is surfaced as status, not folded into the output number.
    auto session = MakeSession({MakeMine("mine_svc", "mine_node", {{"Stone", 2}})},
                               {Owned("mine_svc", "Green", /*locked=*/true, false)});

    const auto preview = session.PreviewMineDailyOutput("mine_svc");
    REQUIRE(AmountOf(preview, ResourceType::Stone) == 2);  // potential, not zero

    // ...but a locked mine pays nothing, so the daily delta is zero (gate applies to payout).
    const int stoneBefore = session.ResourceCount(ResourceType::Stone);
    session.AddMinutes(kOneDay);
    REQUIRE(session.ResourceCount(ResourceType::Stone) == stoneBefore);
}

TEST_CASE("Overview preview - empty for a non-mine or unknown service") {
    auto session = MakeSession({MakeTradingPost("tp_svc", "tp_node")},
                               {Owned("tp_svc", "Green", false, false)});
    REQUIRE(session.PreviewMineDailyOutput("tp_svc").empty());
    REQUIRE(session.PreviewMineDailyOutput("no_such_svc").empty());
}

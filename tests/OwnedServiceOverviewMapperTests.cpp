#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include "app/mappers/OwnedServiceOverviewModelMapper.h"
#include "core/SaveGame.h"
#include "data/ContentRepository.h"
#include "gameplay/GameSession.h"

// M27 Slice 2 — the owned-service overview MAPPER assembles a read-only model from
// existing GameSession accessors against the shipped content directory. It lists
// only player-owned services and never mutates ownership/stationing/payout.

#ifndef LOP01_PROJECT_ROOT
#define LOP01_PROJECT_ROOT "."
#endif

namespace {

std::filesystem::path RealContentDir() {
    return std::filesystem::path(LOP01_PROJECT_ROOT) / "content";
}

// A roster with the Player Character active and a Tunnel Miner (Steel +1) in
// reserve so a stationed-miner ref is stack-backed. Caller fills ownedServices.
core::SaveData BaseSave() {
    core::SaveData s;
    s.schemaVersion = 5;
    s.day = 1;
    s.gold = 500;
    s.mode = "overworld_mode";
    s.regionId = "ashvale_heartland";
    s.destinationId = "home_base";
    s.hasCanonicalRoster = true;
    s.rosterStacks = {
        core::RosterStackSaveState{"stk_1", "hero_player", 1},
        core::RosterStackSaveState{"stk_2", "unit_miner", 1},
    };
    s.activeSlotStackIds = {"stk_1", "", "", "", ""};
    s.reserveSlotStackIds = {"stk_2", "", "", "", "", "", "", ""};
    s.nextStackIdCounter = 3;
    return s;
}

gameplay::GameSession WireSession(const data::ContentRepository& repo, const core::SaveData& save) {
    gameplay::GameSession session;
    session.SetPlayerColor("Green");
    session.SetUnitCatalog(repo.Units());
    session.SetRegionCatalog(repo.Regions());
    session.SetLocationServiceCatalog(repo.LocationServices());
    session.SetTraderCurveCatalog(repo.TraderCurves());
    session.ApplySaveData(save);
    return session;
}

core::OwnedServiceSaveState Owned(const std::string& id, const std::string& owner,
    bool locked, bool destroyed, std::vector<core::StationedUnitSaveState> stationed = {}) {
    return core::OwnedServiceSaveState{id, owner, locked, destroyed, std::move(stationed)};
}

const ashvale::rendering::OwnedServiceRowView* FindRow(
    const ashvale::rendering::OwnedServiceOverviewModel& model, const std::string& serviceId) {
    const auto it = std::find_if(model.rows.begin(), model.rows.end(),
        [&](const ashvale::rendering::OwnedServiceRowView& r) { return r.serviceId == serviceId; });
    return it == model.rows.end() ? nullptr : &*it;
}

bool HasOutputLine(const ashvale::rendering::OwnedServiceRowView& row, const std::string& line) {
    return std::find(row.outputLines.begin(), row.outputLines.end(), line) != row.outputLines.end();
}

} // namespace

TEST_CASE("Overview mapper - lists only player-owned ownable services") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));
    auto save = BaseSave();
    save.ownedServices = {
        Owned("home_base_trading_post", "Green", false, false),
        Owned("iron_mine_svc", "Green", false, false),
        Owned("copper_mine_svc", "Red", false, false),  // enemy-owned -> excluded
    };
    auto session = WireSession(repo, save);

    app::mappers::OwnedServiceOverviewModelMapper mapper;
    const auto model = mapper.Map(repo, session);

    REQUIRE(model.rows.size() == 2);
    REQUIRE(FindRow(model, "home_base_trading_post") != nullptr);
    REQUIRE(FindRow(model, "iron_mine_svc") != nullptr);
    REQUIRE(FindRow(model, "copper_mine_svc") == nullptr);  // not player-owned
}

TEST_CASE("Overview mapper - mine row shows name, stationed unit, and boosted output preview") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));
    auto save = BaseSave();
    save.ownedServices = {
        Owned("iron_mine_svc", "Green", false, false,
            {core::StationedUnitSaveState{"unit_miner", "stk_2"}}),  // Steel +1
    };
    auto session = WireSession(repo, save);

    app::mappers::OwnedServiceOverviewModelMapper mapper;
    const auto model = mapper.Map(repo, session);
    const auto* row = FindRow(model, "iron_mine_svc");
    REQUIRE(row != nullptr);
    REQUIRE(row->isMine);
    REQUIRE(row->displayName == "Steel Mine");
    REQUIRE(row->kindLabel == "Mine");
    REQUIRE(row->statusLabel == "Owned");
    REQUIRE(row->stationedCapacity == 5);
    REQUIRE(row->stationedCount == 1);
    REQUIRE(row->stationedUnitNames.size() == 1);
    REQUIRE(row->stationedUnitNames[0] == "Tunnel Miner");
    REQUIRE(HasOutputLine(*row, "Steel: 2 (+1) = 3"));  // base 2 + strongest-only 1
    REQUIRE(HasOutputLine(*row, "Gold: 200"));
}

TEST_CASE("Overview mapper - trading post row shows kind, owner, and ownership tier") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));
    auto save = BaseSave();
    save.ownedServices = {Owned("home_base_trading_post", "Green", false, false)};
    auto session = WireSession(repo, save);

    app::mappers::OwnedServiceOverviewModelMapper mapper;
    const auto model = mapper.Map(repo, session);
    const auto* row = FindRow(model, "home_base_trading_post");
    REQUIRE(row != nullptr);
    REQUIRE(row->isTrader);
    REQUIRE(row->kindLabel == "Trading Post");
    REQUIRE(row->statusLabel == "Owned");
    REQUIRE(row->displayName == "Home Base");
    REQUIRE(row->traderTier == session.OwnedTraderServiceTierForService("home_base_trading_post"));
    REQUIRE(row->traderTier >= 1);  // owning one TP grants at least tier 1
}

TEST_CASE("Overview mapper - locked/destroyed status is shown without mutating state") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));
    auto save = BaseSave();
    save.ownedServices = {
        Owned("iron_mine_svc", "Green", /*locked=*/true, false),
        Owned("copper_mine_svc", "Green", false, /*destroyed=*/true),
    };
    auto session = WireSession(repo, save);

    const auto before = session.OwnedServices();  // copy
    app::mappers::OwnedServiceOverviewModelMapper mapper;
    const auto model = mapper.Map(repo, session);

    REQUIRE(FindRow(model, "iron_mine_svc")->statusLabel == "Owned (Locked)");
    REQUIRE(FindRow(model, "copper_mine_svc")->statusLabel == "Owned (Destroyed)");

    // Pure read: the mapper changed nothing.
    const auto& after = session.OwnedServices();
    REQUIRE(after.size() == before.size());
    for (std::size_t i = 0; i < after.size(); ++i) {
        REQUIRE(after[i].serviceId == before[i].serviceId);
        REQUIRE(after[i].ownerTeamColor == before[i].ownerTeamColor);
        REQUIRE(after[i].locked == before[i].locked);
        REQUIRE(after[i].destroyed == before[i].destroyed);
        REQUIRE(after[i].stationedUnits.size() == before[i].stationedUnits.size());
    }
}

TEST_CASE("Overview mapper - empty model when the player owns nothing") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));
    auto session = WireSession(repo, BaseSave());  // no ownedServices

    app::mappers::OwnedServiceOverviewModelMapper mapper;
    const auto model = mapper.Map(repo, session);
    REQUIRE(model.rows.empty());
    REQUIRE(model.selectedIndex == 0);
    REQUIRE_FALSE(model.emptyText.empty());
}

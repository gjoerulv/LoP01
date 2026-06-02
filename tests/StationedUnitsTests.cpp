#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <vector>

#include "core/SaveGame.h"
#include "data/definitions/LocationServiceDefinition.h"
#include "data/definitions/UnitDefinition.h"
#include "gameplay/GameSession.h"
#include "gameplay/economy/MineProductionRules.h"

using gameplay::ResourceType;
using gameplay::economy::MineProductionPassive;

namespace {

data::UnitDefinition MakeUnit(const std::string& id,
    data::UnitDefinitionCategory category) {
    data::UnitDefinition u;
    u.id = id;
    u.name = id;
    u.category = category;
    return u;
}

data::UnitDefinition MakeUnitWithPassive(const std::string& id,
    data::UnitDefinitionCategory category, const std::string& resource, int amount) {
    auto u = MakeUnit(id, category);
    data::UnitMineProductionPassive p;
    p.target = "mine";
    p.resource = resource;
    p.amount = amount;
    u.mineProductionPassive = p;
    return u;
}

// A minimally-valid canonical save: roster with the test units, slots sized to
// the canonical 5/8 layout, mode region. Caller fills ownedServices.
//   stk_1 -> hero_smith (Hero, +250 Gold)
//   stk_2 -> kobold     (Generic, +1 Stone)
//   stk_3 -> plain      (Generic, no passive)
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
    s.rosterStacks = {
        core::RosterStackSaveState{"stk_1", "hero_smith", 1},
        core::RosterStackSaveState{"stk_2", "kobold", 3},
        core::RosterStackSaveState{"stk_3", "plain", 2}
    };
    s.activeSlotStackIds = {"stk_1", "", "", "", ""};
    s.reserveSlotStackIds = {"stk_2", "stk_3", "", "", "", "", "", ""};
    s.nextStackIdCounter = 4;
    return s;
}

// Catalog includes "ghost_smith": a unit that carries a passive but has NO
// roster stack in MakeBaseSave — used to prove catalog-only units cannot
// station or contribute.
std::vector<data::UnitDefinition> MakeCatalog() {
    return {
        MakeUnitWithPassive("hero_smith", data::UnitDefinitionCategory::Hero, "Gold", 250),
        MakeUnitWithPassive("kobold", data::UnitDefinitionCategory::Generic, "Stone", 1),
        MakeUnit("plain", data::UnitDefinitionCategory::Generic),
        MakeUnitWithPassive("ghost_smith", data::UnitDefinitionCategory::Hero, "Gold", 999)
    };
}

bool HasPassive(const std::vector<MineProductionPassive>& list,
    ResourceType resource, int amount) {
    return std::ranges::any_of(list, [&](const MineProductionPassive& p) {
        return p.resource == resource && p.amount == amount;
    });
}

gameplay::GameSession LoadWith(std::vector<core::OwnedServiceSaveState> ownedServices) {
    auto save = MakeBaseSave();
    save.ownedServices = std::move(ownedServices);
    gameplay::GameSession session;
    session.SetUnitCatalog(MakeCatalog());
    session.ApplySaveData(save);
    return session;
}

} // namespace

TEST_CASE("Stationing - valid stack-backed hero ref survives and contributes its passive") {
    auto session = LoadWith({
        core::OwnedServiceSaveState{"stone_mine_svc", "Green", false, false,
            {core::StationedUnitSaveState{"hero_smith", "stk_1"}}}
    });

    const auto* owned = session.FindOwnedService("stone_mine_svc");
    REQUIRE(owned != nullptr);
    REQUIRE(owned->stationedUnits.size() == 1);

    const auto passives = session.CollectStationedMineProductionPassives(
        "stone_mine_svc", data::LocationServiceKind::Mine);
    REQUIRE(passives.size() == 1);
    REQUIRE(HasPassive(passives, ResourceType::Gold, 250));
}

TEST_CASE("Stationing - valid stack-backed generic ref survives and contributes its passive") {
    auto session = LoadWith({
        core::OwnedServiceSaveState{"stone_mine_svc", "Green", false, false,
            {core::StationedUnitSaveState{"kobold", "stk_2"}}}
    });

    const auto* owned = session.FindOwnedService("stone_mine_svc");
    REQUIRE(owned != nullptr);
    REQUIRE(owned->stationedUnits.size() == 1);

    const auto passives = session.CollectStationedMineProductionPassives(
        "stone_mine_svc", data::LocationServiceKind::Mine);
    REQUIRE(passives.size() == 1);
    REQUIRE(HasPassive(passives, ResourceType::Stone, 1));
}

TEST_CASE("Stationing - generic ref with valid unitId but empty stackId is dropped and contributes nothing") {
    auto session = LoadWith({
        core::OwnedServiceSaveState{"stone_mine_svc", "Green", false, false,
            {core::StationedUnitSaveState{"kobold", ""}}}  // not stack-backed
    });

    const auto* owned = session.FindOwnedService("stone_mine_svc");
    REQUIRE(owned != nullptr);
    REQUIRE(owned->stationedUnits.empty());

    const auto passives = session.CollectStationedMineProductionPassives(
        "stone_mine_svc", data::LocationServiceKind::Mine);
    REQUIRE(passives.empty());
}

TEST_CASE("Stationing - catalog-only unit with no roster stack is dropped and contributes nothing") {
    // ghost_smith exists in the catalog (and carries a +999 Gold passive) but
    // has no roster stack. Neither an empty stackId nor a fabricated stackId may
    // let it station or contribute — this is the rejected-injection scenario.
    auto session = LoadWith({
        core::OwnedServiceSaveState{"stone_mine_svc", "Green", false, false,
            {
                core::StationedUnitSaveState{"ghost_smith", ""},
                core::StationedUnitSaveState{"ghost_smith", "stk_404"}
            }}
    });

    const auto* owned = session.FindOwnedService("stone_mine_svc");
    REQUIRE(owned != nullptr);
    REQUIRE(owned->stationedUnits.empty());

    const auto passives = session.CollectStationedMineProductionPassives(
        "stone_mine_svc", data::LocationServiceKind::Mine);
    REQUIRE(passives.empty());
    REQUIRE_FALSE(HasPassive(passives, ResourceType::Gold, 999));
}

TEST_CASE("Stationing - mismatched stackId/unitId is dropped") {
    // stk_1 is hero_smith, not kobold -> mismatch.
    auto session = LoadWith({
        core::OwnedServiceSaveState{"stone_mine_svc", "Green", false, false,
            {core::StationedUnitSaveState{"kobold", "stk_1"}}}
    });

    const auto* owned = session.FindOwnedService("stone_mine_svc");
    REQUIRE(owned != nullptr);
    REQUIRE(owned->stationedUnits.empty());
}

TEST_CASE("Stationing - unknown unitId is dropped") {
    // stk_2 is kobold; ref claims an unknown unit at that stack -> mismatch/drop.
    auto session = LoadWith({
        core::OwnedServiceSaveState{"stone_mine_svc", "Green", false, false,
            {core::StationedUnitSaveState{"ghost_unit", "stk_2"}}}
    });

    const auto* owned = session.FindOwnedService("stone_mine_svc");
    REQUIRE(owned != nullptr);
    REQUIRE(owned->stationedUnits.empty());
}

TEST_CASE("Stationing - missing stackId is dropped") {
    auto session = LoadWith({
        core::OwnedServiceSaveState{"stone_mine_svc", "Green", false, false,
            {core::StationedUnitSaveState{"kobold", "stk_999"}}}  // no such stack
    });

    const auto* owned = session.FindOwnedService("stone_mine_svc");
    REQUIRE(owned != nullptr);
    REQUIRE(owned->stationedUnits.empty());
}

TEST_CASE("Stationing - valid refs round-trip and survive normalization") {
    auto session = LoadWith({
        core::OwnedServiceSaveState{"stone_mine_svc", "Green", false, false,
            {
                core::StationedUnitSaveState{"hero_smith", "stk_1"},
                core::StationedUnitSaveState{"kobold", "stk_2"}
            }}
    });

    const auto out = session.ToSaveData();
    REQUIRE(out.ownedServices.size() == 1);
    REQUIRE(out.ownedServices[0].stationedUnits.size() == 2);
    REQUIRE(out.ownedServices[0].stationedUnits[0].unitId == "hero_smith");
    REQUIRE(out.ownedServices[0].stationedUnits[0].stackId == "stk_1");
    REQUIRE(out.ownedServices[0].stationedUnits[1].unitId == "kobold");
    REQUIRE(out.ownedServices[0].stationedUnits[1].stackId == "stk_2");
}

TEST_CASE("Stationing - legacy/Phase-1 owned service with no stationing loads empty") {
    auto session = LoadWith({
        core::OwnedServiceSaveState{"stone_mine_svc", "Green", false, false, {}}
    });

    const auto* owned = session.FindOwnedService("stone_mine_svc");
    REQUIRE(owned != nullptr);
    REQUIRE(owned->stationedUnits.empty());
}

TEST_CASE("Stationing - CollectStationedMineProductionPassives resolves hero and generic, ignores no-passive units") {
    auto session = LoadWith({
        core::OwnedServiceSaveState{"stone_mine_svc", "Green", false, false,
            {
                core::StationedUnitSaveState{"hero_smith", "stk_1"},
                core::StationedUnitSaveState{"kobold", "stk_2"},
                core::StationedUnitSaveState{"plain", "stk_3"}  // no passive
            }}
    });

    const auto passives = session.CollectStationedMineProductionPassives(
        "stone_mine_svc", data::LocationServiceKind::Mine);

    REQUIRE(passives.size() == 2);
    REQUIRE(HasPassive(passives, ResourceType::Gold, 250));
    REQUIRE(HasPassive(passives, ResourceType::Stone, 1));
}

TEST_CASE("Stationing - collecting from an unknown service yields no passives") {
    gameplay::GameSession session;
    session.SetUnitCatalog(MakeCatalog());

    const auto passives = session.CollectStationedMineProductionPassives(
        "no_such_service", data::LocationServiceKind::Mine);

    REQUIRE(passives.empty());
}

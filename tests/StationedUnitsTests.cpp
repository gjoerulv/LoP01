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

// A minimally-valid canonical save: roster with the three test units, slots
// sized to the canonical 5/8 layout, mode region. Caller fills ownedServices.
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

std::vector<data::UnitDefinition> MakeCatalog() {
    return {
        MakeUnitWithPassive("hero_smith", data::UnitDefinitionCategory::Hero, "Gold", 250),
        MakeUnitWithPassive("kobold", data::UnitDefinitionCategory::Generic, "Stone", 1),
        MakeUnit("plain", data::UnitDefinitionCategory::Generic)  // no passive
    };
}

bool HasPassive(const std::vector<MineProductionPassive>& list,
    ResourceType resource, int amount) {
    return std::ranges::any_of(list, [&](const MineProductionPassive& p) {
        return p.resource == resource && p.amount == amount;
    });
}

} // namespace

TEST_CASE("Stationing - valid stationed refs round-trip and survive normalization") {
    auto save = MakeBaseSave();
    save.ownedServices = {
        core::OwnedServiceSaveState{"stone_mine_svc", "Green", false, false,
            {
                core::StationedUnitSaveState{"hero_smith", "stk_1"},
                core::StationedUnitSaveState{"kobold", "stk_2"}
            }}
    };

    gameplay::GameSession session;
    session.SetUnitCatalog(MakeCatalog());
    session.ApplySaveData(save);

    const auto* owned = session.FindOwnedService("stone_mine_svc");
    REQUIRE(owned != nullptr);
    REQUIRE(owned->stationedUnits.size() == 2);

    // Re-serialize: stationing survives the round-trip.
    const auto out = session.ToSaveData();
    REQUIRE(out.ownedServices.size() == 1);
    REQUIRE(out.ownedServices[0].stationedUnits.size() == 2);
    REQUIRE(out.ownedServices[0].stationedUnits[0].unitId == "hero_smith");
    REQUIRE(out.ownedServices[0].stationedUnits[0].stackId == "stk_1");
}

TEST_CASE("Stationing - legacy/Phase-1 owned service with no stationing loads empty") {
    auto save = MakeBaseSave();
    save.ownedServices = {
        core::OwnedServiceSaveState{"stone_mine_svc", "Green", false, false, {}}
    };

    gameplay::GameSession session;
    session.SetUnitCatalog(MakeCatalog());
    session.ApplySaveData(save);

    const auto* owned = session.FindOwnedService("stone_mine_svc");
    REQUIRE(owned != nullptr);
    REQUIRE(owned->stationedUnits.empty());
}

TEST_CASE("Stationing - stale hero/unit ref (unknown unitId) is dropped on load") {
    auto save = MakeBaseSave();
    save.ownedServices = {
        core::OwnedServiceSaveState{"stone_mine_svc", "Green", false, false,
            {
                core::StationedUnitSaveState{"ghost_unit", ""},     // unknown unit -> drop
                core::StationedUnitSaveState{"hero_smith", "stk_1"} // valid -> keep
            }}
    };

    gameplay::GameSession session;
    session.SetUnitCatalog(MakeCatalog());
    session.ApplySaveData(save);

    const auto* owned = session.FindOwnedService("stone_mine_svc");
    REQUIRE(owned != nullptr);
    REQUIRE(owned->stationedUnits.size() == 1);
    REQUIRE(owned->stationedUnits[0].unitId == "hero_smith");
}

TEST_CASE("Stationing - stale generic stack ref (missing or mismatched stackId) is dropped on load") {
    auto save = MakeBaseSave();
    save.ownedServices = {
        core::OwnedServiceSaveState{"stone_mine_svc", "Green", false, false,
            {
                core::StationedUnitSaveState{"kobold", "stk_999"},  // stack missing -> drop
                core::StationedUnitSaveState{"kobold", "stk_1"},    // stk_1 is hero_smith -> mismatch -> drop
                core::StationedUnitSaveState{"kobold", "stk_2"}     // valid -> keep
            }}
    };

    gameplay::GameSession session;
    session.SetUnitCatalog(MakeCatalog());
    session.ApplySaveData(save);

    const auto* owned = session.FindOwnedService("stone_mine_svc");
    REQUIRE(owned != nullptr);
    REQUIRE(owned->stationedUnits.size() == 1);
    REQUIRE(owned->stationedUnits[0].unitId == "kobold");
    REQUIRE(owned->stationedUnits[0].stackId == "stk_2");
}

TEST_CASE("Stationing - CollectStationedMineProductionPassives resolves hero and generic passives") {
    auto save = MakeBaseSave();
    save.ownedServices = {
        core::OwnedServiceSaveState{"stone_mine_svc", "Green", false, false,
            {
                core::StationedUnitSaveState{"hero_smith", "stk_1"},
                core::StationedUnitSaveState{"kobold", "stk_2"},
                core::StationedUnitSaveState{"plain", "stk_3"}  // no passive -> contributes nothing
            }}
    };

    gameplay::GameSession session;
    session.SetUnitCatalog(MakeCatalog());
    session.ApplySaveData(save);

    const auto passives = session.CollectStationedMineProductionPassives(
        "stone_mine_svc", data::LocationServiceKind::Mine);

    REQUIRE(passives.size() == 2);
    REQUIRE(HasPassive(passives, ResourceType::Gold, 250));   // hero
    REQUIRE(HasPassive(passives, ResourceType::Stone, 1));    // generic
}

TEST_CASE("Stationing - collecting from an unknown service yields no passives") {
    gameplay::GameSession session;
    session.SetUnitCatalog(MakeCatalog());

    const auto passives = session.CollectStationedMineProductionPassives(
        "no_such_service", data::LocationServiceKind::Mine);

    REQUIRE(passives.empty());
}

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <string>
#include <vector>

#include "core/GameClock.h"
#include "core/SaveGame.h"
#include "data/definitions/LocationServiceDefinition.h"
#include "data/definitions/RegionDefinition.h"
#include "data/definitions/UnitDefinition.h"
#include "data/definitions/WorldMapDefinition.h"
#include "gameplay/GameSession.h"

// M29 cross-Region generic-unit preservation (docs/core_loop_rules.md §5): only
// traveling (slotted) generic stacks are lost on confirmed World Map travel.
// Stationed (M25) and stored (M28) stacks are not traveling and must survive
// with their refs intact; heroes and the Player Character always travel.
// Regression intent: the M15-era removal iterated the whole roster and would
// have deleted stored/stationed stacks once those placements existed.

namespace {

using data::LocationServiceKind;

data::RegionNodeDefinition Node(const std::string& id) {
    data::RegionNodeDefinition n;
    n.locationId = id;
    return n;
}

data::UnitDefinition Unit(const std::string& id, data::UnitDefinitionCategory cat,
    int agility, bool isPlayerCharacter = false) {
    data::UnitDefinition def;
    def.id = id;
    def.name = id;
    def.category = cat;
    def.stats.agility = agility;
    def.isPlayerCharacter = isPlayerCharacter;
    return def;
}

data::LocationServiceDefinition MakeService(const std::string& id, LocationServiceKind kind) {
    data::LocationServiceDefinition svc;
    svc.id = id;
    svc.kind = kind;
    return svc;
}

// Two-region world ("ashvale_heartland" <-> "riverside_vale"), both with an exit
// node, so travel can round-trip. Roster via save data: PC + hero + a generic
// stack in active, a generic stack in reserve, one generic stationed at an owned
// mine, and one generic stored at an owned storage.
core::SaveData MakeBaseSave() {
    core::SaveData s;
    s.schemaVersion = 5;
    s.day = 1;
    s.mode = "overworld_mode";  // serialized RegionMode (GameSession::ToString)
    s.regionId = "ashvale_heartland";
    s.destinationId = "home_exit";
    s.hasCanonicalRoster = true;
    s.rosterStacks = {
        core::RosterStackSaveState{"stk_pc", "pc_hero", 1},
        core::RosterStackSaveState{"stk_hero", "hero_b", 1},
        core::RosterStackSaveState{"stk_sword", "swordsman", 3},
        core::RosterStackSaveState{"stk_arch", "archer", 2},
        core::RosterStackSaveState{"stk_mine", "miner", 4},
        core::RosterStackSaveState{"stk_store", "swordsman", 5},
    };
    s.activeSlotStackIds = {"stk_pc", "stk_hero", "stk_sword", "", ""};
    s.reserveSlotStackIds = {"stk_arch", "", "", "", "", "", "", ""};
    s.nextStackIdCounter = 7;
    s.ownedServices = {
        core::OwnedServiceSaveState{"steel_mine", "Green", false, false,
            {core::StationedUnitSaveState{"miner", "stk_mine"}}, {}},
        core::OwnedServiceSaveState{"home_storage", "Green", false, false,
            {}, {core::StoredUnitSaveState{"swordsman", "stk_store"}}},
    };
    return s;
}

gameplay::GameSession Load(const core::SaveData& save,
    std::vector<data::UnitDefinition> catalog = {
        Unit("pc_hero", data::UnitDefinitionCategory::Leader, 8, /*isPlayerCharacter=*/true),
        Unit("hero_b", data::UnitDefinitionCategory::Hero, 7),
        Unit("swordsman", data::UnitDefinitionCategory::Generic, 5),
        Unit("archer", data::UnitDefinitionCategory::Generic, 4),
        Unit("miner", data::UnitDefinitionCategory::Generic, 6),
    }) {
    gameplay::GameSession session;

    data::RegionDefinition home;
    home.id = "ashvale_heartland";
    home.arrivalNodeId = "home_arrival";
    home.nodes = { Node("home_arrival"), Node("home_exit") };

    data::RegionDefinition vale;
    vale.id = "riverside_vale";
    vale.arrivalNodeId = "vale_arrival";
    vale.nodes = { Node("vale_arrival") };

    session.SetRegionCatalog({ home, vale });

    data::WorldMapDefinition wm;
    wm.id = "world_map";
    wm.entries = {
        { "ashvale_heartland", true, { "home_exit" }, 0.0f, 0.0f },
        { "riverside_vale", true, { "vale_arrival" }, 0.0f, 0.0f },
    };
    wm.adjacency = { { "ashvale_heartland", "riverside_vale" } };
    session.SetWorldMap(wm);

    session.SetLeaderCapableUnitIds({ "pc_hero", "hero_b" });
    session.SetUnitCatalog(std::move(catalog));
    session.SetLocationServiceCatalog({
        MakeService("steel_mine", LocationServiceKind::Mine),
        MakeService("home_storage", LocationServiceKind::Storage),
    });
    session.ApplySaveData(save);
    session.ApplyDailyStartingEnergy();  // lowest slotted agility 4 -> 1400
    return session;
}

bool SlotsContain(const std::vector<std::string>& slots, const std::string& stackId) {
    return std::find(slots.begin(), slots.end(), stackId) != slots.end();
}

bool IsSlotted(const gameplay::GameSession& session, const std::string& stackId) {
    return SlotsContain(session.ActiveSlotStackIds(), stackId)
        || SlotsContain(session.ReserveSlotStackIds(), stackId);
}

// Combined invariant: every roster stack is in exactly one place — an active slot,
// a reserve slot, stationed at one service, or stored at one service.
void RequireNoOrphans(const gameplay::GameSession& session) {
    const auto& active = session.ActiveSlotStackIds();
    const auto& reserve = session.ReserveSlotStackIds();
    for (const auto& stack : session.RosterStacks()) {
        int placements = 0;
        if (SlotsContain(active, stack.stackId)) ++placements;
        if (SlotsContain(reserve, stack.stackId)) ++placements;
        for (const auto& owned : session.OwnedServices()) {
            for (const auto& ref : owned.stationedUnits) if (ref.stackId == stack.stackId) ++placements;
            for (const auto& ref : owned.storedUnits) if (ref.stackId == stack.stackId) ++placements;
        }
        INFO("stack " << stack.stackId << " placements=" << placements);
        REQUIRE(placements == 1);
    }
}

// Roll the clock to the start of the next day so a fresh departure is legal
// (arrival lands at 11:00, which is already past the departure deadline).
void AdvanceToNextDay(gameplay::GameSession& session) {
    const int minutesInto = session.Snapshot().minutesIntoSliceDay;
    session.AddMinutes(core::GameClock::kMinutesPerSliceDay - minutesInto);
}

} // namespace

TEST_CASE("RegionTravelLoss - only traveling generic stacks are removed; heroes and PC travel") {
    auto session = Load(MakeBaseSave());

    const auto result = session.TravelToRegion("riverside_vale");
    REQUIRE(result.success);
    REQUIRE(result.genericsDropped == 5);  // 3 swordsmen + 2 archers, slotted only

    // Heroes + PC still slotted.
    REQUIRE(IsSlotted(session, "stk_pc"));
    REQUIRE(IsSlotted(session, "stk_hero"));

    // Traveling generics fully removed: no slot refs, no roster stacks.
    REQUIRE_FALSE(IsSlotted(session, "stk_sword"));
    REQUIRE_FALSE(IsSlotted(session, "stk_arch"));
    REQUIRE(session.FindRosterStackById("stk_sword") == nullptr);
    REQUIRE(session.FindRosterStackById("stk_arch") == nullptr);

    // Stationed and stored generic stacks survive untouched.
    REQUIRE(session.FindRosterStackById("stk_mine") != nullptr);
    REQUIRE(session.FindRosterStackById("stk_store") != nullptr);

    // Arrival-day Energy reset reflects the post-drop traveling party only
    // (PC agility 8, hero 7 -> 1700); stored/stationed agility is not consulted.
    REQUIRE(session.MaxEnergy() == 1700);
    REQUIRE(session.Snapshot().regionId == "riverside_vale");
    RequireNoOrphans(session);
}

TEST_CASE("RegionTravelLoss - stored generic stack survives travel and is retrievable after return") {
    auto session = Load(MakeBaseSave());

    REQUIRE(session.TravelToRegion("riverside_vale").success);

    const auto* storage = session.FindOwnedService("home_storage");
    REQUIRE(storage != nullptr);
    REQUIRE(storage->storedUnits.size() == 1);
    REQUIRE(storage->storedUnits[0].stackId == "stk_store");
    REQUIRE(storage->storedUnits[0].unitId == "swordsman");

    // Round-trip home (arrival is 11:00, so depart again the next day).
    AdvanceToNextDay(session);
    session.SetDestination("vale_arrival");  // the vale's exit node
    REQUIRE(session.TravelToRegion("ashvale_heartland").success);

    // Retrieval still works: same stack id returns to reserve.
    REQUIRE(session.TryRetrieveStackFromService("home_storage", "stk_store"));
    REQUIRE(SlotsContain(session.ReserveSlotStackIds(), "stk_store"));
    REQUIRE(session.FindRosterStackById("stk_store")->quantity == 5);
    RequireNoOrphans(session);
}

TEST_CASE("RegionTravelLoss - stationed generic stack survives travel with its ref intact") {
    auto session = Load(MakeBaseSave());

    REQUIRE(session.TravelToRegion("riverside_vale").success);

    const auto* mine = session.FindOwnedService("steel_mine");
    REQUIRE(mine != nullptr);
    REQUIRE(mine->stationedUnits.size() == 1);
    REQUIRE(mine->stationedUnits[0].stackId == "stk_mine");
    REQUIRE(mine->stationedUnits[0].unitId == "miner");
    REQUIRE(session.FindRosterStackById("stk_mine")->quantity == 4);
    RequireNoOrphans(session);
}

TEST_CASE("RegionTravelLoss - preview lists exactly the traveling generic stacks and matches removal") {
    auto session = Load(MakeBaseSave());

    const auto preview = session.PreviewRegionTravelGenericLosses();
    REQUIRE(preview.size() == 2);  // active-then-reserve slot order
    REQUIRE(preview[0].stackId == "stk_sword");
    REQUIRE(preview[0].unitId == "swordsman");
    REQUIRE(preview[0].quantity == 3);
    REQUIRE(preview[1].stackId == "stk_arch");
    REQUIRE(preview[1].unitId == "archer");
    REQUIRE(preview[1].quantity == 2);
    REQUIRE(session.GenericTravelingPartyUnitCount() == 5);

    // Pure read: previewing again is identical and mutates nothing.
    REQUIRE(session.PreviewRegionTravelGenericLosses().size() == 2);
    REQUIRE(session.RosterStacks().size() == 6);

    const auto result = session.TravelToRegion("riverside_vale");
    REQUIRE(result.success);
    REQUIRE(result.genericsDropped == 5);  // exactly the previewed total

    REQUIRE(session.PreviewRegionTravelGenericLosses().empty());
    REQUIRE(session.GenericTravelingPartyUnitCount() == 0);
}

TEST_CASE("RegionTravelLoss - heroes-only traveling party travels without any loss") {
    auto save = MakeBaseSave();
    save.activeSlotStackIds = {"stk_pc", "stk_hero", "", "", ""};
    save.reserveSlotStackIds.assign(8, "");
    save.rosterStacks = {
        core::RosterStackSaveState{"stk_pc", "pc_hero", 1},
        core::RosterStackSaveState{"stk_hero", "hero_b", 1},
        core::RosterStackSaveState{"stk_mine", "miner", 4},
        core::RosterStackSaveState{"stk_store", "swordsman", 5},
    };
    auto session = Load(save);

    REQUIRE(session.PreviewRegionTravelGenericLosses().empty());
    REQUIRE(session.GenericTravelingPartyUnitCount() == 0);

    const auto result = session.TravelToRegion("riverside_vale");
    REQUIRE(result.success);
    REQUIRE(result.genericsDropped == 0);
    REQUIRE(session.FindRosterStackById("stk_mine") != nullptr);
    REQUIRE(session.FindRosterStackById("stk_store") != nullptr);
    RequireNoOrphans(session);
}

TEST_CASE("RegionTravelLoss - a mis-authored generic Player Character is never removed") {
    // Hard PC guard: even if content authors the Player Character with a generic
    // category, travel must not delete it.
    auto save = MakeBaseSave();
    save.rosterStacks = {
        core::RosterStackSaveState{"stk_pc", "rogue_pc", 1},
        core::RosterStackSaveState{"stk_hero", "hero_b", 1},
        core::RosterStackSaveState{"stk_sword", "swordsman", 3},
    };
    save.activeSlotStackIds = {"stk_hero", "", "", "", ""};
    save.reserveSlotStackIds = {"stk_pc", "stk_sword", "", "", "", "", "", ""};
    save.ownedServices.clear();
    auto session = Load(save, {
        Unit("rogue_pc", data::UnitDefinitionCategory::Generic, 6, /*isPlayerCharacter=*/true),
        Unit("hero_b", data::UnitDefinitionCategory::Hero, 7),
        Unit("swordsman", data::UnitDefinitionCategory::Generic, 5),
    });

    const auto preview = session.PreviewRegionTravelGenericLosses();
    REQUIRE(preview.size() == 1);
    REQUIRE(preview[0].stackId == "stk_sword");

    const auto result = session.TravelToRegion("riverside_vale");
    REQUIRE(result.success);
    REQUIRE(result.genericsDropped == 3);
    REQUIRE(IsSlotted(session, "stk_pc"));  // mis-authored PC survives
    REQUIRE(session.FindRosterStackById("stk_pc") != nullptr);
    REQUIRE(session.FindRosterStackById("stk_sword") == nullptr);
}

TEST_CASE("RegionTravelLoss - failed travel removes nothing, traveling or placed") {
    auto session = Load(MakeBaseSave());
    REQUIRE(session.TrySpendEnergy(500));  // 1400 -> 900, below the 1000 cost

    const auto result = session.TravelToRegion("riverside_vale");
    REQUIRE_FALSE(result.success);
    REQUIRE(result.genericsDropped == 0);

    REQUIRE(IsSlotted(session, "stk_sword"));
    REQUIRE(IsSlotted(session, "stk_arch"));
    REQUIRE(session.RosterStacks().size() == 6);
    REQUIRE(session.FindOwnedService("steel_mine")->stationedUnits.size() == 1);
    REQUIRE(session.FindOwnedService("home_storage")->storedUnits.size() == 1);
    RequireNoOrphans(session);
}

TEST_CASE("RegionTravelLoss - post-travel state round-trips through save/load with refs intact") {
    auto session = Load(MakeBaseSave());
    REQUIRE(session.TravelToRegion("riverside_vale").success);

    // Reload into a fresh session; normalization must drop nothing because the
    // placed refs still resolve to live roster stacks.
    auto restored = Load(session.ToSaveData());

    REQUIRE(restored.Snapshot().regionId == "riverside_vale");
    REQUIRE(restored.FindRosterStackById("stk_sword") == nullptr);  // loss persisted
    REQUIRE(restored.FindRosterStackById("stk_arch") == nullptr);
    REQUIRE(IsSlotted(restored, "stk_pc"));
    REQUIRE(IsSlotted(restored, "stk_hero"));
    REQUIRE(restored.FindOwnedService("steel_mine")->stationedUnits.size() == 1);
    REQUIRE(restored.FindOwnedService("home_storage")->storedUnits.size() == 1);
    REQUIRE(restored.FindRosterStackById("stk_mine") != nullptr);
    REQUIRE(restored.FindRosterStackById("stk_store") != nullptr);
    RequireNoOrphans(restored);
}

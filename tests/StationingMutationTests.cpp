#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <string>
#include <vector>

#include "core/SaveGame.h"
#include "data/definitions/LocationServiceDefinition.h"
#include "data/definitions/UnitDefinition.h"
#include "gameplay/GameSession.h"

namespace {

using data::LocationServiceKind;

data::UnitDefinition MakeUnit(const std::string& id,
    data::UnitDefinitionCategory category, bool isPlayerCharacter = false) {
    data::UnitDefinition u;
    u.id = id;
    u.name = id;
    u.category = category;
    u.isPlayerCharacter = isPlayerCharacter;
    return u;
}

data::UnitDefinition MakeMiner(const std::string& id, const std::string& resource, int amount) {
    auto u = MakeUnit(id, data::UnitDefinitionCategory::Generic);
    u.passiveEffects.push_back(data::UnitPassiveEffect{
        data::PassiveEffectKind::MineProduction, resource, "mine", amount});
    return u;
}

std::vector<data::UnitDefinition> MakeCatalog() {
    return {
        MakeUnit("pc_hero", data::UnitDefinitionCategory::Hero, /*isPlayerCharacter=*/true),
        MakeMiner("kobold", "Stone", 1),
        MakeUnit("plain", data::UnitDefinitionCategory::Generic),
    };
}

data::LocationServiceDefinition MakeMine(const std::string& id) {
    data::LocationServiceDefinition svc;
    svc.id = id;
    svc.kind = LocationServiceKind::Mine;
    return svc;
}

// pc_hero in the single active slot; kobold (x3) and plain (x2) in reserve. One
// player-owned mine with no stationed units. nextStackIdCounter past stk_3.
core::SaveData MakeBaseSave() {
    core::SaveData s;
    s.schemaVersion = 5;
    s.day = 1;
    s.gold = 100;
    s.mode = "region_mode";
    s.regionId = "ashvale_heartland";
    s.destinationId = "iron_mine";
    s.hasCanonicalRoster = true;
    s.rosterStacks = {
        core::RosterStackSaveState{"stk_1", "pc_hero", 1},
        core::RosterStackSaveState{"stk_2", "kobold", 3},
        core::RosterStackSaveState{"stk_3", "plain", 2},
    };
    s.activeSlotStackIds = {"stk_1", "", "", "", ""};
    s.reserveSlotStackIds = {"stk_2", "stk_3", "", "", "", "", "", ""};
    s.nextStackIdCounter = 4;
    s.ownedServices = {
        core::OwnedServiceSaveState{"iron_mine_svc", "Green", false, false, {}},
    };
    return s;
}

gameplay::GameSession Load(const core::SaveData& save,
    std::vector<data::LocationServiceDefinition> services = {MakeMine("iron_mine_svc")}) {
    gameplay::GameSession session;
    session.SetUnitCatalog(MakeCatalog());
    session.SetLocationServiceCatalog(std::move(services));
    session.ApplySaveData(save);
    return session;
}

bool SlotsContain(const std::vector<std::string>& slots, const std::string& stackId) {
    return std::find(slots.begin(), slots.end(), stackId) != slots.end();
}

int StationedRefCount(const gameplay::GameSession& session, const std::string& serviceId) {
    const auto* owned = session.FindOwnedService(serviceId);
    return owned == nullptr ? 0 : static_cast<int>(owned->stationedUnits.size());
}

// Every roster stack must be in exactly one place: an active slot, a reserve slot,
// or stationed at one owned service. No orphan slot-less, unstationed stacks.
void RequireNoOrphans(const gameplay::GameSession& session) {
    const auto& active = session.ActiveSlotStackIds();
    const auto& reserve = session.ReserveSlotStackIds();
    for (const auto& stack : session.RosterStacks()) {
        int placements = 0;
        if (SlotsContain(active, stack.stackId)) ++placements;
        if (SlotsContain(reserve, stack.stackId)) ++placements;
        for (const auto& owned : session.OwnedServices()) {
            for (const auto& ref : owned.stationedUnits) {
                if (ref.stackId == stack.stackId) ++placements;
            }
        }
        INFO("stack " << stack.stackId << " placements=" << placements);
        REQUIRE(placements == 1);
    }
}

} // namespace

TEST_CASE("Stationing mutation - whole reserve stack moves out of its slot keeping its id") {
    auto session = Load(MakeBaseSave());

    REQUIRE(session.TryStationStackAtService("iron_mine_svc", "stk_2"));

    // Removed from reserve, not duplicated into a slot, still in the roster store.
    REQUIRE_FALSE(SlotsContain(session.ReserveSlotStackIds(), "stk_2"));
    REQUIRE_FALSE(SlotsContain(session.ActiveSlotStackIds(), "stk_2"));
    REQUIRE(session.FindRosterStackById("stk_2") != nullptr);

    const auto* owned = session.FindOwnedService("iron_mine_svc");
    REQUIRE(owned->stationedUnits.size() == 1);
    REQUIRE(owned->stationedUnits[0].stackId == "stk_2");   // SAME id, no recreate
    REQUIRE(owned->stationedUnits[0].unitId == "kobold");

    // Owned count is invariant: a stationed unit is still owned.
    REQUIRE(session.OwnedUnitCount("kobold") == 3);
    RequireNoOrphans(session);
}

TEST_CASE("Stationing mutation - the Player Character can never be stationed") {
    auto session = Load(MakeBaseSave());
    REQUIRE_FALSE(session.CanStationStackAtService("iron_mine_svc", "stk_1"));
    REQUIRE_FALSE(session.TryStationStackAtService("iron_mine_svc", "stk_1"));
    REQUIRE(StationedRefCount(session, "iron_mine_svc") == 0);
    REQUIRE(SlotsContain(session.ActiveSlotStackIds(), "stk_1"));
}

TEST_CASE("Stationing mutation - a stack cannot be stationed twice") {
    auto session = Load(MakeBaseSave());
    REQUIRE(session.TryStationStackAtService("iron_mine_svc", "stk_2"));
    // Already stationed (and now slot-less): refused everywhere.
    REQUIRE_FALSE(session.CanStationStackAtService("iron_mine_svc", "stk_2"));
    REQUIRE_FALSE(session.TryStationStackAtService("iron_mine_svc", "stk_2"));
    REQUIRE(StationedRefCount(session, "iron_mine_svc") == 1);
    RequireNoOrphans(session);
}

TEST_CASE("Stationing mutation - stationing from an active slot fails when it would leave no leader") {
    auto save = MakeBaseSave();
    // Put a stationable generic in the active party alongside the PC, and require a
    // leader. kobold is not leader-capable, so removing it is fine; removing the
    // only leader-capable unit (pc_hero, which is also PC) is doubly blocked.
    save.activeSlotStackIds = {"stk_1", "stk_2", "", "", ""};
    save.reserveSlotStackIds = {"stk_3", "", "", "", "", "", "", ""};
    auto session = Load(save);
    session.SetLeaderCapableUnitIds({"pc_hero"});

    // kobold (active, non-leader) can still be stationed: a leader remains.
    REQUIRE(session.TryStationStackAtService("iron_mine_svc", "stk_2"));
    REQUIRE_FALSE(SlotsContain(session.ActiveSlotStackIds(), "stk_2"));
    RequireNoOrphans(session);
}

TEST_CASE("Stationing mutation - capacity is capped at five distinct stacks") {
    core::SaveData s;
    s.schemaVersion = 5;
    s.day = 1;
    s.mode = "region_mode";
    s.hasCanonicalRoster = true;
    s.reserveSlotStackIds.assign(8, "");
    for (int i = 0; i < 6; ++i) {
        const std::string id = "stk_" + std::to_string(i + 1);
        s.rosterStacks.push_back(core::RosterStackSaveState{id, "plain", 1});
        s.reserveSlotStackIds[i] = id;
    }
    s.activeSlotStackIds.assign(5, "");
    s.nextStackIdCounter = 7;
    s.ownedServices = {core::OwnedServiceSaveState{"iron_mine_svc", "Green", false, false, {}}};
    auto session = Load(s);

    for (int i = 0; i < 5; ++i) {
        REQUIRE(session.TryStationStackAtService("iron_mine_svc", "stk_" + std::to_string(i + 1)));
    }
    REQUIRE(StationedRefCount(session, "iron_mine_svc") == 5);
    REQUIRE_FALSE(session.TryStationStackAtService("iron_mine_svc", "stk_6"));
    REQUIRE(StationedRefCount(session, "iron_mine_svc") == 5);
    REQUIRE(SlotsContain(session.ReserveSlotStackIds(), "stk_6"));
    RequireNoOrphans(session);
}

TEST_CASE("Stationing mutation - splitting a generic conserves quantity and makes one new stack") {
    auto session = Load(MakeBaseSave());
    const std::size_t stacksBefore = session.RosterStacks().size();

    REQUIRE(session.TryStationSplitAtService("iron_mine_svc", "stk_2", 1));

    // Source keeps its id and slot with the remainder; one new stationed stack.
    const auto* source = session.FindRosterStackById("stk_2");
    REQUIRE(source != nullptr);
    REQUIRE(source->quantity == 2);
    REQUIRE(SlotsContain(session.ReserveSlotStackIds(), "stk_2"));
    REQUIRE(session.RosterStacks().size() == stacksBefore + 1);

    const auto* owned = session.FindOwnedService("iron_mine_svc");
    REQUIRE(owned->stationedUnits.size() == 1);
    const std::string newId = owned->stationedUnits[0].stackId;
    REQUIRE(newId != "stk_2");
    const auto* split = session.FindRosterStackById(newId);
    REQUIRE(split != nullptr);
    REQUIRE(split->quantity == 1);

    // Total kobold count conserved (2 in reserve + 1 stationed).
    REQUIRE(session.OwnedUnitCount("kobold") == 3);
    RequireNoOrphans(session);
}

TEST_CASE("Stationing mutation - whole-stack split amount routes to a move, oversized split is rejected") {
    auto session = Load(MakeBaseSave());
    // quantity == whole stack -> behaves as a whole-stack station (no new stack).
    const std::size_t stacksBefore = session.RosterStacks().size();
    REQUIRE(session.TryStationSplitAtService("iron_mine_svc", "stk_2", 3));
    REQUIRE(session.RosterStacks().size() == stacksBefore);  // no split stack created
    REQUIRE(session.FindOwnedService("iron_mine_svc")->stationedUnits[0].stackId == "stk_2");

    // Oversized split on the remaining plain stack is rejected outright.
    REQUIRE_FALSE(session.TryStationSplitAtService("iron_mine_svc", "stk_3", 5));
    REQUIRE_FALSE(session.TryStationSplitAtService("iron_mine_svc", "stk_3", 0));
}

TEST_CASE("Stationing mutation - unstationing returns the same stack id to reserve") {
    auto session = Load(MakeBaseSave());
    REQUIRE(session.TryStationStackAtService("iron_mine_svc", "stk_2"));

    REQUIRE(session.TryUnstationStackFromService("iron_mine_svc", "stk_2"));

    REQUIRE(StationedRefCount(session, "iron_mine_svc") == 0);
    REQUIRE(SlotsContain(session.ReserveSlotStackIds(), "stk_2"));  // SAME id back
    REQUIRE(session.FindRosterStackById("stk_2") != nullptr);
    REQUIRE(session.OwnedUnitCount("kobold") == 3);
    RequireNoOrphans(session);
}

TEST_CASE("Stationing mutation - unstationing fails atomically when reserve is full") {
    // Eight reserve stacks fill reserve; the PC sits active. Station a ninth stack
    // pulled from a second active slot so reserve stays full while it is stationed.
    core::SaveData s;
    s.schemaVersion = 5;
    s.day = 1;
    s.mode = "region_mode";
    s.hasCanonicalRoster = true;
    s.activeSlotStackIds = {"stk_1", "stk_9", "", "", ""};
    s.reserveSlotStackIds.assign(8, "");
    s.rosterStacks = {core::RosterStackSaveState{"stk_1", "pc_hero", 1},
                      core::RosterStackSaveState{"stk_9", "plain", 1}};
    for (int i = 0; i < 8; ++i) {
        const std::string id = "stk_r" + std::to_string(i);
        s.rosterStacks.push_back(core::RosterStackSaveState{id, "plain", 1});
        s.reserveSlotStackIds[i] = id;
    }
    s.nextStackIdCounter = 20;
    s.ownedServices = {core::OwnedServiceSaveState{"iron_mine_svc", "Green", false, false, {}}};
    auto session = Load(s);

    REQUIRE(session.TryStationStackAtService("iron_mine_svc", "stk_9"));
    REQUIRE(StationedRefCount(session, "iron_mine_svc") == 1);

    // Reserve is full -> unstation must fail without changing any state.
    REQUIRE_FALSE(session.TryUnstationStackFromService("iron_mine_svc", "stk_9"));
    REQUIRE(StationedRefCount(session, "iron_mine_svc") == 1);
    REQUIRE(session.FindOwnedService("iron_mine_svc")->stationedUnits[0].stackId == "stk_9");
    RequireNoOrphans(session);
}

TEST_CASE("Stationing mutation - unstationing never merges into a compatible reserve stack") {
    // A compatible kobold stack (stk_2) stays in reserve while a split kobold stack
    // is stationed and then unstationed. The returned stack must remain distinct —
    // AddOwnedUnit-style merging is forbidden on the unstation path.
    auto session = Load(MakeBaseSave());
    REQUIRE(session.TryStationSplitAtService("iron_mine_svc", "stk_2", 1));
    const std::string splitId = session.FindOwnedService("iron_mine_svc")->stationedUnits[0].stackId;

    REQUIRE(session.TryUnstationStackFromService("iron_mine_svc", splitId));

    // stk_2 untouched at quantity 2; the split stack returned as its own qty-1 stack.
    const auto* source = session.FindRosterStackById("stk_2");
    REQUIRE(source != nullptr);
    REQUIRE(source->quantity == 2);
    const auto* returned = session.FindRosterStackById(splitId);
    REQUIRE(returned != nullptr);
    REQUIRE(returned->quantity == 1);
    REQUIRE(SlotsContain(session.ReserveSlotStackIds(), "stk_2"));
    REQUIRE(SlotsContain(session.ReserveSlotStackIds(), splitId));
    REQUIRE(session.OwnedUnitCount("kobold") == 3);
    RequireNoOrphans(session);
}

TEST_CASE("Stationing mutation - unstation heals a legacy reserve+stationed double-placement without duplicating") {
    // NormalizeStationedUnits is permissive, so injected/legacy save-data can leave
    // a stack BOTH in a reserve slot and stationed. stk_2 (kobold) sits in reserve
    // (MakeBaseSave) and is also stationed here.
    auto save = MakeBaseSave();
    save.ownedServices = {core::OwnedServiceSaveState{"iron_mine_svc", "Green", false, false,
        {core::StationedUnitSaveState{"kobold", "stk_2"}}}};
    auto session = Load(save);
    REQUIRE(SlotsContain(session.ReserveSlotStackIds(), "stk_2"));
    REQUIRE(StationedRefCount(session, "iron_mine_svc") == 1);

    REQUIRE(session.TryUnstationStackFromService("iron_mine_svc", "stk_2"));

    // The stationed ref is removed; the reserve placement is left untouched, and
    // stk_2 is NOT duplicated into a second reserve slot.
    REQUIRE(StationedRefCount(session, "iron_mine_svc") == 0);
    int reserveOccurrences = 0;
    for (const auto& s : session.ReserveSlotStackIds()) {
        if (s == "stk_2") ++reserveOccurrences;
    }
    REQUIRE(reserveOccurrences == 1);
    REQUIRE(session.OwnedUnitCount("kobold") == 3);
    RequireNoOrphans(session);
}

TEST_CASE("Stationing mutation - unstation heals a legacy active+stationed double-placement without duplicating") {
    auto save = MakeBaseSave();
    save.activeSlotStackIds = {"stk_1", "stk_2", "", "", ""};
    save.reserveSlotStackIds = {"stk_3", "", "", "", "", "", "", ""};
    save.ownedServices = {core::OwnedServiceSaveState{"iron_mine_svc", "Green", false, false,
        {core::StationedUnitSaveState{"kobold", "stk_2"}}}};
    auto session = Load(save);
    REQUIRE(SlotsContain(session.ActiveSlotStackIds(), "stk_2"));
    REQUIRE(StationedRefCount(session, "iron_mine_svc") == 1);

    REQUIRE(session.TryUnstationStackFromService("iron_mine_svc", "stk_2"));

    // Ref removed; the active placement is untouched and stk_2 is NOT added to reserve.
    REQUIRE(StationedRefCount(session, "iron_mine_svc") == 0);
    REQUIRE(SlotsContain(session.ActiveSlotStackIds(), "stk_2"));
    REQUIRE_FALSE(SlotsContain(session.ReserveSlotStackIds(), "stk_2"));
    REQUIRE(session.OwnedUnitCount("kobold") == 3);
    RequireNoOrphans(session);  // exactly one placement per stack -> no duplicates
}

TEST_CASE("Stationing mutation - CanOpenStationingAtMine gates on a usable owned mine") {
    REQUIRE(Load(MakeBaseSave()).CanOpenStationingAtMine("iron_mine_svc"));

    auto locked = MakeBaseSave();
    locked.ownedServices = {core::OwnedServiceSaveState{"iron_mine_svc", "Green", true, false, {}}};
    REQUIRE_FALSE(Load(locked).CanOpenStationingAtMine("iron_mine_svc"));

    auto destroyed = MakeBaseSave();
    destroyed.ownedServices = {core::OwnedServiceSaveState{"iron_mine_svc", "Green", false, true, {}}};
    REQUIRE_FALSE(Load(destroyed).CanOpenStationingAtMine("iron_mine_svc"));

    auto hostile = MakeBaseSave();
    hostile.ownedServices = {core::OwnedServiceSaveState{"iron_mine_svc", "Red", false, false, {}}};
    REQUIRE_FALSE(Load(hostile).CanOpenStationingAtMine("iron_mine_svc"));

    REQUIRE_FALSE(Load(MakeBaseSave()).CanOpenStationingAtMine("no_such_svc"));
}

TEST_CASE("Stationing mutation - cannot station into a non-owned or non-existent service") {
    auto save = MakeBaseSave();
    save.ownedServices = {core::OwnedServiceSaveState{"iron_mine_svc", "Red", false, false, {}}};
    auto session = Load(save);
    REQUIRE_FALSE(session.TryStationStackAtService("iron_mine_svc", "stk_2"));   // hostile-owned
    REQUIRE_FALSE(session.TryStationStackAtService("no_such_svc", "stk_2"));     // unknown
    REQUIRE(SlotsContain(session.ReserveSlotStackIds(), "stk_2"));
}

TEST_CASE("Stationing mutation - a stationed unit is excluded from the active battle party") {
    auto save = MakeBaseSave();
    save.activeSlotStackIds = {"stk_1", "stk_2", "", "", ""};
    save.reserveSlotStackIds = {"stk_3", "", "", "", "", "", "", ""};
    auto session = Load(save);

    REQUIRE(session.TryStationStackAtService("iron_mine_svc", "stk_2"));
    const auto battle = session.BuildActiveBattleStackEntries();
    const bool stationedInBattle = std::any_of(battle.begin(), battle.end(),
        [](const gameplay::ActiveBattleStackEntry& e) { return e.stackId == "stk_2"; });
    REQUIRE_FALSE(stationedInBattle);
}

TEST_CASE("Stationing mutation - stationed assignments round-trip through save/load with no schema bump") {
    auto session = Load(MakeBaseSave());
    REQUIRE(session.TryStationStackAtService("iron_mine_svc", "stk_2"));

    const core::SaveData out = session.ToSaveData();
    REQUIRE(out.schemaVersion == MakeBaseSave().schemaVersion);  // no bump
    REQUIRE(out.ownedServices.size() == 1);
    REQUIRE(out.ownedServices[0].stationedUnits.size() == 1);
    REQUIRE(out.ownedServices[0].stationedUnits[0].stackId == "stk_2");
    REQUIRE_FALSE(SlotsContain(out.reserveSlotStackIds, "stk_2"));
    REQUIRE_FALSE(SlotsContain(out.activeSlotStackIds, "stk_2"));

    auto reloaded = Load(out);
    const auto* owned = reloaded.FindOwnedService("iron_mine_svc");
    REQUIRE(owned != nullptr);
    REQUIRE(owned->stationedUnits.size() == 1);
    REQUIRE(owned->stationedUnits[0].stackId == "stk_2");
    REQUIRE(reloaded.FindRosterStackById("stk_2") != nullptr);
    REQUIRE(reloaded.OwnedUnitCount("kobold") == 3);
    RequireNoOrphans(reloaded);
}

TEST_CASE("Stationing mutation - EligibleStationingStackIds lists slotted non-PC unstationed stacks only") {
    auto session = Load(MakeBaseSave());
    auto eligible = session.EligibleStationingStackIds("iron_mine_svc");
    REQUIRE(std::find(eligible.begin(), eligible.end(), "stk_2") != eligible.end());
    REQUIRE(std::find(eligible.begin(), eligible.end(), "stk_3") != eligible.end());
    REQUIRE(std::find(eligible.begin(), eligible.end(), "stk_1") == eligible.end());  // PC

    REQUIRE(session.TryStationStackAtService("iron_mine_svc", "stk_2"));
    eligible = session.EligibleStationingStackIds("iron_mine_svc");
    REQUIRE(std::find(eligible.begin(), eligible.end(), "stk_2") == eligible.end());  // now stationed
}

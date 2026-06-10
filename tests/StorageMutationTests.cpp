#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <string>
#include <vector>

#include "core/SaveGame.h"
#include "data/definitions/LocationServiceDefinition.h"
#include "data/definitions/UnitDefinition.h"
#include "gameplay/GameSession.h"

// M28 GameSession store/retrieve behavior, mirroring StationingMutationTests. Storage
// is a distinct placement bucket (cap 7) from M25 stationing; the two are mutually
// exclusive automatically because both require the stack to be currently slotted.

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

std::vector<data::UnitDefinition> MakeCatalog() {
    return {
        MakeUnit("pc_hero", data::UnitDefinitionCategory::Hero, /*isPlayerCharacter=*/true),
        MakeUnit("hero2", data::UnitDefinitionCategory::Hero),
        MakeUnit("kobold", data::UnitDefinitionCategory::Generic),
        MakeUnit("plain", data::UnitDefinitionCategory::Generic),
    };
}

data::LocationServiceDefinition MakeStorage(const std::string& id) {
    data::LocationServiceDefinition svc;
    svc.id = id;
    svc.kind = LocationServiceKind::Storage;
    return svc;
}

data::LocationServiceDefinition MakeMine(const std::string& id) {
    data::LocationServiceDefinition svc;
    svc.id = id;
    svc.kind = LocationServiceKind::Mine;
    return svc;
}

// pc_hero active; kobold (x3) and plain (x2) in reserve; one player-owned storage.
core::SaveData MakeBaseSave() {
    core::SaveData s;
    s.schemaVersion = 5;
    s.day = 1;
    s.mode = "region_mode";
    s.hasCanonicalRoster = true;
    s.rosterStacks = {
        core::RosterStackSaveState{"stk_1", "pc_hero", 1},
        core::RosterStackSaveState{"stk_2", "kobold", 3},
        core::RosterStackSaveState{"stk_3", "plain", 2},
    };
    s.activeSlotStackIds = {"stk_1", "", "", "", ""};
    s.reserveSlotStackIds = {"stk_2", "stk_3", "", "", "", "", "", ""};
    s.nextStackIdCounter = 4;
    s.ownedServices = {core::OwnedServiceSaveState{"home_storage", "Green", false, false, {}, {}}};
    return s;
}

gameplay::GameSession Load(const core::SaveData& save,
    std::vector<data::LocationServiceDefinition> services = {MakeStorage("home_storage")}) {
    gameplay::GameSession session;
    session.SetUnitCatalog(MakeCatalog());
    session.SetLocationServiceCatalog(std::move(services));
    session.ApplySaveData(save);
    return session;
}

bool SlotsContain(const std::vector<std::string>& slots, const std::string& stackId) {
    return std::find(slots.begin(), slots.end(), stackId) != slots.end();
}

int StoredRefCount(const gameplay::GameSession& session, const std::string& serviceId) {
    const auto* owned = session.FindOwnedService(serviceId);
    return owned == nullptr ? 0 : static_cast<int>(owned->storedUnits.size());
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

} // namespace

TEST_CASE("Storage mutation - store a whole reserve stack moves it out of its slot keeping its id") {
    auto session = Load(MakeBaseSave());

    REQUIRE(session.TryStoreStackAtService("home_storage", "stk_2"));

    REQUIRE_FALSE(SlotsContain(session.ReserveSlotStackIds(), "stk_2"));
    REQUIRE_FALSE(SlotsContain(session.ActiveSlotStackIds(), "stk_2"));
    REQUIRE(session.FindRosterStackById("stk_2") != nullptr);
    const auto* owned = session.FindOwnedService("home_storage");
    REQUIRE(owned->storedUnits.size() == 1);
    REQUIRE(owned->storedUnits[0].stackId == "stk_2");
    REQUIRE(owned->storedUnits[0].unitId == "kobold");
    REQUIRE(session.OwnedUnitCount("kobold") == 3);  // still owned, just not travelling
    RequireNoOrphans(session);
}

TEST_CASE("Storage mutation - the Player Character can never be stored") {
    auto session = Load(MakeBaseSave());
    REQUIRE_FALSE(session.CanStoreStackAtService("home_storage", "stk_1"));
    REQUIRE_FALSE(session.TryStoreStackAtService("home_storage", "stk_1"));
    REQUIRE(StoredRefCount(session, "home_storage") == 0);
    REQUIRE(SlotsContain(session.ActiveSlotStackIds(), "stk_1"));
}

TEST_CASE("Storage mutation - a stack cannot be stored twice") {
    auto session = Load(MakeBaseSave());
    REQUIRE(session.TryStoreStackAtService("home_storage", "stk_2"));
    REQUIRE_FALSE(session.CanStoreStackAtService("home_storage", "stk_2"));  // now slot-less + stored
    REQUIRE_FALSE(session.TryStoreStackAtService("home_storage", "stk_2"));
    REQUIRE(StoredRefCount(session, "home_storage") == 1);
    RequireNoOrphans(session);
}

TEST_CASE("Storage mutation - storing from an active slot respects the leader guard") {
    auto save = MakeBaseSave();
    save.activeSlotStackIds = {"stk_h2", "", "", "", ""};
    save.reserveSlotStackIds = {"", "", "", "", "", "", "", ""};
    save.rosterStacks = {core::RosterStackSaveState{"stk_h2", "hero2", 1}};
    save.nextStackIdCounter = 2;
    auto session = Load(save);
    session.SetLeaderCapableUnitIds({"hero2"});  // hero2 is the sole leader

    // Storing the only leader-capable active unit would leave the party leaderless.
    REQUIRE_FALSE(session.TryStoreStackAtService("home_storage", "stk_h2"));
    REQUIRE(StoredRefCount(session, "home_storage") == 0);
    REQUIRE(SlotsContain(session.ActiveSlotStackIds(), "stk_h2"));
}

TEST_CASE("Storage mutation - capacity is capped at seven distinct stacks") {
    core::SaveData s;
    s.schemaVersion = 5;
    s.day = 1;
    s.mode = "region_mode";
    s.hasCanonicalRoster = true;
    s.reserveSlotStackIds.assign(8, "");
    for (int i = 0; i < 8; ++i) {
        const std::string id = "stk_" + std::to_string(i + 1);
        s.rosterStacks.push_back(core::RosterStackSaveState{id, "plain", 1});
        s.reserveSlotStackIds[i] = id;
    }
    s.activeSlotStackIds.assign(5, "");
    s.nextStackIdCounter = 9;
    s.ownedServices = {core::OwnedServiceSaveState{"home_storage", "Green", false, false, {}, {}}};
    auto session = Load(s);

    for (int i = 0; i < 7; ++i) {
        REQUIRE(session.TryStoreStackAtService("home_storage", "stk_" + std::to_string(i + 1)));
    }
    REQUIRE(StoredRefCount(session, "home_storage") == 7);
    REQUIRE_FALSE(session.TryStoreStackAtService("home_storage", "stk_8"));  // 8th refused
    REQUIRE(StoredRefCount(session, "home_storage") == 7);
    REQUIRE(SlotsContain(session.ReserveSlotStackIds(), "stk_8"));
    RequireNoOrphans(session);
}

TEST_CASE("Storage mutation - retrieving returns the same stack id to reserve") {
    auto session = Load(MakeBaseSave());
    REQUIRE(session.TryStoreStackAtService("home_storage", "stk_2"));

    REQUIRE(session.TryRetrieveStackFromService("home_storage", "stk_2"));

    REQUIRE(StoredRefCount(session, "home_storage") == 0);
    REQUIRE(SlotsContain(session.ReserveSlotStackIds(), "stk_2"));  // same id back
    REQUIRE(session.OwnedUnitCount("kobold") == 3);
    RequireNoOrphans(session);
}

TEST_CASE("Storage mutation - retrieving fails atomically when reserve is full") {
    // Eight reserve stacks fill reserve; the PC sits active alongside a second
    // active stack that gets stored, so reserve stays full while it is stored.
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
    s.ownedServices = {core::OwnedServiceSaveState{"home_storage", "Green", false, false, {}, {}}};
    auto session = Load(s);

    REQUIRE(session.TryStoreStackAtService("home_storage", "stk_9"));
    REQUIRE(StoredRefCount(session, "home_storage") == 1);

    REQUIRE_FALSE(session.TryRetrieveStackFromService("home_storage", "stk_9"));  // reserve full
    REQUIRE(StoredRefCount(session, "home_storage") == 1);
    REQUIRE(session.FindOwnedService("home_storage")->storedUnits[0].stackId == "stk_9");
    RequireNoOrphans(session);
}

TEST_CASE("Storage mutation - retrieve heals a corrupt active+stored double-placement") {
    auto save = MakeBaseSave();
    // stk_1 (pc_hero) is in the active slot AND injected as stored (corrupt state).
    save.ownedServices = {core::OwnedServiceSaveState{"home_storage", "Green", false, false, {},
        {core::StoredUnitSaveState{"pc_hero", "stk_1"}}}};
    auto session = Load(save);
    REQUIRE(SlotsContain(session.ActiveSlotStackIds(), "stk_1"));
    REQUIRE(StoredRefCount(session, "home_storage") == 1);

    REQUIRE(session.TryRetrieveStackFromService("home_storage", "stk_1"));

    // Ref removed; the active placement is untouched and stk_1 is NOT duplicated.
    REQUIRE(StoredRefCount(session, "home_storage") == 0);
    REQUIRE(SlotsContain(session.ActiveSlotStackIds(), "stk_1"));
    REQUIRE_FALSE(SlotsContain(session.ReserveSlotStackIds(), "stk_1"));
    RequireNoOrphans(session);
}

TEST_CASE("Storage mutation - retrieve heals a corrupt reserve+stored double-placement") {
    auto save = MakeBaseSave();
    // stk_2 (kobold) is in reserve (base) AND injected as stored.
    save.ownedServices = {core::OwnedServiceSaveState{"home_storage", "Green", false, false, {},
        {core::StoredUnitSaveState{"kobold", "stk_2"}}}};
    auto session = Load(save);
    REQUIRE(SlotsContain(session.ReserveSlotStackIds(), "stk_2"));
    REQUIRE(StoredRefCount(session, "home_storage") == 1);

    REQUIRE(session.TryRetrieveStackFromService("home_storage", "stk_2"));

    REQUIRE(StoredRefCount(session, "home_storage") == 0);
    int reserveOccurrences = 0;
    for (const auto& s : session.ReserveSlotStackIds()) if (s == "stk_2") ++reserveOccurrences;
    REQUIRE(reserveOccurrences == 1);  // not duplicated
    REQUIRE(session.OwnedUnitCount("kobold") == 3);
    RequireNoOrphans(session);
}

TEST_CASE("Storage mutation - cross-exclusion with stationing is automatic") {
    auto save = MakeBaseSave();
    save.ownedServices = {
        core::OwnedServiceSaveState{"home_storage", "Green", false, false, {}, {}},
        core::OwnedServiceSaveState{"iron_mine_svc", "Green", false, false, {}, {}},
    };
    auto session = Load(save, {MakeStorage("home_storage"), MakeMine("iron_mine_svc")});

    // Station stk_2 at the mine -> it becomes slot-less -> cannot be stored.
    REQUIRE(session.TryStationStackAtService("iron_mine_svc", "stk_2"));
    REQUIRE_FALSE(session.CanStoreStackAtService("home_storage", "stk_2"));

    // Store stk_3 at storage -> it becomes slot-less -> cannot be stationed.
    REQUIRE(session.TryStoreStackAtService("home_storage", "stk_3"));
    REQUIRE_FALSE(session.CanStationStackAtService("iron_mine_svc", "stk_3"));
    RequireNoOrphans(session);
}

TEST_CASE("Storage mutation - a stored unit is excluded from the active battle party") {
    auto save = MakeBaseSave();
    save.activeSlotStackIds = {"stk_1", "stk_2", "", "", ""};
    save.reserveSlotStackIds = {"stk_3", "", "", "", "", "", "", ""};
    auto session = Load(save);

    REQUIRE(session.TryStoreStackAtService("home_storage", "stk_2"));
    const auto battle = session.BuildActiveBattleStackEntries();
    const bool stored = std::any_of(battle.begin(), battle.end(),
        [](const gameplay::ActiveBattleStackEntry& e) { return e.stackId == "stk_2"; });
    REQUIRE_FALSE(stored);
}

TEST_CASE("Storage mutation - cannot store into a non-owned, non-storage, or unknown service") {
    auto save = MakeBaseSave();
    save.ownedServices = {core::OwnedServiceSaveState{"home_storage", "Red", false, false, {}, {}}};
    auto session = Load(save);
    REQUIRE_FALSE(session.TryStoreStackAtService("home_storage", "stk_2"));  // hostile-owned
    REQUIRE_FALSE(session.TryStoreStackAtService("no_such_svc", "stk_2"));   // unknown
    REQUIRE(SlotsContain(session.ReserveSlotStackIds(), "stk_2"));
}

TEST_CASE("Storage mutation - CanOpenStorageAtService gates on a usable owned storage") {
    REQUIRE(Load(MakeBaseSave()).CanOpenStorageAtService("home_storage"));

    auto locked = MakeBaseSave();
    locked.ownedServices = {core::OwnedServiceSaveState{"home_storage", "Green", true, false, {}, {}}};
    REQUIRE_FALSE(Load(locked).CanOpenStorageAtService("home_storage"));

    auto hostile = MakeBaseSave();
    hostile.ownedServices = {core::OwnedServiceSaveState{"home_storage", "Red", false, false, {}, {}}};
    REQUIRE_FALSE(Load(hostile).CanOpenStorageAtService("home_storage"));

    // A mine is not a storage service.
    auto mine = MakeBaseSave();
    mine.ownedServices = {core::OwnedServiceSaveState{"iron_mine_svc", "Green", false, false, {}, {}}};
    REQUIRE_FALSE(Load(mine, {MakeMine("iron_mine_svc")}).CanOpenStorageAtService("iron_mine_svc"));
}

TEST_CASE("Storage mutation - stored assignments round-trip through save/load with no schema bump") {
    auto session = Load(MakeBaseSave());
    REQUIRE(session.TryStoreStackAtService("home_storage", "stk_2"));

    const core::SaveData out = session.ToSaveData();
    REQUIRE(out.schemaVersion == MakeBaseSave().schemaVersion);  // no bump
    REQUIRE(out.ownedServices.size() == 1);
    REQUIRE(out.ownedServices[0].storedUnits.size() == 1);
    REQUIRE(out.ownedServices[0].storedUnits[0].stackId == "stk_2");
    REQUIRE_FALSE(SlotsContain(out.reserveSlotStackIds, "stk_2"));

    auto reloaded = Load(out);
    const auto* owned = reloaded.FindOwnedService("home_storage");
    REQUIRE(owned != nullptr);
    REQUIRE(owned->storedUnits.size() == 1);
    REQUIRE(owned->storedUnits[0].stackId == "stk_2");
    REQUIRE(reloaded.OwnedUnitCount("kobold") == 3);
    RequireNoOrphans(reloaded);
}

TEST_CASE("Storage mutation - EligibleStorageStackIds lists slotted non-PC unplaced stacks only") {
    auto session = Load(MakeBaseSave());
    auto eligible = session.EligibleStorageStackIds("home_storage");
    REQUIRE(std::find(eligible.begin(), eligible.end(), "stk_2") != eligible.end());
    REQUIRE(std::find(eligible.begin(), eligible.end(), "stk_3") != eligible.end());
    REQUIRE(std::find(eligible.begin(), eligible.end(), "stk_1") == eligible.end());  // PC

    REQUIRE(session.TryStoreStackAtService("home_storage", "stk_2"));
    eligible = session.EligibleStorageStackIds("home_storage");
    REQUIRE(std::find(eligible.begin(), eligible.end(), "stk_2") == eligible.end());  // now stored
}

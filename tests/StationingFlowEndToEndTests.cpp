#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

#include "core/GameClock.h"
#include "core/SaveGame.h"
#include "data/definitions/LocationServiceDefinition.h"
#include "data/definitions/UnitDefinition.h"
#include "gameplay/GameSession.h"
#include "gameplay/ResourceState.h"

using gameplay::ResourceType;

namespace {

constexpr int kOneDay = core::GameClock::kMinutesPerSliceDay;

data::UnitDefinition MakeMiner(const std::string& id, const std::string& resource, int amount) {
    data::UnitDefinition u;
    u.id = id;
    u.name = id;
    u.category = data::UnitDefinitionCategory::Generic;
    u.passiveEffects.push_back(data::UnitPassiveEffect{
        data::PassiveEffectKind::MineProduction, resource, "mine", amount});
    return u;
}

data::LocationServiceDefinition MakeMine(const std::string& id, const std::string& locationId,
    std::vector<data::MineOutputDefinition> outputs) {
    data::LocationServiceDefinition svc;
    svc.id = id;
    svc.locationId = locationId;
    svc.zoneId = "mine_face";
    svc.kind = data::LocationServiceKind::Mine;
    svc.mineOutputs = std::move(outputs);
    return svc;
}

// A player-owned Stone mine (base +2) with an idle kobold miner (+1 Stone) sitting
// in reserve and no units stationed yet.
gameplay::GameSession MakeSession() {
    core::SaveData s;
    s.schemaVersion = 5;
    s.day = 1;
    s.minutesIntoSliceDay = 0;
    s.mode = "region_mode";
    s.regionId = "ashvale_heartland";
    s.destinationId = "stone_mine";
    s.hasCanonicalRoster = true;
    s.rosterStacks = {core::RosterStackSaveState{"stk_1", "kobold", 1}};
    s.activeSlotStackIds = {"", "", "", "", ""};
    s.reserveSlotStackIds = {"stk_1", "", "", "", "", "", "", ""};
    s.nextStackIdCounter = 2;
    s.ownedServices = {core::OwnedServiceSaveState{"stone_mine_svc", "Green", false, false, {}}};

    gameplay::GameSession session;
    session.SetUnitCatalog({MakeMiner("kobold", "Stone", 1)});
    session.SetLocationServiceCatalog({MakeMine("stone_mine_svc", "stone_mine", {{"Stone", 2}})});
    session.ApplySaveData(s);
    return session;
}

} // namespace

TEST_CASE("Stationing e2e - stationing through the GameSession API raises the next mine payout") {
    auto session = MakeSession();

    // Day boundary with the miner idle in reserve: base output only.
    session.AddMinutes(kOneDay);
    REQUIRE(session.ResourceCount(ResourceType::Stone) == 2);

    // Reach stationing through the player-facing mutation API (not save-data
    // injection): assign the miner to the mine.
    REQUIRE(session.TryStationStackAtService("stone_mine_svc", "stk_1"));

    // Next day boundary now pays the strongest-only boosted output.
    session.AddMinutes(kOneDay);
    REQUIRE(session.ResourceCount(ResourceType::Stone) == 2 + 3);  // +base+passive

    // The miner is still owned (now a mine worker), just no longer travelling.
    REQUIRE(session.OwnedUnitCount("kobold") == 1);
}

TEST_CASE("Stationing e2e - unstationing through the API drops the payout back to base") {
    auto session = MakeSession();
    REQUIRE(session.TryStationStackAtService("stone_mine_svc", "stk_1"));

    session.AddMinutes(kOneDay);
    REQUIRE(session.ResourceCount(ResourceType::Stone) == 3);  // boosted

    REQUIRE(session.TryUnstationStackFromService("stone_mine_svc", "stk_1"));

    session.AddMinutes(kOneDay);
    REQUIRE(session.ResourceCount(ResourceType::Stone) == 3 + 2);  // base only this day
    REQUIRE(session.OwnedUnitCount("kobold") == 1);
}

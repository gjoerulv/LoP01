#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <string>

#include "data/ContentRepository.h"
#include "data/definitions/LocationSceneDefinition.h"
#include "gameplay/GameSession.h"
#include "gameplay/location/LocationScene.h"

// M25 manual-play regression: the Steel Mine's `mine_face` zone must be a non-NPC,
// service-backed zone so interacting with it dispatches to the mine stationing
// path instead of starting empty NPC dialogue (which soft-locked the scene and
// blocked stationing input). Guards the shipped content + the App open-gate
// predicate together. These load the real content/ directory.

#ifndef LOP01_PROJECT_ROOT
#define LOP01_PROJECT_ROOT "."
#endif

namespace {

std::filesystem::path RealContentDir() {
    return std::filesystem::path(LOP01_PROJECT_ROOT) / "content";
}

const data::LocationSceneZoneDefinition* FindZone(
    const data::LocationSceneDefinition& scene, const std::string& zoneId) {
    const auto it = std::find_if(scene.zones.begin(), scene.zones.end(),
        [&](const data::LocationSceneZoneDefinition& z) { return z.id == zoneId; });
    return it == scene.zones.end() ? nullptr : &*it;
}

} // namespace

TEST_CASE("Mine zone regression - shipped mine_face is a non-NPC service-backed zone") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));

    const auto* scene = repo.FindLocationSceneById("iron_mine_proto");
    REQUIRE(scene != nullptr);
    const auto* zone = FindZone(*scene, "mine_face");
    REQUIRE(zone != nullptr);

    // Must NOT be an NPC zone (NPC zones start dialogue in LocationScene::Interact).
    REQUIRE(zone->type != data::LocationSceneZoneType::Npc);

    // And it must resolve to a Mine service so App dispatches it as a mine.
    const auto* svc = repo.FindLocationService("iron_mine", "mine_face");
    REQUIRE(svc != nullptr);
    REQUIRE(svc->kind == data::LocationServiceKind::Mine);
}

TEST_CASE("Mine zone regression - interacting with mine_face does not start active dialogue") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));
    const auto* sceneDef = repo.FindLocationSceneById("iron_mine_proto");
    REQUIRE(sceneDef != nullptr);

    gameplay::location::LocationScene scene;
    scene.Reset(*sceneDef);

    // Walk the player onto the mine_face zone (authored at x=620, y=320).
    REQUIRE(scene.TryMovePlayer(536.0f, -200.0f));
    const auto outcome = scene.Interact();

    REQUIRE(outcome.has_value());
    REQUIRE(outcome->zoneId == "mine_face");
    REQUIRE_FALSE(outcome->requiresDialogueChoice);
    REQUIRE_FALSE(scene.HasActiveDialogue());  // no empty-NPC soft-lock
}

TEST_CASE("Mine zone regression - CanOpenStationingAtMine gates the real mine service") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));

    gameplay::GameSession session;
    session.SetPlayerColor("Green");
    session.SetUnitCatalog(repo.Units());
    session.SetLocationServiceCatalog(repo.LocationServices());

    auto loadWithMine = [&](const core::OwnedServiceSaveState& mine) {
        core::SaveData s;
        s.schemaVersion = 5;
        s.day = 1;
        s.mode = "region_mode";
        s.ownedServices = {mine};
        session.ApplySaveData(s);
    };

    // Owned, unlocked, undestroyed -> openable.
    loadWithMine(core::OwnedServiceSaveState{"iron_mine_svc", "Green", false, false, {}});
    REQUIRE(session.CanOpenStationingAtMine("iron_mine_svc"));

    // Locked / destroyed / hostile-owned -> not openable (App shows status, no UI).
    loadWithMine(core::OwnedServiceSaveState{"iron_mine_svc", "Green", true, false, {}});
    REQUIRE_FALSE(session.CanOpenStationingAtMine("iron_mine_svc"));
    loadWithMine(core::OwnedServiceSaveState{"iron_mine_svc", "Green", false, true, {}});
    REQUIRE_FALSE(session.CanOpenStationingAtMine("iron_mine_svc"));
    loadWithMine(core::OwnedServiceSaveState{"iron_mine_svc", "Red", false, false, {}});
    REQUIRE_FALSE(session.CanOpenStationingAtMine("iron_mine_svc"));

    // Not owned at all -> not openable.
    core::SaveData empty;
    empty.schemaVersion = 5;
    empty.day = 1;
    empty.mode = "region_mode";
    session.ApplySaveData(empty);
    REQUIRE_FALSE(session.CanOpenStationingAtMine("iron_mine_svc"));
}

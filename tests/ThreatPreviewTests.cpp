#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "core/SaveGame.h"
#include "data/definitions/EnemyGroupDefinition.h"
#include "data/definitions/LocationServiceDefinition.h"
#include "data/definitions/RegionDefinition.h"
#include "data/definitions/UnitDefinition.h"
#include "gameplay/GameSession.h"
#include "gameplay/battle/ThreatPreview.h"

// M33 threat preview: a bounded, read-only danger estimate. The band is a cheap
// §16 power-ratio (not the auto-resolve), reveal-gated so unknown enemies never
// leak, and pure (no runtime mutation).

using gameplay::battle::EstimateThreatBand;
using gameplay::battle::ThreatBand;

TEST_CASE("ThreatPreview - band thresholds follow the §16 ratios") {
    REQUIRE(EstimateThreatBand(80, 20) == ThreatBand::Low);          // player share 0.80
    REQUIRE(EstimateThreatBand(100, 0) == ThreatBand::Low);          // no enemy
    REQUIRE(EstimateThreatBand(60, 40) == ThreatBand::Even);         // 0.60
    REQUIRE(EstimateThreatBand(50, 50) == ThreatBand::Even);         // exactly even
    REQUIRE(EstimateThreatBand(40, 60) == ThreatBand::Dangerous);    // enemy favored
    REQUIRE(EstimateThreatBand(20, 80) == ThreatBand::Overwhelming); // player share 0.20
    REQUIRE(EstimateThreatBand(0, 50) == ThreatBand::Overwhelming);  // no player force
    REQUIRE(EstimateThreatBand(0, 0) == ThreatBand::Even);           // nothing either side
}

namespace {

using data::LocationServiceKind;

data::UnitDefinition MakeUnit(const std::string& id, data::UnitDefinitionCategory category,
    int attack, int defense, int maxHp, bool isPlayerCharacter = false) {
    data::UnitDefinition u;
    u.id = id;
    u.name = id;
    u.category = category;
    u.stats.attack = attack;
    u.stats.defense = defense;
    u.stats.maxHp = maxHp;
    u.stats.agility = 5;
    u.isPlayerCharacter = isPlayerCharacter;
    return u;
}

data::RegionNodeDefinition Node(const std::string& id) {
    data::RegionNodeDefinition n;
    n.locationId = id;
    return n;
}

// Region "alpha": chain a_arr - a_b - a_c - a_d. Player starts at a_arr, so reveal
// (radius 2) covers a_arr/a_b/a_c; a_d stays unknown.
gameplay::GameSession BuildSession() {
    gameplay::GameSession session;
    session.SetPlayerColor("Green");
    session.SetUnitCatalog({
        MakeUnit("pc_hero", data::UnitDefinitionCategory::Leader, 20, 20, 24, /*isPC=*/true),
        MakeUnit("raider", data::UnitDefinitionCategory::Generic, 3, 2, 8),
    });
    session.SetLeaderCapableUnitIds({"pc_hero"});
    session.SetEnemyGroupCatalog({
        data::EnemyGroupDefinition{"eg_weak", "Scout", {"raider"}},
    });
    session.SetLocationServiceCatalog({
        [] {
            data::LocationServiceDefinition svc;
            svc.id = "mine_c"; svc.locationId = "a_c"; svc.kind = LocationServiceKind::Mine;
            return svc;
        }(),
    });

    data::RegionDefinition alpha;
    alpha.id = "alpha";
    alpha.arrivalNodeId = "a_arr";
    alpha.nodes = {Node("a_arr"), Node("a_b"), Node("a_c"), Node("a_d")};
    alpha.links = {{"a_arr", "a_b"}, {"a_b", "a_c"}, {"a_c", "a_d"}};
    session.SetRegionCatalog({alpha});

    gameplay::EnemyTeamState red;     // visible (a_c is within reveal radius)
    red.teamColor = "Red"; red.nodeId = "a_c"; red.enemyGroupId = "eg_weak"; red.active = true;
    gameplay::EnemyTeamState blue;    // hidden (a_d is beyond reveal radius)
    blue.teamColor = "Blue"; blue.nodeId = "a_d"; blue.enemyGroupId = "eg_weak"; blue.active = true;
    session.SetEnemyTeams({red, blue});

    core::SaveData save;
    save.schemaVersion = 5;
    save.day = 1;
    save.mode = "overworld_mode";
    save.regionId = "alpha";
    save.destinationId = "a_arr";
    save.hasCanonicalRoster = true;
    save.rosterStacks = {core::RosterStackSaveState{"stk_pc", "pc_hero", 1}};
    save.activeSlotStackIds = {"stk_pc", "", "", "", ""};
    save.reserveSlotStackIds = {"", "", "", "", "", "", "", ""};
    save.nextStackIdCounter = 2;
    save.ownedServices = {core::OwnedServiceSaveState{"mine_c", "Green", false, false, {}, {}}};
    session.ApplySaveData(save);
    return session;
}

} // namespace

TEST_CASE("ThreatPreview - a revealed hostile node previews a bounded band") {
    auto session = BuildSession();
    REQUIRE(session.IsNodeRevealed("alpha", "a_c"));

    const auto preview = session.ThreatPreviewForNode("a_c");
    REQUIRE(preview.known);
    REQUIRE(preview.enemyColor == "Red");
    // Strong PC vs a single weak scout -> player heavily favored.
    REQUIRE(preview.band == ThreatBand::Low);
}

TEST_CASE("ThreatPreview - an unrevealed hostile node does not leak a preview") {
    auto session = BuildSession();
    REQUIRE_FALSE(session.IsNodeRevealed("alpha", "a_d"));

    const auto preview = session.ThreatPreviewForNode("a_d");
    REQUIRE_FALSE(preview.known);
    REQUIRE(preview.band == ThreatBand::Unknown);
    REQUIRE(preview.enemyColor.empty());
}

TEST_CASE("ThreatPreview - a revealed node with no hostile team is not previewed") {
    auto session = BuildSession();
    REQUIRE(session.IsNodeRevealed("alpha", "a_b"));

    const auto preview = session.ThreatPreviewForNode("a_b");
    REQUIRE_FALSE(preview.known);
}

TEST_CASE("ThreatPreview - service-defense preview surfaces when an enemy holds the service node") {
    auto session = BuildSession();   // mine_c sits on a_c where Red stands (revealed)

    const auto preview = session.ServiceDefensePreview("mine_c");
    REQUIRE(preview.known);
    REQUIRE(preview.enemyColor == "Red");
    // Undefended mine vs a scout -> the player is in danger.
    REQUIRE(preview.band == ThreatBand::Overwhelming);
}

TEST_CASE("ThreatPreview - service-defense preview is silent when no enemy occupies the node") {
    auto session = BuildSession();
    // Move Red off a_c by clearing it; the mine node is no longer contested.
    session.ClearEnemyTeamByColor("Red");

    const auto preview = session.ServiceDefensePreview("mine_c");
    REQUIRE_FALSE(preview.known);
}

namespace {
// Serialize the full save state to a string via the real save serializer, so the
// purity check covers every persisted field (roster, services, enemy teams,
// reveal, resources, energy, ...).
std::string SaveFingerprint(const gameplay::GameSession& session, const std::string& path) {
    core::SaveGameRepository repo;
    REQUIRE(repo.SaveToFile(session.ToSaveData(), path));
    std::ifstream in(path);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}
} // namespace

TEST_CASE("ThreatPreview - preview calls are pure (no runtime mutation)") {
    auto session = BuildSession();
    const std::string before = SaveFingerprint(session, "saves/tp_purity_before.json");

    // Exercise every preview entry point, including hidden/empty cases.
    (void)session.ThreatPreviewForNode("a_c");
    (void)session.ThreatPreviewForNode("a_d");
    (void)session.ThreatPreviewForNode("a_b");
    (void)session.ThreatPreviewForNode("");
    (void)session.ServiceDefensePreview("mine_c");
    (void)session.ServiceDefensePreview("nonexistent");

    const std::string after = SaveFingerprint(session, "saves/tp_purity_after.json");
    REQUIRE(before == after);

    std::filesystem::remove("saves/tp_purity_before.json");
    std::filesystem::remove("saves/tp_purity_after.json");
}

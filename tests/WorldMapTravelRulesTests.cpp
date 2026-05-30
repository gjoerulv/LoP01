#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

#include "data/definitions/WorldMapDefinition.h"
#include "gameplay/worldmap/WorldMapTravelRules.h"

using namespace gameplay::worldmap;
using data::WorldMapAdjacency;
using data::WorldMapRegionEntry;

namespace {

// Linear chain a - b - c - d for hop-count / path tests.
std::vector<WorldMapAdjacency> ChainAdjacency() {
    return {
        {"region_a", "region_b"},
        {"region_b", "region_c"},
        {"region_c", "region_d"},
    };
}

constexpr int kBefore11 = 0;     // 06:00, well before the 11:00 deadline
constexpr int kAt11 = 300;       // exactly 11:00
constexpr int kEnough = 1000;
constexpr int kNotEnough = 999;

} // namespace

TEST_CASE("WorldMapTravelRules - adjacent unlocked regions are legal, 1 day") {
    const auto eval = EvaluateWorldMapTravel(
        "region_a", "region_b",
        {"region_a", "region_b"},
        ChainAdjacency(),
        kBefore11, kEnough);
    REQUIRE(eval.legal);
    REQUIRE(eval.days == 1);
    REQUIRE(eval.reason == WorldMapTravelBlockReason::None);
}

TEST_CASE("WorldMapTravelRules - multi-hop day count equals shortest hop count") {
    // a -> d through b, c: 3 hops => 3 days (1 + N where N=2 intermediates).
    const auto eval = EvaluateWorldMapTravel(
        "region_a", "region_d",
        {"region_a", "region_b", "region_c", "region_d"},
        ChainAdjacency(),
        kBefore11, kEnough);
    REQUIRE(eval.legal);
    REQUIRE(eval.days == 3);
}

TEST_CASE("WorldMapTravelRules - same region is AlreadyHere") {
    const auto eval = EvaluateWorldMapTravel(
        "region_a", "region_a",
        {"region_a"},
        ChainAdjacency(),
        kBefore11, kEnough);
    REQUIRE_FALSE(eval.legal);
    REQUIRE(eval.reason == WorldMapTravelBlockReason::AlreadyHere);
}

TEST_CASE("WorldMapTravelRules - locked destination is rejected") {
    const auto eval = EvaluateWorldMapTravel(
        "region_a", "region_b",
        {"region_a"}, // region_b not unlocked
        ChainAdjacency(),
        kBefore11, kEnough);
    REQUIRE_FALSE(eval.legal);
    REQUIRE(eval.reason == WorldMapTravelBlockReason::DestinationLocked);
}

TEST_CASE("WorldMapTravelRules - a locked intermediate region breaks the path") {
    // Travel a -> d, but region_c is locked, so no contiguous unlocked path.
    const auto eval = EvaluateWorldMapTravel(
        "region_a", "region_d",
        {"region_a", "region_b", "region_d"}, // region_c missing
        ChainAdjacency(),
        kBefore11, kEnough);
    REQUIRE_FALSE(eval.legal);
    REQUIRE(eval.reason == WorldMapTravelBlockReason::NoPath);
}

TEST_CASE("WorldMapTravelRules - disconnected destination is NoPath") {
    const std::vector<WorldMapAdjacency> disjoint = {
        {"region_a", "region_b"},
        // region_x has no edges
    };
    const auto eval = EvaluateWorldMapTravel(
        "region_a", "region_x",
        {"region_a", "region_b", "region_x"},
        disjoint,
        kBefore11, kEnough);
    REQUIRE_FALSE(eval.legal);
    REQUIRE(eval.reason == WorldMapTravelBlockReason::NoPath);
}

TEST_CASE("WorldMapTravelRules - at or after 11:00 is past the departure deadline") {
    const auto eval = EvaluateWorldMapTravel(
        "region_a", "region_b",
        {"region_a", "region_b"},
        ChainAdjacency(),
        kAt11, kEnough);
    REQUIRE_FALSE(eval.legal);
    REQUIRE(eval.reason == WorldMapTravelBlockReason::PastDepartureDeadline);
}

TEST_CASE("WorldMapTravelRules - insufficient energy is rejected") {
    const auto eval = EvaluateWorldMapTravel(
        "region_a", "region_b",
        {"region_a", "region_b"},
        ChainAdjacency(),
        kBefore11, kNotEnough);
    REQUIRE_FALSE(eval.legal);
    REQUIRE(eval.reason == WorldMapTravelBlockReason::InsufficientEnergy);
}

TEST_CASE("WorldMapTravelRules - exactly 1000 energy is enough") {
    const auto eval = EvaluateWorldMapTravel(
        "region_a", "region_b",
        {"region_a", "region_b"},
        ChainAdjacency(),
        kBefore11, 1000);
    REQUIRE(eval.legal);
}

TEST_CASE("WorldMapTravelRules - reason precedence: locked beats deadline beats energy beats no-path") {
    // Destination locked AND past deadline AND no energy -> DestinationLocked wins.
    const auto locked = EvaluateWorldMapTravel(
        "region_a", "region_b", {"region_a"}, ChainAdjacency(), kAt11, kNotEnough);
    REQUIRE(locked.reason == WorldMapTravelBlockReason::DestinationLocked);

    // Unlocked dest, past deadline AND no energy -> PastDepartureDeadline wins.
    const auto deadline = EvaluateWorldMapTravel(
        "region_a", "region_b", {"region_a", "region_b"}, ChainAdjacency(), kAt11, kNotEnough);
    REQUIRE(deadline.reason == WorldMapTravelBlockReason::PastDepartureDeadline);

    // Unlocked dest, before deadline, no energy -> InsufficientEnergy wins.
    const auto energy = EvaluateWorldMapTravel(
        "region_a", "region_b", {"region_a", "region_b"}, ChainAdjacency(), kBefore11, kNotEnough);
    REQUIRE(energy.reason == WorldMapTravelBlockReason::InsufficientEnergy);
}

TEST_CASE("WorldMapTravelRules - IsWorldMapExitNode matches authored exit nodes") {
    WorldMapRegionEntry entry;
    entry.id = "region_a";
    entry.exitNodeIds = {"gate_node", "harbor_node"};
    REQUIRE(IsWorldMapExitNode(entry, "gate_node"));
    REQUIRE(IsWorldMapExitNode(entry, "harbor_node"));
    REQUIRE_FALSE(IsWorldMapExitNode(entry, "town_center"));
    REQUIRE_FALSE(IsWorldMapExitNode(entry, ""));
}

TEST_CASE("WorldMapTravelRules - FindRegionHopCount returns -1 when an endpoint is locked") {
    REQUIRE(FindRegionHopCount("region_a", "region_b", {"region_a"}, ChainAdjacency()) == -1);
    REQUIRE(FindRegionHopCount("region_a", "region_a", {"region_a"}, ChainAdjacency()) == 0);
    REQUIRE(FindRegionHopCount("region_a", "region_c",
        {"region_a", "region_b", "region_c"}, ChainAdjacency()) == 2);
}

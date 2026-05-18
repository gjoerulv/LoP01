#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "data/definitions/RegionDefinition.h"
#include "gameplay/region/RegionTravelRules.h"

namespace {

std::vector<data::RegionLinkDefinition> MakeLinks() {
    return {
        data::RegionLinkDefinition{"home_base", "town_center"},
        data::RegionLinkDefinition{"town_center", "old_inn"}
    };
}

} // namespace

TEST_CASE("Region travel rules allow same-node travel with zero minutes") {
    const auto result = gameplay::region::EvaluateTravel(
        "home_base",
        "home_base",
        true,
        1199,
        MakeLinks());

    REQUIRE(result.legal);
    REQUIRE(result.hopCount == 0);
    REQUIRE(result.minutes == 0);
    REQUIRE(result.reason == gameplay::region::TravelBlockReason::None);
}

TEST_CASE("Region travel rules block unavailable destinations") {
    const auto result = gameplay::region::EvaluateTravel(
        "home_base",
        "town_center",
        false,
        0,
        MakeLinks());

    REQUIRE_FALSE(result.legal);
    REQUIRE(result.hopCount == 0);
    REQUIRE(result.minutes == 0);
    REQUIRE(result.reason == gameplay::region::TravelBlockReason::DestinationUnavailable);
}

TEST_CASE("Region travel rules allow direct links at fixed travel time") {
    const auto result = gameplay::region::EvaluateTravel(
        "home_base",
        "town_center",
        true,
        0,
        MakeLinks());

    REQUIRE(result.legal);
    REQUIRE(result.hopCount == 1);
    REQUIRE(result.minutes == 15);
    REQUIRE(result.reason == gameplay::region::TravelBlockReason::None);
}

TEST_CASE("Region travel rules treat links as bidirectional") {
    const auto result = gameplay::region::EvaluateTravel(
        "town_center",
        "home_base",
        true,
        0,
        MakeLinks());

    REQUIRE(result.legal);
    REQUIRE(result.hopCount == 1);
    REQUIRE(result.minutes == 15);
    REQUIRE(result.reason == gameplay::region::TravelBlockReason::None);
}

TEST_CASE("Region travel rules allow multi-hop shortest path travel") {
    const auto result = gameplay::region::EvaluateTravel(
        "home_base",
        "old_inn",
        true,
        0,
        MakeLinks());

    REQUIRE(result.legal);
    REQUIRE(result.hopCount == 2);
    REQUIRE(result.minutes == 30);
    REQUIRE(result.reason == gameplay::region::TravelBlockReason::None);
}

TEST_CASE("Region travel rules block travel when no path exists") {
    const std::vector<data::RegionLinkDefinition> disconnectedLinks = {
        data::RegionLinkDefinition{"home_base", "town_center"},
        data::RegionLinkDefinition{"sunken_ruin", "mine_entrance"}
    };

    const auto result = gameplay::region::EvaluateTravel(
        "home_base",
        "mine_entrance",
        true,
        0,
        disconnectedLinks);

    REQUIRE_FALSE(result.legal);
    REQUIRE(result.hopCount == 0);
    REQUIRE(result.minutes == 0);
    REQUIRE(result.reason == gameplay::region::TravelBlockReason::NoRouteLink);
}

TEST_CASE("Region travel rules choose shortest hop path") {
    const std::vector<data::RegionLinkDefinition> linksWithShortcut = {
        data::RegionLinkDefinition{"home_base", "town_center"},
        data::RegionLinkDefinition{"town_center", "old_inn"},
        data::RegionLinkDefinition{"old_inn", "clocktower_square"},
        data::RegionLinkDefinition{"home_base", "bridge_checkpoint"},
        data::RegionLinkDefinition{"bridge_checkpoint", "clocktower_square"}
    };

    const auto result = gameplay::region::EvaluateTravel(
        "home_base",
        "clocktower_square",
        true,
        0,
        linksWithShortcut);

    REQUIRE(result.legal);
    REQUIRE(result.hopCount == 2);
    REQUIRE(result.minutes == 30);
    REQUIRE(result.reason == gameplay::region::TravelBlockReason::None);
}

TEST_CASE("Region travel rules allow arrival exactly at 02:00") {
    const auto result = gameplay::region::EvaluateTravel(
        "home_base",
        "town_center",
        true,
        1185,
        MakeLinks());

    REQUIRE(result.legal);
    REQUIRE(result.hopCount == 1);
    REQUIRE(result.minutes == 15);
    REQUIRE(result.reason == gameplay::region::TravelBlockReason::None);
}

TEST_CASE("Region travel rules block travel that arrives past 02:00") {
    const auto result = gameplay::region::EvaluateTravel(
        "home_base",
        "town_center",
        true,
        1186,
        MakeLinks());

    REQUIRE_FALSE(result.legal);
    REQUIRE(result.hopCount == 1);
    REQUIRE(result.minutes == 15);
    REQUIRE(result.reason == gameplay::region::TravelBlockReason::ArrivalPastDayEnd);
}

TEST_CASE("Region travel rules block transit through uncleared blocker node") {
    const std::vector<data::RegionLinkDefinition> links = {
        data::RegionLinkDefinition{"home_base", "bridge_checkpoint"},
        data::RegionLinkDefinition{"bridge_checkpoint", "clocktower_square"}
    };

    const auto result = gameplay::region::EvaluateTravel(
        "home_base",
        "clocktower_square",
        true,
        0,
        links,
        {"bridge_checkpoint"});

    REQUIRE_FALSE(result.legal);
    REQUIRE(result.reason == gameplay::region::TravelBlockReason::BlockedByUnclearedTransitNode);
}

TEST_CASE("Region travel rules allow travel to uncleared blocker node itself") {
    const std::vector<data::RegionLinkDefinition> links = {
        data::RegionLinkDefinition{"home_base", "bridge_checkpoint"},
        data::RegionLinkDefinition{"bridge_checkpoint", "clocktower_square"}
    };

    const auto result = gameplay::region::EvaluateTravel(
        "home_base",
        "bridge_checkpoint",
        true,
        0,
        links,
        {"bridge_checkpoint"});

    REQUIRE(result.legal);
    REQUIRE(result.hopCount == 1);
    REQUIRE(result.reason == gameplay::region::TravelBlockReason::None);
}

TEST_CASE("Region travel rules do not block transit through uncleared non-blocker combat node") {
    const std::vector<data::RegionLinkDefinition> links = {
        data::RegionLinkDefinition{"home_base", "orchard_pass"},
        data::RegionLinkDefinition{"orchard_pass", "clocktower_square"}
    };

    const auto result = gameplay::region::EvaluateTravel(
        "home_base",
        "clocktower_square",
        true,
        0,
        links,
        {});

    REQUIRE(result.legal);
    REQUIRE(result.hopCount == 2);
    REQUIRE(result.reason == gameplay::region::TravelBlockReason::None);
}

TEST_CASE("EvaluateTravel hostile occupied destination is blocked when arrivalNodeId is empty") {
    // arrivalNodeId = "" → IsBlockedByHostileOccupation still fires → HostileOccupied
    const auto result = gameplay::region::EvaluateTravel(
        "home_base",
        "town_center",
        true,
        0,
        MakeLinks(),
        /*blockedTransitNodeIds=*/{},
        /*perHopTravelMinutes=*/15,
        /*hostileOccupiedNodeIds=*/{"town_center"},
        /*arrivalNodeId=*/{});

    REQUIRE(!result.legal);
    REQUIRE(result.reason == gameplay::region::TravelBlockReason::HostileOccupied);
}

TEST_CASE("EvaluateTravel hostile occupied arrival node is not blocked") {
    // destination == arrivalNodeId → IsBlockedByHostileOccupation returns false → legal
    const auto result = gameplay::region::EvaluateTravel(
        "home_base",
        "town_center",
        true,
        0,
        MakeLinks(),
        /*blockedTransitNodeIds=*/{},
        /*perHopTravelMinutes=*/15,
        /*hostileOccupiedNodeIds=*/{"town_center"},
        /*arrivalNodeId=*/"town_center");

    REQUIRE(result.legal);
    REQUIRE(result.reason == gameplay::region::TravelBlockReason::None);
}

TEST_CASE("EvaluateTravel destination not in hostile list is unaffected by occupation params") {
    // destination absent from hostileOccupiedNodeIds → passes through to normal evaluation
    const auto result = gameplay::region::EvaluateTravel(
        "home_base",
        "town_center",
        true,
        0,
        MakeLinks(),
        /*blockedTransitNodeIds=*/{},
        /*perHopTravelMinutes=*/15,
        /*hostileOccupiedNodeIds=*/{"old_inn"},
        /*arrivalNodeId=*/"home_base");

    REQUIRE(result.legal);
    REQUIRE(result.reason == gameplay::region::TravelBlockReason::None);
}

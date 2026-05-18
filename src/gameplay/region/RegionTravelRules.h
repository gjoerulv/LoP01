#pragma once

#include <string>
#include <vector>

#include "data/definitions/RegionDefinition.h"

namespace gameplay::region {

enum class TravelBlockReason {
    None,
    DestinationUnavailable,
    NoRouteLink,
    ArrivalPastDayEnd,
    BlockedByUnclearedTransitNode
};

struct TravelEvaluation {
    bool legal = false;
    int hopCount = 0;
    int minutes = 0;
    TravelBlockReason reason = TravelBlockReason::None;
};

[[nodiscard]] TravelEvaluation EvaluateTravel(
    const std::string& currentLocationId,
    const std::string& selectedLocationId,
    bool destinationTravelAvailable,
    int minutesIntoSliceDay,
    const std::vector<data::RegionLinkDefinition>& routeLinks,
    const std::vector<std::string>& blockedTransitNodeIds = {},
    int perHopTravelMinutes = 15);

// Returns the shortest hop count between two nodes via bidirectional links,
// or -1 if unreachable. No blocked-transit filter applied.
[[nodiscard]] int FindHopCount(
    const std::string& fromLocationId,
    const std::string& toLocationId,
    const std::vector<data::RegionLinkDefinition>& routeLinks);

} // namespace gameplay::region

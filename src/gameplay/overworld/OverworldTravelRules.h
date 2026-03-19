#pragma once

#include <string>
#include <vector>

#include "data/definitions/RegionDefinition.h"

namespace gameplay::overworld {

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

} // namespace gameplay::overworld

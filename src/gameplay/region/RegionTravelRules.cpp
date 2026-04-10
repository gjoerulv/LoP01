#include "gameplay/region/RegionTravelRules.h"

#include <algorithm>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "core/GameClock.h"

namespace gameplay::region {

namespace {

int FindShortestHopCount(
    const std::string& currentLocationId,
    const std::string& selectedLocationId,
    const std::vector<data::RegionLinkDefinition>& routeLinks,
    const std::unordered_set<std::string>& blockedTransitNodeIds) {
    if (currentLocationId == selectedLocationId) {
        return 0;
    }

    std::unordered_map<std::string, std::vector<std::string>> adjacency;
    for (const auto& link : routeLinks) {
        if (link.fromLocationId.empty() || link.toLocationId.empty()) {
            continue;
        }

        adjacency[link.fromLocationId].push_back(link.toLocationId);
        adjacency[link.toLocationId].push_back(link.fromLocationId);
    }

    if (!adjacency.contains(currentLocationId) || !adjacency.contains(selectedLocationId)) {
        return -1;
    }

    std::queue<std::string> frontier;
    std::unordered_map<std::string, int> distanceByNode;
    frontier.push(currentLocationId);
    distanceByNode[currentLocationId] = 0;

    while (!frontier.empty()) {
        const std::string node = frontier.front();
        frontier.pop();

        const int nodeDistance = distanceByNode[node];
        if (node == selectedLocationId) {
            return nodeDistance;
        }

        const auto it = adjacency.find(node);
        if (it == adjacency.end()) {
            continue;
        }

        for (const auto& neighbor : it->second) {
            if (distanceByNode.contains(neighbor)) {
                continue;
            }

            if (blockedTransitNodeIds.contains(neighbor) && neighbor != selectedLocationId) {
                continue;
            }

            distanceByNode[neighbor] = nodeDistance + 1;
            frontier.push(neighbor);
        }
    }

    return -1;
}

} // namespace

TravelEvaluation EvaluateTravel(
    const std::string& currentLocationId,
    const std::string& selectedLocationId,
    const bool destinationTravelAvailable,
    const int minutesIntoSliceDay,
    const std::vector<data::RegionLinkDefinition>& routeLinks,
    const std::vector<std::string>& blockedTransitNodeIds,
    const int perHopTravelMinutes) {
    if (currentLocationId == selectedLocationId) {
        return TravelEvaluation{ true, 0, 0, TravelBlockReason::None };
    }

    if (!destinationTravelAvailable) {
        return TravelEvaluation{ false, 0, 0, TravelBlockReason::DestinationUnavailable };
    }

    const std::unordered_set<std::string> blockedSet(blockedTransitNodeIds.begin(), blockedTransitNodeIds.end());
    const int hopCount = FindShortestHopCount(currentLocationId, selectedLocationId, routeLinks, blockedSet);
    if (hopCount >= 0) {
        const int safePerHopMinutes = std::max(0, perHopTravelMinutes);
        const int travelMinutes = hopCount * safePerHopMinutes;
        const int safeMinutesIntoSliceDay = std::max(0, minutesIntoSliceDay);
        const int arrivalMinutesIntoSliceDay = safeMinutesIntoSliceDay + travelMinutes;
        if (arrivalMinutesIntoSliceDay > core::GameClock::kMinutesPerSliceDay) {
            return TravelEvaluation{ false, hopCount, travelMinutes, TravelBlockReason::ArrivalPastDayEnd };
        }

        return TravelEvaluation{ true, hopCount, travelMinutes, TravelBlockReason::None };
    }

    const int hopCountIgnoringBlockedTransit = FindShortestHopCount(
        currentLocationId,
        selectedLocationId,
        routeLinks,
        {});
    if (hopCountIgnoringBlockedTransit >= 0) {
        return TravelEvaluation{ false, 0, 0, TravelBlockReason::BlockedByUnclearedTransitNode };
    }

    return TravelEvaluation{ false, 0, 0, TravelBlockReason::NoRouteLink };
}

} // namespace gameplay::region

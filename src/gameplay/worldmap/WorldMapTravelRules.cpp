#include "gameplay/worldmap/WorldMapTravelRules.h"

#include <algorithm>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace gameplay::worldmap {

bool IsWorldMapExitNode(
    const data::WorldMapRegionEntry& entry,
    const std::string& currentNodeId) {
    return std::find(entry.exitNodeIds.begin(), entry.exitNodeIds.end(), currentNodeId)
        != entry.exitNodeIds.end();
}

std::string DescribeWorldMapTravelBlockReason(WorldMapTravelBlockReason reason) {
    switch (reason) {
        case WorldMapTravelBlockReason::None:                  return "Travel available";
        case WorldMapTravelBlockReason::AlreadyHere:           return "Already in this region";
        case WorldMapTravelBlockReason::DestinationLocked:     return "Region locked";
        case WorldMapTravelBlockReason::NoPath:                return "No route";
        case WorldMapTravelBlockReason::PastDepartureDeadline: return "Too late (after 11:00)";
        case WorldMapTravelBlockReason::InsufficientEnergy:    return "Not enough Energy";
        case WorldMapTravelBlockReason::NotAtExitNode:         return "Not at an exit node";
    }
    return "Travel unavailable";
}

int FindRegionHopCount(
    const std::string& fromRegionId,
    const std::string& toRegionId,
    const std::vector<std::string>& unlockedRegionIds,
    const std::vector<data::WorldMapAdjacency>& adjacency) {
    const std::unordered_set<std::string> unlocked(
        unlockedRegionIds.begin(), unlockedRegionIds.end());

    // Both endpoints must be unlocked for any path to exist.
    if (!unlocked.contains(fromRegionId) || !unlocked.contains(toRegionId)) {
        return -1;
    }
    if (fromRegionId == toRegionId) {
        return 0;
    }

    // Build a bidirectional adjacency map restricted to unlocked regions: a
    // locked region on either endpoint of an edge removes that edge.
    std::unordered_map<std::string, std::vector<std::string>> graph;
    for (const auto& edge : adjacency) {
        if (edge.regionA.empty() || edge.regionB.empty()) {
            continue;
        }
        if (!unlocked.contains(edge.regionA) || !unlocked.contains(edge.regionB)) {
            continue;
        }
        graph[edge.regionA].push_back(edge.regionB);
        graph[edge.regionB].push_back(edge.regionA);
    }

    std::queue<std::string> frontier;
    std::unordered_map<std::string, int> distance;
    frontier.push(fromRegionId);
    distance[fromRegionId] = 0;

    while (!frontier.empty()) {
        const std::string node = frontier.front();
        frontier.pop();
        const int nodeDistance = distance[node];
        if (node == toRegionId) {
            return nodeDistance;
        }

        const auto it = graph.find(node);
        if (it == graph.end()) {
            continue;
        }
        for (const auto& neighbor : it->second) {
            if (distance.contains(neighbor)) {
                continue;
            }
            distance[neighbor] = nodeDistance + 1;
            frontier.push(neighbor);
        }
    }

    return -1;
}

WorldMapTravelEvaluation EvaluateWorldMapTravel(
    const std::string& currentRegionId,
    const std::string& destinationRegionId,
    const std::vector<std::string>& unlockedRegionIds,
    const std::vector<data::WorldMapAdjacency>& adjacency,
    const int minutesIntoSliceDay,
    const int currentEnergy,
    const int energyCost,
    const int departureDeadlineMinutes) {
    if (currentRegionId == destinationRegionId) {
        return WorldMapTravelEvaluation{ false, 0, WorldMapTravelBlockReason::AlreadyHere };
    }

    const std::unordered_set<std::string> unlocked(
        unlockedRegionIds.begin(), unlockedRegionIds.end());
    if (!unlocked.contains(destinationRegionId)) {
        return WorldMapTravelEvaluation{ false, 0, WorldMapTravelBlockReason::DestinationLocked };
    }

    // Cheap time/Energy gates before the path search.
    if (minutesIntoSliceDay >= departureDeadlineMinutes) {
        return WorldMapTravelEvaluation{ false, 0, WorldMapTravelBlockReason::PastDepartureDeadline };
    }
    if (currentEnergy < energyCost) {
        return WorldMapTravelEvaluation{ false, 0, WorldMapTravelBlockReason::InsufficientEnergy };
    }

    const int hopCount = FindRegionHopCount(
        currentRegionId, destinationRegionId, unlockedRegionIds, adjacency);
    if (hopCount <= 0) {
        // hopCount == 0 only when from == to, already handled; <0 = unreachable.
        return WorldMapTravelEvaluation{ false, 0, WorldMapTravelBlockReason::NoPath };
    }

    return WorldMapTravelEvaluation{ true, hopCount, WorldMapTravelBlockReason::None };
}

} // namespace gameplay::worldmap

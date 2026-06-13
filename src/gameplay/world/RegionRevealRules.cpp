#include "gameplay/world/RegionRevealRules.h"

#include <deque>
#include <unordered_map>
#include <unordered_set>

namespace gameplay::world {

std::vector<std::string> NodesWithinGraphRadius(
    const data::RegionDefinition& region,
    const std::string& centerNodeId,
    const int radius) {
    std::vector<std::string> result;
    if (centerNodeId.empty()) {
        return result;
    }

    // The center must be an actual node in the Region; otherwise reveal nothing.
    bool centerIsNode = false;
    for (const auto& node : region.nodes) {
        if (node.locationId == centerNodeId) {
            centerIsNode = true;
            break;
        }
    }
    if (!centerIsNode) {
        return result;
    }

    // Undirected adjacency from the authored links.
    std::unordered_map<std::string, std::vector<std::string>> adjacency;
    for (const auto& link : region.links) {
        if (link.fromLocationId.empty() || link.toLocationId.empty() ||
            link.fromLocationId == link.toLocationId) {
            continue;
        }
        adjacency[link.fromLocationId].push_back(link.toLocationId);
        adjacency[link.toLocationId].push_back(link.fromLocationId);
    }

    std::unordered_set<std::string> visited;
    visited.insert(centerNodeId);
    result.push_back(centerNodeId);

    if (radius <= 0) {
        return result;
    }

    // BFS bounded by `radius` hops.
    std::deque<std::pair<std::string, int>> frontier;
    frontier.push_back({centerNodeId, 0});
    while (!frontier.empty()) {
        const auto [nodeId, dist] = frontier.front();
        frontier.pop_front();
        if (dist >= radius) {
            continue;
        }
        const auto it = adjacency.find(nodeId);
        if (it == adjacency.end()) {
            continue;
        }
        for (const auto& neighbor : it->second) {
            if (visited.insert(neighbor).second) {
                result.push_back(neighbor);
                frontier.push_back({neighbor, dist + 1});
            }
        }
    }

    return result;
}

} // namespace gameplay::world

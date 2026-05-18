#pragma once

#include <algorithm>
#include <string>
#include <vector>

namespace gameplay {

// Returns true if nodeId is hostile-occupied and therefore inaccessible to the player.
// Arrival nodes are always accessible and are never considered blocked.
[[nodiscard]] inline bool IsBlockedByHostileOccupation(
    const std::string& nodeId,
    const std::string& arrivalNodeId,
    const std::vector<std::string>& hostileOccupiedNodeIds) {
    if (!arrivalNodeId.empty() && nodeId == arrivalNodeId) {
        return false;
    }
    return std::find(hostileOccupiedNodeIds.begin(),
                     hostileOccupiedNodeIds.end(),
                     nodeId) != hostileOccupiedNodeIds.end();
}

} // namespace gameplay

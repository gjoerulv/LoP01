#include "gameplay/world/NodeWorldState.h"

#include <algorithm>

namespace gameplay::world {

void NodeWorldState::MarkCombatNodeCleared(const std::string& nodeId) {
    if (nodeId.empty()) {
        return;
    }

    if (std::ranges::find(clearedCombatNodeIds_, nodeId) != clearedCombatNodeIds_.end()) {
        return;
    }

    clearedCombatNodeIds_.push_back(nodeId);
}

bool NodeWorldState::IsCombatNodeCleared(const std::string& nodeId) const {
    return std::ranges::find(clearedCombatNodeIds_, nodeId) != clearedCombatNodeIds_.end();
}

const std::vector<std::string>& NodeWorldState::ClearedCombatNodeIds() const {
    return clearedCombatNodeIds_;
}

void NodeWorldState::RestoreClearedCombatNodeIds(const std::vector<std::string>& nodeIds) {
    clearedCombatNodeIds_.clear();
    for (const auto& nodeId : nodeIds) {
        MarkCombatNodeCleared(nodeId);
    }
}

} // namespace gameplay::world

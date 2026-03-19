#pragma once

#include <string>
#include <vector>

namespace gameplay::world {

class NodeWorldState {
public:
    void MarkCombatNodeCleared(const std::string& nodeId);
    [[nodiscard]] bool IsCombatNodeCleared(const std::string& nodeId) const;
    [[nodiscard]] const std::vector<std::string>& ClearedCombatNodeIds() const;
    void RestoreClearedCombatNodeIds(const std::vector<std::string>& nodeIds);

private:
    std::vector<std::string> clearedCombatNodeIds_;
};

} // namespace gameplay::world

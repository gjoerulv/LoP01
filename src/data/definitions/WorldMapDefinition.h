#pragma once

#include <string>
#include <vector>

namespace data {

    // One Region's presence on the World Map. Carries inter-region travel
    // metadata only. Arrival nodes are NOT stored here — RegionDefinition::
    // arrivalNodeId remains the single source of truth (resolved at travel time).
    struct WorldMapRegionEntry {
        std::string id;                        // references a RegionDefinition::id
        bool unlocked = false;                 // authored start unlock state
        std::vector<std::string> exitNodeIds;  // Region nodes from which travel may begin
        float x = 0.0f;                        // optional layout hint (render only)
        float y = 0.0f;
    };

    // Bidirectional adjacency between two World Map region entries. Symmetric:
    // the loader/rules treat {a,b} the same as {b,a}, mirroring RegionLink.
    struct WorldMapAdjacency {
        std::string regionA;
        std::string regionB;
    };

    // Authored World Map: the scenario-level Region selection layer. Optional —
    // when absent, the Scenario has a single Region and no inter-region travel.
    struct WorldMapDefinition {
        std::string id;
        std::string name;
        std::vector<WorldMapRegionEntry> entries;
        std::vector<WorldMapAdjacency> adjacency;

        [[nodiscard]] const WorldMapRegionEntry* FindEntry(const std::string& regionId) const {
            for (const auto& entry : entries) {
                if (entry.id == regionId) {
                    return &entry;
                }
            }
            return nullptr;
        }
    };

}

#pragma once

#include <string>
#include <vector>

#include "data/definitions/RegionDefinition.h"

namespace gameplay::world {

// Pure reveal geometry for the M32 fog/reveal foundation. Returns the node ids
// within `radius` undirected graph hops of `centerNodeId` (inclusive of the
// center), traversing the Region's authored links. This is the bounded HoMM-like
// "reveal N nodes ahead" model (docs/core_loop_rules.md §15) expressed as graph
// distance rather than line-of-sight geometry.
//
// Rules:
//   - returns an empty list when `centerNodeId` is not a node in `region`;
//   - returns just the center when radius <= 0 (and the center is a node);
//   - links are treated as undirected; duplicate/self links are tolerated.
[[nodiscard]] std::vector<std::string> NodesWithinGraphRadius(
    const data::RegionDefinition& region,
    const std::string& centerNodeId,
    int radius);

} // namespace gameplay::world

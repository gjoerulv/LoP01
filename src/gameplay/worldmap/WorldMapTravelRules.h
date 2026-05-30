#pragma once

#include <string>
#include <vector>

#include "data/definitions/WorldMapDefinition.h"

namespace gameplay::worldmap {

// Why a given World Map trip is (il)legal. Mirrors the region-travel reason
// enum style. Precedence is documented in EvaluateWorldMapTravel.
enum class WorldMapTravelBlockReason {
    None,
    AlreadyHere,            // destination == current region
    DestinationLocked,      // destination region not in the unlocked set
    NoPath,                 // no contiguous path over unlocked adjacency
    PastDepartureDeadline,  // current time is at/after the 11:00 deadline
    InsufficientEnergy,     // party has < cost Energy
    NotAtExitNode           // set by the session layer only (not the Region
                            // layer, or not standing on a world-map exit node).
                            // EvaluateWorldMapTravel never returns this.
};

struct WorldMapTravelEvaluation {
    bool legal = false;
    int days = 0;                  // shortest-path hop count (adjacent = 1)
    WorldMapTravelBlockReason reason = WorldMapTravelBlockReason::None;
};

// Pure travel-legality + duration rule. Raylib-free and clock-free: callers
// pass the current time-of-day and Energy as plain values.
//
// `days` is the BFS hop count over adjacency restricted to unlocked regions
// (a locked intermediate region breaks the path). Adjacent regions => 1 day;
// N regions between => 1 + N days.
//
// Reason precedence when illegal:
//   AlreadyHere > DestinationLocked > PastDepartureDeadline >
//   InsufficientEnergy > NoPath
// (Cheap state checks first; the path search last.)
[[nodiscard]] WorldMapTravelEvaluation EvaluateWorldMapTravel(
    const std::string& currentRegionId,
    const std::string& destinationRegionId,
    const std::vector<std::string>& unlockedRegionIds,
    const std::vector<data::WorldMapAdjacency>& adjacency,
    int minutesIntoSliceDay,
    int currentEnergy,
    int energyCost = 1000,
    int departureDeadlineMinutes = 300);

// Shortest hop count between two regions over adjacency restricted to the
// unlocked set, or -1 if unreachable. Endpoints must themselves be unlocked.
[[nodiscard]] int FindRegionHopCount(
    const std::string& fromRegionId,
    const std::string& toRegionId,
    const std::vector<std::string>& unlockedRegionIds,
    const std::vector<data::WorldMapAdjacency>& adjacency);

// True when `currentNodeId` is one of the region entry's authored exit nodes.
[[nodiscard]] bool IsWorldMapExitNode(
    const data::WorldMapRegionEntry& entry,
    const std::string& currentNodeId);

} // namespace gameplay::worldmap

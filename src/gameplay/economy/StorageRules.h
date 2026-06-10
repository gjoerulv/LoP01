#pragma once

#include "data/definitions/LocationServiceDefinition.h"

namespace gameplay::economy {

// Maximum number of stored stack entries at one storage service. Per the
// final-vision rules (docs/core_loop_rules.md §4: "Each storage has up to 7
// slots"), this is DISTINCT from the mine-stationing cap of 5 — storage and
// stationing are different concepts and must not share a capacity.
inline constexpr int kMaxStoredUnitsPerService = 7;

// Pure gate: may a player-owned storage service RECEIVE another stored unit right
// now? True iff the service is a Storage kind, player-owned, not locked, not
// destroyed, and currently below capacity. Pure: no I/O, no global state.
[[nodiscard]] bool ServiceCanReceiveStoredUnit(
    data::LocationServiceKind kind,
    bool playerOwned,
    bool locked,
    bool destroyed,
    int currentStoredCount);

// Pure gate: is a specific roster stack eligible to be stored, given the facts the
// GameSession resolves about it? Eligible iff it is a real owned stack, is NOT the
// Player Character (who must stay with the travelling party), and is NOT already
// placed (stationed or stored) anywhere. Slot presence and the active-party leader
// guard are enforced separately by GameSession. Pure: no I/O.
[[nodiscard]] bool StackIsStorable(
    bool stackExists,
    bool isPlayerCharacter,
    bool alreadyPlaced);

} // namespace gameplay::economy

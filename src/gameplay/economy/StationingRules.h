#pragma once

#include "data/definitions/LocationServiceDefinition.h"

namespace gameplay::economy {

// Maximum number of stationed stack refs allowed at one eligible owned service.
// M25 stationing is guard/worker capacity only — not Storage/Garrison, stationed-
// defender combat, or siege. Mine production still applies strongest-only per
// resource regardless of how many units guard the mine, so capacity is about
// guard presence, not stacking payout.
inline constexpr int kMaxStationedUnitsPerService = 5;

// Pure gate: may a player-owned service RECEIVE another stationed unit right now?
// True iff the service is a Mine (M25 restricts stationing targets to mines even
// though other ownable kinds exist), player-owned, not locked, not destroyed, and
// currently below capacity. Pure: no I/O, no global state.
[[nodiscard]] bool ServiceCanReceiveStationedUnit(
    data::LocationServiceKind kind,
    bool playerOwned,
    bool locked,
    bool destroyed,
    int currentStationedCount);

// Pure gate: is a specific roster stack eligible to be stationed, given the facts
// the GameSession resolves about it? Eligible iff it is a real owned stack, is NOT
// the Player Character (who must stay with the travelling party), and is NOT
// already stationed anywhere. Slot presence and the active-party leader guard are
// enforced separately by GameSession, which needs live slot state. Pure: no I/O.
[[nodiscard]] bool StackIsStationable(
    bool stackExists,
    bool isPlayerCharacter,
    bool alreadyStationed);

// Pure gate: is `quantity` a legal SPLIT amount for a generic stack holding
// `stackQuantity`? A split must leave at least one unit on each side, so
// 1 <= quantity < stackQuantity. quantity == stackQuantity is a whole-stack
// station (handled separately); quantity <= 0 or >= stackQuantity is not a split.
[[nodiscard]] bool IsLegalSplitQuantity(int quantity, int stackQuantity);

} // namespace gameplay::economy

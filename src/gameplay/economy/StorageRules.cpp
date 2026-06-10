#include "gameplay/economy/StorageRules.h"

namespace gameplay::economy {

bool ServiceCanReceiveStoredUnit(
    const data::LocationServiceKind kind,
    const bool playerOwned,
    const bool locked,
    const bool destroyed,
    const int currentStoredCount) {
    if (kind != data::LocationServiceKind::Storage) {
        return false;
    }
    if (!playerOwned || locked || destroyed) {
        return false;
    }
    return currentStoredCount < kMaxStoredUnitsPerService;
}

bool StackIsStorable(
    const bool stackExists,
    const bool isPlayerCharacter,
    const bool alreadyPlaced) {
    return stackExists && !isPlayerCharacter && !alreadyPlaced;
}

} // namespace gameplay::economy

#include "gameplay/economy/StationingRules.h"

namespace gameplay::economy {

bool ServiceCanReceiveStationedUnit(
    const data::LocationServiceKind kind,
    const bool playerOwned,
    const bool locked,
    const bool destroyed,
    const int currentStationedCount) {
    if (kind != data::LocationServiceKind::Mine) {
        return false;
    }
    if (!playerOwned || locked || destroyed) {
        return false;
    }
    return currentStationedCount < kMaxStationedUnitsPerService;
}

bool StackIsStationable(
    const bool stackExists,
    const bool isPlayerCharacter,
    const bool alreadyStationed) {
    return stackExists && !isPlayerCharacter && !alreadyStationed;
}

bool IsLegalSplitQuantity(const int quantity, const int stackQuantity) {
    return quantity >= 1 && quantity < stackQuantity;
}

} // namespace gameplay::economy

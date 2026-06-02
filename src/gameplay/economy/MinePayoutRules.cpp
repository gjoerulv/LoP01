#include "gameplay/economy/MinePayoutRules.h"

namespace gameplay::economy {

bool MineServiceIsPayable(
    const data::LocationServiceKind kind,
    const bool hasAuthoredOutputs,
    const std::string& ownerTeamColor,
    const bool locked,
    const bool destroyed,
    const std::string& receivingTeamColor,
    const bool hostileOccupied) {
    if (kind != data::LocationServiceKind::Mine) {
        return false;
    }
    if (!hasAuthoredOutputs) {
        return false;
    }
    if (ownerTeamColor.empty() || ownerTeamColor != receivingTeamColor) {
        return false;
    }
    if (locked || destroyed || hostileOccupied) {
        return false;
    }
    return true;
}

} // namespace gameplay::economy

#include "gameplay/economy/ServiceDefenseRules.h"

#include <algorithm>

namespace gameplay::economy {

bool ServiceKindIsAttackable(const data::LocationServiceKind kind) {
    return kind == data::LocationServiceKind::Storage || data::IsOwnableServiceKind(kind);
}

int UnitDefensePower(const data::UnitStatsDefinition& stats) {
    return std::max(0, stats.attack) + std::max(0, stats.defense) + std::max(0, stats.maxHp);
}

int StackDefensePower(const data::UnitStatsDefinition& stats, const int quantity) {
    return UnitDefensePower(stats) * std::max(0, quantity);
}

bool DefendersHoldService(const int defenderPower, const int attackerPower) {
    return defenderPower > 0 && defenderPower >= attackerPower;
}

} // namespace gameplay::economy

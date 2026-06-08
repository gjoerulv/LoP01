#include "gameplay/economy/ServiceClaimRules.h"

namespace gameplay::economy {

bool ServiceIsClaimable(
    const data::LocationServiceKind kind,
    const bool locked,
    const bool destroyed,
    const ServiceOwnerRelationship ownerRelationship) {
    if (!data::IsOwnableServiceKind(kind)) {
        return false;
    }
    if (locked || destroyed) {
        return false;
    }
    switch (ownerRelationship) {
    case ServiceOwnerRelationship::Unowned:
    case ServiceOwnerRelationship::HostileToPlayer:
        return true;
    case ServiceOwnerRelationship::Player:
    case ServiceOwnerRelationship::AlliedToPlayer:
        return false;
    }
    return false;
}

} // namespace gameplay::economy

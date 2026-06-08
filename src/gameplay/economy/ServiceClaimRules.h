#pragma once

#include "data/definitions/LocationServiceDefinition.h"

namespace gameplay::economy {

// Relationship of an owned service's current owner to the claiming player.
// Resolved by the caller from the owner team color (empty => Unowned) and the
// existing alliance determination.
enum class ServiceOwnerRelationship {
    Unowned,
    Player,
    HostileToPlayer,
    AlliedToPlayer
};

// Pure gate: may the player claim this ownable service after defeating the
// hostile team guarding its node? Ownership never bypasses ordinary availability
// rules (docs/core_loop_rules.md §18, docs/content_scope_v1.md §5), so a service
// is claimable iff ALL hold:
//   * kind is an ownable service kind (mine / trader);
//   * it is not locked;
//   * it is not destroyed;
//   * its current owner is unowned/neutral OR hostile to the player.
// Already player-owned services are left unchanged; allied-owned services are
// conservatively not claimed in M23. Pure: no I/O, no global state.
[[nodiscard]] bool ServiceIsClaimable(
    data::LocationServiceKind kind,
    bool locked,
    bool destroyed,
    ServiceOwnerRelationship ownerRelationship);

} // namespace gameplay::economy

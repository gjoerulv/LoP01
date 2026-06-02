#pragma once

#include <string>

#include "data/definitions/LocationServiceDefinition.h"

namespace gameplay::economy {

// Pure gate: may an owned mine/resource service pay its daily output to
// `receivingTeamColor` right now? Ownership never bypasses ordinary availability
// rules (docs/core_loop_rules.md §"Ownership"/§"Mines"), so a service pays iff
// ALL hold:
//   * kind is the mine/resource-producing kind;
//   * it has authored outputs;
//   * it is owned (ownerTeamColor non-empty) by the receiving team (allied /
//     enemy / neutral owners do not pay into this team);
//   * it is not locked;
//   * it is not destroyed;
//   * its node is not blocked by hostile occupation.
// Pure: no I/O, no global state. The caller resolves `hostileOccupied` from the
// existing occupation rules and supplies the receiving team.
[[nodiscard]] bool MineServiceIsPayable(
    data::LocationServiceKind kind,
    bool hasAuthoredOutputs,
    const std::string& ownerTeamColor,
    bool locked,
    bool destroyed,
    const std::string& receivingTeamColor,
    bool hostileOccupied);

} // namespace gameplay::economy

#pragma once

#include <vector>

#include "data/definitions/LocationServiceDefinition.h"
#include "data/definitions/UnitDefinition.h"
#include "gameplay/economy/MineProductionRules.h"

namespace gameplay::economy {

// Pure collection of applicable mine-production passives from a set of stationed
// unit definitions, for a producing service of `serviceKind`.
//
// A unit contributes a passive for each MineProduction entry in its
// passiveEffects whose:
//   * target matches `serviceKind` (target "mine" <-> LocationServiceKind::Mine);
//   * resource parses to a valid ResourceType; and
//   * amount is positive.
// Unit category (hero / leader / generic) is NEVER consulted — eligibility is
// purely "does this unit's definition carry an applicable passive". Null entries
// are skipped. No deduplication or strongest-only resolution happens here; that
// is ComputeMineDailyOutput's job. Pure: no I/O, no global state.
[[nodiscard]] std::vector<MineProductionPassive> CollectMineProductionPassives(
    const std::vector<const data::UnitDefinition*>& stationedUnits,
    data::LocationServiceKind serviceKind);

} // namespace gameplay::economy

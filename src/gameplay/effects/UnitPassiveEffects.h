#pragma once

#include <vector>

#include "data/definitions/UnitDefinition.h"

namespace gameplay::effects {

// Returns pointers to a unit's passive effects of the given kind, in authored
// order. Reads only the canonical UnitDefinition::passiveEffects vector (the
// single source of truth; the legacy mine_production_passive JSON key is folded
// into it at load). Pure: no I/O, no global state. Pointers are valid for the
// lifetime of `unit`.
[[nodiscard]] std::vector<const data::UnitPassiveEffect*> CollectUnitPassiveEffects(
    const data::UnitDefinition& unit, data::PassiveEffectKind kind);

} // namespace gameplay::effects

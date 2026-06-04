#include "gameplay/effects/UnitPassiveEffects.h"

namespace gameplay::effects {

std::vector<const data::UnitPassiveEffect*> CollectUnitPassiveEffects(
    const data::UnitDefinition& unit, data::PassiveEffectKind kind) {
    std::vector<const data::UnitPassiveEffect*> matches;
    for (const auto& effect : unit.passiveEffects) {
        if (effect.kind == kind) {
            matches.push_back(&effect);
        }
    }
    return matches;
}

} // namespace gameplay::effects

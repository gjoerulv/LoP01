#include "gameplay/economy/StationedProductionRules.h"

#include "gameplay/ResourceState.h"
#include "gameplay/effects/UnitPassiveEffects.h"

namespace gameplay::economy {

namespace {

// Maps an authored passive `target` string to a producing-service kind. Only
// "mine" is meaningful in M17; anything else matches no producing service.
[[nodiscard]] bool TargetMatchesServiceKind(const std::string& target,
                                             data::LocationServiceKind serviceKind) {
    if (serviceKind == data::LocationServiceKind::Mine) {
        return target == "mine";
    }
    return false;
}

} // namespace

std::vector<MineProductionPassive> CollectMineProductionPassives(
    const std::vector<const data::UnitDefinition*>& stationedUnits,
    data::LocationServiceKind serviceKind) {

    std::vector<MineProductionPassive> passives;

    for (const auto* unit : stationedUnits) {
        if (unit == nullptr) {
            continue;
        }
        for (const auto* effect : gameplay::effects::CollectUnitPassiveEffects(
                 *unit, data::PassiveEffectKind::MineProduction)) {
            if (!TargetMatchesServiceKind(effect->target, serviceKind)) {
                continue;
            }
            if (effect->amount <= 0) {
                continue;
            }
            ResourceType resource;
            if (!TryResourceTypeFromString(effect->resource, resource)) {
                continue;
            }
            passives.push_back(MineProductionPassive{resource, effect->amount});
        }
    }

    return passives;
}

} // namespace gameplay::economy

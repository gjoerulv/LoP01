#include "gameplay/economy/StationedProductionRules.h"

#include "gameplay/ResourceState.h"

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
        if (unit == nullptr || !unit->mineProductionPassive.has_value()) {
            continue;
        }
        const auto& authored = *unit->mineProductionPassive;

        if (!TargetMatchesServiceKind(authored.target, serviceKind)) {
            continue;
        }
        if (authored.amount <= 0) {
            continue;
        }
        ResourceType resource;
        if (!TryResourceTypeFromString(authored.resource, resource)) {
            continue;
        }

        passives.push_back(MineProductionPassive{resource, authored.amount});
    }

    return passives;
}

} // namespace gameplay::economy

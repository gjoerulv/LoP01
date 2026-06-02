#include "gameplay/economy/MineProductionRules.h"

#include <algorithm>

namespace gameplay::economy {

std::vector<MineResourceOutput> ComputeMineDailyOutput(
    const std::vector<MineResourceOutput>& baseOutputs,
    const std::vector<MineProductionPassive>& applicableStationedPassives) {

    std::vector<MineResourceOutput> result = baseOutputs;

    for (auto& output : result) {
        // Strongest-only, non-stacking: take the single largest applicable
        // passive bonus for THIS output's resource. Ties collapse to one value
        // (max of equal numbers is that number), so equal-strength passives do
        // not stack. Passives for other resources are skipped here, and because
        // we only iterate existing base outputs, a passive for a resource the
        // mine does not produce can never introduce a new output line.
        int strongestBonus = 0;
        bool anyApplicable = false;
        for (const auto& passive : applicableStationedPassives) {
            if (passive.resource != output.resource) {
                continue;
            }
            if (!anyApplicable || passive.amount > strongestBonus) {
                strongestBonus = passive.amount;
                anyApplicable = true;
            }
        }

        if (anyApplicable) {
            output.amount += strongestBonus;
        }
        output.amount = std::max(0, output.amount);
    }

    return result;
}

} // namespace gameplay::economy

#pragma once

#include <string>

#include "data/definitions/LocationServiceDefinition.h"

namespace app {

    struct LocationServicePromptContext {
        int remainingRecruitStock = 0;
        int currentWeek = 1;
        int remainingDailyUses = 0;
        bool hasActiveTravelPrep = false;
    };

    [[nodiscard]] std::string BuildLocationServicePrompt(
        const data::LocationServiceDefinition& service,
        const LocationServicePromptContext& context);

} // namespace app
#pragma once

#include <string>
#include <vector>

#include "gameplay/battle/Battle.h"

namespace app
{
    class BattleEventTextFormatter
    {
    public:
        [[nodiscard]] std::string FormatSummary(
            const std::vector<gameplay::battle::BattleEvent>& events) const;
    };
}
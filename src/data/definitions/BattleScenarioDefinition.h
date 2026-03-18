#pragma once

#include <string>
#include <vector>

namespace data
{
    struct BattleScenarioEntryDefinition
    {
        std::string unitId;
        int lifeOverride = 0;
    };

    struct BattleScenarioDefinition
    {
        std::string id;
        std::string name;
        unsigned int seed = 7;
        std::vector<BattleScenarioEntryDefinition> allies;
        std::vector<BattleScenarioEntryDefinition> enemies;
    };
}
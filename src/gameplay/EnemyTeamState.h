#pragma once

#include <string>
#include <vector>

namespace gameplay {

enum class Personality { Warrior, Builder, Explorer };
enum class Aggression { Berserk, Reckless, Opportunistic, Careful, Coward, Pacifist };

struct EnemyPatrol {
    bool enabled = false;
    std::string centerNodeId;
    int radius = 0;
};

struct EnemyTeamState {
    std::string teamColor;
    std::string nodeId;   // maps to RegionNodeDefinition::locationId
    Personality personality = Personality::Warrior;
    Aggression aggression = Aggression::Opportunistic;
    EnemyPatrol patrol;
    std::vector<std::string> alliances;
    bool active = true;
    int energy = 0;                    // placeholder; full formula in M11-b+
    int cooldownExpiresAtMinutes = 0;
    // M30: authored enemy-group id giving the team its deterministic service-
    // attack strength (EnemyGroupDefinition lookup). Empty -> zero strength;
    // the team can still capture undefended services.
    std::string enemyGroupId;
};

} // namespace gameplay

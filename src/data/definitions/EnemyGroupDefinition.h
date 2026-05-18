#pragma once

#include <string>
#include <vector>

namespace data {

struct EnemyGroupDefinition {
    std::string id;
    std::string name;
    std::vector<std::string> unitIds;
};

} // namespace data

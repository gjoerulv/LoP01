#include "data/ContentRepository.h"

#include <fstream>

namespace data {

namespace {

bool LoadJsonFile(const std::filesystem::path& path, nlohmann::json& output) {
    std::ifstream input(path);
    if (!input.is_open()) {
        return false;
    }

    input >> output;
    return true;
}

} // namespace

bool ContentRepository::LoadFromDirectory(const std::filesystem::path& root) {
    const bool regionsLoaded = LoadJsonFile(root / "regions.json", regions_);
    const bool locationsLoaded = LoadJsonFile(root / "locations.json", locations_);
    const bool unitsLoaded = LoadJsonFile(root / "units.json", units_);
    const bool enemyGroupsLoaded = LoadJsonFile(root / "enemy_groups.json", enemyGroups_);
    const bool questsLoaded = LoadJsonFile(root / "quests.json", quests_);

    return regionsLoaded && locationsLoaded && unitsLoaded && enemyGroupsLoaded && questsLoaded;
}

const nlohmann::json& ContentRepository::Regions() const {
    return regions_;
}

const nlohmann::json& ContentRepository::Locations() const {
    return locations_;
}

const nlohmann::json& ContentRepository::Units() const {
    return units_;
}

const nlohmann::json& ContentRepository::EnemyGroups() const {
    return enemyGroups_;
}

const nlohmann::json& ContentRepository::Quests() const {
    return quests_;
}

} // namespace data

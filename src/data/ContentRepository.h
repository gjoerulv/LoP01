#pragma once

#include <filesystem>

#include <nlohmann/json.hpp>

namespace data {

class ContentRepository {
public:
    [[nodiscard]] bool LoadFromDirectory(const std::filesystem::path& root);

    [[nodiscard]] const nlohmann::json& Regions() const;
    [[nodiscard]] const std::vector<LocationDefinition>& Locations() const;
    [[nodiscard]] const LocationDefinition* FindLocationById(const std::string& id) const;
    [[nodiscard]] const nlohmann::json& Units() const;
    [[nodiscard]] const nlohmann::json& EnemyGroups() const;
    [[nodiscard]] const nlohmann::json& Quests() const;

private:
    nlohmann::json regions_;
    nlohmann::json locations_;
    nlohmann::json units_;
    nlohmann::json enemyGroups_;
    nlohmann::json quests_;
};

} // namespace data

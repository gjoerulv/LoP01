#include "core/SaveGame.h"

#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>

namespace core {

using nlohmann::json;

void to_json(json& j, const SaveData& data) {
    j = json{
        {"day", data.day},
        {"minutes_into_slice_day", data.minutesIntoSliceDay},
        {"gold", data.gold},
        {"mode", data.mode},
        {"region_id", data.regionId},
        {"destination_id", data.destinationId},
        {"completed_quest_ids", data.completedQuestIds}
    };
}

void from_json(const json& j, SaveData& data) {
    j.at("day").get_to(data.day);
    j.at("minutes_into_slice_day").get_to(data.minutesIntoSliceDay);
    j.at("gold").get_to(data.gold);
    j.at("mode").get_to(data.mode);
    j.at("region_id").get_to(data.regionId);
    j.at("destination_id").get_to(data.destinationId);

    data.completedQuestIds.clear();
    if (j.contains("completed_quest_ids") && j["completed_quest_ids"].is_array()) {
        data.completedQuestIds = j["completed_quest_ids"].get<std::vector<std::string>>();
    }
}

bool SaveGameRepository::SaveToFile(const SaveData& saveData, const std::string& filePath) const {
    std::filesystem::path path(filePath);
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path());
    }

    std::ofstream output(path, std::ios::trunc);
    if (!output.is_open()) {
        return false;
    }

    output << json(saveData).dump(2);
    return true;
}

std::optional<SaveData> SaveGameRepository::LoadFromFile(const std::string& filePath) const {
    std::ifstream input(filePath);
    if (!input.is_open()) {
        return std::nullopt;
    }

    json j;
    input >> j;
    return j.get<SaveData>();
}

} // namespace core

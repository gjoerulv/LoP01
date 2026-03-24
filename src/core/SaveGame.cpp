#include "core/SaveGame.h"

#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>

namespace core {

using nlohmann::json;

void to_json(json& j, const RecruitServiceState& data) {
    j = json{
        {"service_id", data.serviceId},
        {"remaining_stock", data.remainingStock},
        {"last_refresh_week", data.lastRefreshWeek}
    };
}

void to_json(json& j, const DailyServiceState& data) {
    j = json{
        {"service_id", data.serviceId},
        {"remaining_uses_today", data.remainingUsesToday},
        {"last_refresh_day", data.lastRefreshDay}
    };
}

void from_json(const json& j, RecruitServiceState& data) {
    j.at("service_id").get_to(data.serviceId);
    j.at("remaining_stock").get_to(data.remainingStock);
    j.at("last_refresh_week").get_to(data.lastRefreshWeek);
}

void from_json(const json& j, DailyServiceState& data) {
    j.at("service_id").get_to(data.serviceId);
    j.at("remaining_uses_today").get_to(data.remainingUsesToday);
    j.at("last_refresh_day").get_to(data.lastRefreshDay);
}

void to_json(json& j, const SaveData& data) {
    j = json{
        {"day", data.day},
        {"minutes_into_slice_day", data.minutesIntoSliceDay},
        {"gold", data.gold},
        {"mode", data.mode},
        {"region_id", data.regionId},
        {"destination_id", data.destinationId},
        {"completed_quest_ids", data.completedQuestIds},
        {"cleared_combat_node_ids", data.clearedCombatNodeIds},
        {"recruit_service_states", data.recruitServiceStates},
        {"daily_service_states", data.dailyServiceStates},
        {"travel_prep_discount_minutes", data.travelPrepDiscountMinutes},
        {"travel_prep_remaining_charges", data.travelPrepRemainingCharges},
        {"travel_prep_granted_day", data.travelPrepGrantedDay}
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

    data.clearedCombatNodeIds.clear();
    if (j.contains("cleared_combat_node_ids") && j["cleared_combat_node_ids"].is_array()) {
        data.clearedCombatNodeIds = j["cleared_combat_node_ids"].get<std::vector<std::string>>();
    }
    data.recruitServiceStates.clear();
    if (j.contains("recruit_service_states") && j["recruit_service_states"].is_array()) {
        data.recruitServiceStates = j["recruit_service_states"].get<std::vector<RecruitServiceState>>();
    }

    data.dailyServiceStates.clear();
    if (j.contains("daily_service_states") && j["daily_service_states"].is_array()) {
        data.dailyServiceStates = j["daily_service_states"].get<std::vector<DailyServiceState>>();
    }

    data.travelPrepDiscountMinutes = j.value("travel_prep_discount_minutes", 0);
    data.travelPrepRemainingCharges = j.value("travel_prep_remaining_charges", 0);
    data.travelPrepGrantedDay = j.value("travel_prep_granted_day", 0);
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

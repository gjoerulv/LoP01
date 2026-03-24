#pragma once

#include <optional>
#include <string>
#include <vector>

namespace core {

struct RecruitServiceState {
    std::string serviceId;
    int remainingStock = 0;
    int lastRefreshWeek = 1;
};

struct DailyServiceState {
    std::string serviceId;
    int remainingUsesToday = 0;
    int lastRefreshDay = 1;
};

struct SaveData {
    int day = 1;
    int minutesIntoSliceDay = 0;
    int gold = 0;
    std::string mode;
    std::string regionId;
    std::string destinationId;
    std::vector<std::string> completedQuestIds;
    std::vector<std::string> clearedCombatNodeIds;
    std::vector<RecruitServiceState> recruitServiceStates;
    std::vector<DailyServiceState> dailyServiceStates;
    int travelPrepDiscountMinutes = 0;
    int travelPrepRemainingCharges = 0;
    int travelPrepGrantedDay = 0;
};

class SaveGameRepository {
public:
    [[nodiscard]] bool SaveToFile(const SaveData& saveData, const std::string& filePath) const;
    [[nodiscard]] std::optional<SaveData> LoadFromFile(const std::string& filePath) const;
};

} // namespace core

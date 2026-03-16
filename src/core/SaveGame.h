#pragma once

#include <optional>
#include <string>

namespace core {

struct SaveData {
    int day = 1;
    int minutesIntoSliceDay = 0;
    int gold = 0;
    std::string mode;
    std::string regionId;
    std::string destinationId;
};

class SaveGameRepository {
public:
    [[nodiscard]] bool SaveToFile(const SaveData& saveData, const std::string& filePath) const;
    [[nodiscard]] std::optional<SaveData> LoadFromFile(const std::string& filePath) const;
};

} // namespace core

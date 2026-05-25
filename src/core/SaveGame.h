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

struct OwnedUnitCountSaveState {
    std::string unitId;
    int count = 0;
};

struct RosterStackSaveState {
    std::string stackId;
    std::string unitId;
    int quantity = 0;
};

struct EnemyTeamSaveState {
    std::string teamColor;
    std::string nodeId;
    bool active = false;
    int energy = 0;
    int cooldownExpiresAtMinutes = 0;
    std::vector<std::string> alliances;
};

struct SaveData {
    int schemaVersion = 1;
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

    bool hasCanonicalRoster = false;
    std::vector<RosterStackSaveState> rosterStacks;
    std::vector<std::string> activeSlotStackIds;
    std::vector<std::string> reserveSlotStackIds;
    int nextStackIdCounter = 1;

    std::vector<OwnedUnitCountSaveState> ownedUnitCounts;
    std::vector<std::string> activePartyUnitIds;

    std::vector<std::string> firedEventIds;
    std::vector<std::string> storyFlags;
    std::vector<EnemyTeamSaveState> enemyTeams;

    // Latched scenario outcome. State is "" (Ongoing / not latched), "victory",
    // or "defeat". matchedConditionIndex is -1 when absent (default victory or
    // unset). reason is the human-readable text captured at latch time.
    std::string scenarioOutcomeState;
    int scenarioOutcomeMatchedConditionIndex = -1;
    std::string scenarioOutcomeReason;
};

class SaveGameRepository {
public:
    [[nodiscard]] bool SaveToFile(const SaveData& saveData, const std::string& filePath) const;
    [[nodiscard]] std::optional<SaveData> LoadFromFile(const std::string& filePath) const;
};

} // namespace core

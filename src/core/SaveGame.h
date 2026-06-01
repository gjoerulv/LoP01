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

// M13-b runtime inventory persistence. items and artifacts are flat
// {id, quantity} stacks. heroEquipment is per-hero with the five named slots
// from gameplay::HeroEquipmentState. Legacy saves (no M13-b keys) load as
// empty inventories and equipment — no schemaVersion bump.
struct ItemSaveState {
    std::string itemId;
    int quantity = 0;
};

struct ArtifactSaveState {
    std::string artifactId;
    int quantity = 0;
};

struct HeroEquipmentSaveState {
    std::string heroId;
    std::string attackArtifactId;
    std::string defenseArtifactId;
    std::string misc1ArtifactId;
    std::string misc2ArtifactId;
    std::string misc3ArtifactId;
};

// M17 team resource pool persistence. resource is a canonical non-gold resource
// name ("Wood", "Stone", ...). Gold is NEVER persisted here — it lives solely in
// SaveData::gold (single source of truth). Legacy saves (no M17 keys) load as an
// empty resource vector — no schemaVersion bump.
struct ResourceSaveState {
    std::string resource;
    int amount = 0;
};

// M17 owned-service runtime state. Stable fields only (Phase 1). Stationing is
// deferred to a later milestone and added additively; no stationedUnits field
// here. serviceId is the global owned-service instance key
// (LocationServiceDefinition::id, validated unique). Legacy saves → empty vector.
struct OwnedServiceSaveState {
    std::string serviceId;
    std::string ownerTeamColor;
    bool locked = false;
    bool destroyed = false;
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

    // M13-b runtime inventory + equipment. Each defaults to an empty vector
    // so legacy saves without these keys load as fresh empty inventories.
    std::vector<ItemSaveState> items;
    std::vector<ArtifactSaveState> artifacts;
    std::vector<HeroEquipmentSaveState> heroEquipment;

    // M14-a team Energy pool. Sentinel -1 means "absent" (legacy save predating
    // Energy); GameSession::ApplySaveData recomputes a fresh daily value in that
    // case rather than loading a 0 pool. Distinct from the nested enemy-team
    // energy field (enemy_teams[].energy).
    int energy = -1;
    int maxEnergy = -1;

    // M15-b World Map unlocked-region set. Absent (legacy) → empty vector, in
    // which case GameSession::ApplySaveData keeps the authored seed rather than
    // clearing unlock state. Every world map keeps the start region unlocked, so
    // a non-empty vector unambiguously marks an M15+ save.
    std::vector<std::string> unlockedRegionIds;

    // M16-b campaign progression. All default to empty (legacy saves load as no
    // campaign). campaignState is "" (none), "in_progress", "completed", or
    // "failed". No schemaVersion bump — additive optional fields.
    std::string campaignId;
    std::string currentScenarioId;
    std::vector<std::string> completedScenarioIds;
    std::vector<std::string> campaignFlags;
    std::string campaignState;

    // M17 owned-services / economy foundation. Both default to empty so legacy
    // saves load as no resources and no owned services. Additive optional fields
    // — no schemaVersion bump. Gold is intentionally absent here; it remains the
    // single `gold` field above.
    std::vector<ResourceSaveState> resources;
    std::vector<OwnedServiceSaveState> ownedServices;
};

class SaveGameRepository {
public:
    [[nodiscard]] bool SaveToFile(const SaveData& saveData, const std::string& filePath) const;
    [[nodiscard]] std::optional<SaveData> LoadFromFile(const std::string& filePath) const;
};

} // namespace core

#pragma once

#include <filesystem>
#include <vector>

#include <nlohmann/json.hpp>

#include "data/ContentValidator.h"
#include "data/definitions/ArtifactDefinition.h"
#include "data/definitions/BattleScenarioDefinition.h"
#include "data/definitions/CampaignDefinition.h"
#include "data/definitions/EnemyGroupDefinition.h"
#include "gameplay/events/EventParser.h"
#include "data/definitions/ItemDefinition.h"
#include "data/definitions/LocationDefinition.h"
#include "data/definitions/LocationSceneDefinition.h"
#include "data/definitions/LocationServiceDefinition.h"
#include "data/definitions/QuestDefinition.h"
#include "data/definitions/RegionDefinition.h"
#include "data/definitions/ScenarioDefinition.h"
#include "data/definitions/ScenarioOutcomeDefinition.h"
#include "data/definitions/TraderOwnershipCurve.h"
#include "data/definitions/UnitDefinition.h"
#include "data/definitions/WorldMapDefinition.h"

namespace data {

    class ContentRepository {
    public:
        [[nodiscard]] bool LoadFromDirectory(const std::filesystem::path& root);

        [[nodiscard]] const std::vector<ValidationMessage>& ValidationMessages() const;

        [[nodiscard]] const std::vector<RegionDefinition>& Regions() const;
        [[nodiscard]] const RegionDefinition* FindRegionById(const std::string& id) const;

        [[nodiscard]] const std::vector<LocationDefinition>& Locations() const;
        [[nodiscard]] const LocationDefinition* FindLocationById(const std::string& id) const;

        [[nodiscard]] const std::vector<LocationSceneDefinition>& LocationScenes() const;
        [[nodiscard]] const LocationSceneDefinition* FindLocationSceneById(const std::string& id) const;

        [[nodiscard]] const std::vector<UnitDefinition>& Units() const;
        [[nodiscard]] const UnitDefinition* FindUnitById(const std::string& id) const;

        [[nodiscard]] const std::vector<BattleScenarioDefinition>& BattleScenarios() const;
        [[nodiscard]] const BattleScenarioDefinition* FindBattleScenarioById(const std::string& id) const;

        [[nodiscard]] const std::vector<QuestDefinition>& QuestDefinitions() const;

        [[nodiscard]] const std::vector<EnemyGroupDefinition>& EnemyGroups() const;
        [[nodiscard]] const EnemyGroupDefinition* FindEnemyGroupById(const std::string& id) const;

        [[nodiscard]] const nlohmann::json& Quests() const;

        [[nodiscard]] const std::vector<LocationServiceDefinition>& LocationServices() const;
        [[nodiscard]] const LocationServiceDefinition* FindLocationService(
            const std::string& locationId,
            const std::string& zoneId) const;

        // M17 Phase 4: authored trader ownership curves (optional content).
        [[nodiscard]] const std::vector<TraderOwnershipCurve>& TraderCurves() const;

        [[nodiscard]] const std::vector<gameplay::events::EventDefinition>& EventDefinitions() const;

        [[nodiscard]] const ScenarioOutcomeDefinition& ScenarioOutcome() const;

        [[nodiscard]] const std::vector<ItemDefinition>& Items() const;
        [[nodiscard]] const ItemDefinition* FindItemById(const std::string& id) const;

        [[nodiscard]] const std::vector<ArtifactDefinition>& Artifacts() const;
        [[nodiscard]] const ArtifactDefinition* FindArtifactById(const std::string& id) const;

        [[nodiscard]] const WorldMapDefinition& WorldMap() const;
        [[nodiscard]] const WorldMapRegionEntry* FindWorldMapRegionEntry(const std::string& regionId) const;

        [[nodiscard]] const std::vector<ScenarioDefinition>& Scenarios() const;
        [[nodiscard]] const ScenarioDefinition* FindScenarioById(const std::string& id) const;

        [[nodiscard]] const std::vector<CampaignDefinition>& Campaigns() const;
        [[nodiscard]] const CampaignDefinition* FindCampaignById(const std::string& id) const;

    private:
        std::vector<RegionDefinition> regions_;
        std::vector<LocationDefinition> locations_;
        std::vector<LocationSceneDefinition> locationScenes_;
        std::vector<UnitDefinition> units_;
        std::vector<BattleScenarioDefinition> battleScenarios_;
        std::vector<QuestDefinition> questDefinitions_;
        std::vector<LocationServiceDefinition> locationServices_;
        std::vector<TraderOwnershipCurve> traderCurves_;
        std::vector<EnemyGroupDefinition> enemyGroups_;
        nlohmann::json quests_;
        std::vector<gameplay::events::EventDefinition> eventDefinitions_;
        ScenarioOutcomeDefinition scenarioOutcome_;
        std::vector<ItemDefinition> items_;
        std::vector<ArtifactDefinition> artifacts_;
        WorldMapDefinition worldMap_;
        std::vector<ScenarioDefinition> scenarios_;
        std::vector<CampaignDefinition> campaigns_;
        std::vector<ValidationMessage> messages_;
    };

} // namespace data
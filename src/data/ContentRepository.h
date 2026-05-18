#pragma once

#include <filesystem>
#include <vector>

#include <nlohmann/json.hpp>

#include "data/ContentValidator.h"
#include "data/definitions/BattleScenarioDefinition.h"
#include "data/definitions/EnemyGroupDefinition.h"
#include "gameplay/events/EventParser.h"
#include "data/definitions/LocationDefinition.h"
#include "data/definitions/LocationSceneDefinition.h"
#include "data/definitions/LocationServiceDefinition.h"
#include "data/definitions/QuestDefinition.h"
#include "data/definitions/RegionDefinition.h"
#include "data/definitions/UnitDefinition.h"

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

        [[nodiscard]] const std::vector<gameplay::events::EventDefinition>& EventDefinitions() const;

    private:
        std::vector<RegionDefinition> regions_;
        std::vector<LocationDefinition> locations_;
        std::vector<LocationSceneDefinition> locationScenes_;
        std::vector<UnitDefinition> units_;
        std::vector<BattleScenarioDefinition> battleScenarios_;
        std::vector<QuestDefinition> questDefinitions_;
        std::vector<LocationServiceDefinition> locationServices_;
        std::vector<EnemyGroupDefinition> enemyGroups_;
        nlohmann::json quests_;
        std::vector<gameplay::events::EventDefinition> eventDefinitions_;
        std::vector<ValidationMessage> messages_;
    };

} // namespace data
#include "data/ContentRepository.h"
#include "data/ContentValidator.h"

#include <algorithm>
#include <fstream>
#include <map>
#include <optional>
#include <set>

namespace data {

    namespace {

        std::optional<nlohmann::json> ParseJsonFile(const std::filesystem::path& path) {
            std::ifstream input(path);
            if (!input.is_open()) {
                return std::nullopt;
            }
            nlohmann::json root;
            try {
                input >> root;
            } catch (...) {
                return std::nullopt;
            }
            return root;
        }

        bool LoadLocationsFile(const nlohmann::json& root, std::vector<LocationDefinition>& output) {
            if (!root.contains("locations") || !root["locations"].is_array()) {
                return false;
            }

            output.clear();

            for (const auto& location : root["locations"]) {
                LocationDefinition def;
                def.id = location.value("id", "unknown");
                def.name = location.value("name", "Unknown");
                def.type = LocationTypeFromString(location.value("type", "unknown"));
                def.allowsSleep = location.value("allows_sleep", false);
                def.blocksTransitUntilCleared = location.value("blocks_transit_until_cleared", false);
                def.overworldDestination = location.value("overworld_destination", true);
                def.sceneId = location.value("scene_id", "");
                def.battleScenarioId = location.value("battle_scenario_id", "");
                output.push_back(def);
            }

            return true;
        }

        bool LoadRegionsFile(const nlohmann::json& root, std::vector<RegionDefinition>& output) {
            if (!root.contains("regions") || !root["regions"].is_array()) {
                return false;
            }

            output.clear();

            for (const auto& regionJson : root["regions"]) {
                RegionDefinition region;
                region.id = regionJson.value("id", "unknown");
                region.name = regionJson.value("name", "Unknown");
                region.unlocked = regionJson.value("unlocked", false);

                if (regionJson.contains("nodes") && regionJson["nodes"].is_array()) {
                    for (const auto& nodeJson : regionJson["nodes"]) {
                        RegionNodeDefinition node;
                        node.locationId = nodeJson.value("location_id", "unknown");
                        node.x = nodeJson.value("x", 0.0f);
                        node.y = nodeJson.value("y", 0.0f);
                        node.discovered = nodeJson.value("discovered", true);
                        node.travelAvailable = nodeJson.value("travel_available", true);
                        region.nodes.push_back(node);
                    }
                }

                if (regionJson.contains("links") && regionJson["links"].is_array()) {
                    for (const auto& linkJson : regionJson["links"]) {
                        RegionLinkDefinition link;

                        if (linkJson.is_array() && linkJson.size() >= 2) {
                            link.fromLocationId = linkJson[0].get<std::string>();
                            link.toLocationId = linkJson[1].get<std::string>();
                        }
                        else if (linkJson.is_object()) {
                            link.fromLocationId = linkJson.value("from", "");
                            link.toLocationId = linkJson.value("to", "");
                        }

                        if (!link.fromLocationId.empty() && !link.toLocationId.empty()) {
                            region.links.push_back(link);
                        }
                    }
                }

                output.push_back(region);
            }

            return true;
        }

        bool LoadLocationScenesFile(const nlohmann::json& root, std::vector<LocationSceneDefinition>& output) {
            if (!root.contains("location_scenes") || !root["location_scenes"].is_array()) {
                return false;
            }

            output.clear();

            for (const auto& sceneJson : root["location_scenes"]) {
                LocationSceneDefinition scene;
                scene.id = sceneJson.value("id", "unknown");

                if (sceneJson.contains("spawn") && sceneJson["spawn"].is_object()) {
                    const auto& spawn = sceneJson["spawn"];
                    scene.spawn.x = spawn.value("x", 0.0f);
                    scene.spawn.y = spawn.value("y", 0.0f);
                    scene.spawn.width = spawn.value("width", 0.0f);
                    scene.spawn.height = spawn.value("height", 0.0f);
                }

                if (sceneJson.contains("blocking_rects") && sceneJson["blocking_rects"].is_array()) {
                    for (const auto& rectJson : sceneJson["blocking_rects"]) {
                        SceneRectDefinition rect;
                        rect.x = rectJson.value("x", 0.0f);
                        rect.y = rectJson.value("y", 0.0f);
                        rect.width = rectJson.value("width", 0.0f);
                        rect.height = rectJson.value("height", 0.0f);
                        scene.blockingRects.push_back(rect);
                    }
                }

                if (sceneJson.contains("zones") && sceneJson["zones"].is_array()) {
                    for (const auto& zoneJson : sceneJson["zones"]) {
                        LocationSceneZoneDefinition zone;
                        zone.id = zoneJson.value("id", "unknown");
                        zone.type = LocationSceneZoneTypeFromString(zoneJson.value("type", "unknown"));

                        if (zoneJson.contains("area") && zoneJson["area"].is_object()) {
                            const auto& area = zoneJson["area"];
                            zone.area.x = area.value("x", 0.0f);
                            zone.area.y = area.value("y", 0.0f);
                            zone.area.width = area.value("width", 0.0f);
                            zone.area.height = area.value("height", 0.0f);
                        }

                        zone.promptText = zoneJson.value("prompt_text", "");
                        zone.resultText = zoneJson.value("result_text", "");
                        zone.failureText = zoneJson.value("failure_text", "");
                        zone.timeCostMinutes = zoneJson.value("time_cost_minutes", 0);
                        zone.goldCost = zoneJson.value("gold_cost", 0);
                        zone.recruitCount = zoneJson.value("recruit_count", 0);
                        zone.dialogueChoiceTimeCostMinutes = zoneJson.value("dialogue_choice_time_cost_minutes", 1);

                        if (zoneJson.contains("dialogue_choices") && zoneJson["dialogue_choices"].is_array()) {
                            for (const auto& choice : zoneJson["dialogue_choices"]) {
                                zone.dialogueChoices.push_back(choice.get<std::string>());
                            }
                        }

                        scene.zones.push_back(zone);
                    }
                }

                output.push_back(scene);
            }

            return true;
        }

        bool LoadUnitsFile(const nlohmann::json& root, std::vector<UnitDefinition>& output) {
            if (!root.contains("units") || !root["units"].is_array()) {
                return false;
            }

            output.clear();

            for (const auto& unitJson : root["units"]) {
                UnitDefinition def;
                def.id = unitJson.value("id", "unknown");
                def.name = unitJson.value("name", "Unknown");
                def.category = UnitDefinitionCategoryFromString(unitJson.value("category", "generic"));
                def.isPlayerCharacter = unitJson.value("is_player_character", false);

                def.stats.attack = unitJson.value("attack", 0);
                def.stats.defense = unitJson.value("defense", 0);
                def.stats.magic = unitJson.value("magic", 0);
                def.stats.resistance = unitJson.value("resistance", 0);
                def.stats.minDamage = unitJson.value("min_damage", 1);
                def.stats.maxDamage = unitJson.value("max_damage", 1);
                def.stats.maxHp = unitJson.value("max_hp", 1);
                def.stats.maxMp = unitJson.value("max_mp", 0);
                def.stats.agility = unitJson.value("agility", 1);
                def.stats.life = unitJson.value("life", 1);
                def.stats.position = UnitDefinitionPositionFromString(unitJson.value("position", "front"));
                def.stats.range = UnitDefinitionRangeFromString(unitJson.value("range", "melee"));

                output.push_back(def);
            }

            return true;
        }

        bool LoadBattleScenariosFile(const nlohmann::json& root, std::vector<BattleScenarioDefinition>& output) {
            if (!root.contains("battle_scenarios") || !root["battle_scenarios"].is_array()) {
                return false;
            }

            output.clear();

            for (const auto& scenarioJson : root["battle_scenarios"]) {
                BattleScenarioDefinition def;
                def.id = scenarioJson.value("id", "unknown");
                def.name = scenarioJson.value("name", "Unknown");
                def.seed = scenarioJson.value("seed", 7u);

                if (scenarioJson.contains("allies") && scenarioJson["allies"].is_array()) {
                    for (const auto& entryJson : scenarioJson["allies"]) {
                        BattleScenarioEntryDefinition entry;
                        entry.unitId = entryJson.value("unit_id", "unknown");
                        entry.lifeOverride = entryJson.value("life_override", 0);
                        def.allies.push_back(entry);
                    }
                }

                if (scenarioJson.contains("enemies") && scenarioJson["enemies"].is_array()) {
                    for (const auto& entryJson : scenarioJson["enemies"]) {
                        BattleScenarioEntryDefinition entry;
                        entry.unitId = entryJson.value("unit_id", "unknown");
                        entry.lifeOverride = entryJson.value("life_override", 0);
                        def.enemies.push_back(entry);
                    }
                }

                output.push_back(def);
            }

            return true;
        }

        bool LoadQuestDefinitionsFile(const nlohmann::json& root, std::vector<QuestDefinition>& output) {
            if (!root.contains("quests") || !root["quests"].is_array()) {
                return false;
            }

            output.clear();

            for (const auto& questJson : root["quests"]) {
                QuestDefinition definition;
                definition.id = questJson.value("id", "unknown");
                definition.name = questJson.value("name", "Unknown");
                definition.description = questJson.value("description", "");
                definition.objective = QuestObjectiveTypeFromString(questJson.value("objective", "unknown"));
                definition.target = questJson.value("target", "");
                output.push_back(definition);
            }

            return true;
        }

        bool LoadLocationServicesFile(const nlohmann::json& root, std::vector<LocationServiceDefinition>& output) {
            if (!root.contains("location_services") || !root["location_services"].is_array()) {
                return false;
            }

            output.clear();

            for (const auto& serviceJson : root["location_services"]) {
                LocationServiceDefinition service;
                service.id = serviceJson.value("id", "unknown");
                service.locationId = serviceJson.value("location_id", "");
                service.zoneId = serviceJson.value("zone_id", "");
                service.kind = LocationServiceKindFromString(serviceJson.value("kind", "unknown"));

                service.promptText = serviceJson.value("prompt_text", "");
                service.successText = serviceJson.value("success_text", "");
                service.failureText = serviceJson.value("failure_text", "");

                service.goldCost = serviceJson.value("gold_cost", 0);
                service.timeCostMinutes = serviceJson.value("time_cost_minutes", 0);

                service.restKind = RestServiceKindFromString(serviceJson.value("rest_kind", "unknown"));

                service.unitId = serviceJson.value("unit_id", "");
                service.unitDisplayName = serviceJson.value("unit_display_name", "");
                service.weeklyStock = serviceJson.value("weekly_stock", 0);
                service.dailyUseLimit = serviceJson.value("daily_use_limit", 0);
                service.travelPrepDiscountMinutes = serviceJson.value("travel_prep_discount_minutes", 0);
                service.travelPrepCharges = serviceJson.value("travel_prep_charges", 0);

                output.push_back(service);
            }

            return true;
        }

        bool LoadEventDefinitionsFile(
            const nlohmann::json& root,
            std::vector<gameplay::events::EventDefinition>& output,
            std::vector<ValidationMessage>& msgs)
        {
            if (!root.contains("events") || !root["events"].is_array())
                return false;

            output.clear();

            std::set<std::string> seenIds;
            for (const auto& e : root["events"]) {
                const auto def = gameplay::events::ParseEventDefinition(e);
                if (!seenIds.insert(def.id).second) {
                    msgs.push_back({Severity::Error, "EVENT_ID_DUPLICATE", "events",
                        "Duplicate event id \"" + def.id + "\".", ""});
                }
                output.push_back(def);
            }

            std::map<gameplay::events::EventTriggerType, std::set<int>> seenPriorities;
            for (const auto& def : output) {
                if (!def.priority.has_value()) continue;
                auto& pset = seenPriorities[def.trigger.type];
                if (!pset.insert(*def.priority).second) {
                    msgs.push_back({Severity::Error, "EVENT_PRIORITY_DUPLICATE", "events",
                        "Duplicate priority " + std::to_string(*def.priority)
                        + " for trigger type in event \"" + def.id + "\".", ""});
                }
            }
            return true;
        }

    } // namespace

    bool ContentRepository::LoadFromDirectory(const std::filesystem::path& root) {
        messages_.clear();

        ContentValidator validator;

        auto load = [&](const std::filesystem::path& path) -> std::optional<nlohmann::json> {
            auto doc = ParseJsonFile(path);
            if (!doc) return std::nullopt;
            auto idMsgs = validator.ValidateIdentity(*doc);
            messages_.insert(messages_.end(), idMsgs.begin(), idMsgs.end());
            return doc;
        };

        auto regionsDoc   = load(root / "regions.json");
        auto locationsDoc = load(root / "locations.json");
        auto scenesDoc    = load(root / "location_scenes.json");
        auto unitsDoc     = load(root / "units.json");
        auto scenariosDoc = load(root / "battle_scenarios.json");
        auto enemyDoc     = load(root / "enemy_groups.json");
        auto questsDoc    = load(root / "quests.json");
        auto servicesDoc  = load(root / "location_services.json");

        if (!regionsDoc || !locationsDoc || !scenesDoc || !unitsDoc ||
            !scenariosDoc || !enemyDoc || !questsDoc || !servicesDoc) {
            return false;
        }

        const bool regionsLoaded   = LoadRegionsFile(*regionsDoc, regions_);
        const bool locationsLoaded = LoadLocationsFile(*locationsDoc, locations_);
        const bool scenesLoaded    = LoadLocationScenesFile(*scenesDoc, locationScenes_);
        const bool unitsLoaded     = LoadUnitsFile(*unitsDoc, units_);
        const bool scenariosLoaded = LoadBattleScenariosFile(*scenariosDoc, battleScenarios_);
        enemyGroups_               = *enemyDoc;
        const bool questDefLoaded  = LoadQuestDefinitionsFile(*questsDoc, questDefinitions_);
        quests_                    = *questsDoc;
        const bool servicesLoaded  = LoadLocationServicesFile(*servicesDoc, locationServices_);

        if (!regionsLoaded || !locationsLoaded || !scenesLoaded || !unitsLoaded ||
            !scenariosLoaded || !questDefLoaded || !servicesLoaded) {
            return false;
        }

        auto eventsDoc = load(root / "events.json");
        if (eventsDoc) {
            LoadEventDefinitionsFile(*eventsDoc, eventDefinitions_, messages_);
        }

        auto refMsgs = validator.ValidateReferences(
            regions_, locations_, locationScenes_, units_,
            battleScenarios_, locationServices_, questDefinitions_);
        messages_.insert(messages_.end(), refMsgs.begin(), refMsgs.end());

        const bool anyRefError = std::ranges::any_of(refMsgs,
            [](const ValidationMessage& m) { return m.severity == Severity::Error; });
        return !anyRefError;
    }

    const std::vector<ValidationMessage>& ContentRepository::ValidationMessages() const {
        return messages_;
    }

    const std::vector<RegionDefinition>& ContentRepository::Regions() const {
        return regions_;
    }

    const RegionDefinition* ContentRepository::FindRegionById(const std::string& id) const {
        for (const auto& region : regions_) {
            if (region.id == id) {
                return &region;
            }
        }
        return nullptr;
    }

    const std::vector<LocationDefinition>& ContentRepository::Locations() const {
        return locations_;
    }

    const LocationDefinition* ContentRepository::FindLocationById(const std::string& id) const {
        for (const auto& location : locations_) {
            if (location.id == id) {
                return &location;
            }
        }
        return nullptr;
    }

    const std::vector<LocationSceneDefinition>& ContentRepository::LocationScenes() const {
        return locationScenes_;
    }

    const LocationSceneDefinition* ContentRepository::FindLocationSceneById(const std::string& id) const {
        for (const auto& scene : locationScenes_) {
            if (scene.id == id) {
                return &scene;
            }
        }
        return nullptr;
    }

    const std::vector<UnitDefinition>& ContentRepository::Units() const {
        return units_;
    }

    const UnitDefinition* ContentRepository::FindUnitById(const std::string& id) const {
        for (const auto& unit : units_) {
            if (unit.id == id) {
                return &unit;
            }
        }
        return nullptr;
    }

    const std::vector<BattleScenarioDefinition>& ContentRepository::BattleScenarios() const {
        return battleScenarios_;
    }

    const BattleScenarioDefinition* ContentRepository::FindBattleScenarioById(const std::string& id) const {
        for (const auto& scenario : battleScenarios_) {
            if (scenario.id == id) {
                return &scenario;
            }
        }
        return nullptr;
    }

    const std::vector<QuestDefinition>& ContentRepository::QuestDefinitions() const {
        return questDefinitions_;
    }

    const std::vector<LocationServiceDefinition>& ContentRepository::LocationServices() const {
        return locationServices_;
    }

    const LocationServiceDefinition* ContentRepository::FindLocationService(
        const std::string& locationId,
        const std::string& zoneId) const {
        for (const auto& service : locationServices_) {
            if (service.locationId == locationId && service.zoneId == zoneId) {
                return &service;
            }
        }

        return nullptr;
    }

    const nlohmann::json& ContentRepository::EnemyGroups() const {
        return enemyGroups_;
    }

    const nlohmann::json& ContentRepository::Quests() const {
        return quests_;
    }

    const std::vector<gameplay::events::EventDefinition>& ContentRepository::EventDefinitions() const {
        return eventDefinitions_;
    }

} // namespace data

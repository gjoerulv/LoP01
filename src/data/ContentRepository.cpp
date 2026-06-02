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
                region.arrivalNodeId = regionJson.value("arrivalNodeId", "");

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

                // M17 Phase 3a: optional narrow mine-production passive. Absent
                // on every existing unit, so legacy content loads unchanged.
                // Field validity (target/resource/amount) is checked by
                // ContentValidator, not here.
                if (unitJson.contains("mine_production_passive") &&
                    unitJson["mine_production_passive"].is_object()) {
                    const auto& passiveJson = unitJson["mine_production_passive"];
                    UnitMineProductionPassive passive;
                    passive.target = passiveJson.value("target", "mine");
                    passive.resource = passiveJson.value("resource", "");
                    passive.amount = passiveJson.value("amount", 0);
                    def.mineProductionPassive = passive;
                }

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

                // M17 Phase 2: optional authored mine base outputs. Absent for
                // every existing (non-mine) service, so legacy content loads
                // unchanged. Resource-name/amount validity is checked by
                // ContentValidator, not here.
                if (serviceJson.contains("mine_outputs") && serviceJson["mine_outputs"].is_array()) {
                    for (const auto& outputJson : serviceJson["mine_outputs"]) {
                        if (!outputJson.is_object()) {
                            continue;
                        }
                        MineOutputDefinition mineOutput;
                        mineOutput.resource = outputJson.value("resource", "");
                        mineOutput.amount = outputJson.value("amount", 0);
                        service.mineOutputs.push_back(mineOutput);
                    }
                }

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

        bool LoadScenarioOutcomeFile(
            const nlohmann::json& root,
            ScenarioOutcomeDefinition& output,
            std::vector<ValidationMessage>& msgs)
        {
            output = {};

            auto loadList = [&](const char* key, const std::string& pathLabel,
                                std::vector<gameplay::events::EventCondition>& dst)
            {
                if (!root.contains(key)) return;
                if (!root[key].is_array()) {
                    msgs.push_back({Severity::Error, "SCENARIO_OUTCOME_LIST_NOT_ARRAY",
                        pathLabel,
                        std::string{"\""} + key + "\" must be a JSON array.", ""});
                    return;
                }
                for (size_t i = 0; i < root[key].size(); ++i) {
                    const auto path = pathLabel + "[" + std::to_string(i) + "]";
                    gameplay::events::ValidateConditionTree(root[key][i], path, msgs);
                    dst.push_back(gameplay::events::ParseCondition(root[key][i]));
                }
            };

            loadList("victoryConditions", "victoryConditions", output.victoryConditions);
            loadList("defeatConditions",  "defeatConditions",  output.defeatConditions);
            return true;
        }

        // M13-a: items.json loader. Optional file; empty/absent is legal.
        // Inline validation matches the EventParser pattern. Each error appends a
        // typed ValidationMessage; invalid entries are skipped so a single bad
        // item does not erase the rest of the collection.
        //
        // Unsupported in M13:
        //   - Item `effects` field. Authoring an item with effects emits an
        //     ITEM_EFFECTS_UNSUPPORTED error rather than a silent ignore. M13
        //     intentionally implements no item-effect resolution.
        bool LoadItemsFile(
            const nlohmann::json& root,
            std::vector<ItemDefinition>& output,
            std::vector<ValidationMessage>& msgs)
        {
            if (!root.contains("items") || !root["items"].is_array())
                return false;

            output.clear();

            // Track ids already accepted into `output` so a second occurrence
            // can be rejected without erasing the first. Mirrors the
            // EVENT_ID_DUPLICATE pattern in LoadEventDefinitionsFile.
            std::set<std::string> seenItemIds;

            for (size_t i = 0; i < root["items"].size(); ++i) {
                const auto& entry = root["items"][i];
                const std::string path = "items[" + std::to_string(i) + "]";
                if (!entry.is_object()) {
                    msgs.push_back({Severity::Error, "ITEM_ENTRY_NOT_OBJECT", path,
                        "Item entry must be a JSON object.", ""});
                    continue;
                }

                ItemDefinition def;
                def.id   = entry.value("id",   "");
                def.icon = entry.value("icon", "");
                if (entry.contains("name") && entry["name"].is_object()) {
                    def.name = entry["name"].value("en", "");
                } else {
                    def.name = entry.value("name", "");
                }

                if (def.id.empty()) {
                    msgs.push_back({Severity::Error, "ITEM_ID_EMPTY", path + ".id",
                        "Item \"id\" is required and must be a non-empty string.", ""});
                    continue;
                }

                if (!seenItemIds.insert(def.id).second) {
                    msgs.push_back({Severity::Error, "ITEM_ID_DUPLICATE", path + ".id",
                        "Duplicate item id \"" + def.id + "\". "
                        "Item ids must be unique within items.json.", ""});
                    continue;
                }

                const std::string subtypeStr = entry.value("subtype", "");
                if (!ItemSubtypeFromString(subtypeStr, def.subtype)) {
                    msgs.push_back({Severity::Error, "ITEM_SUBTYPE_UNKNOWN",
                        path + ".subtype",
                        "Item subtype \"" + subtypeStr + "\" is not recognised. "
                        "Known subtypes: consumable, quest, seed, ingredient, food, material.", ""});
                    continue;
                }

                def.stackCap  = entry.value("stackCap",  999);
                def.baseValue = entry.value("baseValue", 0);

                if (def.stackCap <= 0) {
                    msgs.push_back({Severity::Error, "ITEM_STACK_CAP_INVALID",
                        path + ".stackCap",
                        "Item \"stackCap\" must be a positive integer.", ""});
                    continue;
                }
                if (def.baseValue < 0) {
                    msgs.push_back({Severity::Error, "ITEM_BASE_VALUE_NEGATIVE",
                        path + ".baseValue",
                        "Item \"baseValue\" must be non-negative.", ""});
                    continue;
                }

                // Explicit guardrail: M13 does not implement item effects. Any
                // authored `effects` field is an error rather than a silent accept.
                if (entry.contains("effects")) {
                    msgs.push_back({Severity::Error, "ITEM_EFFECTS_UNSUPPORTED",
                        path + ".effects",
                        "Item \"effects\" are not supported in M13. Remove the field "
                        "or wait for a later milestone that implements item effects.", ""});
                    continue;
                }

                output.push_back(def);
            }

            return true;
        }

        // M13-a: artifacts.json loader. Optional file; empty/absent is legal.
        //
        // Effect-handling guardrail:
        //   - Only `statBonus` effects are recognised in M13.
        //   - Any other `effect.type` (including the doc's `specialEffect`)
        //     produces an explicit ARTIFACT_EFFECT_TYPE_UNSUPPORTED error.
        //   - Missing or malformed effect type produces ARTIFACT_EFFECT_TYPE_MISSING.
        bool LoadArtifactsFile(
            const nlohmann::json& root,
            std::vector<ArtifactDefinition>& output,
            std::vector<ValidationMessage>& msgs)
        {
            if (!root.contains("artifacts") || !root["artifacts"].is_array())
                return false;

            output.clear();

            // Track ids already accepted into `output` so a second occurrence
            // can be rejected without erasing the first. Mirrors the items
            // duplicate-id pattern above and the EVENT_ID_DUPLICATE pattern.
            std::set<std::string> seenArtifactIds;

            for (size_t i = 0; i < root["artifacts"].size(); ++i) {
                const auto& entry = root["artifacts"][i];
                const std::string path = "artifacts[" + std::to_string(i) + "]";
                if (!entry.is_object()) {
                    msgs.push_back({Severity::Error, "ARTIFACT_ENTRY_NOT_OBJECT", path,
                        "Artifact entry must be a JSON object.", ""});
                    continue;
                }

                ArtifactDefinition def;
                def.id     = entry.value("id",     "");
                def.icon   = entry.value("icon",   "");
                def.rarity = entry.value("rarity", "");
                if (entry.contains("name") && entry["name"].is_object()) {
                    def.name = entry["name"].value("en", "");
                } else {
                    def.name = entry.value("name", "");
                }
                def.tier       = entry.value("tier",       0);
                def.baseValue  = entry.value("baseValue",  0);
                def.combinable = entry.value("combinable", false);

                if (def.id.empty()) {
                    msgs.push_back({Severity::Error, "ARTIFACT_ID_EMPTY", path + ".id",
                        "Artifact \"id\" is required and must be a non-empty string.", ""});
                    continue;
                }

                if (!seenArtifactIds.insert(def.id).second) {
                    msgs.push_back({Severity::Error, "ARTIFACT_ID_DUPLICATE", path + ".id",
                        "Duplicate artifact id \"" + def.id + "\". "
                        "Artifact ids must be unique within artifacts.json.", ""});
                    continue;
                }

                if (def.baseValue < 0) {
                    msgs.push_back({Severity::Error, "ARTIFACT_BASE_VALUE_NEGATIVE",
                        path + ".baseValue",
                        "Artifact \"baseValue\" must be non-negative.", ""});
                    continue;
                }

                // allowedSlots: required, must be a non-empty array of known slot
                // kinds (Attack, Defense, Misc).
                bool slotsOk = true;
                if (!entry.contains("allowedSlots") || !entry["allowedSlots"].is_array()) {
                    msgs.push_back({Severity::Error, "ARTIFACT_ALLOWED_SLOTS_MISSING",
                        path + ".allowedSlots",
                        "Artifact \"allowedSlots\" is required and must be a JSON array.", ""});
                    slotsOk = false;
                } else if (entry["allowedSlots"].empty()) {
                    msgs.push_back({Severity::Error, "ARTIFACT_ALLOWED_SLOTS_EMPTY",
                        path + ".allowedSlots",
                        "Artifact \"allowedSlots\" must contain at least one slot.", ""});
                    slotsOk = false;
                } else {
                    for (size_t s = 0; s < entry["allowedSlots"].size(); ++s) {
                        const auto& slotEntry = entry["allowedSlots"][s];
                        if (!slotEntry.is_string()) {
                            msgs.push_back({Severity::Error, "ARTIFACT_ALLOWED_SLOT_NOT_STRING",
                                path + ".allowedSlots[" + std::to_string(s) + "]",
                                "Artifact allowed-slot entry must be a string.", ""});
                            slotsOk = false;
                            continue;
                        }
                        ArtifactSlotKind kind;
                        const std::string slotStr = slotEntry.get<std::string>();
                        if (!ArtifactSlotKindFromString(slotStr, kind)) {
                            msgs.push_back({Severity::Error, "ARTIFACT_ALLOWED_SLOT_UNKNOWN",
                                path + ".allowedSlots[" + std::to_string(s) + "]",
                                "Artifact slot kind \"" + slotStr + "\" is not recognised. "
                                "Known kinds: Attack, Defense, Misc.", ""});
                            slotsOk = false;
                            continue;
                        }
                        def.allowedSlots.push_back(kind);
                    }
                }
                if (!slotsOk) continue;

                // effects: optional, but every entry must be `statBonus` with a
                // recognised stat and integer amount. Anything else is rejected.
                bool effectsOk = true;
                if (entry.contains("effects")) {
                    if (!entry["effects"].is_array()) {
                        msgs.push_back({Severity::Error, "ARTIFACT_EFFECTS_NOT_ARRAY",
                            path + ".effects",
                            "Artifact \"effects\" must be a JSON array.", ""});
                        effectsOk = false;
                    } else {
                        for (size_t e = 0; e < entry["effects"].size(); ++e) {
                            const auto& effectEntry = entry["effects"][e];
                            const std::string ePath = path + ".effects[" + std::to_string(e) + "]";
                            if (!effectEntry.is_object()) {
                                msgs.push_back({Severity::Error, "ARTIFACT_EFFECT_NOT_OBJECT",
                                    ePath, "Artifact effect entry must be a JSON object.", ""});
                                effectsOk = false;
                                continue;
                            }
                            if (!effectEntry.contains("type") || !effectEntry["type"].is_string()) {
                                msgs.push_back({Severity::Error, "ARTIFACT_EFFECT_TYPE_MISSING",
                                    ePath + ".type",
                                    "Artifact effect is missing a \"type\" string.", ""});
                                effectsOk = false;
                                continue;
                            }
                            const std::string effectType = effectEntry["type"].get<std::string>();
                            if (effectType != "statBonus") {
                                msgs.push_back({Severity::Error, "ARTIFACT_EFFECT_TYPE_UNSUPPORTED",
                                    ePath + ".type",
                                    "Artifact effect type \"" + effectType + "\" is not supported in M13. "
                                    "Supported effect types: statBonus.", ""});
                                effectsOk = false;
                                continue;
                            }
                            const std::string statStr = effectEntry.value("stat", "");
                            ArtifactStatBonusStat stat;
                            if (!ArtifactStatBonusStatFromString(statStr, stat)) {
                                msgs.push_back({Severity::Error, "ARTIFACT_STAT_BONUS_STAT_UNKNOWN",
                                    ePath + ".stat",
                                    "Artifact statBonus stat \"" + statStr + "\" is not recognised. "
                                    "Known stats: Attack, Defense, Magic, Resistance.", ""});
                                effectsOk = false;
                                continue;
                            }
                            if (!effectEntry.contains("amount")
                                || !effectEntry["amount"].is_number_integer()) {
                                msgs.push_back({Severity::Error, "ARTIFACT_STAT_BONUS_AMOUNT_INVALID",
                                    ePath + ".amount",
                                    "Artifact statBonus \"amount\" must be an integer.", ""});
                                effectsOk = false;
                                continue;
                            }
                            ArtifactStatBonus bonus;
                            bonus.stat   = stat;
                            bonus.amount = effectEntry["amount"].get<int>();
                            def.statBonuses.push_back(bonus);
                        }
                    }
                }
                if (!effectsOk) continue;

                output.push_back(def);
            }

            return true;
        }

        bool LoadEnemyGroupsFile(const nlohmann::json& root, std::vector<EnemyGroupDefinition>& output) {
            if (!root.contains("enemy_groups") || !root["enemy_groups"].is_array()) {
                return false;
            }

            output.clear();

            for (const auto& entry : root["enemy_groups"]) {
                EnemyGroupDefinition def;
                def.id = entry.value("id", "");
                def.name = entry.value("name", "");
                if (entry.contains("units") && entry["units"].is_array()) {
                    for (const auto& unit : entry["units"]) {
                        def.unitIds.push_back(unit.get<std::string>());
                    }
                }
                output.push_back(def);
            }

            return true;
        }

        // M15-a: world_map.json loader. Optional file; empty/absent is legal
        // (single-Region scenario, no inter-region travel). Carries inter-region
        // travel metadata only — arrival nodes are owned by RegionDefinition and
        // resolved at travel time. Structural validation resolves region/node ids
        // against the already-loaded `regions`. Invalid entries are skipped so a
        // single bad entry does not erase the rest.
        bool LoadWorldMapFile(
            const nlohmann::json& root,
            WorldMapDefinition& output,
            const std::vector<RegionDefinition>& regions,
            std::vector<ValidationMessage>& msgs)
        {
            output = WorldMapDefinition{};

            output.id = root.value("id", "");
            if (root.contains("name") && root["name"].is_object()) {
                output.name = root["name"].value("en", "");
            } else {
                output.name = root.value("name", "");
            }

            auto findRegion = [&](const std::string& regionId) -> const RegionDefinition* {
                for (const auto& region : regions) {
                    if (region.id == regionId) {
                        return &region;
                    }
                }
                return nullptr;
            };

            auto regionHasNode = [](const RegionDefinition& region, const std::string& nodeId) {
                for (const auto& node : region.nodes) {
                    if (node.locationId == nodeId) {
                        return true;
                    }
                }
                return false;
            };

            std::set<std::string> seenEntryIds;
            if (root.contains("entries") && root["entries"].is_array()) {
                for (size_t i = 0; i < root["entries"].size(); ++i) {
                    const auto& entry = root["entries"][i];
                    const std::string path = "entries[" + std::to_string(i) + "]";
                    if (!entry.is_object()) {
                        msgs.push_back({Severity::Error, "WORLDMAP_ENTRY_NOT_OBJECT", path,
                            "World map entry must be a JSON object.", ""});
                        continue;
                    }

                    WorldMapRegionEntry def;
                    def.id       = entry.value("id", "");
                    def.unlocked = entry.value("unlocked", false);
                    def.x        = entry.value("x", 0.0f);
                    def.y        = entry.value("y", 0.0f);
                    if (entry.contains("exitNodeIds") && entry["exitNodeIds"].is_array()) {
                        for (const auto& nodeId : entry["exitNodeIds"]) {
                            if (nodeId.is_string()) {
                                def.exitNodeIds.push_back(nodeId.get<std::string>());
                            }
                        }
                    }

                    if (def.id.empty()) {
                        msgs.push_back({Severity::Error, "WORLDMAP_ENTRY_ID_EMPTY", path + ".id",
                            "World map entry \"id\" is required and must be a non-empty string.", ""});
                        continue;
                    }

                    if (!seenEntryIds.insert(def.id).second) {
                        msgs.push_back({Severity::Error, "WORLDMAP_ENTRY_DUPLICATE", path + ".id",
                            "Duplicate world map region entry \"" + def.id + "\".", ""});
                        continue;
                    }

                    const RegionDefinition* region = findRegion(def.id);
                    if (region == nullptr) {
                        msgs.push_back({Severity::Error, "WORLDMAP_REGION_UNKNOWN", path + ".id",
                            "World map entry references unknown Region \"" + def.id + "\".", ""});
                        continue;
                    }

                    // Arrival node is owned by RegionDefinition. It must be present
                    // and resolve to a real node, since travel arrival lands there.
                    if (region->arrivalNodeId.empty()) {
                        msgs.push_back({Severity::Error, "WORLDMAP_ARRIVAL_NODE_MISSING", path + ".id",
                            "Region \"" + def.id + "\" used on the world map has no arrivalNodeId "
                            "(set it in regions.json).", ""});
                        continue;
                    }
                    if (!regionHasNode(*region, region->arrivalNodeId)) {
                        msgs.push_back({Severity::Error, "WORLDMAP_ARRIVAL_NODE_UNKNOWN", path + ".id",
                            "Region \"" + def.id + "\" arrivalNodeId \"" + region->arrivalNodeId +
                            "\" does not exist among its nodes.", ""});
                        continue;
                    }

                    // Exit nodes must exist in this (source) Region.
                    bool exitNodesOk = true;
                    for (const auto& exitNodeId : def.exitNodeIds) {
                        if (!regionHasNode(*region, exitNodeId)) {
                            msgs.push_back({Severity::Error, "WORLDMAP_EXIT_NODE_UNKNOWN",
                                path + ".exitNodeIds",
                                "World map entry \"" + def.id + "\" exit node \"" + exitNodeId +
                                "\" does not exist in that Region.", ""});
                            exitNodesOk = false;
                        }
                    }
                    if (!exitNodesOk) {
                        continue;
                    }

                    output.entries.push_back(std::move(def));
                }
            }

            // Adjacency: both endpoints must be present in the accepted entries.
            if (root.contains("adjacency") && root["adjacency"].is_array()) {
                for (size_t i = 0; i < root["adjacency"].size(); ++i) {
                    const auto& pair = root["adjacency"][i];
                    const std::string path = "adjacency[" + std::to_string(i) + "]";

                    std::string regionA;
                    std::string regionB;
                    if (pair.is_array() && pair.size() >= 2) {
                        regionA = pair[0].get<std::string>();
                        regionB = pair[1].get<std::string>();
                    } else if (pair.is_object()) {
                        regionA = pair.value("from", "");
                        regionB = pair.value("to", "");
                    }

                    if (regionA.empty() || regionB.empty()) {
                        msgs.push_back({Severity::Error, "WORLDMAP_ADJACENCY_MALFORMED", path,
                            "World map adjacency must reference two region ids.", ""});
                        continue;
                    }
                    if (output.FindEntry(regionA) == nullptr || output.FindEntry(regionB) == nullptr) {
                        msgs.push_back({Severity::Error, "WORLDMAP_ADJACENCY_UNKNOWN_ENTRY", path,
                            "World map adjacency references a region not present in entries: \"" +
                            regionA + "\" / \"" + regionB + "\".", ""});
                        continue;
                    }

                    output.adjacency.push_back({regionA, regionB});
                }
            }

            return true;
        }

        // M16-a: scenarios.json loader. Optional file; absent/empty is legal
        // (the slice runs as a single implicit scenario with no campaign).
        // Thin Scenario model: a Scenario selects a starting Region/node and
        // optionally its own win/loss conditions. Invalid entries are skipped so
        // a single bad scenario does not erase the rest, mirroring the world-map
        // and items loaders.
        bool LoadScenariosFile(
            const nlohmann::json& root,
            std::vector<ScenarioDefinition>& output,
            const std::vector<RegionDefinition>& regions,
            std::vector<ValidationMessage>& msgs)
        {
            output.clear();
            if (!root.contains("scenarios") || !root["scenarios"].is_array()) {
                return true;
            }

            auto findRegion = [&](const std::string& regionId) -> const RegionDefinition* {
                for (const auto& region : regions) {
                    if (region.id == regionId) return &region;
                }
                return nullptr;
            };
            auto regionHasNode = [](const RegionDefinition& region, const std::string& nodeId) {
                for (const auto& node : region.nodes) {
                    if (node.locationId == nodeId) return true;
                }
                return false;
            };

            std::set<std::string> seenIds;
            for (size_t i = 0; i < root["scenarios"].size(); ++i) {
                const auto& entry = root["scenarios"][i];
                const std::string path = "scenarios[" + std::to_string(i) + "]";
                if (!entry.is_object()) {
                    msgs.push_back({Severity::Error, "SCENARIO_NOT_OBJECT", path,
                        "Scenario entry must be a JSON object.", ""});
                    continue;
                }

                ScenarioDefinition def;
                def.id            = entry.value("id", "");
                if (entry.contains("name") && entry["name"].is_object()) {
                    def.name = entry["name"].value("en", "");
                } else {
                    def.name = entry.value("name", "");
                }
                def.startRegionId = entry.value("startRegionId", "");
                def.startNodeId   = entry.value("startNodeId", "");
                def.standaloneSelectable = entry.value("standaloneSelectable", false);
                if (entry.contains("startGold") && entry["startGold"].is_number_integer()) {
                    def.startGold = entry["startGold"].get<int>();
                }

                if (def.id.empty()) {
                    msgs.push_back({Severity::Error, "SCENARIO_ID_EMPTY", path + ".id",
                        "Scenario \"id\" is required and must be a non-empty string.", ""});
                    continue;
                }
                if (!seenIds.insert(def.id).second) {
                    msgs.push_back({Severity::Error, "SCENARIO_DUPLICATE", path + ".id",
                        "Duplicate scenario id \"" + def.id + "\".", ""});
                    continue;
                }

                const RegionDefinition* region = findRegion(def.startRegionId);
                if (region == nullptr) {
                    msgs.push_back({Severity::Error, "SCENARIO_START_REGION_UNKNOWN",
                        path + ".startRegionId",
                        "Scenario \"" + def.id + "\" references unknown start Region \"" +
                        def.startRegionId + "\".", ""});
                    continue;
                }
                if (!def.startNodeId.empty() && !regionHasNode(*region, def.startNodeId)) {
                    msgs.push_back({Severity::Error, "SCENARIO_START_NODE_UNKNOWN",
                        path + ".startNodeId",
                        "Scenario \"" + def.id + "\" start node \"" + def.startNodeId +
                        "\" does not exist in Region \"" + def.startRegionId + "\".", ""});
                    continue;
                }

                // Inline outcome (optional). Key presence selects inline vs the
                // global M12 fallback. Reuses the shared condition tree exactly.
                const bool hasVictory = entry.contains("victoryConditions");
                const bool hasDefeat  = entry.contains("defeatConditions");
                def.hasInlineOutcome = hasVictory || hasDefeat;
                auto loadConditions = [&](const char* key, const std::string& label,
                                          std::vector<gameplay::events::EventCondition>& dst) {
                    if (!entry.contains(key)) return;
                    if (!entry[key].is_array()) {
                        msgs.push_back({Severity::Error, "SCENARIO_OUTCOME_LIST_NOT_ARRAY",
                            path + "." + label,
                            std::string{"\""} + key + "\" must be a JSON array.", ""});
                        return;
                    }
                    for (size_t j = 0; j < entry[key].size(); ++j) {
                        const auto condPath = path + "." + label + "[" + std::to_string(j) + "]";
                        gameplay::events::ValidateConditionTree(entry[key][j], condPath, msgs);
                        dst.push_back(gameplay::events::ParseCondition(entry[key][j]));
                    }
                };
                loadConditions("victoryConditions", "victoryConditions", def.victoryConditions);
                loadConditions("defeatConditions",  "defeatConditions",  def.defeatConditions);

                output.push_back(std::move(def));
            }
            return true;
        }

        // M16-a: campaigns.json loader. Optional file; absent/empty means no
        // campaign (standalone startup is preserved). References are validated
        // against the already-loaded scenarios. Campaign-level id problems skip
        // the whole campaign; bad scenario entries / refs emit errors but keep
        // the surrounding valid data.
        bool LoadCampaignsFile(
            const nlohmann::json& root,
            std::vector<CampaignDefinition>& output,
            const std::vector<ScenarioDefinition>& scenarios,
            std::vector<ValidationMessage>& msgs)
        {
            output.clear();
            if (!root.contains("campaigns") || !root["campaigns"].is_array()) {
                return true;
            }

            auto scenarioExists = [&](const std::string& id) {
                for (const auto& s : scenarios) {
                    if (s.id == id) return true;
                }
                return false;
            };

            std::set<std::string> seenCampaignIds;
            for (size_t i = 0; i < root["campaigns"].size(); ++i) {
                const auto& entry = root["campaigns"][i];
                const std::string path = "campaigns[" + std::to_string(i) + "]";
                if (!entry.is_object()) {
                    msgs.push_back({Severity::Error, "CAMPAIGN_NOT_OBJECT", path,
                        "Campaign entry must be a JSON object.", ""});
                    continue;
                }

                CampaignDefinition def;
                def.id = entry.value("id", "");
                if (entry.contains("name") && entry["name"].is_object()) {
                    def.name = entry["name"].value("en", "");
                } else {
                    def.name = entry.value("name", "");
                }
                if (entry.contains("description") && entry["description"].is_object()) {
                    def.description = entry["description"].value("en", "");
                } else {
                    def.description = entry.value("description", "");
                }
                def.startScenarioId = entry.value("startScenarioId", "");

                if (def.id.empty()) {
                    msgs.push_back({Severity::Error, "CAMPAIGN_ID_EMPTY", path + ".id",
                        "Campaign \"id\" is required and must be a non-empty string.", ""});
                    continue;
                }
                if (!seenCampaignIds.insert(def.id).second) {
                    msgs.push_back({Severity::Error, "CAMPAIGN_DUPLICATE", path + ".id",
                        "Duplicate campaign id \"" + def.id + "\".", ""});
                    continue;
                }

                if (entry.contains("campaignFlags") && entry["campaignFlags"].is_array()) {
                    for (const auto& flag : entry["campaignFlags"]) {
                        if (flag.is_string()) def.campaignFlags.push_back(flag.get<std::string>());
                    }
                }

                // Carry-over rules (defined before scenario entries reference them).
                std::set<std::string> seenRuleIds;
                if (entry.contains("carryOverRules") && entry["carryOverRules"].is_array()) {
                    for (size_t r = 0; r < entry["carryOverRules"].size(); ++r) {
                        const auto& ruleJson = entry["carryOverRules"][r];
                        const std::string rpath = path + ".carryOverRules[" + std::to_string(r) + "]";
                        if (!ruleJson.is_object()) continue;
                        CarryOverRule rule;
                        rule.id            = ruleJson.value("id", "");
                        rule.carryGold     = ruleJson.value("carryGold", false);
                        rule.carryRoster   = ruleJson.value("carryRoster", false);
                        rule.carryItems    = ruleJson.value("carryItems", false);
                        rule.carryArtifacts= ruleJson.value("carryArtifacts", false);
                        if (ruleJson.contains("carryStoryFlags") && ruleJson["carryStoryFlags"].is_array()) {
                            for (const auto& f : ruleJson["carryStoryFlags"]) {
                                if (f.is_string()) rule.carryStoryFlags.push_back(f.get<std::string>());
                            }
                        }
                        if (rule.id.empty()) continue;
                        if (!seenRuleIds.insert(rule.id).second) {
                            msgs.push_back({Severity::Error, "CAMPAIGN_CARRYOVER_RULE_DUPLICATE",
                                rpath + ".id",
                                "Duplicate carry-over rule id \"" + rule.id + "\" in campaign \"" +
                                def.id + "\".", ""});
                            continue;
                        }
                        def.carryOverRules.push_back(std::move(rule));
                    }
                }

                // Scenario entries (transition graph nodes).
                std::set<std::string> seenEntryIds;
                if (entry.contains("scenarios") && entry["scenarios"].is_array()) {
                    for (size_t s = 0; s < entry["scenarios"].size(); ++s) {
                        const auto& sJson = entry["scenarios"][s];
                        const std::string spath = path + ".scenarios[" + std::to_string(s) + "]";
                        if (!sJson.is_object()) continue;
                        CampaignScenarioEntry sEntry;
                        sEntry.scenarioId     = sJson.value("scenarioId", "");
                        sEntry.carryOverRuleId= sJson.value("carryOverRuleId", "");
                        if (sJson.contains("nextScenarioIds") && sJson["nextScenarioIds"].is_array()) {
                            for (const auto& n : sJson["nextScenarioIds"]) {
                                if (n.is_string()) sEntry.nextScenarioIds.push_back(n.get<std::string>());
                            }
                        }

                        if (sEntry.scenarioId.empty()) continue;
                        if (!seenEntryIds.insert(sEntry.scenarioId).second) {
                            msgs.push_back({Severity::Error, "CAMPAIGN_SCENARIO_ENTRY_DUPLICATE",
                                spath + ".scenarioId",
                                "Duplicate scenario entry \"" + sEntry.scenarioId + "\" in campaign \"" +
                                def.id + "\".", ""});
                            continue;
                        }
                        if (!scenarioExists(sEntry.scenarioId)) {
                            msgs.push_back({Severity::Error, "CAMPAIGN_SCENARIO_UNKNOWN",
                                spath + ".scenarioId",
                                "Campaign \"" + def.id + "\" references unknown scenario \"" +
                                sEntry.scenarioId + "\".", ""});
                        }
                        for (const auto& nextId : sEntry.nextScenarioIds) {
                            if (!scenarioExists(nextId)) {
                                msgs.push_back({Severity::Error, "CAMPAIGN_NEXT_SCENARIO_UNKNOWN",
                                    spath + ".nextScenarioIds",
                                    "Campaign \"" + def.id + "\" next scenario \"" + nextId +
                                    "\" is unknown.", ""});
                            }
                        }
                        if (!sEntry.carryOverRuleId.empty() &&
                            def.FindCarryOverRule(sEntry.carryOverRuleId) == nullptr) {
                            msgs.push_back({Severity::Error, "CAMPAIGN_CARRYOVER_RULE_UNKNOWN",
                                spath + ".carryOverRuleId",
                                "Campaign \"" + def.id + "\" scenario \"" + sEntry.scenarioId +
                                "\" references unknown carry-over rule \"" + sEntry.carryOverRuleId +
                                "\".", ""});
                        }
                        def.scenarios.push_back(std::move(sEntry));
                    }
                }

                // Start scenario must be present and a node in this campaign.
                if (def.startScenarioId.empty() ||
                    def.FindScenarioEntry(def.startScenarioId) == nullptr) {
                    msgs.push_back({Severity::Error, "CAMPAIGN_START_SCENARIO_UNKNOWN",
                        path + ".startScenarioId",
                        "Campaign \"" + def.id + "\" startScenarioId \"" + def.startScenarioId +
                        "\" is empty or not one of its scenario entries.", ""});
                }

                output.push_back(std::move(def));
            }
            return true;
        }

    } // namespace

    bool ContentRepository::LoadFromDirectory(const std::filesystem::path& root) {
        messages_.clear();

        // Reset optional-loader state up front. Required content files
        // (regions, locations, units, etc.) are always cleared inside their
        // own LoadXxxFile helpers — but optional loaders (events,
        // scenario_outcome, items, artifacts) are only invoked when their
        // file is present. If a caller reuses the same repository and the
        // second load lacks an optional file, the old state would otherwise
        // bleed through. Clear here so reloads are deterministic.
        eventDefinitions_.clear();
        scenarioOutcome_ = ScenarioOutcomeDefinition{};
        items_.clear();
        artifacts_.clear();
        worldMap_ = WorldMapDefinition{};
        scenarios_.clear();
        campaigns_.clear();

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
        const bool enemyLoaded     = LoadEnemyGroupsFile(*enemyDoc, enemyGroups_);
        const bool questDefLoaded  = LoadQuestDefinitionsFile(*questsDoc, questDefinitions_);
        quests_                    = *questsDoc;
        const bool servicesLoaded  = LoadLocationServicesFile(*servicesDoc, locationServices_);

        if (!regionsLoaded || !locationsLoaded || !scenesLoaded || !unitsLoaded ||
            !scenariosLoaded || !enemyLoaded || !questDefLoaded || !servicesLoaded) {
            return false;
        }

        auto eventsDoc = load(root / "events.json");
        if (eventsDoc) {
            LoadEventDefinitionsFile(*eventsDoc, eventDefinitions_, messages_);
        }

        // scenario_outcome.json is optional. Absent/empty means default victory
        // (no hostile teams remain) is the only win path.
        auto outcomeDoc = load(root / "scenario_outcome.json");
        if (outcomeDoc) {
            LoadScenarioOutcomeFile(*outcomeDoc, scenarioOutcome_, messages_);
        }

        // items.json and artifacts.json are optional (M13-a). Absent/empty is
        // legal — runtime layers default to empty inventories.
        auto itemsDoc = load(root / "items.json");
        if (itemsDoc) {
            LoadItemsFile(*itemsDoc, items_, messages_);
        }
        auto artifactsDoc = load(root / "artifacts.json");
        if (artifactsDoc) {
            LoadArtifactsFile(*artifactsDoc, artifacts_, messages_);
        }

        // world_map.json is optional (M15-a). Absent/empty → single-Region
        // scenario, no inter-region travel. Validated against loaded regions_.
        auto worldMapDoc = load(root / "world_map.json");
        if (worldMapDoc) {
            LoadWorldMapFile(*worldMapDoc, worldMap_, regions_, messages_);
        }

        // scenarios.json and campaigns.json are optional (M16-a). Absent/empty →
        // no campaign; standalone startup is preserved. Scenarios load first so
        // campaign references resolve against them.
        auto scenariosFileDoc = load(root / "scenarios.json");
        if (scenariosFileDoc) {
            LoadScenariosFile(*scenariosFileDoc, scenarios_, regions_, messages_);
        }
        auto campaignsDoc = load(root / "campaigns.json");
        if (campaignsDoc) {
            LoadCampaignsFile(*campaignsDoc, campaigns_, scenarios_, messages_);
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

    const std::vector<EnemyGroupDefinition>& ContentRepository::EnemyGroups() const {
        return enemyGroups_;
    }

    const EnemyGroupDefinition* ContentRepository::FindEnemyGroupById(const std::string& id) const {
        for (const auto& group : enemyGroups_) {
            if (group.id == id) {
                return &group;
            }
        }
        return nullptr;
    }

    const nlohmann::json& ContentRepository::Quests() const {
        return quests_;
    }

    const std::vector<gameplay::events::EventDefinition>& ContentRepository::EventDefinitions() const {
        return eventDefinitions_;
    }

    const ScenarioOutcomeDefinition& ContentRepository::ScenarioOutcome() const {
        return scenarioOutcome_;
    }

    const std::vector<ItemDefinition>& ContentRepository::Items() const {
        return items_;
    }

    const ItemDefinition* ContentRepository::FindItemById(const std::string& id) const {
        for (const auto& item : items_) {
            if (item.id == id) {
                return &item;
            }
        }
        return nullptr;
    }

    const std::vector<ArtifactDefinition>& ContentRepository::Artifacts() const {
        return artifacts_;
    }

    const ArtifactDefinition* ContentRepository::FindArtifactById(const std::string& id) const {
        for (const auto& artifact : artifacts_) {
            if (artifact.id == id) {
                return &artifact;
            }
        }
        return nullptr;
    }

    const WorldMapDefinition& ContentRepository::WorldMap() const {
        return worldMap_;
    }

    const WorldMapRegionEntry* ContentRepository::FindWorldMapRegionEntry(const std::string& regionId) const {
        return worldMap_.FindEntry(regionId);
    }

    const std::vector<ScenarioDefinition>& ContentRepository::Scenarios() const {
        return scenarios_;
    }

    const ScenarioDefinition* ContentRepository::FindScenarioById(const std::string& id) const {
        for (const auto& scenario : scenarios_) {
            if (scenario.id == id) {
                return &scenario;
            }
        }
        return nullptr;
    }

    const std::vector<CampaignDefinition>& ContentRepository::Campaigns() const {
        return campaigns_;
    }

    const CampaignDefinition* ContentRepository::FindCampaignById(const std::string& id) const {
        for (const auto& campaign : campaigns_) {
            if (campaign.id == id) {
                return &campaign;
            }
        }
        return nullptr;
    }

} // namespace data

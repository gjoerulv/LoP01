#include "data/ContentValidator.h"

#include <algorithm>
#include <map>
#include <string>

std::vector<ValidationMessage> ContentValidator::ValidateIdentity(const nlohmann::json& doc) const
{
    std::vector<ValidationMessage> msgs;

    if (!doc.is_object()) {
        msgs.push_back({
            Severity::Error,
            "ROOT_NOT_OBJECT",
            "$",
            "Content document root must be a JSON object.",
            ""
        });
        return msgs;
    }

    // schemaVersion
    if (!doc.contains("schemaVersion")) {
        msgs.push_back({
            Severity::Error,
            "SCHEMA_VERSION_MISSING",
            "schemaVersion",
            "\"schemaVersion\" is required.",
            ""
        });
    } else if (!doc["schemaVersion"].is_number_integer()) {
        msgs.push_back({
            Severity::Error,
            "SCHEMA_VERSION_TYPE",
            "schemaVersion",
            "\"schemaVersion\" must be an integer.",
            ""
        });
    }

    // kind
    if (!doc.contains("kind")) {
        msgs.push_back({
            Severity::Error,
            "KIND_MISSING",
            "kind",
            "\"kind\" is required.",
            ""
        });
    } else if (!doc["kind"].is_string()) {
        msgs.push_back({
            Severity::Error,
            "KIND_TYPE",
            "kind",
            "\"kind\" must be a string.",
            ""
        });
    } else if (doc["kind"].get<std::string>().empty()) {
        msgs.push_back({
            Severity::Error,
            "KIND_EMPTY",
            "kind",
            "\"kind\" must not be empty.",
            ""
        });
    }

    // id
    if (!doc.contains("id")) {
        msgs.push_back({
            Severity::Error,
            "ID_MISSING",
            "id",
            "\"id\" is required.",
            ""
        });
    } else if (!doc["id"].is_string()) {
        msgs.push_back({
            Severity::Error,
            "ID_TYPE",
            "id",
            "\"id\" must be a string.",
            ""
        });
    } else if (doc["id"].get<std::string>().empty()) {
        msgs.push_back({
            Severity::Error,
            "ID_EMPTY",
            "id",
            "\"id\" must not be empty.",
            ""
        });
    }

    return msgs;
}

std::vector<ValidationMessage> ContentValidator::ValidateReferences(
    const std::vector<data::RegionDefinition>& regions,
    const std::vector<data::LocationDefinition>& locations,
    const std::vector<data::LocationSceneDefinition>& scenes,
    const std::vector<data::UnitDefinition>& units,
    const std::vector<data::BattleScenarioDefinition>& battleScenarios,
    const std::vector<data::LocationServiceDefinition>& services,
    const std::vector<data::QuestDefinition>& quests) const
{
    std::vector<ValidationMessage> msgs;

    auto hasLocation = [&](const std::string& id) {
        return std::ranges::any_of(locations, [&](const auto& x) { return x.id == id; });
    };

    auto findLocation = [&](const std::string& id) -> const data::LocationDefinition* {
        for (const auto& loc : locations) {
            if (loc.id == id) return &loc;
        }
        return nullptr;
    };

    auto hasScene = [&](const std::string& id) {
        return std::ranges::any_of(scenes, [&](const auto& x) { return x.id == id; });
    };

    auto findScene = [&](const std::string& id) -> const data::LocationSceneDefinition* {
        for (const auto& s : scenes) {
            if (s.id == id) return &s;
        }
        return nullptr;
    };

    auto hasBattleScenario = [&](const std::string& id) {
        return std::ranges::any_of(battleScenarios, [&](const auto& x) { return x.id == id; });
    };

    auto hasUnit = [&](const std::string& id) {
        return std::ranges::any_of(units, [&](const auto& x) { return x.id == id; });
    };

    // Region node and link reference checks
    for (size_t i = 0; i < regions.size(); ++i) {
        const auto& region = regions[i];
        const std::string ri = "regions[" + std::to_string(i) + "]";

        for (size_t j = 0; j < region.nodes.size(); ++j) {
            const auto& node = region.nodes[j];
            if (!hasLocation(node.locationId)) {
                msgs.push_back({Severity::Error, "NODE_LOCATION_NOT_FOUND",
                    ri + ".nodes[" + std::to_string(j) + "].location_id",
                    "Region node references unknown location \"" + node.locationId + "\".", ""});
            }
        }

        for (size_t j = 0; j < region.links.size(); ++j) {
            const auto& link = region.links[j];
            const std::string li = ri + ".links[" + std::to_string(j) + "]";
            if (!hasLocation(link.fromLocationId)) {
                msgs.push_back({Severity::Error, "LINK_LOCATION_NOT_FOUND",
                    li + ".from",
                    "Adjacency link references unknown location \"" + link.fromLocationId + "\".", ""});
            }
            if (!hasLocation(link.toLocationId)) {
                msgs.push_back({Severity::Error, "LINK_LOCATION_NOT_FOUND",
                    li + ".to",
                    "Adjacency link references unknown location \"" + link.toLocationId + "\".", ""});
            }
        }
    }

    // M17 owned-service instance-identity invariant (1 of 2): each location may
    // be placed in at most one RegionNode across all Regions. This guarantees a
    // service definition (bound to a location) resolves to exactly one placed
    // instance, so LocationServiceDefinition::id is a safe global ownership key.
    {
        std::map<std::string, std::string> firstPlacementPath;
        for (size_t i = 0; i < regions.size(); ++i) {
            const auto& region = regions[i];
            for (size_t j = 0; j < region.nodes.size(); ++j) {
                const auto& node = region.nodes[j];
                if (node.locationId.empty()) {
                    continue;
                }
                const std::string path =
                    "regions[" + std::to_string(i) + "].nodes[" + std::to_string(j) + "].location_id";
                const auto [it, inserted] = firstPlacementPath.try_emplace(node.locationId, path);
                if (!inserted) {
                    msgs.push_back({Severity::Error, "LOCATION_MULTIPLY_PLACED", path,
                        "Location \"" + node.locationId + "\" is placed in more than one region node "
                        "(first at " + it->second + "). M17 owned-service ownership requires each "
                        "location to map to a single placed instance.", ""});
                }
            }
        }
    }

    // Location scene and battle scenario reference checks
    for (size_t i = 0; i < locations.size(); ++i) {
        const auto& location = locations[i];
        const std::string li = "locations[" + std::to_string(i) + "]";

        if (!location.sceneId.empty() && !hasScene(location.sceneId)) {
            msgs.push_back({Severity::Error, "LOCATION_SCENE_NOT_FOUND",
                li + ".scene_id",
                "Location references unknown scene \"" + location.sceneId + "\".", ""});
        }

        if (!location.battleScenarioId.empty() && !hasBattleScenario(location.battleScenarioId)) {
            msgs.push_back({Severity::Error, "LOCATION_BATTLE_SCENARIO_NOT_FOUND",
                li + ".battle_scenario_id",
                "Location references unknown battle scenario \"" + location.battleScenarioId + "\".", ""});
        }
    }

    // M17 owned-service instance-identity invariant (2 of 2): service ids must be
    // globally unique. With the single-placement invariant above, this makes
    // LocationServiceDefinition::id a provably-unique owned-service instance key.
    {
        std::map<std::string, size_t> firstServiceIndex;
        for (size_t i = 0; i < services.size(); ++i) {
            const auto& service = services[i];
            if (service.id.empty()) {
                continue;
            }
            const auto [it, inserted] = firstServiceIndex.try_emplace(service.id, i);
            if (!inserted) {
                msgs.push_back({Severity::Error, "SERVICE_ID_DUPLICATE",
                    "services[" + std::to_string(i) + "].id",
                    "Service id \"" + service.id + "\" is not unique (first defined at services["
                    + std::to_string(it->second) + "]). Owned-service ownership requires globally "
                    "unique service ids.", ""});
            }
        }
    }

    // Service reference checks
    for (size_t i = 0; i < services.size(); ++i) {
        const auto& service = services[i];
        const std::string si = "services[" + std::to_string(i) + "]";

        const auto* location = findLocation(service.locationId);
        if (location == nullptr) {
            msgs.push_back({Severity::Error, "SERVICE_LOCATION_NOT_FOUND",
                si + ".location_id",
                "Service references unknown location \"" + service.locationId + "\".", ""});
            continue;
        }

        if (location->sceneId.empty()) {
            msgs.push_back({Severity::Error, "SERVICE_ZONE_NOT_FOUND",
                si + ".zone_id",
                "Service zone \"" + service.zoneId + "\" cannot be resolved because location \""
                + service.locationId + "\" has no scene.", ""});
            continue;
        }

        const auto* scene = findScene(location->sceneId);
        if (scene == nullptr) {
            msgs.push_back({Severity::Error, "SERVICE_ZONE_NOT_FOUND",
                si + ".zone_id",
                "Service zone \"" + service.zoneId + "\" cannot be resolved because scene \""
                + location->sceneId + "\" does not exist.", ""});
            continue;
        }

        const bool zoneExists = std::ranges::any_of(scene->zones,
            [&](const auto& zone) { return zone.id == service.zoneId; });
        if (!zoneExists) {
            msgs.push_back({Severity::Error, "SERVICE_ZONE_NOT_FOUND",
                si + ".zone_id",
                "Service references unknown zone \"" + service.zoneId + "\" in scene \""
                + location->sceneId + "\".", ""});
        }

        if (service.kind == data::LocationServiceKind::Recruit) {
            if (service.unitId.empty() || !hasUnit(service.unitId)) {
                msgs.push_back({Severity::Error, "SERVICE_UNIT_NOT_FOUND",
                    si + ".unit_id",
                    "Recruit service references unknown unit \"" + service.unitId + "\".", ""});
            }
        }
    }

    // Quest target reference checks
    for (size_t i = 0; i < quests.size(); ++i) {
        const auto& quest = quests[i];
        if (!quest.target.empty() &&
            (quest.objective == data::QuestObjectiveType::BringResource ||
             quest.objective == data::QuestObjectiveType::ClearCombatNode ||
             quest.objective == data::QuestObjectiveType::MeetHero))
        {
            if (!hasLocation(quest.target)) {
                msgs.push_back({Severity::Error, "QUEST_TARGET_NOT_FOUND",
                    "quests[" + std::to_string(i) + "].target",
                    "Quest target \"" + quest.target + "\" not found in locations.", ""});
            }
        }
    }

    return msgs;
}

#include "data/ContentValidator.h"

#include <algorithm>
#include <map>
#include <set>
#include <string>

#include "gameplay/ResourceState.h"

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

    // M17 Phase 2: mine/resource-service base-output validation. Independent of
    // the reference checks above so an invalid resource name always reports,
    // regardless of any location/zone issue on the same service. Services with
    // no authored mineOutputs (every existing service) produce no messages.
    for (size_t i = 0; i < services.size(); ++i) {
        const auto& service = services[i];
        const std::string si = "services[" + std::to_string(i) + "]";
        const bool isMine = service.kind == data::LocationServiceKind::Mine;

        // Phase 2b: enforce the service-kind <-> mine_outputs relationship.
        // A mine must produce something; a non-mine service must not carry
        // mine outputs at all (existing non-mine services have none, so they
        // stay valid).
        if (isMine && service.mineOutputs.empty()) {
            msgs.push_back({Severity::Error, "MINE_OUTPUTS_REQUIRED_FOR_MINE",
                si + ".mine_outputs",
                "Mine service \"" + service.id + "\" must define at least one mine output.", ""});
        }
        if (!isMine && !service.mineOutputs.empty()) {
            msgs.push_back({Severity::Error, "MINE_OUTPUTS_FOR_NON_MINE_SERVICE",
                si + ".mine_outputs",
                "Service \"" + service.id + "\" is not a mine but defines mine outputs.", ""});
        }

        // Phase 2b: a given ResourceType may appear at most once per service.
        // ComputeMineDailyOutput resolves strongest-only per output resource, so
        // duplicate authored lines would let a same-resource passive apply more
        // than once at payout time. Tracked by parsed type so an unparseable
        // name reports as RESOURCE_INVALID rather than masking as a duplicate.
        std::set<int> seenResources;

        for (size_t k = 0; k < service.mineOutputs.size(); ++k) {
            const auto& output = service.mineOutputs[k];
            const std::string oi = si + ".mine_outputs[" + std::to_string(k) + "]";

            gameplay::ResourceType parsed;
            if (!gameplay::TryResourceTypeFromString(output.resource, parsed)) {
                msgs.push_back({Severity::Error, "MINE_OUTPUT_RESOURCE_INVALID",
                    oi + ".resource",
                    "Mine output references invalid resource \"" + output.resource
                    + "\". Must be a canonical ResourceType name.", ""});
            }
            else if (!seenResources.insert(static_cast<int>(parsed)).second) {
                msgs.push_back({Severity::Error, "MINE_OUTPUT_RESOURCE_DUPLICATE",
                    oi + ".resource",
                    "Mine output resource \"" + output.resource
                    + "\" appears more than once in this service. Each resource may "
                    "appear at most once.", ""});
            }

            // Authored base outputs must be strictly positive — a zero or
            // negative base output is meaningless for a producing service.
            if (output.amount <= 0) {
                msgs.push_back({Severity::Error, "MINE_OUTPUT_AMOUNT_INVALID",
                    oi + ".amount",
                    "Mine output amount must be positive (got "
                    + std::to_string(output.amount) + ").", ""});
            }
        }
    }

    // Unit passive-effect validation (the canonical passiveEffects vector;
    // legacy mine_production_passive is converted into it at load). Units with no
    // effects (every existing unit) produce no messages.
    for (size_t i = 0; i < units.size(); ++i) {
        const auto& unit = units[i];
        for (size_t k = 0; k < unit.passiveEffects.size(); ++k) {
            const auto& effect = unit.passiveEffects[k];
            const std::string pi =
                "units[" + std::to_string(i) + "].passive_effects[" + std::to_string(k) + "]";

            if (effect.amount <= 0) {
                msgs.push_back({Severity::Error, "PASSIVE_EFFECT_AMOUNT_INVALID",
                    pi + ".amount",
                    "Passive effect amount must be positive (got "
                    + std::to_string(effect.amount) + ").", ""});
            }

            switch (effect.kind) {
                case data::PassiveEffectKind::MineProduction: {
                    if (effect.target != "mine") {
                        msgs.push_back({Severity::Error, "PASSIVE_EFFECT_TARGET_INVALID",
                            pi + ".target",
                            "Mine-production effect has unknown target \"" + effect.target
                            + "\". Only \"mine\" is supported.", ""});
                    }
                    gameplay::ResourceType parsed;
                    if (!gameplay::TryResourceTypeFromString(effect.resource, parsed)) {
                        msgs.push_back({Severity::Error, "PASSIVE_EFFECT_RESOURCE_INVALID",
                            pi + ".resource",
                            "Mine-production effect references invalid resource \""
                            + effect.resource + "\". Must be a canonical ResourceType name.", ""});
                    }
                    break;
                }
                case data::PassiveEffectKind::LeaderEnergy: {
                    // A leader-energy effect carries no resource/target.
                    if (!effect.resource.empty() || !effect.target.empty()) {
                        msgs.push_back({Severity::Error, "PASSIVE_EFFECT_FIELD_UNSUPPORTED",
                            pi,
                            "Leader-energy effect must not define a resource or target.", ""});
                    }
                    break;
                }
                case data::PassiveEffectKind::Unknown:
                default:
                    msgs.push_back({Severity::Error, "PASSIVE_EFFECT_KIND_INVALID",
                        pi + ".kind",
                        "Passive effect has an unknown kind. Supported: "
                        "\"mine_production\", \"leader_energy\".", ""});
                    break;
            }
        }
    }

    // (trader ownership curves are validated separately; they are an optional
    // content collection, not part of the cross-reference graph.)

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

std::vector<ValidationMessage> ContentValidator::ValidateTraderOwnershipCurves(
    const std::vector<data::TraderOwnershipCurve>& curves) const
{
    // Ownership tiers are capped at 8 owned services (docs/core_loop_rules.md §23).
    constexpr int kMaxTier = 8;

    std::vector<ValidationMessage> msgs;

    // Curves are per service type, so a type may appear at most once.
    std::set<int> seenTypes;

    for (size_t i = 0; i < curves.size(); ++i) {
        const auto& curve = curves[i];
        const std::string ci = "trader_curves[" + std::to_string(i) + "]";

        if (!data::IsTraderServiceKind(curve.kind)) {
            msgs.push_back({Severity::Error, "TRADER_CURVE_TYPE_INVALID",
                ci + ".type",
                "Trader ownership curve has unsupported service type \"" + curve.rawType
                + "\". Must be a trader service type.", ""});
            // Without a known type the tier shape is meaningless; skip the rest.
            continue;
        }

        if (!seenTypes.insert(static_cast<int>(curve.kind)).second) {
            msgs.push_back({Severity::Error, "TRADER_CURVE_TYPE_DUPLICATE",
                ci + ".type",
                "Trader ownership curve for service type \"" + curve.rawType
                + "\" is defined more than once. Curves are per service type.", ""});
        }

        const bool isTradingPost = curve.kind == data::LocationServiceKind::TradingPost;

        // Tier numbers must be unique within a curve so resolution is not
        // order-dependent.
        std::set<int> seenTiers;

        for (size_t t = 0; t < curve.tiers.size(); ++t) {
            const auto& tier = curve.tiers[t];
            const std::string ti = ci + ".tiers[" + std::to_string(t) + "]";

            if (tier.tier < 0 || tier.tier > kMaxTier) {
                msgs.push_back({Severity::Error, "TRADER_CURVE_TIER_OUT_OF_RANGE",
                    ti + ".tier",
                    "Trader ownership tier " + std::to_string(tier.tier)
                    + " is out of range (0.." + std::to_string(kMaxTier) + ").", ""});
            }
            else if (!seenTiers.insert(tier.tier).second) {
                msgs.push_back({Severity::Error, "TRADER_CURVE_TIER_DUPLICATE",
                    ti + ".tier",
                    "Trader ownership tier " + std::to_string(tier.tier)
                    + " is defined more than once in this curve.", ""});
            }

            if (isTradingPost) {
                // A Trading Post tier is authored to override the barter rates;
                // an empty matrix is partial authored data, not a "use default"
                // marker (sparse fallback is the no-curve case, not an empty
                // authored tier).
                if (tier.exchangeMatrix.empty()) {
                    msgs.push_back({Severity::Error, "TRADER_EXCHANGE_MATRIX_EMPTY",
                        ti + ".exchange_matrix",
                        "Authored Trading Post tier has an empty exchange matrix. "
                        "Omit the curve to use defaults, or author at least one entry.", ""});
                }
                for (size_t e = 0; e < tier.exchangeMatrix.size(); ++e) {
                    const auto& entry = tier.exchangeMatrix[e];
                    const std::string ei = ti + ".exchange_matrix[" + std::to_string(e) + "]";

                    gameplay::ResourceType from;
                    gameplay::ResourceType to;
                    const bool fromOk = gameplay::TryResourceTypeFromString(entry.from, from);
                    const bool toOk = gameplay::TryResourceTypeFromString(entry.to, to);
                    if (!fromOk) {
                        msgs.push_back({Severity::Error, "TRADER_EXCHANGE_RESOURCE_INVALID",
                            ei + ".from",
                            "Trading Post exchange references invalid resource \"" + entry.from
                            + "\". Must be a canonical ResourceType name.", ""});
                    }
                    if (!toOk) {
                        msgs.push_back({Severity::Error, "TRADER_EXCHANGE_RESOURCE_INVALID",
                            ei + ".to",
                            "Trading Post exchange references invalid resource \"" + entry.to
                            + "\". Must be a canonical ResourceType name.", ""});
                    }
                    if (fromOk && toOk && from == to) {
                        msgs.push_back({Severity::Error, "TRADER_EXCHANGE_SELF",
                            ei,
                            "Trading Post exchange may not exchange a resource for itself ("
                            + entry.from + ").", ""});
                    }
                    // Barter is non-Gold resource-for-resource only; Gold buy/sell
                    // uses base values and the tier price factor, not the matrix.
                    if (fromOk && gameplay::IsGoldResource(from)) {
                        msgs.push_back({Severity::Error, "TRADER_EXCHANGE_GOLD",
                            ei + ".from",
                            "Trading Post barter may not involve Gold. Gold buy/sell uses "
                            "base values and the tier price factor, not the exchange matrix.", ""});
                    }
                    if (toOk && gameplay::IsGoldResource(to)) {
                        msgs.push_back({Severity::Error, "TRADER_EXCHANGE_GOLD",
                            ei + ".to",
                            "Trading Post barter may not involve Gold. Gold buy/sell uses "
                            "base values and the tier price factor, not the exchange matrix.", ""});
                    }
                    if (entry.cost <= 0) {
                        msgs.push_back({Severity::Error, "TRADER_EXCHANGE_COST_INVALID",
                            ei + ".cost",
                            "Trading Post exchange cost must be positive (got "
                            + std::to_string(entry.cost) + ").", ""});
                    }
                }
            }

            // priceFactor applies to every trader tier: for Trading Post it is
            // the Gold-trade favorability scalar; for other types it is the
            // placeholder price scalar. Either way it must be positive.
            if (tier.priceFactor <= 0) {
                msgs.push_back({Severity::Error, "TRADER_CURVE_PRICE_FACTOR_INVALID",
                    ti + ".price_factor",
                    "Trader ownership price factor must be positive (got "
                    + std::to_string(tier.priceFactor) + ").", ""});
            }
        }
    }

    return msgs;
}

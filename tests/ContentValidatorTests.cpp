#include <catch2/catch_test_macros.hpp>
#include "data/ContentValidator.h"
#include <nlohmann/json.hpp>
#include <utility>
#include <vector>

static nlohmann::json ValidDoc()
{
    return nlohmann::json{{"schemaVersion", 1}, {"kind", "Region"}, {"id", "r01"}};
}

TEST_CASE("ContentValidator - valid identity produces no messages")
{
    ContentValidator v;
    auto msgs = v.ValidateIdentity(ValidDoc());
    REQUIRE(msgs.empty());
}

TEST_CASE("ContentValidator - root array produces ROOT_NOT_OBJECT and no other messages")
{
    ContentValidator v;
    auto msgs = v.ValidateIdentity(nlohmann::json::array());
    REQUIRE(msgs.size() == 1);
    REQUIRE(msgs[0].code == "ROOT_NOT_OBJECT");
    REQUIRE(msgs[0].severity == Severity::Error);
    REQUIRE(msgs[0].path == "$");
}

TEST_CASE("ContentValidator - missing schemaVersion produces SCHEMA_VERSION_MISSING")
{
    auto doc = ValidDoc();
    doc.erase("schemaVersion");
    ContentValidator v;
    auto msgs = v.ValidateIdentity(doc);
    REQUIRE(msgs.size() == 1);
    REQUIRE(msgs[0].code == "SCHEMA_VERSION_MISSING");
    REQUIRE(msgs[0].severity == Severity::Error);
    REQUIRE(msgs[0].path == "schemaVersion");
}

TEST_CASE("ContentValidator - non-integer schemaVersion produces SCHEMA_VERSION_TYPE")
{
    auto doc = ValidDoc();
    doc["schemaVersion"] = "1";
    ContentValidator v;
    auto msgs = v.ValidateIdentity(doc);
    REQUIRE(msgs.size() == 1);
    REQUIRE(msgs[0].code == "SCHEMA_VERSION_TYPE");
    REQUIRE(msgs[0].severity == Severity::Error);
    REQUIRE(msgs[0].path == "schemaVersion");
}

TEST_CASE("ContentValidator - non-string kind produces KIND_TYPE")
{
    auto doc = ValidDoc();
    doc["kind"] = 123;
    ContentValidator v;
    auto msgs = v.ValidateIdentity(doc);
    REQUIRE(msgs.size() == 1);
    REQUIRE(msgs[0].code == "KIND_TYPE");
    REQUIRE(msgs[0].severity == Severity::Error);
    REQUIRE(msgs[0].path == "kind");
}

TEST_CASE("ContentValidator - missing kind produces KIND_MISSING")
{
    auto doc = ValidDoc();
    doc.erase("kind");
    ContentValidator v;
    auto msgs = v.ValidateIdentity(doc);
    REQUIRE(msgs.size() == 1);
    REQUIRE(msgs[0].code == "KIND_MISSING");
    REQUIRE(msgs[0].severity == Severity::Error);
    REQUIRE(msgs[0].path == "kind");
}

TEST_CASE("ContentValidator - empty kind produces KIND_EMPTY")
{
    auto doc = ValidDoc();
    doc["kind"] = "";
    ContentValidator v;
    auto msgs = v.ValidateIdentity(doc);
    REQUIRE(msgs.size() == 1);
    REQUIRE(msgs[0].code == "KIND_EMPTY");
    REQUIRE(msgs[0].severity == Severity::Error);
    REQUIRE(msgs[0].path == "kind");
}

TEST_CASE("ContentValidator - non-string id produces ID_TYPE")
{
    auto doc = ValidDoc();
    doc["id"] = 123;
    ContentValidator v;
    auto msgs = v.ValidateIdentity(doc);
    REQUIRE(msgs.size() == 1);
    REQUIRE(msgs[0].code == "ID_TYPE");
    REQUIRE(msgs[0].severity == Severity::Error);
    REQUIRE(msgs[0].path == "id");
}

TEST_CASE("ContentValidator - missing id produces ID_MISSING")
{
    auto doc = ValidDoc();
    doc.erase("id");
    ContentValidator v;
    auto msgs = v.ValidateIdentity(doc);
    REQUIRE(msgs.size() == 1);
    REQUIRE(msgs[0].code == "ID_MISSING");
    REQUIRE(msgs[0].severity == Severity::Error);
    REQUIRE(msgs[0].path == "id");
}

TEST_CASE("ContentValidator - empty id produces ID_EMPTY")
{
    auto doc = ValidDoc();
    doc["id"] = "";
    ContentValidator v;
    auto msgs = v.ValidateIdentity(doc);
    REQUIRE(msgs.size() == 1);
    REQUIRE(msgs[0].code == "ID_EMPTY");
    REQUIRE(msgs[0].severity == Severity::Error);
    REQUIRE(msgs[0].path == "id");
}

// ---------------------------------------------------------------------------
// ValidateReferences tests — pure in-memory, no file I/O
// ---------------------------------------------------------------------------

namespace {

data::LocationSceneDefinition MakeScene(const std::string& id, std::vector<std::string> zoneIds = {}) {
    data::LocationSceneDefinition scene;
    scene.id = id;
    for (const auto& zid : zoneIds) {
        data::LocationSceneZoneDefinition zone;
        zone.id = zid;
        scene.zones.push_back(zone);
    }
    return scene;
}

data::LocationDefinition MakeLocation(const std::string& id,
    const std::string& sceneId = "",
    const std::string& battleScenarioId = "") {
    data::LocationDefinition loc;
    loc.id = id;
    loc.sceneId = sceneId;
    loc.battleScenarioId = battleScenarioId;
    return loc;
}

data::RegionDefinition MakeRegion(const std::string& id,
    std::vector<std::string> nodeLocationIds = {},
    std::vector<std::pair<std::string, std::string>> links = {}) {
    data::RegionDefinition region;
    region.id = id;
    for (const auto& locId : nodeLocationIds) {
        data::RegionNodeDefinition node;
        node.locationId = locId;
        region.nodes.push_back(node);
    }
    for (const auto& [from, to] : links) {
        data::RegionLinkDefinition link;
        link.fromLocationId = from;
        link.toLocationId = to;
        region.links.push_back(link);
    }
    return region;
}

data::LocationServiceDefinition MakeService(const std::string& id,
    const std::string& locationId, const std::string& zoneId,
    data::LocationServiceKind kind = data::LocationServiceKind::Shop,
    const std::string& unitId = "") {
    data::LocationServiceDefinition svc;
    svc.id = id;
    svc.locationId = locationId;
    svc.zoneId = zoneId;
    svc.kind = kind;
    svc.unitId = unitId;
    return svc;
}

data::QuestDefinition MakeQuest(const std::string& id,
    data::QuestObjectiveType objective, const std::string& target) {
    data::QuestDefinition q;
    q.id = id;
    q.objective = objective;
    q.target = target;
    return q;
}

data::UnitDefinition MakeUnit(const std::string& id) {
    data::UnitDefinition u;
    u.id = id;
    return u;
}

data::UnitDefinition MakeUnitWithMinePassive(const std::string& id,
    const std::string& target, const std::string& resource, int amount) {
    data::UnitDefinition u;
    u.id = id;
    u.passiveEffects.push_back(data::UnitPassiveEffect{
        data::PassiveEffectKind::MineProduction, resource, target, amount});
    return u;
}

data::UnitDefinition MakeUnitWithLeaderEnergy(const std::string& id, int amount,
    const std::string& resource = "", const std::string& target = "") {
    data::UnitDefinition u;
    u.id = id;
    u.passiveEffects.push_back(data::UnitPassiveEffect{
        data::PassiveEffectKind::LeaderEnergy, resource, target, amount});
    return u;
}

data::BattleScenarioDefinition MakeBattleScenario(const std::string& id) {
    data::BattleScenarioDefinition bs;
    bs.id = id;
    return bs;
}

} // namespace

TEST_CASE("ContentValidator::ValidateReferences - valid references produce no messages")
{
    const auto loc   = MakeLocation("loc1", "scene1");
    const auto scene = MakeScene("scene1");
    const auto region = MakeRegion("r1", {"loc1"});

    ContentValidator v;
    auto msgs = v.ValidateReferences({region}, {loc}, {scene}, {}, {}, {}, {});
    REQUIRE(msgs.empty());
}

TEST_CASE("ContentValidator::ValidateReferences - region node unknown location produces NODE_LOCATION_NOT_FOUND")
{
    const auto region = MakeRegion("r1", {"ghost_loc"});

    ContentValidator v;
    auto msgs = v.ValidateReferences({region}, {}, {}, {}, {}, {}, {});
    REQUIRE(msgs.size() == 1);
    REQUIRE(msgs[0].code == "NODE_LOCATION_NOT_FOUND");
    REQUIRE(msgs[0].severity == Severity::Error);
    REQUIRE(msgs[0].path == "regions[0].nodes[0].location_id");
}

TEST_CASE("ContentValidator::ValidateReferences - adjacency link unknown to-location produces LINK_LOCATION_NOT_FOUND")
{
    const auto loc    = MakeLocation("loc1");
    const auto region = MakeRegion("r1", {"loc1"}, {{"loc1", "ghost_loc"}});

    ContentValidator v;
    auto msgs = v.ValidateReferences({region}, {loc}, {}, {}, {}, {}, {});

    const bool found = std::ranges::any_of(msgs, [](const auto& m) {
        return m.code == "LINK_LOCATION_NOT_FOUND" && m.path == "regions[0].links[0].to";
    });
    REQUIRE(found);
}

TEST_CASE("ContentValidator::ValidateReferences - location unknown scene produces LOCATION_SCENE_NOT_FOUND")
{
    const auto loc = MakeLocation("loc1", "ghost_scene");

    ContentValidator v;
    auto msgs = v.ValidateReferences({}, {loc}, {}, {}, {}, {}, {});
    REQUIRE(msgs.size() == 1);
    REQUIRE(msgs[0].code == "LOCATION_SCENE_NOT_FOUND");
    REQUIRE(msgs[0].severity == Severity::Error);
    REQUIRE(msgs[0].path == "locations[0].scene_id");
}

TEST_CASE("ContentValidator::ValidateReferences - location unknown battle scenario produces LOCATION_BATTLE_SCENARIO_NOT_FOUND")
{
    const auto loc = MakeLocation("loc1", "", "ghost_bs");

    ContentValidator v;
    auto msgs = v.ValidateReferences({}, {loc}, {}, {}, {}, {}, {});
    REQUIRE(msgs.size() == 1);
    REQUIRE(msgs[0].code == "LOCATION_BATTLE_SCENARIO_NOT_FOUND");
    REQUIRE(msgs[0].severity == Severity::Error);
    REQUIRE(msgs[0].path == "locations[0].battle_scenario_id");
}

TEST_CASE("ContentValidator::ValidateReferences - service unknown location produces SERVICE_LOCATION_NOT_FOUND")
{
    const auto svc = MakeService("svc1", "ghost_loc", "zone_a");

    ContentValidator v;
    auto msgs = v.ValidateReferences({}, {}, {}, {}, {}, {svc}, {});
    REQUIRE(msgs.size() == 1);
    REQUIRE(msgs[0].code == "SERVICE_LOCATION_NOT_FOUND");
    REQUIRE(msgs[0].severity == Severity::Error);
    REQUIRE(msgs[0].path == "services[0].location_id");
}

TEST_CASE("ContentValidator::ValidateReferences - service unknown zone produces SERVICE_ZONE_NOT_FOUND")
{
    const auto loc   = MakeLocation("loc1", "scene1");
    const auto scene = MakeScene("scene1", {"real_zone"});
    const auto svc   = MakeService("svc1", "loc1", "ghost_zone");

    ContentValidator v;
    auto msgs = v.ValidateReferences({}, {loc}, {scene}, {}, {}, {svc}, {});
    REQUIRE(msgs.size() == 1);
    REQUIRE(msgs[0].code == "SERVICE_ZONE_NOT_FOUND");
    REQUIRE(msgs[0].severity == Severity::Error);
    REQUIRE(msgs[0].path == "services[0].zone_id");
}

TEST_CASE("ContentValidator::ValidateReferences - recruit service unknown unit produces SERVICE_UNIT_NOT_FOUND")
{
    const auto loc   = MakeLocation("loc1", "scene1");
    const auto scene = MakeScene("scene1", {"recruit_zone"});
    const auto svc   = MakeService("svc1", "loc1", "recruit_zone",
        data::LocationServiceKind::Recruit, "ghost_unit");

    ContentValidator v;
    auto msgs = v.ValidateReferences({}, {loc}, {scene}, {}, {}, {svc}, {});
    REQUIRE(msgs.size() == 1);
    REQUIRE(msgs[0].code == "SERVICE_UNIT_NOT_FOUND");
    REQUIRE(msgs[0].severity == Severity::Error);
    REQUIRE(msgs[0].path == "services[0].unit_id");
}

TEST_CASE("ContentValidator::ValidateReferences - quest target unknown location produces QUEST_TARGET_NOT_FOUND")
{
    const auto quest = MakeQuest("q1", data::QuestObjectiveType::ClearCombatNode, "ghost_loc");

    ContentValidator v;
    auto msgs = v.ValidateReferences({}, {}, {}, {}, {}, {}, {quest});
    REQUIRE(msgs.size() == 1);
    REQUIRE(msgs[0].code == "QUEST_TARGET_NOT_FOUND");
    REQUIRE(msgs[0].severity == Severity::Error);
    REQUIRE(msgs[0].path == "quests[0].target");
}

// ---------------------------------------------------------------------------
// M17 owned-service instance-identity invariants.
// ---------------------------------------------------------------------------

TEST_CASE("ContentValidator::ValidateReferences - duplicate service id produces SERVICE_ID_DUPLICATE")
{
    const auto loc   = MakeLocation("loc1", "scene1");
    const auto scene = MakeScene("scene1", {"zone_a", "zone_b"});
    const auto svc1  = MakeService("dup_svc", "loc1", "zone_a");
    const auto svc2  = MakeService("dup_svc", "loc1", "zone_b");

    ContentValidator v;
    auto msgs = v.ValidateReferences({}, {loc}, {scene}, {}, {}, {svc1, svc2}, {});

    const bool found = std::ranges::any_of(msgs, [](const auto& m) {
        return m.code == "SERVICE_ID_DUPLICATE" && m.path == "services[1].id"
            && m.severity == Severity::Error;
    });
    REQUIRE(found);
}

TEST_CASE("ContentValidator::ValidateReferences - same location in two region nodes produces LOCATION_MULTIPLY_PLACED")
{
    const auto loc    = MakeLocation("loc1", "scene1");
    const auto scene  = MakeScene("scene1");
    const auto region1 = MakeRegion("r1", {"loc1"});
    const auto region2 = MakeRegion("r2", {"loc1"});

    ContentValidator v;
    auto msgs = v.ValidateReferences({region1, region2}, {loc}, {scene}, {}, {}, {}, {});

    const bool found = std::ranges::any_of(msgs, [](const auto& m) {
        return m.code == "LOCATION_MULTIPLY_PLACED"
            && m.path == "regions[1].nodes[0].location_id"
            && m.severity == Severity::Error;
    });
    REQUIRE(found);
}

TEST_CASE("ContentValidator::ValidateReferences - unique service ids and single placements produce no identity errors")
{
    const auto loc1  = MakeLocation("loc1", "scene1");
    const auto loc2  = MakeLocation("loc2", "scene1");
    const auto scene = MakeScene("scene1", {"zone_a"});
    const auto region = MakeRegion("r1", {"loc1", "loc2"});
    const auto svc1  = MakeService("svc_a", "loc1", "zone_a");
    const auto svc2  = MakeService("svc_b", "loc2", "zone_a");

    ContentValidator v;
    auto msgs = v.ValidateReferences({region}, {loc1, loc2}, {scene}, {}, {}, {svc1, svc2}, {});

    const bool anyIdentityError = std::ranges::any_of(msgs, [](const auto& m) {
        return m.code == "SERVICE_ID_DUPLICATE" || m.code == "LOCATION_MULTIPLY_PLACED";
    });
    REQUIRE_FALSE(anyIdentityError);
}

// ---------------------------------------------------------------------------
// M17 Phase 2 mine/resource-service output validation.
// ---------------------------------------------------------------------------

namespace {

data::LocationServiceDefinition MakeMineService(const std::string& id,
    const std::string& locationId, const std::string& zoneId,
    std::vector<data::MineOutputDefinition> outputs) {
    data::LocationServiceDefinition svc;
    svc.id = id;
    svc.locationId = locationId;
    svc.zoneId = zoneId;
    svc.kind = data::LocationServiceKind::Mine;
    svc.mineOutputs = std::move(outputs);
    return svc;
}

} // namespace

TEST_CASE("ContentValidator::ValidateReferences - valid mine outputs produce no output errors")
{
    const auto loc   = MakeLocation("mine_loc", "scene1");
    const auto scene = MakeScene("scene1", {"mine_face"});
    const auto svc   = MakeMineService("stone_mine", "mine_loc", "mine_face",
        {{"Stone", 2}, {"Gold", 1000}});

    ContentValidator v;
    auto msgs = v.ValidateReferences({}, {loc}, {scene}, {}, {}, {svc}, {});

    const bool anyMineError = std::ranges::any_of(msgs, [](const auto& m) {
        return m.code == "MINE_OUTPUT_RESOURCE_INVALID" || m.code == "MINE_OUTPUT_AMOUNT_INVALID";
    });
    REQUIRE_FALSE(anyMineError);
}

TEST_CASE("ContentValidator::ValidateReferences - invalid mine output resource produces MINE_OUTPUT_RESOURCE_INVALID")
{
    const auto loc   = MakeLocation("mine_loc", "scene1");
    const auto scene = MakeScene("scene1", {"mine_face"});
    const auto svc   = MakeMineService("bad_mine", "mine_loc", "mine_face",
        {{"Mithril", 2}});

    ContentValidator v;
    auto msgs = v.ValidateReferences({}, {loc}, {scene}, {}, {}, {svc}, {});

    const bool found = std::ranges::any_of(msgs, [](const auto& m) {
        return m.code == "MINE_OUTPUT_RESOURCE_INVALID"
            && m.path == "services[0].mine_outputs[0].resource"
            && m.severity == Severity::Error;
    });
    REQUIRE(found);
}

TEST_CASE("ContentValidator::ValidateReferences - non-positive mine output amount produces MINE_OUTPUT_AMOUNT_INVALID")
{
    const auto loc   = MakeLocation("mine_loc", "scene1");
    const auto scene = MakeScene("scene1", {"mine_face"});
    const auto svc   = MakeMineService("zero_mine", "mine_loc", "mine_face",
        {{"Stone", 0}});

    ContentValidator v;
    auto msgs = v.ValidateReferences({}, {loc}, {scene}, {}, {}, {svc}, {});

    const bool found = std::ranges::any_of(msgs, [](const auto& m) {
        return m.code == "MINE_OUTPUT_AMOUNT_INVALID"
            && m.path == "services[0].mine_outputs[0].amount"
            && m.severity == Severity::Error;
    });
    REQUIRE(found);
}

TEST_CASE("ContentValidator::ValidateReferences - service without mine outputs remains valid")
{
    const auto loc   = MakeLocation("loc1", "scene1");
    const auto scene = MakeScene("scene1", {"zone_a"});
    const auto svc   = MakeService("svc_a", "loc1", "zone_a");  // no mineOutputs

    ContentValidator v;
    auto msgs = v.ValidateReferences({}, {loc}, {scene}, {}, {}, {svc}, {});

    const bool anyMineError = std::ranges::any_of(msgs, [](const auto& m) {
        return m.code == "MINE_OUTPUT_RESOURCE_INVALID" || m.code == "MINE_OUTPUT_AMOUNT_INVALID"
            || m.code == "MINE_OUTPUT_RESOURCE_DUPLICATE" || m.code == "MINE_OUTPUTS_REQUIRED_FOR_MINE"
            || m.code == "MINE_OUTPUTS_FOR_NON_MINE_SERVICE";
    });
    REQUIRE_FALSE(anyMineError);
}

TEST_CASE("ContentValidator::ValidateReferences - duplicate mine output resource produces MINE_OUTPUT_RESOURCE_DUPLICATE")
{
    const auto loc   = MakeLocation("mine_loc", "scene1");
    const auto scene = MakeScene("scene1", {"mine_face"});
    const auto svc   = MakeMineService("dup_out_mine", "mine_loc", "mine_face",
        {{"Stone", 2}, {"Stone", 3}});

    ContentValidator v;
    auto msgs = v.ValidateReferences({}, {loc}, {scene}, {}, {}, {svc}, {});

    const bool found = std::ranges::any_of(msgs, [](const auto& m) {
        return m.code == "MINE_OUTPUT_RESOURCE_DUPLICATE"
            && m.path == "services[0].mine_outputs[1].resource"
            && m.severity == Severity::Error;
    });
    REQUIRE(found);
}

TEST_CASE("ContentValidator::ValidateReferences - mine service without outputs produces MINE_OUTPUTS_REQUIRED_FOR_MINE")
{
    const auto loc   = MakeLocation("mine_loc", "scene1");
    const auto scene = MakeScene("scene1", {"mine_face"});
    const auto svc   = MakeMineService("empty_mine", "mine_loc", "mine_face", {});  // kind Mine, no outputs

    ContentValidator v;
    auto msgs = v.ValidateReferences({}, {loc}, {scene}, {}, {}, {svc}, {});

    const bool found = std::ranges::any_of(msgs, [](const auto& m) {
        return m.code == "MINE_OUTPUTS_REQUIRED_FOR_MINE"
            && m.path == "services[0].mine_outputs"
            && m.severity == Severity::Error;
    });
    REQUIRE(found);
}

TEST_CASE("ContentValidator::ValidateReferences - non-mine service with outputs produces MINE_OUTPUTS_FOR_NON_MINE_SERVICE")
{
    const auto loc   = MakeLocation("loc1", "scene1");
    const auto scene = MakeScene("scene1", {"zone_a"});
    auto svc = MakeService("shop_svc", "loc1", "zone_a");  // kind Shop
    svc.mineOutputs = {{"Stone", 2}};

    ContentValidator v;
    auto msgs = v.ValidateReferences({}, {loc}, {scene}, {}, {}, {svc}, {});

    const bool found = std::ranges::any_of(msgs, [](const auto& m) {
        return m.code == "MINE_OUTPUTS_FOR_NON_MINE_SERVICE"
            && m.path == "services[0].mine_outputs"
            && m.severity == Severity::Error;
    });
    REQUIRE(found);
}

// ---------------------------------------------------------------------------
// M17 Phase 3a unit mine-production passive validation.
// ---------------------------------------------------------------------------

namespace {

bool AnyPassiveEffectError(const std::vector<ValidationMessage>& msgs) {
    return std::ranges::any_of(msgs, [](const auto& m) {
        return m.code.rfind("PASSIVE_EFFECT_", 0) == 0;
    });
}

} // namespace

TEST_CASE("ContentValidator::ValidateReferences - valid mine-production effect produces no passive errors")
{
    const auto unit = MakeUnitWithMinePassive("kobold", "mine", "Stone", 1);

    ContentValidator v;
    auto msgs = v.ValidateReferences({}, {}, {}, {unit}, {}, {}, {});
    REQUIRE_FALSE(AnyPassiveEffectError(msgs));
}

TEST_CASE("ContentValidator::ValidateReferences - valid leader-energy effect produces no passive errors")
{
    const auto unit = MakeUnitWithLeaderEnergy("captain", 50);

    ContentValidator v;
    auto msgs = v.ValidateReferences({}, {}, {}, {unit}, {}, {}, {});
    REQUIRE_FALSE(AnyPassiveEffectError(msgs));
}

TEST_CASE("ContentValidator::ValidateReferences - invalid effect resource produces PASSIVE_EFFECT_RESOURCE_INVALID")
{
    const auto unit = MakeUnitWithMinePassive("kobold", "mine", "Mithril", 1);

    ContentValidator v;
    auto msgs = v.ValidateReferences({}, {}, {}, {unit}, {}, {}, {});

    const bool found = std::ranges::any_of(msgs, [](const auto& m) {
        return m.code == "PASSIVE_EFFECT_RESOURCE_INVALID"
            && m.path == "units[0].passive_effects[0].resource"
            && m.severity == Severity::Error;
    });
    REQUIRE(found);
}

TEST_CASE("ContentValidator::ValidateReferences - non-positive effect amount produces PASSIVE_EFFECT_AMOUNT_INVALID")
{
    const auto unit = MakeUnitWithMinePassive("kobold", "mine", "Stone", 0);

    ContentValidator v;
    auto msgs = v.ValidateReferences({}, {}, {}, {unit}, {}, {}, {});

    const bool found = std::ranges::any_of(msgs, [](const auto& m) {
        return m.code == "PASSIVE_EFFECT_AMOUNT_INVALID"
            && m.path == "units[0].passive_effects[0].amount"
            && m.severity == Severity::Error;
    });
    REQUIRE(found);
}

TEST_CASE("ContentValidator::ValidateReferences - unknown mine-production target produces PASSIVE_EFFECT_TARGET_INVALID")
{
    const auto unit = MakeUnitWithMinePassive("kobold", "market", "Stone", 1);

    ContentValidator v;
    auto msgs = v.ValidateReferences({}, {}, {}, {unit}, {}, {}, {});

    const bool found = std::ranges::any_of(msgs, [](const auto& m) {
        return m.code == "PASSIVE_EFFECT_TARGET_INVALID"
            && m.path == "units[0].passive_effects[0].target"
            && m.severity == Severity::Error;
    });
    REQUIRE(found);
}

TEST_CASE("ContentValidator::ValidateReferences - unknown effect kind produces PASSIVE_EFFECT_KIND_INVALID")
{
    data::UnitDefinition unit;
    unit.id = "kobold";
    unit.passiveEffects.push_back(data::UnitPassiveEffect{
        data::PassiveEffectKind::Unknown, "", "", 1});

    ContentValidator v;
    auto msgs = v.ValidateReferences({}, {}, {}, {unit}, {}, {}, {});

    const bool found = std::ranges::any_of(msgs, [](const auto& m) {
        return m.code == "PASSIVE_EFFECT_KIND_INVALID"
            && m.path == "units[0].passive_effects[0].kind"
            && m.severity == Severity::Error;
    });
    REQUIRE(found);
}

TEST_CASE("ContentValidator::ValidateReferences - leader-energy with a resource produces PASSIVE_EFFECT_FIELD_UNSUPPORTED")
{
    const auto unit = MakeUnitWithLeaderEnergy("captain", 50, /*resource=*/"Stone");

    ContentValidator v;
    auto msgs = v.ValidateReferences({}, {}, {}, {unit}, {}, {}, {});

    const bool found = std::ranges::any_of(msgs, [](const auto& m) {
        return m.code == "PASSIVE_EFFECT_FIELD_UNSUPPORTED"
            && m.path == "units[0].passive_effects[0]"
            && m.severity == Severity::Error;
    });
    REQUIRE(found);
}

TEST_CASE("ContentValidator::ValidateReferences - unit without passive effects remains valid")
{
    const auto unit = MakeUnit("plain_unit");

    ContentValidator v;
    auto msgs = v.ValidateReferences({}, {}, {}, {unit}, {}, {}, {});
    REQUIRE_FALSE(AnyPassiveEffectError(msgs));
}

// ---------------------------------------------------------------------------
// M17 Phase 4 trader ownership curve validation.
// ---------------------------------------------------------------------------

namespace {

data::TraderOwnershipCurve MakeTradingPostCurve(
    int tier, std::vector<data::TraderExchangeEntry> matrix) {
    data::TraderOwnershipCurve curve;
    curve.kind = data::LocationServiceKind::TradingPost;
    curve.rawType = "trading_post";
    data::TraderTierEntry entry;
    entry.tier = tier;
    entry.exchangeMatrix = std::move(matrix);
    curve.tiers = {entry};
    return curve;
}

bool HasCode(const std::vector<ValidationMessage>& msgs, const std::string& code) {
    return std::ranges::any_of(msgs, [&](const auto& m) {
        return m.code == code && m.severity == Severity::Error;
    });
}

} // namespace

TEST_CASE("ContentValidator::ValidateTraderOwnershipCurves - empty input produces no messages")
{
    ContentValidator v;
    REQUIRE(v.ValidateTraderOwnershipCurves({}).empty());
}

TEST_CASE("ContentValidator::ValidateTraderOwnershipCurves - valid curves produce no messages")
{
    const auto tp = MakeTradingPostCurve(1, {{"Wood", "Stone", 8}});
    data::TraderOwnershipCurve market;
    market.kind = data::LocationServiceKind::Market;
    market.rawType = "market";
    data::TraderTierEntry m1; m1.tier = 2; m1.priceFactor = 95;
    market.tiers = {m1};

    ContentValidator v;
    REQUIRE(v.ValidateTraderOwnershipCurves({tp, market}).empty());
}

TEST_CASE("ContentValidator::ValidateTraderOwnershipCurves - unsupported type produces TRADER_CURVE_TYPE_INVALID")
{
    data::TraderOwnershipCurve curve;
    curve.kind = data::LocationServiceKind::Shop;  // not a trader type
    curve.rawType = "shop";

    ContentValidator v;
    REQUIRE(HasCode(v.ValidateTraderOwnershipCurves({curve}), "TRADER_CURVE_TYPE_INVALID"));
}

TEST_CASE("ContentValidator::ValidateTraderOwnershipCurves - tier above the cap produces TRADER_CURVE_TIER_OUT_OF_RANGE")
{
    const auto curve = MakeTradingPostCurve(9, {{"Wood", "Stone", 8}});  // > 8

    ContentValidator v;
    REQUIRE(HasCode(v.ValidateTraderOwnershipCurves({curve}), "TRADER_CURVE_TIER_OUT_OF_RANGE"));
}

TEST_CASE("ContentValidator::ValidateTraderOwnershipCurves - invalid exchange resource produces TRADER_EXCHANGE_RESOURCE_INVALID")
{
    const auto curve = MakeTradingPostCurve(1, {{"Mithril", "Stone", 8}});

    ContentValidator v;
    REQUIRE(HasCode(v.ValidateTraderOwnershipCurves({curve}), "TRADER_EXCHANGE_RESOURCE_INVALID"));
}

TEST_CASE("ContentValidator::ValidateTraderOwnershipCurves - self-exchange produces TRADER_EXCHANGE_SELF")
{
    const auto curve = MakeTradingPostCurve(1, {{"Stone", "Stone", 8}});

    ContentValidator v;
    REQUIRE(HasCode(v.ValidateTraderOwnershipCurves({curve}), "TRADER_EXCHANGE_SELF"));
}

TEST_CASE("ContentValidator::ValidateTraderOwnershipCurves - Gold as exchange source produces TRADER_EXCHANGE_GOLD")
{
    const auto curve = MakeTradingPostCurve(1, {{"Gold", "Stone", 8}});

    ContentValidator v;
    REQUIRE(HasCode(v.ValidateTraderOwnershipCurves({curve}), "TRADER_EXCHANGE_GOLD"));
}

TEST_CASE("ContentValidator::ValidateTraderOwnershipCurves - Gold as exchange target produces TRADER_EXCHANGE_GOLD")
{
    const auto curve = MakeTradingPostCurve(1, {{"Wood", "Gold", 8}});

    ContentValidator v;
    REQUIRE(HasCode(v.ValidateTraderOwnershipCurves({curve}), "TRADER_EXCHANGE_GOLD"));
}

TEST_CASE("ContentValidator::ValidateTraderOwnershipCurves - non-positive exchange cost produces TRADER_EXCHANGE_COST_INVALID")
{
    const auto curve = MakeTradingPostCurve(1, {{"Wood", "Stone", 0}});

    ContentValidator v;
    REQUIRE(HasCode(v.ValidateTraderOwnershipCurves({curve}), "TRADER_EXCHANGE_COST_INVALID"));
}

TEST_CASE("ContentValidator::ValidateTraderOwnershipCurves - Trading Post zero price factor produces TRADER_CURVE_PRICE_FACTOR_INVALID")
{
    auto curve = MakeTradingPostCurve(1, {{"Wood", "Stone", 8}});
    curve.tiers[0].priceFactor = 0;

    ContentValidator v;
    REQUIRE(HasCode(v.ValidateTraderOwnershipCurves({curve}), "TRADER_CURVE_PRICE_FACTOR_INVALID"));
}

TEST_CASE("ContentValidator::ValidateTraderOwnershipCurves - Trading Post negative price factor produces TRADER_CURVE_PRICE_FACTOR_INVALID")
{
    auto curve = MakeTradingPostCurve(1, {{"Wood", "Stone", 8}});
    curve.tiers[0].priceFactor = -5;

    ContentValidator v;
    REQUIRE(HasCode(v.ValidateTraderOwnershipCurves({curve}), "TRADER_CURVE_PRICE_FACTOR_INVALID"));
}

TEST_CASE("ContentValidator::ValidateTraderOwnershipCurves - Trading Post positive price factor with a non-Gold matrix passes")
{
    auto curve = MakeTradingPostCurve(1, {{"Wood", "Stone", 8}});
    curve.tiers[0].priceFactor = 150;

    ContentValidator v;
    REQUIRE(v.ValidateTraderOwnershipCurves({curve}).empty());
}

TEST_CASE("ContentValidator::ValidateTraderOwnershipCurves - non-positive price factor produces TRADER_CURVE_PRICE_FACTOR_INVALID")
{
    data::TraderOwnershipCurve market;
    market.kind = data::LocationServiceKind::Market;
    market.rawType = "market";
    data::TraderTierEntry m1; m1.tier = 1; m1.priceFactor = 0;
    market.tiers = {m1};

    ContentValidator v;
    REQUIRE(HasCode(v.ValidateTraderOwnershipCurves({market}), "TRADER_CURVE_PRICE_FACTOR_INVALID"));
}

TEST_CASE("ContentValidator::ValidateTraderOwnershipCurves - duplicate type produces TRADER_CURVE_TYPE_DUPLICATE")
{
    const auto a = MakeTradingPostCurve(0, {{"Wood", "Stone", 10}});
    const auto b = MakeTradingPostCurve(1, {{"Wood", "Stone", 8}});  // second Trading Post curve

    ContentValidator v;
    REQUIRE(HasCode(v.ValidateTraderOwnershipCurves({a, b}), "TRADER_CURVE_TYPE_DUPLICATE"));
}

TEST_CASE("ContentValidator::ValidateTraderOwnershipCurves - duplicate tier within a curve produces TRADER_CURVE_TIER_DUPLICATE")
{
    data::TraderOwnershipCurve curve;
    curve.kind = data::LocationServiceKind::TradingPost;
    curve.rawType = "trading_post";
    data::TraderTierEntry t1; t1.tier = 1; t1.exchangeMatrix = {{"Wood", "Stone", 8}};
    data::TraderTierEntry t1b; t1b.tier = 1; t1b.exchangeMatrix = {{"Stone", "Wood", 8}};
    curve.tiers = {t1, t1b};

    ContentValidator v;
    REQUIRE(HasCode(v.ValidateTraderOwnershipCurves({curve}), "TRADER_CURVE_TIER_DUPLICATE"));
}

TEST_CASE("ContentValidator::ValidateTraderOwnershipCurves - empty authored Trading Post matrix produces TRADER_EXCHANGE_MATRIX_EMPTY")
{
    const auto curve = MakeTradingPostCurve(1, {});  // authored tier with no entries

    ContentValidator v;
    REQUIRE(HasCode(v.ValidateTraderOwnershipCurves({curve}), "TRADER_EXCHANGE_MATRIX_EMPTY"));
}

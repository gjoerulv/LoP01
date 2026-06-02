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

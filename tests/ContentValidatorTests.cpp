#include <catch2/catch_test_macros.hpp>
#include "data/ContentValidator.h"
#include <nlohmann/json.hpp>

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

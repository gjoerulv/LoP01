#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "data/definitions/ArtifactDefinition.h"
#include "data/definitions/ItemDefinition.h"
#include "gameplay/GameSession.h"
#include "gameplay/events/EventEngine.h"

using namespace gameplay;
using namespace gameplay::events;

// Pure-rules tests for the M13-b runtime inventory layer. The rules
// (consumable duplicate-add forbidden, non-consumable stacking up to authored
// stackCap, artifact 999 cap) live inside the giveItem/giveArtifact event
// actions, so the tests exercise them via the action surface — but the
// assertions are about the resulting session inventory state.

namespace {

data::ItemDefinition MakeItem(
    const std::string& id,
    data::ItemSubtype subtype,
    int stackCap = 999)
{
    data::ItemDefinition def;
    def.id = id;
    def.name = id;
    def.subtype = subtype;
    def.stackCap = stackCap;
    return def;
}

data::ArtifactDefinition MakeArtifact(const std::string& id) {
    data::ArtifactDefinition def;
    def.id = id;
    def.name = id;
    def.allowedSlots = { data::ArtifactSlotKind::Misc };
    return def;
}

EventAction MakeAction(const nlohmann::json& j) {
    EventAction a;
    a.type = j.value("type", "");
    a.args = j;
    return a;
}

// Builds a context wired against the session's actual inventory containers
// plus the supplied catalogs.
EventEvaluationContext MakeCtx(
    GameSession&,
    std::vector<ItemStackState>& items,
    std::vector<ArtifactStackState>& artifacts,
    const std::vector<data::ItemDefinition>& itemCatalog,
    const std::vector<data::ArtifactDefinition>& artifactCatalog)
{
    EventEvaluationContext ctx;
    ctx.items           = &items;
    ctx.artifacts       = &artifacts;
    ctx.itemCatalog     = &itemCatalog;
    ctx.artifactCatalog = &artifactCatalog;
    return ctx;
}

} // namespace

TEST_CASE("InventoryRules - fresh session has empty inventory and equipment") {
    GameSession session;
    REQUIRE(session.Items().empty());
    REQUIRE(session.Artifacts().empty());

    const auto eq = session.HeroEquipment("hero_player");
    REQUIRE(eq.attackArtifactId.empty());
    REQUIRE(eq.defenseArtifactId.empty());
    REQUIRE(eq.misc1ArtifactId.empty());
    REQUIRE(eq.misc2ArtifactId.empty());
    REQUIRE(eq.misc3ArtifactId.empty());
}

TEST_CASE("InventoryRules - consumable max-1 enforced: duplicate-add fails explicitly") {
    std::vector<ItemStackState> items;
    std::vector<ArtifactStackState> artifacts;
    const std::vector<data::ItemDefinition> itemCatalog = {
        MakeItem("item_ration", data::ItemSubtype::Consumable, 1)
    };
    const std::vector<data::ArtifactDefinition> artifactCatalog;

    GameSession session;
    auto ctx = MakeCtx(session, items, artifacts, itemCatalog, artifactCatalog);

    const auto first = ExecuteAction(ctx, MakeAction(
        {{"type", "giveItem"}, {"itemId", "item_ration"}, {"amount", 1}}));
    REQUIRE(first.success);
    REQUIRE(items.size() == 1);
    REQUIRE(items[0].itemId == "item_ration");
    REQUIRE(items[0].quantity == 1);

    const auto duplicate = ExecuteAction(ctx, MakeAction(
        {{"type", "giveItem"}, {"itemId", "item_ration"}, {"amount", 1}}));
    REQUIRE_FALSE(duplicate.success);
    REQUIRE_FALSE(duplicate.message.empty());
    REQUIRE(items.size() == 1); // unchanged
    REQUIRE(items[0].quantity == 1);
}

TEST_CASE("InventoryRules - consumable with amount > 1 fails explicitly") {
    std::vector<ItemStackState> items;
    std::vector<ArtifactStackState> artifacts;
    const std::vector<data::ItemDefinition> itemCatalog = {
        MakeItem("item_ration", data::ItemSubtype::Consumable, 1)
    };
    const std::vector<data::ArtifactDefinition> artifactCatalog;

    GameSession session;
    auto ctx = MakeCtx(session, items, artifacts, itemCatalog, artifactCatalog);

    const auto result = ExecuteAction(ctx, MakeAction(
        {{"type", "giveItem"}, {"itemId", "item_ration"}, {"amount", 2}}));
    REQUIRE_FALSE(result.success);
    REQUIRE(items.empty());
}

TEST_CASE("InventoryRules - non-consumable stacks up to authored stackCap") {
    std::vector<ItemStackState> items;
    std::vector<ArtifactStackState> artifacts;
    const std::vector<data::ItemDefinition> itemCatalog = {
        MakeItem("item_log", data::ItemSubtype::Material, 10)
    };
    const std::vector<data::ArtifactDefinition> artifactCatalog;

    GameSession session;
    auto ctx = MakeCtx(session, items, artifacts, itemCatalog, artifactCatalog);

    const auto add1 = ExecuteAction(ctx, MakeAction(
        {{"type", "giveItem"}, {"itemId", "item_log"}, {"amount", 7}}));
    REQUIRE(add1.success);
    REQUIRE(items[0].quantity == 7);

    const auto add2 = ExecuteAction(ctx, MakeAction(
        {{"type", "giveItem"}, {"itemId", "item_log"}, {"amount", 3}}));
    REQUIRE(add2.success);
    REQUIRE(items[0].quantity == 10);
}

TEST_CASE("InventoryRules - non-consumable overflow above stackCap fails explicitly") {
    std::vector<ItemStackState> items;
    std::vector<ArtifactStackState> artifacts;
    const std::vector<data::ItemDefinition> itemCatalog = {
        MakeItem("item_log", data::ItemSubtype::Material, 10)
    };
    const std::vector<data::ArtifactDefinition> artifactCatalog;

    GameSession session;
    auto ctx = MakeCtx(session, items, artifacts, itemCatalog, artifactCatalog);

    static_cast<void>(ExecuteAction(ctx, MakeAction(
        {{"type", "giveItem"}, {"itemId", "item_log"}, {"amount", 8}})));
    const auto overflow = ExecuteAction(ctx, MakeAction(
        {{"type", "giveItem"}, {"itemId", "item_log"}, {"amount", 5}}));
    REQUIRE_FALSE(overflow.success);
    REQUIRE(items[0].quantity == 8); // unchanged after failure
}

TEST_CASE("InventoryRules - artifact stacks cap at 999") {
    std::vector<ItemStackState> items;
    std::vector<ArtifactStackState> artifacts;
    const std::vector<data::ItemDefinition> itemCatalog;
    const std::vector<data::ArtifactDefinition> artifactCatalog = {
        MakeArtifact("artifact_amulet")
    };

    GameSession session;
    auto ctx = MakeCtx(session, items, artifacts, itemCatalog, artifactCatalog);

    const auto fill = ExecuteAction(ctx, MakeAction(
        {{"type", "giveArtifact"}, {"artifactId", "artifact_amulet"}, {"amount", 999}}));
    REQUIRE(fill.success);
    REQUIRE(artifacts[0].quantity == 999);

    const auto overflow = ExecuteAction(ctx, MakeAction(
        {{"type", "giveArtifact"}, {"artifactId", "artifact_amulet"}, {"amount", 1}}));
    REQUIRE_FALSE(overflow.success);
    REQUIRE(artifacts[0].quantity == 999);
}

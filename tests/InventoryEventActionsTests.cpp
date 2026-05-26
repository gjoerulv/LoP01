#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "data/definitions/ArtifactDefinition.h"
#include "data/definitions/ItemDefinition.h"
#include "gameplay/GameSession.h"
#include "gameplay/InventoryState.h"
#include "gameplay/events/EventEngine.h"
#include "gameplay/events/EventParser.h"

using namespace gameplay;
using namespace gameplay::events;

// Contract tests for the four M13-b inventory event actions. Focused on:
//   * happy-path success
//   * missing/invalid args
//   * unknown ids
//   * null context
//   * takeArtifact's no-auto-unequip rule

namespace {

EventAction MakeAction(const nlohmann::json& j) {
    EventAction a;
    a.type = j.value("type", "");
    a.args = j;
    return a;
}

data::ItemDefinition MakeItem(
    const std::string& id,
    data::ItemSubtype subtype = data::ItemSubtype::Material,
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
    def.allowedSlots = { data::ArtifactSlotKind::Attack };
    return def;
}

struct ActionHarness {
    std::vector<ItemStackState> items;
    std::vector<ArtifactStackState> artifacts;
    std::vector<data::ItemDefinition> itemCatalog;
    std::vector<data::ArtifactDefinition> artifactCatalog;

    EventEvaluationContext MakeCtx() {
        EventEvaluationContext ctx;
        ctx.items           = &items;
        ctx.artifacts       = &artifacts;
        ctx.itemCatalog     = &itemCatalog;
        ctx.artifactCatalog = &artifactCatalog;
        return ctx;
    }
};

} // namespace

// ---------------------------------------------------------------------------
// giveItem
// ---------------------------------------------------------------------------

TEST_CASE("giveItem - happy path adds non-consumable to inventory") {
    ActionHarness h;
    h.itemCatalog.push_back(MakeItem("item_log"));
    auto ctx = h.MakeCtx();

    const auto result = ExecuteAction(ctx, MakeAction(
        {{"type", "giveItem"}, {"itemId", "item_log"}, {"amount", 3}}));
    REQUIRE(result.success);
    REQUIRE(h.items.size() == 1);
    REQUIRE(h.items[0].itemId == "item_log");
    REQUIRE(h.items[0].quantity == 3);
}

TEST_CASE("giveItem - unknown item id fails explicitly") {
    ActionHarness h;
    h.itemCatalog.push_back(MakeItem("item_log"));
    auto ctx = h.MakeCtx();

    const auto result = ExecuteAction(ctx, MakeAction(
        {{"type", "giveItem"}, {"itemId", "item_phantom"}, {"amount", 1}}));
    REQUIRE_FALSE(result.success);
    REQUIRE_FALSE(result.message.empty());
    REQUIRE(h.items.empty());
}

TEST_CASE("giveItem - missing amount arg fails explicitly") {
    ActionHarness h;
    h.itemCatalog.push_back(MakeItem("item_log"));
    auto ctx = h.MakeCtx();

    const auto result = ExecuteAction(ctx, MakeAction(
        {{"type", "giveItem"}, {"itemId", "item_log"}}));
    REQUIRE_FALSE(result.success);
}

TEST_CASE("giveItem - non-positive amount fails explicitly") {
    ActionHarness h;
    h.itemCatalog.push_back(MakeItem("item_log"));
    auto ctx = h.MakeCtx();

    const auto zero = ExecuteAction(ctx, MakeAction(
        {{"type", "giveItem"}, {"itemId", "item_log"}, {"amount", 0}}));
    REQUIRE_FALSE(zero.success);
    const auto negative = ExecuteAction(ctx, MakeAction(
        {{"type", "giveItem"}, {"itemId", "item_log"}, {"amount", -5}}));
    REQUIRE_FALSE(negative.success);
}

TEST_CASE("giveItem - null context fails with 'context unavailable'") {
    EventEvaluationContext ctx; // items/artifacts/catalogs all null
    const auto result = ExecuteAction(ctx, MakeAction(
        {{"type", "giveItem"}, {"itemId", "item_log"}, {"amount", 1}}));
    REQUIRE_FALSE(result.success);
    REQUIRE_FALSE(result.message.empty());
}

// ---------------------------------------------------------------------------
// takeItem
// ---------------------------------------------------------------------------

TEST_CASE("takeItem - happy path decrements quantity") {
    ActionHarness h;
    h.itemCatalog.push_back(MakeItem("item_log"));
    h.items.push_back({"item_log", 10});
    auto ctx = h.MakeCtx();

    const auto result = ExecuteAction(ctx, MakeAction(
        {{"type", "takeItem"}, {"itemId", "item_log"}, {"amount", 4}}));
    REQUIRE(result.success);
    REQUIRE(h.items.size() == 1);
    REQUIRE(h.items[0].quantity == 6);
}

TEST_CASE("takeItem - exact-quantity take removes stack entry entirely") {
    ActionHarness h;
    h.itemCatalog.push_back(MakeItem("item_log"));
    h.items.push_back({"item_log", 5});
    auto ctx = h.MakeCtx();

    const auto result = ExecuteAction(ctx, MakeAction(
        {{"type", "takeItem"}, {"itemId", "item_log"}, {"amount", 5}}));
    REQUIRE(result.success);
    REQUIRE(h.items.empty());
}

TEST_CASE("takeItem - insufficient quantity fails and leaves inventory unchanged") {
    ActionHarness h;
    h.itemCatalog.push_back(MakeItem("item_log"));
    h.items.push_back({"item_log", 3});
    auto ctx = h.MakeCtx();

    const auto result = ExecuteAction(ctx, MakeAction(
        {{"type", "takeItem"}, {"itemId", "item_log"}, {"amount", 10}}));
    REQUIRE_FALSE(result.success);
    REQUIRE(h.items.size() == 1);
    REQUIRE(h.items[0].quantity == 3);
}

TEST_CASE("takeItem - unknown id fails explicitly even on empty inventory") {
    ActionHarness h;
    h.itemCatalog.push_back(MakeItem("item_log"));
    auto ctx = h.MakeCtx();

    const auto result = ExecuteAction(ctx, MakeAction(
        {{"type", "takeItem"}, {"itemId", "item_phantom"}, {"amount", 1}}));
    REQUIRE_FALSE(result.success);
}

// ---------------------------------------------------------------------------
// giveArtifact
// ---------------------------------------------------------------------------

TEST_CASE("giveArtifact - happy path adds to unequipped inventory") {
    ActionHarness h;
    h.artifactCatalog.push_back(MakeArtifact("artifact_sword"));
    auto ctx = h.MakeCtx();

    const auto result = ExecuteAction(ctx, MakeAction(
        {{"type", "giveArtifact"}, {"artifactId", "artifact_sword"}, {"amount", 2}}));
    REQUIRE(result.success);
    REQUIRE(h.artifacts.size() == 1);
    REQUIRE(h.artifacts[0].artifactId == "artifact_sword");
    REQUIRE(h.artifacts[0].quantity == 2);
}

TEST_CASE("giveArtifact - unknown id fails explicitly") {
    ActionHarness h;
    h.artifactCatalog.push_back(MakeArtifact("artifact_sword"));
    auto ctx = h.MakeCtx();

    const auto result = ExecuteAction(ctx, MakeAction(
        {{"type", "giveArtifact"}, {"artifactId", "artifact_phantom"}, {"amount", 1}}));
    REQUIRE_FALSE(result.success);
    REQUIRE(h.artifacts.empty());
}

TEST_CASE("giveArtifact - null context fails with 'context unavailable'") {
    EventEvaluationContext ctx;
    const auto result = ExecuteAction(ctx, MakeAction(
        {{"type", "giveArtifact"}, {"artifactId", "artifact_sword"}, {"amount", 1}}));
    REQUIRE_FALSE(result.success);
}

// ---------------------------------------------------------------------------
// takeArtifact — including the no-auto-unequip contract
// ---------------------------------------------------------------------------

TEST_CASE("takeArtifact - happy path removes from unequipped inventory") {
    ActionHarness h;
    h.artifactCatalog.push_back(MakeArtifact("artifact_sword"));
    h.artifacts.push_back({"artifact_sword", 3});
    auto ctx = h.MakeCtx();

    const auto result = ExecuteAction(ctx, MakeAction(
        {{"type", "takeArtifact"}, {"artifactId", "artifact_sword"}, {"amount", 2}}));
    REQUIRE(result.success);
    REQUIRE(h.artifacts.size() == 1);
    REQUIRE(h.artifacts[0].quantity == 1);
}

TEST_CASE("takeArtifact - insufficient unequipped quantity fails explicitly") {
    ActionHarness h;
    h.artifactCatalog.push_back(MakeArtifact("artifact_sword"));
    h.artifacts.push_back({"artifact_sword", 1});
    auto ctx = h.MakeCtx();

    const auto result = ExecuteAction(ctx, MakeAction(
        {{"type", "takeArtifact"}, {"artifactId", "artifact_sword"}, {"amount", 2}}));
    REQUIRE_FALSE(result.success);
    REQUIRE(h.artifacts.size() == 1);
    REQUIRE(h.artifacts[0].quantity == 1);
}

TEST_CASE("takeArtifact - does NOT auto-unequip equipped copies") {
    // Drive through a real GameSession: equip the only copy, then attempt to
    // take it via the event-action surface and verify the slot is untouched
    // and the action fails with insufficient-unequipped.
    auto sword = MakeArtifact("artifact_sword");
    GameSession session;
    session.SetLeaderCapableUnitIds({"hero_player"});
    REQUIRE(session.AddOwnedUnit("hero_player", 1));
    REQUIRE(session.TryAddUnitToActiveParty("hero_player"));
    session.SetArtifactCatalog({ sword });

    // Use a startOfDay event chain that grants then attempts a take.
    EventDefinition give;
    give.id = "evt_give";
    give.trigger.type = EventTriggerType::StartOfDay;
    give.repeat.mode = "once";
    EventAction giveAct;
    giveAct.type = "giveArtifact";
    giveAct.args = nlohmann::json{
        {"type", "giveArtifact"}, {"artifactId", "artifact_sword"}, {"amount", 1}
    };
    give.actions.push_back(giveAct);
    session.InitializeEventDefinitions({give});
    static_cast<void>(session.NotifyStartOfDay());

    // Equip it.
    REQUIRE(session.TryEquipArtifact("hero_player", ArtifactEquipSlot::Attack, "artifact_sword").success);
    REQUIRE(session.Artifacts().empty());
    REQUIRE(session.HeroEquipment("hero_player").attackArtifactId == "artifact_sword");

    // Now fire a second startOfDay event with takeArtifact. Because the slot
    // has the only copy and `artifacts_` is empty, the action must fail and
    // leave the equipped slot untouched.
    EventDefinition take;
    take.id = "evt_take";
    take.trigger.type = EventTriggerType::StartOfDay;
    take.repeat.mode = "once";
    EventAction takeAct;
    takeAct.type = "takeArtifact";
    takeAct.args = nlohmann::json{
        {"type", "takeArtifact"}, {"artifactId", "artifact_sword"}, {"amount", 1}
    };
    take.actions.push_back(takeAct);
    session.InitializeEventDefinitions({give, take});
    // Need a fresh day so the once-only events can fire — call NotifyStartOfDay
    // which uses the firedEventIds_ list. The `give` event already fired in
    // the previous call, so its id is in firedEventIds_; it won't refire.
    const auto results = session.NotifyStartOfDay();

    // Find the take result and assert it failed.
    bool sawTakeFailure = false;
    for (const auto& r : results) {
        if (!r.success) { sawTakeFailure = true; break; }
    }
    REQUIRE(sawTakeFailure);
    // Equipped slot is intact.
    REQUIRE(session.HeroEquipment("hero_player").attackArtifactId == "artifact_sword");
    REQUIRE(session.Artifacts().empty());
}

TEST_CASE("takeArtifact - unknown id fails explicitly") {
    ActionHarness h;
    h.artifactCatalog.push_back(MakeArtifact("artifact_sword"));
    auto ctx = h.MakeCtx();

    const auto result = ExecuteAction(ctx, MakeAction(
        {{"type", "takeArtifact"}, {"artifactId", "artifact_phantom"}, {"amount", 1}}));
    REQUIRE_FALSE(result.success);
}

// ---------------------------------------------------------------------------
// EventParser action-type recognition
// ---------------------------------------------------------------------------

TEST_CASE("EventParser - giveItem/takeItem/giveArtifact/takeArtifact recognised by validator") {
    // The validator's IsKnownActionType must accept all four new action types.
    // Use the same path the content loader exercises: ValidateEventDefinition.
    const nlohmann::json doc = {
        {"id", "evt_x"},
        {"trigger", {{"type", "startOfDay"}}},
        {"actions", nlohmann::json::array({
            {{"type", "giveItem"},     {"itemId", "x"},     {"amount", 1}},
            {{"type", "takeItem"},     {"itemId", "x"},     {"amount", 1}},
            {{"type", "giveArtifact"}, {"artifactId", "x"}, {"amount", 1}},
            {{"type", "takeArtifact"}, {"artifactId", "x"}, {"amount", 1}}
        })}
    };
    const auto msgs = gameplay::events::ValidateEventDefinition(doc);
    // No EVENT_ACTION_TYPE_UNKNOWN among the messages.
    for (const auto& m : msgs) {
        REQUIRE(m.code != "EVENT_ACTION_TYPE_UNKNOWN");
    }
}

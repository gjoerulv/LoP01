#include <catch2/catch_test_macros.hpp>
#include "gameplay/events/EventEngine.h"
#include "gameplay/events/EventParser.h"
#include <nlohmann/json.hpp>

using namespace gameplay::events;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static EventCondition LeafCondition(const nlohmann::json& j)
{
    return ParseCondition(j);
}

static EventCondition AlwaysCondition()
{
    return LeafCondition({{"type", "always"}});
}

static EventCondition HasResourceCondition(const std::string& resource, int amount)
{
    return LeafCondition({{"type", "teamHasResource"}, {"resource", resource}, {"amount", amount}});
}

static EventCondition HasHeroCondition(const std::string& heroId)
{
    return LeafCondition({{"type", "teamHasHero"}, {"heroId", heroId}});
}

static EventCondition FlagSetCondition(const std::string& flag)
{
    return LeafCondition({{"type", "storyFlagSet"}, {"flag", flag}});
}

static EventAction MakeAction(const nlohmann::json& j)
{
    EventAction a;
    a.type = j.value("type", "");
    a.args = j;
    return a;
}

// ---------------------------------------------------------------------------
// EvaluateCondition — leaf types
// ---------------------------------------------------------------------------

TEST_CASE("EvaluateCondition - always returns true")
{
    EventEvaluationContext ctx;
    REQUIRE(EvaluateCondition(ctx, AlwaysCondition()));
}

TEST_CASE("EvaluateCondition - Unknown kind (absent condition) returns true")
{
    EventEvaluationContext ctx;
    REQUIRE(EvaluateCondition(ctx, EventCondition{}));
}

TEST_CASE("EvaluateCondition - teamHasResource sufficient returns true")
{
    EventEvaluationContext ctx;
    ctx.resources["Wood"] = 20;
    REQUIRE(EvaluateCondition(ctx, HasResourceCondition("Wood", 10)));
}

TEST_CASE("EvaluateCondition - teamHasResource exact match returns true")
{
    EventEvaluationContext ctx;
    ctx.resources["Gold"] = 10;
    REQUIRE(EvaluateCondition(ctx, HasResourceCondition("Gold", 10)));
}

TEST_CASE("EvaluateCondition - teamHasResource insufficient returns false")
{
    EventEvaluationContext ctx;
    ctx.resources["Wood"] = 5;
    REQUIRE_FALSE(EvaluateCondition(ctx, HasResourceCondition("Wood", 10)));
}

TEST_CASE("EvaluateCondition - teamHasResource absent resource treated as zero")
{
    EventEvaluationContext ctx;
    REQUIRE_FALSE(EvaluateCondition(ctx, HasResourceCondition("Wood", 1)));
}

TEST_CASE("EvaluateCondition - teamHasHero present returns true")
{
    EventEvaluationContext ctx;
    ctx.heroIds = {"hero_jon", "hero_arya"};
    REQUIRE(EvaluateCondition(ctx, HasHeroCondition("hero_jon")));
}

TEST_CASE("EvaluateCondition - teamHasHero absent returns false")
{
    EventEvaluationContext ctx;
    ctx.heroIds = {"hero_jon"};
    REQUIRE_FALSE(EvaluateCondition(ctx, HasHeroCondition("hero_sansa")));
}

TEST_CASE("EvaluateCondition - storyFlagSet flag is set returns true")
{
    EventEvaluationContext ctx;
    ctx.storyFlags = {"chapter2_started"};
    REQUIRE(EvaluateCondition(ctx, FlagSetCondition("chapter2_started")));
}

TEST_CASE("EvaluateCondition - storyFlagSet flag not set returns false")
{
    EventEvaluationContext ctx;
    REQUIRE_FALSE(EvaluateCondition(ctx, FlagSetCondition("chapter2_started")));
}

TEST_CASE("EvaluateCondition - unknown leaf type returns false")
{
    EventEvaluationContext ctx;
    const auto cond = LeafCondition({{"type", "unknownConditionType"}});
    REQUIRE_FALSE(EvaluateCondition(ctx, cond));
}

// ---------------------------------------------------------------------------
// EvaluateCondition — composition
// ---------------------------------------------------------------------------

TEST_CASE("EvaluateCondition - All with all true returns true")
{
    EventEvaluationContext ctx;
    ctx.resources["Wood"] = 10;
    ctx.heroIds = {"hero_jon"};

    EventCondition all;
    all.kind = EventConditionKind::All;
    all.operands = {AlwaysCondition(), HasResourceCondition("Wood", 5)};
    REQUIRE(EvaluateCondition(ctx, all));
}

TEST_CASE("EvaluateCondition - All with one false returns false")
{
    EventEvaluationContext ctx;
    ctx.resources["Wood"] = 3;

    EventCondition all;
    all.kind = EventConditionKind::All;
    all.operands = {AlwaysCondition(), HasResourceCondition("Wood", 10)};
    REQUIRE_FALSE(EvaluateCondition(ctx, all));
}

TEST_CASE("EvaluateCondition - All with empty operands returns true")
{
    EventEvaluationContext ctx;
    EventCondition all;
    all.kind = EventConditionKind::All;
    REQUIRE(EvaluateCondition(ctx, all));
}

TEST_CASE("EvaluateCondition - Any with all false returns false")
{
    EventEvaluationContext ctx;

    EventCondition any;
    any.kind = EventConditionKind::Any;
    any.operands = {HasResourceCondition("Wood", 1), HasHeroCondition("hero_jon")};
    REQUIRE_FALSE(EvaluateCondition(ctx, any));
}

TEST_CASE("EvaluateCondition - Any with one true returns true")
{
    EventEvaluationContext ctx;
    ctx.heroIds = {"hero_jon"};

    EventCondition any;
    any.kind = EventConditionKind::Any;
    any.operands = {HasResourceCondition("Wood", 100), HasHeroCondition("hero_jon")};
    REQUIRE(EvaluateCondition(ctx, any));
}

TEST_CASE("EvaluateCondition - Any with empty operands returns false")
{
    EventEvaluationContext ctx;
    EventCondition any;
    any.kind = EventConditionKind::Any;
    REQUIRE_FALSE(EvaluateCondition(ctx, any));
}

TEST_CASE("EvaluateCondition - Not wraps false to true")
{
    EventEvaluationContext ctx;

    EventCondition notCond;
    notCond.kind = EventConditionKind::Not;
    notCond.operands = {HasHeroCondition("hero_absent")};
    REQUIRE(EvaluateCondition(ctx, notCond));
}

TEST_CASE("EvaluateCondition - Not wraps true to false")
{
    EventEvaluationContext ctx;

    EventCondition notCond;
    notCond.kind = EventConditionKind::Not;
    notCond.operands = {AlwaysCondition()};
    REQUIRE_FALSE(EvaluateCondition(ctx, notCond));
}

TEST_CASE("EvaluateCondition - nested all/any/leaf evaluates correctly")
{
    EventEvaluationContext ctx;
    ctx.resources["Wood"] = 10;
    ctx.heroIds = {"hero_jon"};

    // all [ always, any [ hasResource(Wood,5), hasHero(hero_missing) ] ] → true
    EventCondition innerAny;
    innerAny.kind = EventConditionKind::Any;
    innerAny.operands = {HasResourceCondition("Wood", 5), HasHeroCondition("hero_missing")};

    EventCondition outer;
    outer.kind = EventConditionKind::All;
    outer.operands = {AlwaysCondition(), innerAny};

    REQUIRE(EvaluateCondition(ctx, outer));
}

// ---------------------------------------------------------------------------
// ExecuteAction — individual actions
// ---------------------------------------------------------------------------

TEST_CASE("ExecuteAction - showMessage returns success and populates message")
{
    EventEvaluationContext ctx;
    auto result = ExecuteAction(ctx,
        MakeAction({{"type", "showMessage"}, {"text", {{"en", "Hello!"}}}}));
    REQUIRE(result.success);
    REQUIRE(result.message == "Hello!");
}

TEST_CASE("ExecuteAction - showMessage with no text returns success and empty message")
{
    EventEvaluationContext ctx;
    auto result = ExecuteAction(ctx, MakeAction({{"type", "showMessage"}}));
    REQUIRE(result.success);
    REQUIRE(result.message.empty());
}

TEST_CASE("ExecuteAction - giveResource increments resource in context")
{
    EventEvaluationContext ctx;
    ctx.resources["Wood"] = 5;
    auto result = ExecuteAction(ctx,
        MakeAction({{"type", "giveResource"}, {"resource", "Wood"}, {"amount", 10}}));
    REQUIRE(result.success);
    REQUIRE(ctx.resources["Wood"] == 15);
}

TEST_CASE("ExecuteAction - giveResource adds new resource if absent")
{
    EventEvaluationContext ctx;
    auto result = ExecuteAction(ctx,
        MakeAction({{"type", "giveResource"}, {"resource", "Gold"}, {"amount", 100}}));
    REQUIRE(result.success);
    REQUIRE(ctx.resources["Gold"] == 100);
}

TEST_CASE("ExecuteAction - takeResource with sufficient amount decrements resource")
{
    EventEvaluationContext ctx;
    ctx.resources["Wood"] = 20;
    auto result = ExecuteAction(ctx,
        MakeAction({{"type", "takeResource"}, {"resource", "Wood"}, {"amount", 10}}));
    REQUIRE(result.success);
    REQUIRE(ctx.resources["Wood"] == 10);
}

TEST_CASE("ExecuteAction - takeResource with exact amount succeeds")
{
    EventEvaluationContext ctx;
    ctx.resources["Wood"] = 10;
    auto result = ExecuteAction(ctx,
        MakeAction({{"type", "takeResource"}, {"resource", "Wood"}, {"amount", 10}}));
    REQUIRE(result.success);
    REQUIRE(ctx.resources["Wood"] == 0);
}

TEST_CASE("ExecuteAction - takeResource with insufficient amount fails and leaves resource unchanged")
{
    EventEvaluationContext ctx;
    ctx.resources["Wood"] = 5;
    auto result = ExecuteAction(ctx,
        MakeAction({{"type", "takeResource"}, {"resource", "Wood"}, {"amount", 10}}));
    REQUIRE_FALSE(result.success);
    REQUIRE_FALSE(result.message.empty());
    REQUIRE(ctx.resources["Wood"] == 5);
}

TEST_CASE("ExecuteAction - takeResource absent resource fails hard")
{
    EventEvaluationContext ctx;
    auto result = ExecuteAction(ctx,
        MakeAction({{"type", "takeResource"}, {"resource", "Wood"}, {"amount", 1}}));
    REQUIRE_FALSE(result.success);
    REQUIRE(ctx.resources.find("Wood") == ctx.resources.end());
}

TEST_CASE("ExecuteAction - setStoryFlag inserts flag into context")
{
    EventEvaluationContext ctx;
    auto result = ExecuteAction(ctx,
        MakeAction({{"type", "setStoryFlag"}, {"flag", "chapter2_started"}}));
    REQUIRE(result.success);
    REQUIRE(ctx.storyFlags.contains("chapter2_started"));
}

TEST_CASE("ExecuteAction - clearStoryFlag removes flag from context")
{
    EventEvaluationContext ctx;
    ctx.storyFlags.insert("old_flag");
    auto result = ExecuteAction(ctx,
        MakeAction({{"type", "clearStoryFlag"}, {"flag", "old_flag"}}));
    REQUIRE(result.success);
    REQUIRE_FALSE(ctx.storyFlags.contains("old_flag"));
}

TEST_CASE("ExecuteAction - clearStoryFlag on absent flag succeeds silently")
{
    EventEvaluationContext ctx;
    auto result = ExecuteAction(ctx,
        MakeAction({{"type", "clearStoryFlag"}, {"flag", "nonexistent"}}));
    REQUIRE(result.success);
}

TEST_CASE("ExecuteAction - if condition true executes then branch")
{
    EventEvaluationContext ctx;
    // if (always) then [setStoryFlag "branch_taken"] else [setStoryFlag "else_taken"]
    auto result = ExecuteAction(ctx, MakeAction({
        {"type", "if"},
        {"condition", {{"type", "always"}}},
        {"then", nlohmann::json::array({{{"type", "setStoryFlag"}, {"flag", "branch_taken"}}})},
        {"else", nlohmann::json::array({{{"type", "setStoryFlag"}, {"flag", "else_taken"}}})}
    }));
    REQUIRE(ctx.storyFlags.contains("branch_taken"));
    REQUIRE_FALSE(ctx.storyFlags.contains("else_taken"));
}

TEST_CASE("ExecuteAction - if condition false executes else branch")
{
    EventEvaluationContext ctx;
    const auto neverCond = nlohmann::json{{"type", "storyFlagSet"}, {"flag", "absent"}};
    auto result = ExecuteAction(ctx, MakeAction({
        {"type", "if"},
        {"condition", neverCond},
        {"then", nlohmann::json::array({{{"type", "setStoryFlag"}, {"flag", "branch_taken"}}})},
        {"else", nlohmann::json::array({{{"type", "setStoryFlag"}, {"flag", "else_taken"}}})}
    }));
    REQUIRE_FALSE(ctx.storyFlags.contains("branch_taken"));
    REQUIRE(ctx.storyFlags.contains("else_taken"));
}

TEST_CASE("ExecuteAction - if condition false with no else is no-op success")
{
    EventEvaluationContext ctx;
    const auto neverCond = nlohmann::json{{"type", "storyFlagSet"}, {"flag", "absent"}};
    auto result = ExecuteAction(ctx, MakeAction({
        {"type", "if"},
        {"condition", neverCond},
        {"then", nlohmann::json::array({{{"type", "setStoryFlag"}, {"flag", "branch_taken"}}})}
    }));
    REQUIRE(result.success);
    REQUIRE(ctx.storyFlags.empty());
}

TEST_CASE("ExecuteAction - unknown action type returns failure with message")
{
    EventEvaluationContext ctx;
    auto result = ExecuteAction(ctx, MakeAction({{"type", "doSomethingMagical"}}));
    REQUIRE_FALSE(result.success);
    REQUIRE_FALSE(result.message.empty());
}

// ---------------------------------------------------------------------------
// ExecuteActions — ordering and non-atomic semantics
// ---------------------------------------------------------------------------

TEST_CASE("ExecuteActions - empty list returns empty results")
{
    EventEvaluationContext ctx;
    auto results = ExecuteActions(ctx, {});
    REQUIRE(results.empty());
}

TEST_CASE("ExecuteActions - two successful actions both succeed and return two results")
{
    EventEvaluationContext ctx;
    ctx.resources["Wood"] = 20;
    const std::vector<EventAction> actions = {
        MakeAction({{"type", "setStoryFlag"}, {"flag", "flag_a"}}),
        MakeAction({{"type", "takeResource"}, {"resource", "Wood"}, {"amount", 5}})
    };
    auto results = ExecuteActions(ctx, actions);
    REQUIRE(results.size() == 2);
    REQUIRE(results[0].success);
    REQUIRE(results[1].success);
    REQUIRE(ctx.storyFlags.contains("flag_a"));
    REQUIRE(ctx.resources["Wood"] == 15);
}

// ---------------------------------------------------------------------------
// ExecuteAction — enemy team mutation actions
// ---------------------------------------------------------------------------

TEST_CASE("ExecuteAction - spawnTeam records pending mutation")
{
    std::vector<EnemyTeamMutation> mutations;
    EventEvaluationContext ctx;
    ctx.pendingTeamMutations = &mutations;
    auto result = ExecuteAction(ctx, MakeAction(
        {{"type", "spawnTeam"}, {"teamColor", "Red"}, {"nodeId", "forest_camp"}}));
    REQUIRE(result.success);
    REQUIRE(mutations.size() == 1);
    REQUIRE(mutations[0].type == EnemyTeamMutationType::Spawn);
    REQUIRE(mutations[0].teamColor == "Red");
    REQUIRE(mutations[0].nodeId == "forest_camp");
}

TEST_CASE("ExecuteAction - removeTeam records pending mutation")
{
    std::vector<EnemyTeamMutation> mutations;
    EventEvaluationContext ctx;
    ctx.pendingTeamMutations = &mutations;
    auto result = ExecuteAction(ctx, MakeAction(
        {{"type", "removeTeam"}, {"teamColor", "Blue"}}));
    REQUIRE(result.success);
    REQUIRE(mutations.size() == 1);
    REQUIRE(mutations[0].type == EnemyTeamMutationType::Remove);
    REQUIRE(mutations[0].teamColor == "Blue");
}

TEST_CASE("ExecuteAction - changeAlliance records pending mutation")
{
    std::vector<EnemyTeamMutation> mutations;
    EventEvaluationContext ctx;
    ctx.pendingTeamMutations = &mutations;
    auto result = ExecuteAction(ctx, MakeAction(
        {{"type", "changeAlliance"}, {"teamColor", "Red"}, {"allyColor", "Blue"}, {"add", true}}));
    REQUIRE(result.success);
    REQUIRE(mutations.size() == 1);
    REQUIRE(mutations[0].type == EnemyTeamMutationType::ChangeAlliance);
    REQUIRE(mutations[0].teamColor == "Red");
    REQUIRE(mutations[0].allyColor == "Blue");
    REQUIRE(mutations[0].addAlliance);
}

TEST_CASE("ExecuteAction - team mutation actions fail when context is null")
{
    EventEvaluationContext ctx; // pendingTeamMutations == nullptr
    auto r1 = ExecuteAction(ctx, MakeAction({{"type", "spawnTeam"}, {"teamColor", "Red"}, {"nodeId", "x"}}));
    auto r2 = ExecuteAction(ctx, MakeAction({{"type", "removeTeam"}, {"teamColor", "Red"}}));
    auto r3 = ExecuteAction(ctx, MakeAction({{"type", "changeAlliance"}, {"teamColor", "Red"}, {"allyColor", "Blue"}}));
    REQUIRE_FALSE(r1.success); REQUIRE_FALSE(r1.message.empty());
    REQUIRE_FALSE(r2.success); REQUIRE_FALSE(r2.message.empty());
    REQUIRE_FALSE(r3.success); REQUIRE_FALSE(r3.message.empty());
}

TEST_CASE("ExecuteAction - spawnTeam fails when required arg is missing")
{
    std::vector<EnemyTeamMutation> mutations;
    EventEvaluationContext ctx;
    ctx.pendingTeamMutations = &mutations;
    // Missing nodeId
    auto result = ExecuteAction(ctx, MakeAction({{"type", "spawnTeam"}, {"teamColor", "Red"}}));
    REQUIRE_FALSE(result.success);
    REQUIRE(mutations.empty());
}

TEST_CASE("ExecuteAction - changeAlliance fails when 'add' is missing")
{
    std::vector<EnemyTeamMutation> mutations;
    EventEvaluationContext ctx;
    ctx.pendingTeamMutations = &mutations;
    // Omitting 'add' must produce an explicit failure, not a silent default.
    auto result = ExecuteAction(ctx, MakeAction(
        {{"type", "changeAlliance"}, {"teamColor", "Red"}, {"allyColor", "Blue"}}));
    REQUIRE_FALSE(result.success);
    REQUIRE_FALSE(result.message.empty());
    REQUIRE(mutations.empty());
}

TEST_CASE("ExecuteAction - changeAlliance fails when 'add' is not a boolean")
{
    std::vector<EnemyTeamMutation> mutations;
    EventEvaluationContext ctx;
    ctx.pendingTeamMutations = &mutations;
    auto result = ExecuteAction(ctx, MakeAction(
        {{"type", "changeAlliance"}, {"teamColor", "Red"}, {"allyColor", "Blue"}, {"add", "yes"}}));
    REQUIRE_FALSE(result.success);
    REQUIRE(mutations.empty());
}

TEST_CASE("ExecuteAction - changeAlliance accepts add=false explicitly")
{
    std::vector<EnemyTeamMutation> mutations;
    EventEvaluationContext ctx;
    ctx.pendingTeamMutations = &mutations;
    auto result = ExecuteAction(ctx, MakeAction(
        {{"type", "changeAlliance"}, {"teamColor", "Red"}, {"allyColor", "Blue"}, {"add", false}}));
    REQUIRE(result.success);
    REQUIRE(mutations.size() == 1);
    REQUIRE(mutations[0].type == EnemyTeamMutationType::ChangeAlliance);
    REQUIRE_FALSE(mutations[0].addAlliance);
}

TEST_CASE("ExecuteActions - first action succeeds then second fails; first result persists (non-atomic)")
{
    EventEvaluationContext ctx;
    ctx.resources["Wood"] = 5;
    // First: setStoryFlag (succeeds)
    // Second: takeResource 10 Wood (fails — only 5 available)
    const std::vector<EventAction> actions = {
        MakeAction({{"type", "setStoryFlag"}, {"flag", "checkpoint"}}),
        MakeAction({{"type", "takeResource"}, {"resource", "Wood"}, {"amount", 10}})
    };
    auto results = ExecuteActions(ctx, actions);
    REQUIRE(results.size() == 2);
    REQUIRE(results[0].success);
    REQUIRE_FALSE(results[1].success);
    // First action's side effect persists — non-atomic per Decision #155
    REQUIRE(ctx.storyFlags.contains("checkpoint"));
    // Wood unchanged by failed take
    REQUIRE(ctx.resources["Wood"] == 5);
}

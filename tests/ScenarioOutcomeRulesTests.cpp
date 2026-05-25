#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "data/definitions/ScenarioOutcomeDefinition.h"
#include "gameplay/EnemyTeamState.h"
#include "gameplay/events/EventDefinition.h"
#include "gameplay/events/EventEngine.h"
#include "gameplay/events/EventParser.h"
#include "gameplay/scenario/ScenarioOutcomeRules.h"

using namespace gameplay::scenario;
using gameplay::EnemyTeamState;
using gameplay::events::EventCondition;
using gameplay::events::EventEvaluationContext;
using gameplay::events::ParseCondition;

namespace {

EnemyTeamState MakeTeam(const std::string& color, bool active = true,
                       std::vector<std::string> alliances = {})
{
    EnemyTeamState t;
    t.teamColor = color;
    t.active = active;
    t.alliances = std::move(alliances);
    return t;
}

EventCondition FlagSet(const std::string& flag)
{
    return ParseCondition({{"type", "storyFlagSet"}, {"flag", flag}});
}

} // namespace

TEST_CASE("ScenarioOutcomeRules - IsHostileToPlayer: inactive team is not hostile")
{
    auto t = MakeTeam("Red", /*active=*/false);
    REQUIRE_FALSE(IsHostileToPlayer(t, "Green"));
}

TEST_CASE("ScenarioOutcomeRules - IsHostileToPlayer: player's own color is not hostile")
{
    auto t = MakeTeam("Green");
    REQUIRE_FALSE(IsHostileToPlayer(t, "Green"));
}

TEST_CASE("ScenarioOutcomeRules - IsHostileToPlayer: allied team is not hostile")
{
    auto t = MakeTeam("Red", /*active=*/true, /*alliances=*/{"Green"});
    REQUIRE_FALSE(IsHostileToPlayer(t, "Green"));
}

TEST_CASE("ScenarioOutcomeRules - IsHostileToPlayer: non-allied active team is hostile")
{
    auto t = MakeTeam("Red");
    REQUIRE(IsHostileToPlayer(t, "Green"));
}

TEST_CASE("ScenarioOutcomeRules - default victory when no authored conditions and no hostile teams")
{
    EventEvaluationContext condCtx;
    std::vector<EnemyTeamState> teams; // empty roster

    ScenarioOutcomeContext ctx{"Green", &teams, &condCtx};
    data::ScenarioOutcomeDefinition def; // no authored conditions

    auto result = EvaluateScenarioOutcome(ctx, def);
    REQUIRE(result.state == ScenarioOutcomeState::Victory);
    REQUIRE_FALSE(result.matchedConditionIndex.has_value());
}

TEST_CASE("ScenarioOutcomeRules - default victory ignores allied teams")
{
    EventEvaluationContext condCtx;
    std::vector<EnemyTeamState> teams = {
        MakeTeam("Red", true, {"Green"}) // allied to player
    };

    ScenarioOutcomeContext ctx{"Green", &teams, &condCtx};
    data::ScenarioOutcomeDefinition def;

    auto result = EvaluateScenarioOutcome(ctx, def);
    REQUIRE(result.state == ScenarioOutcomeState::Victory);
}

TEST_CASE("ScenarioOutcomeRules - default victory blocked by active hostile team")
{
    EventEvaluationContext condCtx;
    std::vector<EnemyTeamState> teams = { MakeTeam("Red") };

    ScenarioOutcomeContext ctx{"Green", &teams, &condCtx};
    data::ScenarioOutcomeDefinition def;

    auto result = EvaluateScenarioOutcome(ctx, def);
    REQUIRE(result.state == ScenarioOutcomeState::Ongoing);
}

TEST_CASE("ScenarioOutcomeRules - authored victory condition latches with matched index")
{
    EventEvaluationContext condCtx;
    condCtx.storyFlags.insert("ashvale_cleansed");
    std::vector<EnemyTeamState> teams = { MakeTeam("Red") }; // still active — proves authored win bypasses default

    ScenarioOutcomeContext ctx{"Green", &teams, &condCtx};
    data::ScenarioOutcomeDefinition def;
    def.victoryConditions.push_back(FlagSet("absent_flag"));
    def.victoryConditions.push_back(FlagSet("ashvale_cleansed"));

    auto result = EvaluateScenarioOutcome(ctx, def);
    REQUIRE(result.state == ScenarioOutcomeState::Victory);
    REQUIRE(result.matchedConditionIndex.has_value());
    REQUIRE(*result.matchedConditionIndex == 1);
}

TEST_CASE("ScenarioOutcomeRules - authored victoryConditions non-empty disables default victory")
{
    EventEvaluationContext condCtx; // no flag set
    std::vector<EnemyTeamState> teams; // no hostile teams either

    ScenarioOutcomeContext ctx{"Green", &teams, &condCtx};
    data::ScenarioOutcomeDefinition def;
    def.victoryConditions.push_back(FlagSet("never_set"));

    // With an authored victory list, default victory is intentionally disabled —
    // the designer required a specific win path.
    auto result = EvaluateScenarioOutcome(ctx, def);
    REQUIRE(result.state == ScenarioOutcomeState::Ongoing);
}

TEST_CASE("ScenarioOutcomeRules - authored defeat condition latches with matched index")
{
    EventEvaluationContext condCtx;
    condCtx.storyFlags.insert("ashvale_lost");
    std::vector<EnemyTeamState> teams;

    ScenarioOutcomeContext ctx{"Green", &teams, &condCtx};
    data::ScenarioOutcomeDefinition def;
    def.defeatConditions.push_back(FlagSet("ashvale_lost"));

    auto result = EvaluateScenarioOutcome(ctx, def);
    REQUIRE(result.state == ScenarioOutcomeState::Defeat);
    REQUIRE(*result.matchedConditionIndex == 0);
}

TEST_CASE("ScenarioOutcomeRules - defeat takes priority over victory when both match")
{
    EventEvaluationContext condCtx;
    condCtx.storyFlags.insert("ashvale_cleansed");
    condCtx.storyFlags.insert("ashvale_lost");
    std::vector<EnemyTeamState> teams;

    ScenarioOutcomeContext ctx{"Green", &teams, &condCtx};
    data::ScenarioOutcomeDefinition def;
    def.victoryConditions.push_back(FlagSet("ashvale_cleansed"));
    def.defeatConditions.push_back(FlagSet("ashvale_lost"));

    auto result = EvaluateScenarioOutcome(ctx, def);
    REQUIRE(result.state == ScenarioOutcomeState::Defeat);
}

TEST_CASE("ScenarioOutcomeRules - Ongoing when nothing matches")
{
    EventEvaluationContext condCtx;
    std::vector<EnemyTeamState> teams = { MakeTeam("Red") };

    ScenarioOutcomeContext ctx{"Green", &teams, &condCtx};
    data::ScenarioOutcomeDefinition def;
    def.victoryConditions.push_back(FlagSet("never_set"));
    def.defeatConditions.push_back(FlagSet("never_set_either"));

    auto result = EvaluateScenarioOutcome(ctx, def);
    REQUIRE(result.state == ScenarioOutcomeState::Ongoing);
}

#include "gameplay/scenario/ScenarioOutcomeRules.h"

#include <algorithm>

namespace gameplay::scenario {

bool IsHostileToPlayer(const EnemyTeamState& team, const std::string& playerColor)
{
    if (!team.active) return false;
    if (team.teamColor == playerColor) return false;
    return std::find(team.alliances.begin(), team.alliances.end(), playerColor)
        == team.alliances.end();
}

namespace {

bool AnyHostileTeam(const std::vector<EnemyTeamState>& teams, const std::string& playerColor)
{
    return std::any_of(teams.begin(), teams.end(),
        [&](const EnemyTeamState& t) { return IsHostileToPlayer(t, playerColor); });
}

} // namespace

ScenarioOutcome EvaluateScenarioOutcome(
    const ScenarioOutcomeContext& ctx,
    const data::ScenarioOutcomeDefinition& outcome)
{
    // Defeat wins over victory in the same evaluation — see core_loop_rules.md §36.
    if (ctx.conditionContext != nullptr) {
        for (std::size_t i = 0; i < outcome.defeatConditions.size(); ++i) {
            if (events::EvaluateCondition(*ctx.conditionContext, outcome.defeatConditions[i])) {
                return {ScenarioOutcomeState::Defeat, i,
                    "Authored defeat condition #" + std::to_string(i) + " satisfied"};
            }
        }

        for (std::size_t i = 0; i < outcome.victoryConditions.size(); ++i) {
            if (events::EvaluateCondition(*ctx.conditionContext, outcome.victoryConditions[i])) {
                return {ScenarioOutcomeState::Victory, i,
                    "Authored victory condition #" + std::to_string(i) + " satisfied"};
            }
        }
    }

    // Default victory: only when no authored victory conditions exist and no
    // hostile team blocks the field. Authored victory list non-empty disables
    // default victory entirely so designers can require a specific win path.
    if (outcome.victoryConditions.empty() && ctx.enemyTeams != nullptr
        && !AnyHostileTeam(*ctx.enemyTeams, ctx.playerColor)) {
        return {ScenarioOutcomeState::Victory, std::nullopt,
            "All hostile teams defeated"};
    }

    return {ScenarioOutcomeState::Ongoing, std::nullopt, ""};
}

} // namespace gameplay::scenario

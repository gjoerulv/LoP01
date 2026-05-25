#include "gameplay/events/EventEngine.h"
#include "gameplay/events/EventParser.h"

#include <algorithm>
#include <string>

namespace gameplay::events {

bool EvaluateCondition(const EventEvaluationContext& ctx, const EventCondition& cond)
{
    switch (cond.kind) {
        case EventConditionKind::Unknown:
            return true;   // absent condition is always satisfied

        case EventConditionKind::Leaf: {
            const auto& t = cond.leafType;
            const auto& a = cond.leafArgs;

            if (t == "always")
                return true;

            if (t == "teamHasResource") {
                const std::string resource = a.value("resource", "");
                const int amount           = a.value("amount", 0);
                const auto it = ctx.resources.find(resource);
                const int have = (it != ctx.resources.end()) ? it->second : 0;
                return have >= amount;
            }

            if (t == "teamHasHero") {
                const std::string heroId = a.value("heroId", "");
                return std::ranges::any_of(ctx.heroIds,
                    [&](const auto& id) { return id == heroId; });
            }

            if (t == "storyFlagSet") {
                const std::string flag = a.value("flag", "");
                return ctx.storyFlags.contains(flag);
            }

            return false;   // unknown leaf type — safe default; validator should catch this
        }

        case EventConditionKind::All:
            return std::ranges::all_of(cond.operands,
                [&](const auto& op) { return EvaluateCondition(ctx, op); });

        case EventConditionKind::Any:
            return std::ranges::any_of(cond.operands,
                [&](const auto& op) { return EvaluateCondition(ctx, op); });

        case EventConditionKind::Not:
            if (cond.operands.empty()) return true;
            return !EvaluateCondition(ctx, cond.operands[0]);
    }
    return false;
}

ActionResult ExecuteAction(EventEvaluationContext& ctx, const EventAction& action)
{
    const auto& type = action.type;
    const auto& args = action.args;

    if (type == "showMessage") {
        std::string msg;
        if (args.contains("text") && args["text"].is_object()) {
            msg = args["text"].value("en", "");
        }
        return {true, msg};
    }

    if (type == "giveResource") {
        const std::string resource = args.value("resource", "");
        const int amount           = args.value("amount", 0);
        ctx.resources[resource] += amount;
        return {true, ""};
    }

    if (type == "takeResource") {
        const std::string resource = args.value("resource", "");
        const int amount           = args.value("amount", 0);
        const auto it = ctx.resources.find(resource);
        const int have = (it != ctx.resources.end()) ? it->second : 0;
        if (have < amount) {
            return {false,
                "Could not take " + std::to_string(amount) + " " + resource
                + ": only " + std::to_string(have) + " available."};
        }
        ctx.resources[resource] = have - amount;
        return {true, ""};
    }

    if (type == "setStoryFlag") {
        const std::string flag = args.value("flag", "");
        ctx.storyFlags.insert(flag);
        return {true, ""};
    }

    if (type == "clearStoryFlag") {
        const std::string flag = args.value("flag", "");
        ctx.storyFlags.erase(flag);
        return {true, ""};
    }

    if (type == "if") {
        const EventCondition subCond = args.contains("condition") && args["condition"].is_object()
            ? ParseCondition(args["condition"])
            : EventCondition{};   // Unknown → always satisfied

        const bool branch = EvaluateCondition(ctx, subCond);
        const std::string branchKey = branch ? "then" : "else";

        if (!args.contains(branchKey) || !args[branchKey].is_array())
            return {true, ""};

        const auto subActions = ParseActions(args[branchKey]);
        auto results = ExecuteActions(ctx, subActions);

        // The if-action reports failure if any executed branch action fails.
		// Earlier successful side effects are not rolled back.
        ActionResult combined;
        combined.success = true;
        for (const auto& r : results) {
            if (!r.message.empty()) {
                combined.message += r.message + "\n";
            }
            if (!r.success) combined.success = false;
        }
        return combined;
    }

    if (type == "spawnTeam") {
        if (ctx.pendingTeamMutations == nullptr)
            return {false, "spawnTeam: mutation context unavailable"};
        const std::string teamColor = args.value("teamColor", "");
        const std::string nodeId    = args.value("nodeId", "");
        if (teamColor.empty())
            return {false, "spawnTeam: missing required arg 'teamColor'"};
        if (nodeId.empty())
            return {false, "spawnTeam: missing required arg 'nodeId'"};
        ctx.pendingTeamMutations->push_back(
            {EnemyTeamMutationType::Spawn, teamColor, nodeId, {}, true});
        return {true, ""};
    }

    if (type == "removeTeam") {
        if (ctx.pendingTeamMutations == nullptr)
            return {false, "removeTeam: mutation context unavailable"};
        const std::string teamColor = args.value("teamColor", "");
        if (teamColor.empty())
            return {false, "removeTeam: missing required arg 'teamColor'"};
        ctx.pendingTeamMutations->push_back(
            {EnemyTeamMutationType::Remove, teamColor, {}, {}, true});
        return {true, ""};
    }

    if (type == "changeAlliance") {
        if (ctx.pendingTeamMutations == nullptr)
            return {false, "changeAlliance: mutation context unavailable"};
        const std::string teamColor = args.value("teamColor", "");
        const std::string allyColor = args.value("allyColor", "");
        if (teamColor.empty())
            return {false, "changeAlliance: missing required arg 'teamColor'"};
        if (allyColor.empty())
            return {false, "changeAlliance: missing required arg 'allyColor'"};
        // 'add' must be authored explicitly as a boolean. No silent default —
        // omitting the field flipped behavior invisibly between add-alliance and
        // remove-alliance, which contradicts Decision #155's explicit-failure rule.
        if (!args.contains("add") || !args["add"].is_boolean())
            return {false, "changeAlliance: missing required arg 'add' (boolean)"};
        const bool add = args["add"].get<bool>();
        ctx.pendingTeamMutations->push_back(
            {EnemyTeamMutationType::ChangeAlliance, teamColor, {}, allyColor, add});
        return {true, ""};
    }

    return {false, "Unknown action type: \"" + type + "\"."};
}

std::vector<ActionResult> ExecuteActions(
    EventEvaluationContext& ctx,
    const std::vector<EventAction>& actions)
{
    std::vector<ActionResult> results;
    results.reserve(actions.size());
    for (const auto& action : actions) {
        results.push_back(ExecuteAction(ctx, action));
        // Non-atomic: continue executing after failure (Decision #155)
    }
    return results;
}

} // namespace gameplay::events

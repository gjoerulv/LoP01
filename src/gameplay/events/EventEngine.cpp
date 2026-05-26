#include "gameplay/events/EventEngine.h"
#include "gameplay/events/EventParser.h"

#include <algorithm>
#include <string>

#include "data/definitions/ArtifactDefinition.h"
#include "data/definitions/ItemDefinition.h"
#include "gameplay/InventoryState.h"

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

    // ----- M13-b inventory actions ---------------------------------------
    //
    // All four actions follow the giveResource/takeResource failure-mode style:
    // any unknown id, missing/invalid arg, insufficient quantity, or capacity
    // overflow is an explicit hard failure. No silent clamping. No partial
    // success. The four ctx pointers (items, artifacts, itemCatalog,
    // artifactCatalog) must be wired by GameSession::FireMatchingEvents; if
    // any is null the action fails with a "context unavailable" message.

    if (type == "giveItem") {
        if (ctx.items == nullptr || ctx.itemCatalog == nullptr)
            return {false, "giveItem: inventory context unavailable"};
        const std::string itemId = args.value("itemId", "");
        if (itemId.empty())
            return {false, "giveItem: missing required arg 'itemId'"};
        if (!args.contains("amount") || !args["amount"].is_number_integer())
            return {false, "giveItem: missing required arg 'amount' (integer)"};
        const int amount = args["amount"].get<int>();
        if (amount <= 0)
            return {false, "giveItem: 'amount' must be a positive integer"};

        const data::ItemDefinition* def = nullptr;
        for (const auto& d : *ctx.itemCatalog) {
            if (d.id == itemId) { def = &d; break; }
        }
        if (def == nullptr)
            return {false, "giveItem: unknown item id \"" + itemId + "\""};

        // Consumables enforce quantity == 1 and forbid duplicate-add.
        auto existing = std::find_if(ctx.items->begin(), ctx.items->end(),
            [&](const gameplay::ItemStackState& s) { return s.itemId == itemId; });

        if (def->subtype == data::ItemSubtype::Consumable) {
            if (amount != 1)
                return {false, "giveItem: consumable \"" + itemId + "\" must be added with amount 1"};
            if (existing != ctx.items->end())
                return {false, "giveItem: consumable \"" + itemId + "\" already owned (duplicate-add is illegal)"};
            ctx.items->push_back({itemId, 1});
            return {true, ""};
        }

        const int currentQty = (existing != ctx.items->end()) ? existing->quantity : 0;
        if (currentQty + amount > def->stackCap) {
            return {false, "giveItem: \"" + itemId + "\" would exceed stackCap "
                + std::to_string(def->stackCap) + " (have " + std::to_string(currentQty)
                + ", trying to add " + std::to_string(amount) + ")"};
        }
        if (existing == ctx.items->end()) {
            ctx.items->push_back({itemId, amount});
        } else {
            existing->quantity += amount;
        }
        return {true, ""};
    }

    if (type == "takeItem") {
        if (ctx.items == nullptr || ctx.itemCatalog == nullptr)
            return {false, "takeItem: inventory context unavailable"};
        const std::string itemId = args.value("itemId", "");
        if (itemId.empty())
            return {false, "takeItem: missing required arg 'itemId'"};
        if (!args.contains("amount") || !args["amount"].is_number_integer())
            return {false, "takeItem: missing required arg 'amount' (integer)"};
        const int amount = args["amount"].get<int>();
        if (amount <= 0)
            return {false, "takeItem: 'amount' must be a positive integer"};

        // Unknown items still fail explicitly even if the inventory happens to
        // be empty — matches the giveItem id-checking policy.
        bool inCatalog = false;
        for (const auto& d : *ctx.itemCatalog) {
            if (d.id == itemId) { inCatalog = true; break; }
        }
        if (!inCatalog)
            return {false, "takeItem: unknown item id \"" + itemId + "\""};

        auto existing = std::find_if(ctx.items->begin(), ctx.items->end(),
            [&](const gameplay::ItemStackState& s) { return s.itemId == itemId; });
        const int currentQty = (existing != ctx.items->end()) ? existing->quantity : 0;
        if (currentQty < amount) {
            return {false, "takeItem: insufficient quantity of \"" + itemId
                + "\" (have " + std::to_string(currentQty) + ", need "
                + std::to_string(amount) + ")"};
        }
        existing->quantity -= amount;
        if (existing->quantity == 0) {
            ctx.items->erase(existing);
        }
        return {true, ""};
    }

    if (type == "giveArtifact") {
        if (ctx.artifacts == nullptr || ctx.artifactCatalog == nullptr)
            return {false, "giveArtifact: inventory context unavailable"};
        const std::string artifactId = args.value("artifactId", "");
        if (artifactId.empty())
            return {false, "giveArtifact: missing required arg 'artifactId'"};
        if (!args.contains("amount") || !args["amount"].is_number_integer())
            return {false, "giveArtifact: missing required arg 'amount' (integer)"};
        const int amount = args["amount"].get<int>();
        if (amount <= 0)
            return {false, "giveArtifact: 'amount' must be a positive integer"};

        bool inCatalog = false;
        for (const auto& d : *ctx.artifactCatalog) {
            if (d.id == artifactId) { inCatalog = true; break; }
        }
        if (!inCatalog)
            return {false, "giveArtifact: unknown artifact id \"" + artifactId + "\""};

        // Cap per docs §22: artifacts stack up to 999 copies. Overflow is a
        // hard failure (consistent with giveItem) rather than a silent clamp.
        auto existing = std::find_if(ctx.artifacts->begin(), ctx.artifacts->end(),
            [&](const gameplay::ArtifactStackState& s) { return s.artifactId == artifactId; });
        const int currentQty = (existing != ctx.artifacts->end()) ? existing->quantity : 0;
        constexpr int kArtifactStackCap = 999;
        if (currentQty + amount > kArtifactStackCap) {
            return {false, "giveArtifact: \"" + artifactId + "\" would exceed stack cap "
                + std::to_string(kArtifactStackCap) + " (have " + std::to_string(currentQty)
                + ", trying to add " + std::to_string(amount) + ")"};
        }
        if (existing == ctx.artifacts->end()) {
            ctx.artifacts->push_back({artifactId, amount});
        } else {
            existing->quantity += amount;
        }
        return {true, ""};
    }

    if (type == "takeArtifact") {
        if (ctx.artifacts == nullptr || ctx.artifactCatalog == nullptr)
            return {false, "takeArtifact: inventory context unavailable"};
        const std::string artifactId = args.value("artifactId", "");
        if (artifactId.empty())
            return {false, "takeArtifact: missing required arg 'artifactId'"};
        if (!args.contains("amount") || !args["amount"].is_number_integer())
            return {false, "takeArtifact: missing required arg 'amount' (integer)"};
        const int amount = args["amount"].get<int>();
        if (amount <= 0)
            return {false, "takeArtifact: 'amount' must be a positive integer"};

        bool inCatalog = false;
        for (const auto& d : *ctx.artifactCatalog) {
            if (d.id == artifactId) { inCatalog = true; break; }
        }
        if (!inCatalog)
            return {false, "takeArtifact: unknown artifact id \"" + artifactId + "\""};

        // M13-b contract: takeArtifact only removes from the unequipped
        // inventory and never auto-unequips. Equipped copies are invisible to
        // this action.
        auto existing = std::find_if(ctx.artifacts->begin(), ctx.artifacts->end(),
            [&](const gameplay::ArtifactStackState& s) { return s.artifactId == artifactId; });
        const int currentQty = (existing != ctx.artifacts->end()) ? existing->quantity : 0;
        if (currentQty < amount) {
            return {false, "takeArtifact: insufficient unequipped copies of \"" + artifactId
                + "\" (have " + std::to_string(currentQty) + ", need "
                + std::to_string(amount) + "); equipped copies are not auto-unequipped"};
        }
        existing->quantity -= amount;
        if (existing->quantity == 0) {
            ctx.artifacts->erase(existing);
        }
        return {true, ""};
    }
    // ----- end M13-b inventory actions -----------------------------------

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

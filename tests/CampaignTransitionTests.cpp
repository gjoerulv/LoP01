#include <catch2/catch_test_macros.hpp>

#include <algorithm>

#include "data/definitions/CampaignDefinition.h"
#include "data/definitions/ScenarioDefinition.h"
#include "data/definitions/UnitDefinition.h"
#include "gameplay/GameSession.h"
#include "gameplay/events/EventDefinition.h"

using namespace gameplay;

namespace {

data::UnitDefinition MakeUnit(const std::string& id, int agility, bool player,
                              data::UnitDefinitionCategory category) {
    data::UnitDefinition u;
    u.id = id;
    u.name = id;
    u.category = category;
    u.isPlayerCharacter = player;
    u.stats.agility = agility;
    return u;
}

events::EventCondition AlwaysLeaf() {
    events::EventCondition c;
    c.kind = events::EventConditionKind::Leaf;
    c.leafType = "always";
    return c;
}

data::ScenarioDefinition MakeScenario(const std::string& id, const std::string& region,
                                      const std::string& node, int gold,
                                      bool inlineVictory) {
    data::ScenarioDefinition s;
    s.id = id;
    s.startRegionId = region;
    s.startNodeId = node;
    s.startGold = gold;
    if (inlineVictory) {
        s.hasInlineOutcome = true;
        s.victoryConditions = {AlwaysLeaf()};
    }
    return s;
}

// Campaign s1 -> s2 with one carry rule. carryAll toggles every category on.
data::CampaignDefinition MakeCampaign(const data::CarryOverRule& rule) {
    data::CampaignDefinition c;
    c.id = "camp";
    c.startScenarioId = "s1";
    c.campaignFlags = {"flag_campaign"};
    c.carryOverRules = {rule};
    c.scenarios = {
        {"s1", {"s2"}, rule.id},
        {"s2", {}, ""},
    };
    return c;
}

// Wires a session with a hero (player, agility 10), ally (hero, agility 1),
// grunt (generic, agility 5); hero + ally active, grunt reserve.
GameSession MakeWiredSession(const data::CampaignDefinition& campaign,
                             bool inlineVictoryS1 = true,
                             bool inlineVictoryS2 = true) {
    GameSession session;
    session.SetUnitCatalog({
        MakeUnit("hero", 10, true, data::UnitDefinitionCategory::Hero),
        MakeUnit("ally", 1, false, data::UnitDefinitionCategory::Hero),
        MakeUnit("grunt", 5, false, data::UnitDefinitionCategory::Generic),
    });
    session.SetLeaderCapableUnitIds({"hero", "ally"});
    REQUIRE(session.AddOwnedUnit("hero", 1));
    REQUIRE(session.TryAddUnitToActiveParty("hero"));
    REQUIRE(session.AddOwnedUnit("ally", 1));
    REQUIRE(session.TryAddUnitToActiveParty("ally"));
    REQUIRE(session.AddOwnedUnit("grunt", 1));   // stays in reserve

    session.SetScenarioCatalog({
        MakeScenario("s1", "alpha", "n1", 500, inlineVictoryS1),
        MakeScenario("s2", "beta", "n2", 777, inlineVictoryS2),
    });
    session.SetCampaignCatalog({campaign});
    return session;
}

bool RosterHasUnit(const GameSession& s, const std::string& unitId) {
    const auto& stacks = s.RosterStacks();
    return std::any_of(stacks.begin(), stacks.end(),
        [&](const RosterStackState& st) { return st.unitId == unitId && st.quantity > 0; });
}

} // namespace

TEST_CASE("Campaign transition: victory advances with full carry-over; energy reflects carried roster") {
    data::CarryOverRule rule;
    rule.id = "carry";
    rule.carryGold = true;
    rule.carryRoster = true;
    rule.carryItems = true;
    rule.carryArtifacts = true;
    GameSession session = MakeWiredSession(MakeCampaign(rule));

    session.StartCampaign("camp");
    REQUIRE(session.CurrentScenarioId() == "s1");
    REQUIRE(session.GetCampaignState() == CampaignState::InProgress);
    // Initial energy reflects the baseline roster (lowest agility 1 -> 1100).
    REQUIRE(session.MaxEnergy() == 1100);

    // Latch victory (inline always-condition) and advance.
    session.CheckAndLatchOutcome();
    REQUIRE(session.IsScenarioEnded());
    session.AdvanceCampaignOnVictory();

    REQUIRE(session.CurrentScenarioId() == "s2");
    REQUIRE(session.Snapshot().regionId == "beta");
    REQUIRE(session.Snapshot().destinationId == "n2");
    REQUIRE(session.GetCampaignState() == CampaignState::InProgress);
    // s1 recorded as completed exactly once.
    REQUIRE(session.CompletedScenarioIds().size() == 1);
    REQUIRE(session.CompletedScenarioIds().front() == "s1");
    // Carried roster present; gold carried (500, unchanged).
    REQUIRE(RosterHasUnit(session, "hero"));
    REQUIRE(RosterHasUnit(session, "ally"));
    REQUIRE(RosterHasUnit(session, "grunt"));
    REQUIRE(session.Snapshot().gold == 500);
    // Energy recomputed AFTER carry: ally (agility 1) carried -> still 1100,
    // not the 1000 floor that a pre-carry empty roster would yield.
    REQUIRE(session.MaxEnergy() == 1100);
    // Latch cleared for the new scenario.
    REQUIRE_FALSE(session.IsScenarioEnded());
}

TEST_CASE("Campaign transition: carryRoster=false keeps only player hero; carryGold=false uses next startGold") {
    data::CarryOverRule rule;
    rule.id = "carry";
    rule.carryGold = false;
    rule.carryRoster = false;
    GameSession session = MakeWiredSession(MakeCampaign(rule));

    session.StartCampaign("camp");
    session.CheckAndLatchOutcome();
    session.AdvanceCampaignOnVictory();

    REQUIRE(session.CurrentScenarioId() == "s2");
    REQUIRE(RosterHasUnit(session, "hero"));        // player hero always retained
    REQUIRE_FALSE(RosterHasUnit(session, "ally"));
    REQUIRE_FALSE(RosterHasUnit(session, "grunt"));
    // Gold reset to s2 startGold (777).
    REQUIRE(session.Snapshot().gold == 777);
    // Energy recomputed after carry: only hero (agility 10) -> 2000. Proves the
    // recompute saw the filtered roster, not the pre-transition one.
    REQUIRE(session.MaxEnergy() == 2000);
}

TEST_CASE("Campaign transition: defeat ends the campaign run (Failed), no scenario change") {
    data::CarryOverRule rule;
    rule.id = "carry";
    GameSession session = MakeWiredSession(MakeCampaign(rule));
    session.StartCampaign("camp");

    // Force a defeat by giving s1 an inline always-defeat after start.
    // Re-build with an inline defeat scenario instead.
    data::ScenarioDefinition s1;
    s1.id = "s1"; s1.startRegionId = "alpha"; s1.startNodeId = "n1";
    s1.hasInlineOutcome = true; s1.defeatConditions = {AlwaysLeaf()};
    data::ScenarioDefinition s2 = MakeScenario("s2", "beta", "n2", 777, true);
    session.SetScenarioCatalog({s1, s2});
    session.StartCampaign("camp");   // re-enter s1 with the defeat definition active

    session.CheckAndLatchOutcome();
    REQUIRE(session.IsScenarioEnded());
    session.ResolveCampaignAfterOutcome();

    REQUIRE(session.GetCampaignState() == CampaignState::Failed);
    REQUIRE(session.CurrentScenarioId() == "s1");   // no advance
}

TEST_CASE("Campaign transition: winning the final scenario marks the run Completed") {
    data::CarryOverRule rule;
    rule.id = "carry";
    GameSession session = MakeWiredSession(MakeCampaign(rule));
    session.StartCampaign("camp");

    // Win s1 -> advance to s2.
    session.CheckAndLatchOutcome();
    session.AdvanceCampaignOnVictory();
    REQUIRE(session.CurrentScenarioId() == "s2");

    // Win s2 -> no next scenario -> Completed.
    session.CheckAndLatchOutcome();
    session.AdvanceCampaignOnVictory();
    REQUIRE(session.GetCampaignState() == CampaignState::Completed);
    REQUIRE(session.CurrentScenarioId() == "s2");
}

TEST_CASE("Campaign transition: enemy teams reset to the authored seed on transition") {
    data::CarryOverRule rule;
    rule.id = "carry";
    GameSession session = MakeWiredSession(MakeCampaign(rule));
    // Seed a non-hostile team (player color "Green" => not hostile, default
    // victory still fires). Mutate it during s1, then advance.
    EnemyTeamState team;
    team.teamColor = "Green";
    team.nodeId = "seed_node";
    team.active = true;
    session.SetEnemyTeams({team});

    session.StartCampaign("camp");
    session.ClearEnemyTeamByColor("Green");   // deactivates the team
    REQUIRE_FALSE(session.EnemyTeams().front().active);

    session.CheckAndLatchOutcome();
    session.AdvanceCampaignOnVictory();

    // Reset to seed: the team is active again at its seeded node.
    REQUIRE(session.EnemyTeams().size() == 1);
    REQUIRE(session.EnemyTeams().front().active);
    REQUIRE(session.EnemyTeams().front().nodeId == "seed_node");
}

TEST_CASE("Campaign transition: declared campaign flags persist; outcome falls back to global when no inline") {
    data::CarryOverRule rule;
    rule.id = "carry";
    // s1 has inline victory; s2 has NO inline outcome (fallback case).
    GameSession session = MakeWiredSession(MakeCampaign(rule),
                                           /*inlineVictoryS1=*/true,
                                           /*inlineVictoryS2=*/false);

    // Global fallback outcome with a distinctive condition.
    data::ScenarioOutcomeDefinition global;
    events::EventCondition globalLeaf;
    globalLeaf.kind = events::EventConditionKind::Leaf;
    globalLeaf.leafType = "storyFlagSet";
    globalLeaf.leafArgs = {{"flag", "global_win"}};
    global.victoryConditions = {globalLeaf};
    session.SetScenarioOutcomeDefinition(global);

    // An event sets both a campaign flag and a scenario-only flag at start of day.
    events::EventDefinition ev;
    ev.id = "ev_flags";
    ev.trigger.type = events::EventTriggerType::StartOfDay;
    events::EventAction a1; a1.type = "setStoryFlag"; a1.args = {{"flag", "flag_campaign"}};
    events::EventAction a2; a2.type = "setStoryFlag"; a2.args = {{"flag", "flag_scenario"}};
    ev.actions = {a1, a2};
    session.InitializeEventDefinitions({ev});

    session.StartCampaign("camp");
    // s1 active outcome is the inline always-victory (NOT the global fallback).
    REQUIRE(session.ActiveScenarioOutcomeDefinition().victoryConditions.size() == 1);
    REQUIRE(session.ActiveScenarioOutcomeDefinition().victoryConditions.front().leafType == "always");

    (void)session.NotifyStartOfDay();   // sets flags, then latches victory
    REQUIRE(session.IsScenarioEnded());
    session.AdvanceCampaignOnVictory();

    // Campaign flag promoted + persisted; scenario flag not carried.
    REQUIRE(session.CampaignFlags().count("flag_campaign") == 1);
    REQUIRE(session.CampaignFlags().count("flag_scenario") == 0);
    // s2 has no inline outcome => active definition is the global fallback.
    REQUIRE(session.ActiveScenarioOutcomeDefinition().victoryConditions.size() == 1);
    REQUIRE(session.ActiveScenarioOutcomeDefinition().victoryConditions.front().leafType == "storyFlagSet");
}

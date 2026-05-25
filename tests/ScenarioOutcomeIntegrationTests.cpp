#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "core/SaveGame.h"
#include "data/definitions/ScenarioOutcomeDefinition.h"
#include "gameplay/EnemyTeamState.h"
#include "gameplay/GameSession.h"
#include "gameplay/events/EventDefinition.h"
#include "gameplay/events/EventParser.h"
#include "gameplay/scenario/ScenarioOutcomeRules.h"

using namespace gameplay;
using namespace gameplay::events;

namespace {

EventAction MakeAction(const nlohmann::json& j) {
    EventAction a;
    a.type = j.value("type", "");
    a.args = j;
    return a;
}

EventDefinition MakeRegionNodeEntryEvent(
    const std::string& id,
    const std::string& nodeId,
    std::vector<EventAction> actions)
{
    EventDefinition def;
    def.id = id;
    def.trigger.type = EventTriggerType::RegionNodeEntry;
    def.trigger.targetId = nodeId;
    def.actions = std::move(actions);
    def.repeat.mode = "once";
    return def;
}

EnemyTeamState MakeHostile(const std::string& color, const std::string& nodeId = "node_a") {
    EnemyTeamState t;
    t.teamColor = color;
    t.nodeId = nodeId;
    t.active = true;
    return t;
}

EventCondition FlagSet(const std::string& flag) {
    return ParseCondition({{"type", "storyFlagSet"}, {"flag", flag}});
}

} // namespace

// ---------------------------------------------------------------------------
// Check #1 — regionNodeEntry event removes last hostile team -> default victory
// latches before enemy phase
// ---------------------------------------------------------------------------

TEST_CASE("ScenarioOutcome integration - regionNodeEntry removeTeam latches default victory") {
    GameSession session;
    session.SetPlayerColor("Green");
    session.SetEnemyTeams({MakeHostile("Red")});

    // Authored event on arrival at "node_a" removes the only hostile team.
    session.InitializeEventDefinitions({
        MakeRegionNodeEntryEvent("evt_clear_red", "node_a", {
            MakeAction({{"type", "removeTeam"}, {"teamColor", "Red"}})
        })
    });

    REQUIRE_FALSE(session.IsScenarioEnded());

    const auto results = session.NotifyRegionNodeEntry("node_a");
    REQUIRE(results.size() == 1);
    REQUIRE(results[0].success);

    // Outcome must be latched as Victory by FireMatchingEvents' boundary call.
    REQUIRE(session.IsScenarioEnded());
    REQUIRE(session.Outcome().has_value());
    REQUIRE(session.Outcome()->state == scenario::ScenarioOutcomeState::Victory);
    // Default victory => no matched condition index
    REQUIRE_FALSE(session.Outcome()->matchedConditionIndex.has_value());
}

// ---------------------------------------------------------------------------
// Check #1 — regionNodeEntry sets story flag matching authored victory
// ---------------------------------------------------------------------------

TEST_CASE("ScenarioOutcome integration - regionNodeEntry setStoryFlag matches authored victory") {
    GameSession session;
    session.SetPlayerColor("Green");
    // Keep a hostile team alive — proves authored victory bypasses default rule.
    session.SetEnemyTeams({MakeHostile("Red")});

    data::ScenarioOutcomeDefinition def;
    def.victoryConditions.push_back(FlagSet("ashvale_cleansed"));
    session.SetScenarioOutcomeDefinition(def);

    session.InitializeEventDefinitions({
        MakeRegionNodeEntryEvent("evt_cleanse", "node_shrine", {
            MakeAction({{"type", "setStoryFlag"}, {"flag", "ashvale_cleansed"}})
        })
    });

    REQUIRE_FALSE(session.IsScenarioEnded());

    static_cast<void>(session.NotifyRegionNodeEntry("node_shrine"));

    REQUIRE(session.IsScenarioEnded());
    REQUIRE(session.Outcome()->state == scenario::ScenarioOutcomeState::Victory);
    REQUIRE(session.Outcome()->matchedConditionIndex.has_value());
    REQUIRE(*session.Outcome()->matchedConditionIndex == 0);
}

// ---------------------------------------------------------------------------
// Check #2 — boundary invocation: ProcessEnemyPhase latches at end
// ---------------------------------------------------------------------------

TEST_CASE("ScenarioOutcome integration - ProcessEnemyPhase invokes CheckAndLatchOutcome at boundary") {
    // Current ProcessEnemyPhase only does patrol movement; no current mechanic
    // satisfies victory through enemy phase alone. Verify the boundary call
    // itself by arranging session state that satisfies default victory (no
    // hostile teams) and asserting ProcessEnemyPhase's end-of-call latch fires.
    GameSession session;
    session.SetPlayerColor("Green");
    session.SetEnemyTeams({}); // no hostile teams installed

    REQUIRE_FALSE(session.IsScenarioEnded());

    const std::vector<data::RegionLinkDefinition> noLinks;
    static_cast<void>(session.ProcessEnemyPhase(noLinks));

    REQUIRE(session.IsScenarioEnded());
    REQUIRE(session.Outcome()->state == scenario::ScenarioOutcomeState::Victory);
}

// ---------------------------------------------------------------------------
// Hostile-contact victory: ClearEnemyTeamByColor latches default victory
// when the cleared team was the last hostile.
// ---------------------------------------------------------------------------

TEST_CASE("ScenarioOutcome integration - ClearEnemyTeamByColor on last hostile latches default victory") {
    GameSession session;
    session.SetPlayerColor("Green");
    session.SetEnemyTeams({MakeHostile("Red"), MakeHostile("Blue")});

    // Clearing Red while Blue remains: outcome still Ongoing.
    session.ClearEnemyTeamByColor("Red");
    REQUIRE_FALSE(session.IsScenarioEnded());

    // Clearing Blue (the last hostile): outcome must latch Victory.
    session.ClearEnemyTeamByColor("Blue");
    REQUIRE(session.IsScenarioEnded());
    REQUIRE(session.Outcome()->state == scenario::ScenarioOutcomeState::Victory);
}

// ---------------------------------------------------------------------------
// Latch is persistent and idempotent.
// ---------------------------------------------------------------------------

TEST_CASE("ScenarioOutcome integration - latched outcome is not re-evaluated") {
    GameSession session;
    session.SetPlayerColor("Green");

    // First latch: defeat via authored flag.
    data::ScenarioOutcomeDefinition def;
    def.defeatConditions.push_back(FlagSet("ashvale_lost"));
    session.SetScenarioOutcomeDefinition(def);

    session.InitializeEventDefinitions({
        MakeRegionNodeEntryEvent("evt_lose", "node_trap", {
            MakeAction({{"type", "setStoryFlag"}, {"flag", "ashvale_lost"}})
        })
    });

    static_cast<void>(session.NotifyRegionNodeEntry("node_trap"));
    REQUIRE(session.Outcome()->state == scenario::ScenarioOutcomeState::Defeat);

    // Even if we now arrange a state that would naturally yield Victory
    // (no hostile teams, no defeat condition met), the latch must hold.
    session.SetEnemyTeams({});
    session.CheckAndLatchOutcome();
    REQUIRE(session.Outcome()->state == scenario::ScenarioOutcomeState::Defeat);
}

// ---------------------------------------------------------------------------
// Save/load round-trip preserves latched outcome and remains non-resumable.
// ---------------------------------------------------------------------------

TEST_CASE("ScenarioOutcome integration - latched outcome survives save/load round-trip") {
    GameSession source;
    source.SetPlayerColor("Green");
    source.SetEnemyTeams({}); // empty -> default victory
    source.CheckAndLatchOutcome();
    REQUIRE(source.IsScenarioEnded());

    const auto saveData = source.ToSaveData();
    REQUIRE(saveData.scenarioOutcomeState == "victory");

    GameSession restored;
    restored.ApplySaveData(saveData);

    REQUIRE(restored.IsScenarioEnded());
    REQUIRE(restored.Outcome().has_value());
    REQUIRE(restored.Outcome()->state == scenario::ScenarioOutcomeState::Victory);
}

TEST_CASE("ScenarioOutcome integration - loaded latched Defeat is not re-evaluated to Victory") {
    GameSession source;
    source.SetPlayerColor("Green");

    data::ScenarioOutcomeDefinition def;
    def.defeatConditions.push_back(FlagSet("ashvale_lost"));
    source.SetScenarioOutcomeDefinition(def);

    source.InitializeEventDefinitions({
        MakeRegionNodeEntryEvent("evt_lose", "node_trap", {
            MakeAction({{"type", "setStoryFlag"}, {"flag", "ashvale_lost"}})
        })
    });
    static_cast<void>(source.NotifyRegionNodeEntry("node_trap"));
    REQUIRE(source.Outcome()->state == scenario::ScenarioOutcomeState::Defeat);

    const auto saveData = source.ToSaveData();
    REQUIRE(saveData.scenarioOutcomeState == "defeat");

    GameSession restored;
    // Restored session has empty enemy roster — default victory would normally fire.
    restored.SetPlayerColor("Green");
    restored.SetEnemyTeams({});
    restored.ApplySaveData(saveData);

    // Defeat latch survives; calling CheckAndLatchOutcome must not flip it.
    restored.CheckAndLatchOutcome();
    REQUIRE(restored.Outcome()->state == scenario::ScenarioOutcomeState::Defeat);
}

// ---------------------------------------------------------------------------
// Save/load: legacy save without scenario_outcome_* fields loads as Ongoing.
// ---------------------------------------------------------------------------

TEST_CASE("ScenarioOutcome integration - legacy save without outcome fields restores as Ongoing") {
    const std::filesystem::path savePath = "saves/scenario_outcome_legacy_test/legacy.json";
    std::filesystem::create_directories(savePath.parent_path());

    std::ofstream out(savePath, std::ios::trunc);
    out << R"({
  "schema_version": 5,
  "day": 1,
  "minutes_into_slice_day": 0,
  "gold": 100,
  "mode": "overworld_mode",
  "region_id": "r1",
  "destination_id": "d1",
  "roster_stacks": [],
  "active_slot_stack_ids": ["","","","",""],
  "reserve_slot_stack_ids": ["","","","","","","",""]
})";
    out.close();

    core::SaveGameRepository repo;
    const auto loaded = repo.LoadFromFile(savePath.string());
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->scenarioOutcomeState.empty());
    REQUIRE(loaded->scenarioOutcomeMatchedConditionIndex == -1);
    REQUIRE(loaded->scenarioOutcomeReason.empty());

    GameSession restored;
    restored.ApplySaveData(*loaded);
    REQUIRE_FALSE(restored.IsScenarioEnded());

    std::filesystem::remove(savePath);
}

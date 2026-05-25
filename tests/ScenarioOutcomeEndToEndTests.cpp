#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <string>

#include "data/ContentRepository.h"
#include "gameplay/GameSession.h"
#include "gameplay/scenario/ScenarioOutcomeRules.h"

// LOP01_PROJECT_ROOT is set by CMakeLists.txt to the absolute repo root so that
// these end-to-end tests can load the actual content/ directory used by the
// running app. If absent, the tests are skipped — they validate authored
// content, not engine logic.
#ifndef LOP01_PROJECT_ROOT
#define LOP01_PROJECT_ROOT "."
#endif

namespace {

std::filesystem::path RealContentDir() {
    return std::filesystem::path(LOP01_PROJECT_ROOT) / "content";
}

bool HasErrorMessage(const std::vector<ValidationMessage>& msgs) {
    return std::any_of(msgs.begin(), msgs.end(),
        [](const ValidationMessage& m) { return m.severity == Severity::Error; });
}

// Build a session wired exactly like App::App() does for the running game,
// but loading from the real content directory. Captures the live integration
// path the player exercises in-game.
gameplay::GameSession MakeRealContentSession(data::ContentRepository& repo) {
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));
    REQUIRE_FALSE(HasErrorMessage(repo.ValidationMessages()));

    gameplay::GameSession session;
    session.InitializeQuestState(repo.QuestDefinitions());
    session.InitializeEventDefinitions(repo.EventDefinitions());
    session.SetScenarioOutcomeDefinition(repo.ScenarioOutcome());
    session.SetPlayerColor("Green");
    return session;
}

} // namespace

TEST_CASE("End-to-end - real content/ loads with zero validation errors") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));
    REQUIRE_FALSE(HasErrorMessage(repo.ValidationMessages()));
}

TEST_CASE("End-to-end - real content authors expected scenario outcome conditions") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));

    const auto& outcome = repo.ScenarioOutcome();
    REQUIRE(outcome.victoryConditions.size() == 1);
    REQUIRE(outcome.victoryConditions[0].leafType == "storyFlagSet");
    REQUIRE(outcome.victoryConditions[0].leafArgs.value("flag", std::string{}) == "ashvale_cleansed");

    REQUIRE(outcome.defeatConditions.size() == 1);
    REQUIRE(outcome.defeatConditions[0].leafType == "storyFlagSet");
    REQUIRE(outcome.defeatConditions[0].leafArgs.value("flag", std::string{}) == "ashvale_lost");
}

TEST_CASE("End-to-end - real content authors both demo regionNodeEntry events") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));

    const auto& events = repo.EventDefinitions();
    const bool hasVictoryTrigger = std::any_of(events.begin(), events.end(),
        [](const auto& e) {
            return e.trigger.type == gameplay::events::EventTriggerType::RegionNodeEntry
                && e.trigger.targetId == "sunken_ruin";
        });
    const bool hasDefeatTrigger = std::any_of(events.begin(), events.end(),
        [](const auto& e) {
            return e.trigger.type == gameplay::events::EventTriggerType::RegionNodeEntry
                && e.trigger.targetId == "clocktower_square";
        });
    REQUIRE(hasVictoryTrigger);
    REQUIRE(hasDefeatTrigger);
}

TEST_CASE("End-to-end - day-1 session with no hostile teams stays Ongoing (authored conditions disable default victory)") {
    data::ContentRepository repo;
    auto session = MakeRealContentSession(repo);

    // No travel, no events fired yet. With authored victoryConditions present,
    // default victory must NOT auto-trigger even with an empty enemy roster.
    session.CheckAndLatchOutcome();
    REQUIRE_FALSE(session.IsScenarioEnded());
}

TEST_CASE("End-to-end - visiting sunken_ruin latches Victory via authored condition") {
    data::ContentRepository repo;
    auto session = MakeRealContentSession(repo);

    static_cast<void>(session.NotifyRegionNodeEntry("sunken_ruin"));

    REQUIRE(session.IsScenarioEnded());
    REQUIRE(session.Outcome()->state == gameplay::scenario::ScenarioOutcomeState::Victory);
    REQUIRE(session.Outcome()->matchedConditionIndex.has_value());
    REQUIRE(*session.Outcome()->matchedConditionIndex == 0);
}

TEST_CASE("End-to-end - visiting clocktower_square latches Defeat via authored condition") {
    data::ContentRepository repo;
    auto session = MakeRealContentSession(repo);

    static_cast<void>(session.NotifyRegionNodeEntry("clocktower_square"));

    REQUIRE(session.IsScenarioEnded());
    REQUIRE(session.Outcome()->state == gameplay::scenario::ScenarioOutcomeState::Defeat);
    REQUIRE(session.Outcome()->matchedConditionIndex.has_value());
    REQUIRE(*session.Outcome()->matchedConditionIndex == 0);
}

TEST_CASE("End-to-end - visiting an unrelated node does not latch any outcome") {
    data::ContentRepository repo;
    auto session = MakeRealContentSession(repo);

    static_cast<void>(session.NotifyRegionNodeEntry("town_center"));

    REQUIRE_FALSE(session.IsScenarioEnded());
}

TEST_CASE("End-to-end - latched Victory survives save/load through real content session") {
    data::ContentRepository repo;
    auto session = MakeRealContentSession(repo);
    static_cast<void>(session.NotifyRegionNodeEntry("sunken_ruin"));
    REQUIRE(session.IsScenarioEnded());

    const auto saveData = session.ToSaveData();
    REQUIRE(saveData.scenarioOutcomeState == "victory");

    gameplay::GameSession restored;
    restored.SetScenarioOutcomeDefinition(repo.ScenarioOutcome());
    restored.SetPlayerColor("Green");
    restored.ApplySaveData(saveData);

    REQUIRE(restored.IsScenarioEnded());
    REQUIRE(restored.Outcome()->state == gameplay::scenario::ScenarioOutcomeState::Victory);
}

#include <catch2/catch_test_macros.hpp>

#include "gameplay/overworld/OverworldDeadEndRules.h"

TEST_CASE("Dead-end rule applies wake penalty when already past cutoff") {
    const auto decision = gameplay::overworld::EvaluateDeadEndWakePenalty(
        1201,
        false,
        true,
        true,
        false,
        1200);

    REQUIRE(decision.shouldApplyWakePenalty);
    REQUIRE(decision.reason == gameplay::overworld::DeadEndWakePenaltyReason::AlreadyPastCutoff);
}

TEST_CASE("Dead-end rule does not apply wake penalty when legal travel exists") {
    const auto decision = gameplay::overworld::EvaluateDeadEndWakePenalty(
        1190,
        true,
        true,
        false,
        false,
        1200);

    REQUIRE_FALSE(decision.shouldApplyWakePenalty);
    REQUIRE(decision.reason == gameplay::overworld::DeadEndWakePenaltyReason::None);
}

TEST_CASE("Dead-end rule allows usable local action before auto penalty") {
    const auto decision = gameplay::overworld::EvaluateDeadEndWakePenalty(
        1190,
        false,
        true,
        true,
        false,
        1200);

    REQUIRE_FALSE(decision.shouldApplyWakePenalty);
    REQUIRE(decision.reason == gameplay::overworld::DeadEndWakePenaltyReason::None);
}

TEST_CASE("Dead-end rule applies wake penalty when cutoff blocks remaining progress") {
    const auto decision = gameplay::overworld::EvaluateDeadEndWakePenalty(
        1190,
        false,
        true,
        false,
        false,
        1200);

    REQUIRE(decision.shouldApplyWakePenalty);
    REQUIRE(decision.reason == gameplay::overworld::DeadEndWakePenaltyReason::CutoffBlocksRemainingProgress);
}

TEST_CASE("Dead-end rule applies wake penalty after declining usable local action") {
    const auto decision = gameplay::overworld::EvaluateDeadEndWakePenalty(
        1190,
        false,
        true,
        true,
        true,
        1200);

    REQUIRE(decision.shouldApplyWakePenalty);
    REQUIRE(decision.reason == gameplay::overworld::DeadEndWakePenaltyReason::CutoffBlocksProgressAfterDecliningLocalAction);
}

TEST_CASE("Dead-end rule does not auto apply for ordinary no-route dead end") {
    const auto decision = gameplay::overworld::EvaluateDeadEndWakePenalty(
        1190,
        false,
        false,
        false,
        false,
        1200);

    REQUIRE_FALSE(decision.shouldApplyWakePenalty);
    REQUIRE(decision.reason == gameplay::overworld::DeadEndWakePenaltyReason::None);
}

TEST_CASE("Dead-end rule does not auto apply for declined local action when not cutoff-caused") {
    const auto decision = gameplay::overworld::EvaluateDeadEndWakePenalty(
        1190,
        false,
        false,
        true,
        true,
        1200);

    REQUIRE_FALSE(decision.shouldApplyWakePenalty);
    REQUIRE(decision.reason == gameplay::overworld::DeadEndWakePenaltyReason::None);
}

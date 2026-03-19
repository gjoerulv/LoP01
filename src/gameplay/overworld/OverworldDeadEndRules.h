#pragma once

namespace gameplay::overworld {

enum class DeadEndWakePenaltyReason {
    None,
    AlreadyPastCutoff,
    CutoffBlocksRemainingProgress,
    CutoffBlocksProgressAfterDecliningLocalAction
};

struct DeadEndWakePenaltyDecision {
    bool shouldApplyWakePenalty = false;
    DeadEndWakePenaltyReason reason = DeadEndWakePenaltyReason::None;
};

[[nodiscard]] DeadEndWakePenaltyDecision EvaluateDeadEndWakePenalty(
    int minutesIntoSliceDay,
    bool hasLegalTravelNow,
    bool hasReachableTravelIgnoringCutoff,
    bool hasUsableLocalAction,
    bool declinedUsableLocalAction,
    int cutoffMinutesIntoSliceDay);

} // namespace gameplay::overworld

#include "gameplay/overworld/OverworldDeadEndRules.h"

namespace gameplay::overworld {

DeadEndWakePenaltyDecision EvaluateDeadEndWakePenalty(
    const int minutesIntoSliceDay,
    const bool hasLegalTravelNow,
    const bool hasReachableTravelIgnoringCutoff,
    const bool hasUsableLocalAction,
    const bool declinedUsableLocalAction,
    const int cutoffMinutesIntoSliceDay) {
    if (minutesIntoSliceDay > cutoffMinutesIntoSliceDay) {
        return DeadEndWakePenaltyDecision{ true, DeadEndWakePenaltyReason::AlreadyPastCutoff };
    }

    if (hasLegalTravelNow) {
        return DeadEndWakePenaltyDecision{};
    }

    if (hasUsableLocalAction && !declinedUsableLocalAction) {
        return DeadEndWakePenaltyDecision{};
    }

    if (!hasLegalTravelNow && hasReachableTravelIgnoringCutoff && !hasUsableLocalAction) {
        return DeadEndWakePenaltyDecision{ true, DeadEndWakePenaltyReason::CutoffBlocksRemainingProgress };
    }

    if (declinedUsableLocalAction && hasReachableTravelIgnoringCutoff) {
        return DeadEndWakePenaltyDecision{ true, DeadEndWakePenaltyReason::CutoffBlocksProgressAfterDecliningLocalAction };
    }

    return DeadEndWakePenaltyDecision{};
}

} // namespace gameplay::overworld

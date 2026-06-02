#pragma once

#include <vector>

#include "gameplay/ResourceState.h"

namespace gameplay::economy {

// One resolved output line for a mine/resource service: an amount of a single
// resource produced per day. Gold is a valid resource here — this is pure
// output data, not a session mutation, so representing Gold output never creates
// a second gold store (the Phase 1 Gold delegation still owns the actual payout
// when wiring lands in Phase 3).
struct MineResourceOutput {
    ResourceType resource = ResourceType::Gold;
    int amount = 0;
};

// A single stationed-unit production passive applicable to one mine instance:
// an additive bonus to a specific output resource. This is a narrow Phase 2
// input struct only — there is no unit passive field or stationing schema yet;
// callers (and tests) construct these directly.
struct MineProductionPassive {
    ResourceType resource = ResourceType::Gold;
    int amount = 0;
};

// Pure daily-output calculation for one mine instance.
//
// Rules (docs/content_scope_v1.md §4, docs/core_loop_rules.md §18,
// README_DECISIONS #70a):
//   * base outputs are copied into the result, preserving order;
//   * modifiers are resolved independently per output resource;
//   * only the single strongest applicable passive applies per resource
//     (strongest-only);
//   * ties do not stack (+2 and +2 still gives only +2);
//   * a passive for a resource the mine does not produce creates NO new output;
//   * the resulting amount is floored at zero.
//
// Deterministic and allocation-light: one pass over base outputs, each doing a
// linear scan of the (small) passive list. No global state, no I/O.
[[nodiscard]] std::vector<MineResourceOutput> ComputeMineDailyOutput(
    const std::vector<MineResourceOutput>& baseOutputs,
    const std::vector<MineProductionPassive>& applicableStationedPassives);

} // namespace gameplay::economy

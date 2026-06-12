#pragma once

#include "data/definitions/LocationServiceDefinition.h"
#include "data/definitions/UnitDefinition.h"

namespace gameplay::economy {

// Days before a Temporarily Unavailable hero becomes returnable (weekly cadence;
// the simplest final-rule-compatible return timing while no shared hero pool
// exists — see docs/game_vision.md §5).
inline constexpr int kUnavailableHeroReturnDays = 7;

// Pure gate: which service kinds participate in node-level service attacks.
// Ownable kinds (mines, traders) transfer ownership on capture; Storage joins
// them because the final rules make storage gates capturable defended assets
// (docs/core_loop_rules.md §18/§21). Rest/Shop/Recruit/Muster interactions are
// never systemically attackable.
[[nodiscard]] bool ServiceKindIsAttackable(data::LocationServiceKind kind);

// Deterministic defense power of one unit. Intentionally a narrow stand-in for
// the final full-simulation auto-resolve (deferred battle-depth scope): a
// monotonic mix of offense and durability so stronger/larger garrisons win.
[[nodiscard]] int UnitDefensePower(const data::UnitStatsDefinition& stats);

// Power of a whole placed stack: per-unit power times quantity (empty stacks
// contribute nothing).
[[nodiscard]] int StackDefensePower(const data::UnitStatsDefinition& stats, int quantity);

// Outcome rule: defenders hold the node iff they exist (power > 0) and are at
// least as strong as the attacker. The defender wins ties — attacking a held
// service must require real superiority.
[[nodiscard]] bool DefendersHoldService(int defenderPower, int attackerPower);

} // namespace gameplay::economy

#pragma once

#include "data/definitions/UnitDefinition.h"
#include "gameplay/battle/Battle.h"

// Shared authored-unit -> battle-model conversion. Centralized so every consumer
// (the interactive BattleFactory and the M33 auto-resolve) sees the SAME stat
// mapping — the auto-resolve's "battle-rule-aligned" guarantee depends on this
// being a single source of truth, not a parallel copy.
namespace gameplay::battle {

[[nodiscard]] UnitStats ToBattleStats(const data::UnitStatsDefinition& stats);
[[nodiscard]] UnitCategory ToBattleCategory(data::UnitDefinitionCategory category);
[[nodiscard]] UnitPosition ToBattlePosition(data::UnitDefinitionPosition position);
[[nodiscard]] UnitRange ToBattleRange(data::UnitDefinitionRange range);

} // namespace gameplay::battle

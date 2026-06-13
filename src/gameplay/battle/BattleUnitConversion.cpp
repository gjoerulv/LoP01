#include "gameplay/battle/BattleUnitConversion.h"

namespace gameplay::battle {

UnitCategory ToBattleCategory(const data::UnitDefinitionCategory category) {
    switch (category) {
    case data::UnitDefinitionCategory::Leader: return UnitCategory::Leader;
    case data::UnitDefinitionCategory::Hero:   return UnitCategory::Hero;
    default:                                   return UnitCategory::Generic;
    }
}

UnitPosition ToBattlePosition(const data::UnitDefinitionPosition position) {
    switch (position) {
    case data::UnitDefinitionPosition::Front:  return UnitPosition::Front;
    case data::UnitDefinitionPosition::Middle: return UnitPosition::Middle;
    case data::UnitDefinitionPosition::Back:   return UnitPosition::Back;
    default:                                   return UnitPosition::Leader;
    }
}

UnitRange ToBattleRange(const data::UnitDefinitionRange range) {
    switch (range) {
    case data::UnitDefinitionRange::Melee:  return UnitRange::Melee;
    case data::UnitDefinitionRange::Ranged: return UnitRange::Ranged;
    default:                                return UnitRange::LongRanged;
    }
}

UnitStats ToBattleStats(const data::UnitStatsDefinition& stats) {
    UnitStats result;
    result.attack = stats.attack;
    result.defense = stats.defense;
    result.magic = stats.magic;
    result.resistance = stats.resistance;
    result.minDamage = stats.minDamage;
    result.maxDamage = stats.maxDamage;
    result.maxHp = stats.maxHp;
    result.maxMp = stats.maxMp;
    result.agility = stats.agility;
    result.life = stats.life;
    result.position = ToBattlePosition(stats.position);
    result.range = ToBattleRange(stats.range);
    return result;
}

} // namespace gameplay::battle

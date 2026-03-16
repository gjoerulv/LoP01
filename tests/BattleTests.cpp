#include <catch2/catch_test_macros.hpp>

#include "gameplay/battle/Battle.h"

using gameplay::battle::BattleActionType;
using gameplay::battle::BattleState;
using gameplay::battle::BattleUnit;
using gameplay::battle::TeamSide;
using gameplay::battle::UnitCategory;
using gameplay::battle::UnitPosition;
using gameplay::battle::UnitRange;
using gameplay::battle::UnitStats;

TEST_CASE("Higher agility unit acts first in CTB") {
    BattleUnit fast;
    fast.id = "ally_fast";
    fast.name = "Fast";
    fast.side = TeamSide::Allies;
    fast.category = UnitCategory::Hero;
    fast.stats = UnitStats{3, 3, 3, 3, 5, 5, 20, 10, 12, 1, UnitPosition::Front, UnitRange::Melee};
    fast.hp = fast.stats.maxHp;
    fast.mp = fast.stats.maxMp;
    fast.life = 1;

    BattleUnit slow;
    slow.id = "enemy_slow";
    slow.name = "Slow";
    slow.side = TeamSide::Enemies;
    slow.category = UnitCategory::Hero;
    slow.stats = UnitStats{3, 3, 3, 3, 5, 5, 20, 10, 6, 1, UnitPosition::Front, UnitRange::Melee};
    slow.hp = slow.stats.maxHp;
    slow.mp = slow.stats.maxMp;
    slow.life = 1;

    BattleState battle = BattleState::CreateForTests({fast, slow}, 1);

    REQUIRE(battle.ActiveUnitIndex() >= 0);
    REQUIRE(battle.Units()[battle.ActiveUnitIndex()].id == "ally_fast");
}

TEST_CASE("Generic stack damage bleeds over and removes multiple lives") {
    BattleUnit attacker;
    attacker.id = "ally_attacker";
    attacker.name = "Attacker";
    attacker.side = TeamSide::Allies;
    attacker.category = UnitCategory::Hero;
    attacker.stats = UnitStats{0, 0, 0, 0, 30, 30, 20, 10, 10, 1, UnitPosition::Front, UnitRange::Melee};
    attacker.hp = attacker.stats.maxHp;
    attacker.mp = attacker.stats.maxMp;
    attacker.life = 1;

    BattleUnit target;
    target.id = "enemy_stack";
    target.name = "Stack";
    target.side = TeamSide::Enemies;
    target.category = UnitCategory::Generic;
    target.stats = UnitStats{0, 0, 0, 0, 1, 1, 10, 0, 8, 3, UnitPosition::Front, UnitRange::Melee};
    target.hp = target.stats.maxHp;
    target.mp = 0;
    target.life = target.stats.life;

    BattleState battle = BattleState::CreateForTests({attacker, target}, 2);

    REQUIRE(battle.ExecuteAction(BattleActionType::Attack, 1));

    const auto& units = battle.Units();
    const auto& damaged = units[1];
    REQUIRE(damaged.life == 0);
    REQUIRE(damaged.hp == 0);
    REQUIRE(damaged.ko);
    REQUIRE(battle.IsFinished());
    REQUIRE(battle.Summary().alliesWon);
}

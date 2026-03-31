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

TEST_CASE("Allied victory restores player character to 1 HP when KO") {
    BattleUnit player;
    player.id = "ally_player";
    player.name = "Player";
    player.side = TeamSide::Allies;
    player.category = UnitCategory::Hero;
    player.isPlayerCharacter = true;
    player.stats = UnitStats{2, 2, 2, 2, 3, 3, 20, 5, 4, 1, UnitPosition::Back, UnitRange::Ranged};
    player.hp = 0;
    player.mp = player.stats.maxMp;
    player.life = 1;

    BattleUnit ally;
    ally.id = "ally_finisher";
    ally.name = "Finisher";
    ally.side = TeamSide::Allies;
    ally.category = UnitCategory::Hero;
    ally.stats = UnitStats{5, 2, 2, 2, 20, 20, 20, 5, 10, 1, UnitPosition::Front, UnitRange::Melee};
    ally.hp = ally.stats.maxHp;
    ally.mp = ally.stats.maxMp;
    ally.life = 1;

    BattleUnit enemy;
    enemy.id = "enemy_target";
    enemy.name = "Target";
    enemy.side = TeamSide::Enemies;
    enemy.category = UnitCategory::Generic;
    enemy.stats = UnitStats{1, 1, 1, 1, 1, 1, 10, 0, 3, 1, UnitPosition::Front, UnitRange::Melee};
    enemy.hp = enemy.stats.maxHp;
    enemy.mp = 0;
    enemy.life = enemy.stats.life;

    BattleState battle = BattleState::CreateForTests({player, ally, enemy}, 3);

    REQUIRE(battle.ExecuteAction(BattleActionType::Attack, 2));
    REQUIRE(battle.IsFinished());
    REQUIRE(battle.Summary().alliesWon);
    REQUIRE(battle.Summary().playerSetToOneHp);
    REQUIRE(battle.Units()[0].hp == 1);
}

TEST_CASE("Assigned leader aura applies to allied units including leader") {
    BattleUnit leader;
    leader.id = "ally_leader";
    leader.name = "Leader";
    leader.side = TeamSide::Allies;
    leader.category = UnitCategory::Hero;
    leader.isAssignedLeader = true;
    leader.stats = UnitStats{10, 10, 10, 10, 10, 10, 40, 10, 1, 1, UnitPosition::Front, UnitRange::Ranged};
    leader.hp = leader.stats.maxHp;
    leader.mp = leader.stats.maxMp;
    leader.life = 1;

    BattleUnit attacker;
    attacker.id = "ally_attacker";
    attacker.name = "Attacker";
    attacker.side = TeamSide::Allies;
    attacker.category = UnitCategory::Generic;
    attacker.stats = UnitStats{5, 1, 1, 1, 10, 10, 30, 0, 10, 1, UnitPosition::Front, UnitRange::Melee};
    attacker.hp = attacker.stats.maxHp;
    attacker.mp = 0;
    attacker.life = 1;

    BattleUnit enemy;
    enemy.id = "enemy_target";
    enemy.name = "Enemy";
    enemy.side = TeamSide::Enemies;
    enemy.category = UnitCategory::Generic;
    enemy.stats = UnitStats{1, 0, 1, 1, 1, 1, 200, 0, 1, 1, UnitPosition::Front, UnitRange::Melee};
    enemy.hp = enemy.stats.maxHp;
    enemy.mp = 0;
    enemy.life = 1;

    BattleState battle = BattleState::CreateForTests({leader, attacker, enemy}, 11);
    REQUIRE(battle.ActiveUnitIndex() == 1);
    REQUIRE(battle.Units()[0].stats.position == UnitPosition::Leader);

    REQUIRE(battle.ExecuteAction(BattleActionType::Attack, 2));
    REQUIRE(battle.Units()[2].hp == 175);
}

TEST_CASE("Assigned leader aura uses the same mechanics for enemy team when present") {
    BattleUnit allyTarget;
    allyTarget.id = "ally_target";
    allyTarget.name = "Ally";
    allyTarget.side = TeamSide::Allies;
    allyTarget.category = UnitCategory::Generic;
    allyTarget.stats = UnitStats{1, 0, 1, 1, 1, 1, 200, 0, 1, 1, UnitPosition::Front, UnitRange::Melee};
    allyTarget.hp = allyTarget.stats.maxHp;
    allyTarget.mp = 0;
    allyTarget.life = 1;

    BattleUnit enemyLeader;
    enemyLeader.id = "enemy_leader";
    enemyLeader.name = "Enemy Leader";
    enemyLeader.side = TeamSide::Enemies;
    enemyLeader.category = UnitCategory::Leader;
    enemyLeader.isAssignedLeader = true;
    enemyLeader.stats = UnitStats{10, 10, 10, 10, 10, 10, 40, 10, 1, 1, UnitPosition::Front, UnitRange::Ranged};
    enemyLeader.hp = enemyLeader.stats.maxHp;
    enemyLeader.mp = enemyLeader.stats.maxMp;
    enemyLeader.life = 1;

    BattleUnit enemyAttacker;
    enemyAttacker.id = "enemy_attacker";
    enemyAttacker.name = "Enemy Attacker";
    enemyAttacker.side = TeamSide::Enemies;
    enemyAttacker.category = UnitCategory::Generic;
    enemyAttacker.stats = UnitStats{5, 1, 1, 1, 10, 10, 30, 0, 10, 1, UnitPosition::Front, UnitRange::Melee};
    enemyAttacker.hp = enemyAttacker.stats.maxHp;
    enemyAttacker.mp = 0;
    enemyAttacker.life = 1;

    BattleState battle = BattleState::CreateForTests({allyTarget, enemyLeader, enemyAttacker}, 12);
    REQUIRE(battle.ActiveUnitIndex() == 2);
    REQUIRE(battle.Units()[1].stats.position == UnitPosition::Leader);

    REQUIRE(battle.ExecuteAction(BattleActionType::Attack, 0));
    REQUIRE(battle.Units()[0].hp == 175);
}

TEST_CASE("Leader KO removes aura immediately") {
    BattleUnit allyLeader;
    allyLeader.id = "ally_leader";
    allyLeader.name = "Leader";
    allyLeader.side = TeamSide::Allies;
    allyLeader.category = UnitCategory::Hero;
    allyLeader.isAssignedLeader = true;
    allyLeader.stats = UnitStats{10, 10, 10, 10, 10, 10, 20, 10, 1, 1, UnitPosition::Front, UnitRange::Ranged};
    allyLeader.hp = allyLeader.stats.maxHp;
    allyLeader.mp = allyLeader.stats.maxMp;
    allyLeader.life = 1;

    BattleUnit allyAttacker;
    allyAttacker.id = "ally_attacker";
    allyAttacker.name = "Attacker";
    allyAttacker.side = TeamSide::Allies;
    allyAttacker.category = UnitCategory::Generic;
    allyAttacker.stats = UnitStats{5, 1, 1, 1, 10, 10, 30, 0, 8, 1, UnitPosition::Front, UnitRange::Melee};
    allyAttacker.hp = allyAttacker.stats.maxHp;
    allyAttacker.mp = 0;
    allyAttacker.life = 1;

    BattleUnit enemyKiller;
    enemyKiller.id = "enemy_killer";
    enemyKiller.name = "Killer";
    enemyKiller.side = TeamSide::Enemies;
    enemyKiller.category = UnitCategory::Generic;
    enemyKiller.stats = UnitStats{30, 1, 1, 1, 30, 30, 200, 0, 12, 1, UnitPosition::Front, UnitRange::Melee};
    enemyKiller.hp = enemyKiller.stats.maxHp;
    enemyKiller.mp = 0;
    enemyKiller.life = 1;

    BattleState battle = BattleState::CreateForTests({allyLeader, allyAttacker, enemyKiller}, 13);
    REQUIRE(battle.ActiveUnitIndex() == 2);

    REQUIRE(battle.ExecuteAction(BattleActionType::Attack, 0));
    REQUIRE(battle.Units()[0].hp == 0);
    REQUIRE(battle.Units()[0].ko);

    REQUIRE(battle.ActiveUnitIndex() == 1);
    REQUIRE(battle.ExecuteAction(BattleActionType::Attack, 2));
    REQUIRE(battle.Units()[2].hp == 186);
}

TEST_CASE("Allied victory flags KO non-player heroes as lost after battle") {
    BattleUnit koHero;
    koHero.id = "ally_hero";
    koHero.name = "Hero";
    koHero.side = TeamSide::Allies;
    koHero.category = UnitCategory::Hero;
    koHero.isPlayerCharacter = false;
    koHero.stats = UnitStats{2, 2, 2, 2, 3, 3, 20, 5, 3, 1, UnitPosition::Back, UnitRange::Ranged};
    koHero.hp = 0;
    koHero.mp = koHero.stats.maxMp;
    koHero.life = 1;

    BattleUnit allyFinisher;
    allyFinisher.id = "ally_finisher";
    allyFinisher.name = "Finisher";
    allyFinisher.side = TeamSide::Allies;
    allyFinisher.category = UnitCategory::Hero;
    allyFinisher.stats = UnitStats{5, 2, 2, 2, 20, 20, 20, 5, 10, 1, UnitPosition::Front, UnitRange::Melee};
    allyFinisher.hp = allyFinisher.stats.maxHp;
    allyFinisher.mp = allyFinisher.stats.maxMp;
    allyFinisher.life = 1;

    BattleUnit enemy;
    enemy.id = "enemy_target";
    enemy.name = "Target";
    enemy.side = TeamSide::Enemies;
    enemy.category = UnitCategory::Generic;
    enemy.stats = UnitStats{1, 1, 1, 1, 1, 1, 10, 0, 3, 1, UnitPosition::Front, UnitRange::Melee};
    enemy.hp = enemy.stats.maxHp;
    enemy.mp = 0;
    enemy.life = enemy.stats.life;

    BattleState battle = BattleState::CreateForTests({koHero, allyFinisher, enemy}, 14);

    REQUIRE(battle.ExecuteAction(BattleActionType::Attack, 2));
    REQUIRE(battle.IsFinished());
    REQUIRE(battle.Summary().alliesWon);
    REQUIRE(battle.Units()[0].lostAfterBattle);
}

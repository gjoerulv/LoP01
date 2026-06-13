#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "gameplay/battle/AutoResolve.h"
#include "gameplay/battle/Battle.h"

// M33 battle-rule-aligned auto-resolve: deterministic, expected-damage CTB derived
// from the interactive battle formula + leader aura. Basic attacks only.

using gameplay::battle::AutoResolveUnit;
using gameplay::battle::AutoResolveWinner;
using gameplay::battle::ResolveAuto;
using gameplay::battle::UnitCategory;
using gameplay::battle::UnitStats;

namespace {

UnitStats Stats(int attack, int defense, int maxHp, int minDmg, int maxDmg, int agility = 5) {
    UnitStats s;
    s.attack = attack;
    s.defense = defense;
    s.maxHp = maxHp;
    s.minDamage = minDmg;
    s.maxDamage = maxDmg;
    s.agility = agility;
    return s;
}

AutoResolveUnit Generic(const std::string& id, UnitStats stats, int quantity) {
    AutoResolveUnit u;
    u.unitId = id;
    u.stats = stats;
    u.category = UnitCategory::Generic;
    u.quantity = quantity;
    return u;
}

AutoResolveUnit Hero(const std::string& id, UnitStats stats, bool leader = false) {
    AutoResolveUnit u;
    u.unitId = id;
    u.stats = stats;
    u.category = UnitCategory::Hero;
    u.quantity = 1;
    u.isLeader = leader;
    return u;
}

} // namespace

TEST_CASE("AutoResolve - empty attackers means defenders win; empty defenders means attackers win") {
    const std::vector<AutoResolveUnit> force = {Generic("g", Stats(5, 5, 10, 2, 3), 3)};

    REQUIRE(ResolveAuto({}, force).winner == AutoResolveWinner::Defenders);
    REQUIRE(ResolveAuto(force, {}).winner == AutoResolveWinner::Attackers);
}

TEST_CASE("AutoResolve - an overwhelmingly stronger attacker wins and keeps survivors") {
    const std::vector<AutoResolveUnit> attackers = {Generic("raider", Stats(12, 8, 14, 4, 6), 8)};
    const std::vector<AutoResolveUnit> defenders = {Generic("militia", Stats(3, 3, 8, 1, 2), 1)};

    const auto outcome = ResolveAuto(attackers, defenders);
    REQUIRE(outcome.winner == AutoResolveWinner::Attackers);
    REQUIRE(outcome.attackerSurvivingUnits == 1);
    REQUIRE(outcome.defenderSurvivingUnits == 0);
}

TEST_CASE("AutoResolve - an overwhelmingly stronger defender holds") {
    const std::vector<AutoResolveUnit> attackers = {Generic("raider", Stats(4, 3, 8, 1, 2), 1)};
    const std::vector<AutoResolveUnit> defenders = {Generic("militia", Stats(8, 8, 14, 4, 6), 10)};

    const auto outcome = ResolveAuto(attackers, defenders);
    REQUIRE(outcome.winner == AutoResolveWinner::Defenders);
    REQUIRE(outcome.defenderSurvivingUnits == 1);
}

TEST_CASE("AutoResolve - identical forces hold for the defender (tie goes to defender)") {
    const auto stats = Stats(6, 6, 12, 3, 5);
    const std::vector<AutoResolveUnit> attackers = {Generic("a", stats, 4)};
    const std::vector<AutoResolveUnit> defenders = {Generic("d", stats, 4)};

    REQUIRE(ResolveAuto(attackers, defenders).winner == AutoResolveWinner::Defenders);
}

TEST_CASE("AutoResolve - is deterministic across repeated calls") {
    const std::vector<AutoResolveUnit> attackers = {
        Generic("raider", Stats(9, 6, 12, 3, 5), 3),
        Hero("captain", Stats(12, 8, 24, 5, 7), /*leader=*/true),
    };
    const std::vector<AutoResolveUnit> defenders = {
        Generic("militia", Stats(6, 6, 12, 2, 4), 4),
        Hero("warden", Stats(10, 9, 20, 4, 6), /*leader=*/true),
    };

    const auto first = ResolveAuto(attackers, defenders);
    for (int i = 0; i < 20; ++i) {
        const auto again = ResolveAuto(attackers, defenders);
        REQUIRE(again.winner == first.winner);
        REQUIRE(again.attackerSurvivingUnits == first.attackerSurvivingUnits);
        REQUIRE(again.defenderSurvivingUnits == first.defenderSurvivingUnits);
        REQUIRE(again.attackerSurvivingQuantity == first.attackerSurvivingQuantity);
        REQUIRE(again.defenderSurvivingQuantity == first.defenderSurvivingQuantity);
        REQUIRE(again.actionsTaken == first.actionsTaken);
    }
}

TEST_CASE("AutoResolve - battle-relevant stats decide the outcome (defense flips a fight)") {
    const std::vector<AutoResolveUnit> attackers = {Generic("raider", Stats(8, 5, 12, 3, 5), 3)};

    // Same defender body count, but raising Defense + per-body HP flips a loss to a hold.
    const auto weak = ResolveAuto(attackers, {Generic("militia", Stats(5, 2, 8, 2, 3), 3)});
    const auto tough = ResolveAuto(attackers, {Generic("militia", Stats(5, 9, 20, 2, 3), 3)});

    REQUIRE(weak.winner == AutoResolveWinner::Attackers);
    REQUIRE(tough.winner == AutoResolveWinner::Defenders);
}

TEST_CASE("AutoResolve - a higher quantity stack can flip the outcome") {
    const std::vector<AutoResolveUnit> attackers = {Generic("raider", Stats(8, 6, 12, 3, 5), 4)};

    const auto few = ResolveAuto(attackers, {Generic("militia", Stats(6, 5, 10, 2, 4), 2)});
    const auto many = ResolveAuto(attackers, {Generic("militia", Stats(6, 5, 10, 2, 4), 20)});

    REQUIRE(few.winner == AutoResolveWinner::Attackers);
    REQUIRE(many.winner == AutoResolveWinner::Defenders);
}

TEST_CASE("AutoResolve - the leader aura strengthens its whole side") {
    const std::vector<AutoResolveUnit> defenders = {Generic("militia", Stats(5, 5, 12, 2, 4), 3)};

    // Attackers with no leader lose; the same attackers with a strong aura leader win.
    std::vector<AutoResolveUnit> withoutLeader = {Generic("raider", Stats(6, 5, 12, 2, 4), 3)};
    std::vector<AutoResolveUnit> withLeader = withoutLeader;
    withLeader.push_back(Hero("warlord", Stats(20, 20, 30, 6, 9), /*leader=*/true));

    REQUIRE(ResolveAuto(withoutLeader, defenders).winner == AutoResolveWinner::Defenders);
    REQUIRE(ResolveAuto(withLeader, defenders).winner == AutoResolveWinner::Attackers);
}

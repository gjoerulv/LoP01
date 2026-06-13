#pragma once

#include <string>
#include <vector>

#include "gameplay/battle/Battle.h"   // UnitStats, UnitCategory (shared battle model)

namespace gameplay::battle {

// M33 battle-rule-aligned auto-resolve input. Mirrors the durable battle inputs:
// the same UnitStats, category, stack quantity (Life), and leader/PC flags the
// interactive battle uses. `unitId`/`stackId` are carried through only for the
// caller's consequence mapping and summaries; the resolution math never reads
// them. Heroes use quantity 1; a generic stack uses its stack count.
struct AutoResolveUnit {
    std::string unitId;
    std::string stackId;
    UnitStats stats;
    UnitCategory category = UnitCategory::Generic;
    int quantity = 1;
    bool isLeader = false;          // assigned leader of its side (grants the aura)
    bool isPlayerCharacter = false;
};

enum class AutoResolveWinner { Attackers, Defenders };

struct AutoResolveOutcome {
    AutoResolveWinner winner = AutoResolveWinner::Defenders;
    int attackerStartUnits = 0;
    int defenderStartUnits = 0;
    int attackerSurvivingUnits = 0;
    int defenderSurvivingUnits = 0;
    long long attackerSurvivingQuantity = 0;
    long long defenderSurvivingQuantity = 0;
    long long actionsTaken = 0;     // diagnostics only
};

// Deterministic, battle-rule-aligned auto-resolve of `attackers` versus
// `defenders`. It reuses the interactive battle's physical-damage formula and
// leader-aura rule, but with EXPECTED (average) damage rolls instead of RNG, so a
// given pair of forces always resolves the same way without a seed.
//
// Deliberate M33 simplifications (battle-depth is future scope): basic physical
// attacks only — no skills, items, magic, defend, wait, position changes, status
// effects, escape, surrender, or revives. Target selection is "focus the weakest
// living enemy" (lowest current HP, ties by stable index). CTB ordering is
// agility-driven; on equal timing the DEFENDER acts first, which (with the
// "defenders win when attackers cannot remove them" rule) makes symmetric fights
// hold for the defender — consistent with docs/core_loop_rules.md §13 ("attacking
// a held service must require real superiority").
//
// Empty `attackers` => defenders win; empty `defenders` => attackers win. The
// function is pure: it depends only on its inputs and mutates nothing observable.
[[nodiscard]] AutoResolveOutcome ResolveAuto(
    const std::vector<AutoResolveUnit>& attackers,
    const std::vector<AutoResolveUnit>& defenders);

} // namespace gameplay::battle

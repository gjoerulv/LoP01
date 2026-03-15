---
mode: agent
description: Implement the turn-based battle module for LoP01.
---

# Implement Battle Module

You are adding a complete turn-based combat system to **Lands of Peril**.

## Pre-reading

Before writing any code read:
- `docs/combat_rules.md` – full combat specification
- `docs/core_loop_rules.md` – how combat integrates with the main loop
- `docs/content_scope_v0.md` – which enemies are in scope for v0

## Files to create

| File | Purpose |
|------|---------|
| `src/Combatant.h/.cpp` | Base stats (HP, attack, defence) shared by player and enemies |
| `src/Enemy.h/.cpp` | Enemy type with ID, name, stats and loot table |
| `src/BattleScene.h/.cpp` | Scene subtype that renders the battle UI |
| `src/BattleSystem.h/.cpp` | Turn resolution logic (player action → enemy action → result) |
| `data/enemies.json` | Enemy definitions |
| `tests/test_battle.cpp` | Unit tests for `BattleSystem` |

## Integration points

- `Game::ProcessCommand()` must detect a combat trigger (e.g. player enters a location with an enemy present) and switch the active scene to `BattleScene`.
- `BattleSystem::ResolveTurn()` takes player and enemy actions and returns an outcome (`Hit`, `Miss`, `EnemyDefeated`, `PlayerDefeated`).
- On `PlayerDefeated` switch to `Scene::Type::GameOver`.
- On `EnemyDefeated` remove the enemy from the location and return to the location scene.

## Acceptance criteria

- All existing tests still pass.
- `BattleSystem` unit tests cover: hit, miss, enemy defeated, player defeated.
- A player can enter the Darkwood Forest, fight the enemy and either win or lose.

# Combat Rules

> **Status:** Planned for v1. Not implemented in v0.

## Overview

Combat in Lands of Peril is **turn-based** and **deterministic with randomised damage rolls**.
The player and enemy alternate actions until one side is defeated.

## Trigger

Combat begins when:
- The player moves into a location that contains a living enemy, **or**
- The player issues an `attack` command while in a location with an enemy.

## Turn order

1. **Player acts first** each round.
2. The enemy acts second.
3. Repeat until one combatant's HP reaches 0.

## Actions

### Player actions

| Command | Effect |
|---------|--------|
| `attack` | Deal damage to the enemy |
| `flee` | Attempt to escape to the previous location |

### Enemy actions

Enemies always attack the player (v1). More complex behaviours are deferred to v2.

## Damage calculation

```
damage = max(1, attacker.attack - defender.defence + roll(-2, +2))
```

`roll(-2, +2)` is a uniform random integer in [−2, +2].

## Defeat conditions

| Condition | Outcome |
|-----------|---------|
| Enemy HP ≤ 0 | Enemy removed from location; player may continue exploring |
| Player HP ≤ 0 | Game over |

## Fleeing

- The player rolls a **1-in-3 chance** of a successful flee each attempt.
- On success the player moves to the previous location with no penalty.
- On failure the enemy gets a free attack before the next player turn.

## Enemy definitions

Enemy stats are defined in `data/enemies.json`. See `content_scope_v0.md` for the v0 enemy list.

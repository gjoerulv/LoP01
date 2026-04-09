# Project Ashvale (LoP01)

This repository contains a **playable vertical slice** of a 2D single-player strategy/RPG built with:

- C++20
- raylib
- CMake
- JSON-driven content
- save/load support
- Catch2 tests for the current gameplay slice (mostly logic-focused; the current test target still links rendering)

The project is being extended through small, maintainable milestones while keeping gameplay logic deterministic, explicit, testable, performant, and data-driven.

## Current slice scope

This is not the full game. The current slice is intentionally bounded to a single region and a small set of destinations, locations, encounters, quests, services, roster interactions, and placeholder presentation.

The current slice supports:

- a full app flow with title, opening, overworld selection, and gameplay
- explicit mode transitions between overworld, location, and battle
- content-driven overworld destinations and typed location definitions
- content-driven location scenes and battle scenario selection
- turn-based CTB battle encounters
- day/time progression and wake-up penalty rules
- route-aware overworld travel with blocker-aware routing
- lightweight persistent world state for cleared combat nodes
- simple typed quest progression, including combat-node-clear triggers
- persistent roster state with owned units, active party, and reserves
- recruit and mustering behavior tied to persistent roster state
- battle consequence written back into persistent party state
- save/load for the current gameplay slice
- placeholder content and presentation assets

## Current architecture baseline

Milestones 5 through 8 established the current implementation baseline.

The current baseline includes:

- explicit `App` / `GameSession` flow
- controller / mapper / renderer split
- typed regions, locations, location scenes, battle scenarios, quests, services, and unit content
- unified wake-penalty recovery flow for missed sleep and full defeat
- explicit battle return routing
- route-aware travel rules shared by preview and confirm
- minimal persistent world state for cleared combat nodes
- minimal typed quest progression tied to world actions
- persistent roster state with save/load support and migration from older slice formats
- battle initialization from the active party rather than only static allied setup
- battle result write-back into persistent roster and party state
- leader / aura foundations integrated into the roster and battle model

Future milestones should preserve this architecture unless a change is clearly justified.

## Folder structure

- `src/app`
  - application shell
  - mode transitions
  - input polling / translation
  - controllers
  - render-model mappers

- `src/core`
  - time/day rules
  - save/load serialization

- `src/data`
  - JSON loading
  - content repositories
  - typed content definitions

- `src/gameplay`
  - `GameSession`
  - gameplay state and rules
  - battle runtime
  - location runtime
  - overworld/world-state rules
  - quest runtime
  - roster / party state

- `src/rendering`
  - mode-specific renderers
  - shared HUD and debug overlay
  - render context/theme helpers

- `content`
  - JSON content definitions used by the current slice

- `tests`
  - gameplay/presentation tests for the current slice; most are logic-focused, but the test target currently still links rendering

- `docs`
  - vision, technical direction, combat rules, and milestone-planning docs

## Gameplay and data architecture

### Runtime flow

- `App` owns the main loop, top-level mode handling, and integration of input, gameplay update, and drawing.
- `GameSession` owns gameplay-facing session state and explicit mode transitions.
- `GameClock` owns day/time progression logic.
- Save/load persists only the gameplay state needed for the current slice.
- Content is loaded from JSON and mapped into typed gameplay-facing data structures where needed.
- Static content and runtime world state should remain separate so content stays editor-friendly.

### Gameplay modes

The current playable slice includes these primary modes:

- `title`
- `opening_sequence`
- `overworld_selection`
- `overworld_mode`
- `location_mode`
- `battle_mode`

Mode transitions should remain explicit and easy to follow. Prefer clear mode-entry helpers over generic chained advancement.

### Data-driven content

Content files under `content/` define the current slice, including:

- `regions.json`
- `locations.json`
- `location_scenes.json`
- `location_services.json`
- `battle_scenarios.json`
- `units.json`
- `enemy_groups.json`
- `quests.json`

These files provide the bounded playable-slice content used by the current milestone path. Future progression work should continue to extend content schemas and gameplay-facing runtime state rather than hardcoding milestone-specific behavior into presentation or app glue.

## Battle baseline

The current battle foundation is now a post-M8 baseline, not a prototype direction.

Key battle assumptions:

- battle is **static formation CTB**, not grid movement
- a team can field up to **5 active units**
- the **Leader** position is **one of the 5**, not an extra slot
- only **hero units** can be assigned as leader
- targeting is free; position affects **agility penalty**, not target legality
- agility penalty uses the target's **effective row depth**, not only the stored row label
- position changes are explicit turn-consuming actions
- the leader aura is a baseline hard rule and reactivates immediately when leadership is reassigned
- combat is intended to be highly readable and mostly deterministic, with randomness limited to damage roll
- hero HP persists between battles, generic stack HP resets, generic stack count persists, and MP persists for all units

For detailed battle rules, see `docs/combat_rules.md`.

## Current controls

Controls are still prototype-level and may evolve, but the current slice supports:

- `Enter`: confirm / advance in relevant contexts
- `Escape`: cancel / back where supported
- `E`: interact in location mode
- `Arrow keys` / `WASD`: movement in location mode
- `Left` / `Right`: previous / next selection in menu-style contexts
- `Up` / `Down`: target selection in battle-style contexts
- `1`: option 1 in contexts that expose it
- `2`: option 2 in contexts that expose it
- `B`: debug battle shortcut where enabled
- `F1`: toggle debug overlay
- `F5`: save to `saves/slot_1.json`
- `F9`: load from `saves/slot_1.json`

## Current milestone status

Milestone 8 is complete and merged to `main`.

Delivered Milestone 8 outcomes:

- canonical persistent roster state for owned units, active party, and reserves
- active party size increased to **5**
- recruit flow migrated to persistent roster state
- Home Base mustering integrated into persistent roster management
- save/load migration for older slice state into canonical roster structures
- battle initialization from the current active party
- battle quantity persistence and roster write-back
- leader / aura baseline integrated into battle legality and party state
- player active party legality hardened so a legal leader is always available
- player character seeded into owned + active party at startup
- allied win handling updated so the player character recovers to 1 HP if KO'd
- KO non-player heroes leave the party on allied win and can later be recovered through the game's hero-recovery systems

Milestone 6, 7, and 8 docs/prompts are retained as history once archived.

## Current gameplay baseline

The current branch now includes this post-M8 baseline:

- explicit `App` / `GameSession` flow
- controller / mapper / renderer split
- typed regions, locations, location scenes, battle scenarios, quests, service definitions, and unit content
- unified wake-penalty recovery flow
- route-aware travel replacing placeholder index-distance travel
- blocker-aware routing tied to lightweight persistent world state
- minimal typed quest progression tied into the world loop
- content-driven service/economy behavior for rest, recruit stock, and travel prep
- app-layer service prompt formatting and UI-light readability improvements
- persistent roster state, mustering, and battle-party consequence
- post-battle write-back of unit losses and hero recovery state
- battle foundation aligned with `docs/combat_rules.md`

## Current planning posture

The repo should now be treated as **post-M8 baseline**.

Planning for the next milestone should:

- preserve the existing `App` / `GameSession` and controller / mapper / renderer architecture
- keep gameplay logic separate from rendering/input
- treat `docs/game_vision_complete.md` and `docs/combat_rules.md` as source-of-truth design docs where they are explicit
- tighten vision/docs first when a major system is still ambiguous
- keep new work bounded to the current single-region slice unless a milestone explicitly expands scope
- avoid depending on unresolved campaign/world-map/multi-region decisions unless the milestone is specifically about those decisions
- maintain responsiveness, explicit ownership, leak resistance, and performance

## Build

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

## Notes

- Archived milestone docs and prompts are kept for history and should not be treated as active implementation guidance.
- `docs/content_scope_v0.md` remains a scope cap, not a checklist of completed work.
- The active long-term north-star doc is `docs/game_vision_complete.md`.
- The active battle-rules doc is `docs/combat_rules.md`.
- The current codebase is still a bounded slice and does not yet implement the full campaign/scenario/world-map structure.

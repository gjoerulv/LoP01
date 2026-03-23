# Project Ashvale (LoP01)

This repository contains a **playable vertical slice** of a 2D single-player strategy/RPG built with:

- C++20
- raylib
- CMake
- JSON-driven content
- save/load support
- Catch2 tests for pure logic

The project is being extended through small, maintainable milestones while keeping gameplay logic deterministic, explicit, testable, performant, and data-driven.

## Current slice scope

This is not the full game. The current slice is intentionally bounded to a single region and a small set of destinations, locations, encounters, quests, and placeholder presentation.

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
- save/load for the current gameplay slice
- placeholder content and presentation assets

## Current architecture baseline

Milestones 5 and 6 established the current implementation baseline.

The current baseline includes:

- explicit `App` / `GameSession` flow
- controller / mapper / renderer split
- typed regions, locations, location scenes, battle scenarios, and quests
- unified wake-penalty recovery flow for missed sleep and full defeat
- explicit battle return routing
- route-aware travel rules shared by preview and confirm
- minimal persistent world state for cleared combat nodes
- minimal typed quest progression tied to world actions
- save/load for current slice state and lightweight world/progression state

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

- `src/rendering`
  - mode-specific renderers
  - shared HUD and debug overlay
  - render context/theme helpers

- `content`
  - JSON content definitions used by the current slice

- `tests`
  - gameplay/presentation tests for the current slice; most are logic-focused, but the test target currently still links rendering

- `docs`
  - vision, technical direction, core-loop rules, combat rules, and milestone docs

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
- `battle_scenarios.json`
- `units.json`
- `enemy_groups.json`
- `quests.json`

These files provide the bounded playable-slice content used by the current milestone path. Future service/economy work should continue to extend content schemas rather than hardcoding service behavior in gameplay logic.

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

## Milestone 6 status

Milestone 6 is complete and merged. The current branch starts from that post-M6 baseline.

Delivered Milestone 6 outcomes:

- clearer world identity despite shared prototype scene reuse
- rest validity fixed for the current slice without broad service gating
- route-aware travel replacing placeholder index-distance travel
- blocker-aware routing tied to lightweight persistent world state
- combat-node-clear quest progression tied into the world loop
- readability improvements for travel reasons, blockers, cleared nodes, and current quest hints

Milestone 6 docs/prompts are retained as archived history only.

Location scenes may still reuse prototype layout, but service identity for M7 must come from per-location authored service data rather than from shared scene interaction costs.

## Milestone 7 direction

Milestone 7 should be a larger, more visible update focused on **home base identity, service economy, and weekly cadence**.

Current Milestone 7 priorities:

- make home base feel like the central safe hub of the slice
- make inns usable and paid, while home-base rest remains free
- make recruit locations content-driven with unit offers, quantities, and weekly refresh
- introduce content-driven service/economy state that remains editor-friendly
- preserve separation of concerns and current architecture
- keep the game responsive and performant, with clear ownership and no memory leaks

See `docs/game_vision_complete.md` and `docs/milestone_7_services_economy_weekly_cadence.md` for the active planning direction.

## Build

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

## Notes

- Archived milestone docs and prompts are kept for history and should not be treated as active implementation guidance.
- `docs/content_scope_v0.md` remains a scope cap, not a checklist of completed work.
- The active long-term north-star doc is `docs/game_vision_complete.md`.
- For the finished-direction hierarchy and terminology, see `docs/game_vision_complete.md`.
- The current codebase is still a bounded slice and does not yet implement the full campaign/scenario/world-map structure.

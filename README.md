# Project Ashvale (LoP01)

This repository contains a **playable vertical slice** of a 2D single-player strategy/RPG built with:

- C++20
- raylib
- CMake
- JSON-driven content
- save/load support
- Catch2 tests for pure logic

The project is being extended in small, maintainable milestones while keeping gameplay logic deterministic, testable, explicit, and data-driven.

## Current slice scope

This is not the full game. The current slice is intentionally bounded to a single region and a small set of destinations, locations, encounters, quests, and placeholder presentation.

The current slice supports:

- a full app flow with title, opening, overworld selection, and gameplay
- explicit mode transitions between overworld, location, and battle
- content-driven overworld destinations and typed location definitions
- content-driven location scenes and battle scenario selection
- turn-based CTB battle encounters
- day/time progression
- sleep and wake-up penalty rules
- simple typed quest progression
- save/load for the current gameplay slice
- placeholder content and presentation assets

## Milestone 5 baseline

Milestone 5 is complete and acts as the current implementation baseline.

The current baseline includes:

- explicit `App` / `GameSession` flow
- controller / mapper / renderer split
- typed regions, locations, location scenes, battle scenarios, and quests
- interaction-driven location services
- unified wake-penalty recovery flow for missed sleep and full defeat
- explicit battle return routing
- minimal typed quest progression
- save/load for current slice state and completed quest ids

Milestone 6 should preserve this architecture and build on it incrementally.

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
  - quest runtime

- `src/rendering`
  - mode-specific renderers
  - shared HUD and debug overlay
  - render context/theme helpers

- `content`
  - JSON content definitions used by the current slice

- `tests`
  - pure-logic tests without raylib dependency

- `docs`
  - vision, technical direction, core-loop rules, combat rules, and milestone docs

## Gameplay and data architecture

### Runtime flow

- `App` owns the main loop, top-level mode handling, and integration of input, gameplay update, and drawing.
- `GameSession` owns gameplay-facing session state and explicit mode transitions.
- `GameClock` owns day/time progression logic.
- Save/load persists only the gameplay state needed for the current slice.
- Content is loaded from JSON and mapped into typed gameplay-facing data structures where needed.

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

These files provide the bounded playable-slice content used by the current milestone path.

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

## Milestone 6 direction

Milestone 6 should focus on making the world rules match the current typed content model more closely.

Current Milestone 6 priorities:

- make location identity clearer and more location-specific
- make rest/services valid per location, not only by shared prototype scene layout
- make overworld travel follow the route graph more explicitly
- add minimal persistent node/world state where needed
- preserve the current architecture and implement in small compilable steps

## Build

```bash
cmake -S . -B build -G Ninja
cmake --build build
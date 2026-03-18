# Project Ashvale (LoP01)

This repository contains a **playable vertical slice** of a 2D single-player strategy/RPG built with:

- C++20
- raylib
- CMake
- JSON-driven content
- save/load support
- Catch2 tests for pure logic

The current project focus is to extend the playable slice in small, maintainable milestones while keeping gameplay logic deterministic, testable, and data-driven.

## Current slice scope

This is not the full game. The current slice is intentionally limited to a single bounded region and a small set of destinations, locations, encounters, and quests.

The slice currently aims to support:

- a full app loop with title/opening/gameplay flow
- overworld travel and destination selection
- location scenes and interactions
- turn-based battle encounters
- day/time progression
- sleep and wake-up penalty rules
- simple quest progression
- save/load for the current gameplay slice
- placeholder content and presentation assets

## Folder structure

- `src/app`
  - application shell
  - mode transitions
  - input/update/draw integration
- `src/core`
  - pure logic and foundational systems
  - time/day rules
  - save/load serialization
- `src/data`
  - content loading and repositories
- `src/gameplay`
  - gameplay session state
  - controllers
  - mode-specific gameplay systems
  - typed gameplay/content-facing models
- `src/rendering`
  - render models
  - mappers
  - mode-specific renderers
- `content`
  - JSON content definitions
- `tests`
  - pure logic tests without raylib dependency
- `docs`
  - game vision, technical direction, combat rules, and milestone docs

## Gameplay and data architecture

### Runtime flow

- `App` owns the application run loop and integrates rendering, input, and mode entry.
- `GameSession` owns gameplay-facing state, current mode, and cross-mode progression.
- `GameClock` owns day/time progression logic.
- Save/load code persists the gameplay state needed for the current slice.
- Content is loaded from JSON and mapped into gameplay-facing data structures used by the slice.

### Gameplay modes

The current playable slice includes these primary modes:

- `title`
- `opening_sequence`
- `overworld_selection`
- `overworld_mode`
- `location_mode`
- `battle_mode`

Mode transitions should remain explicit and easy to follow. Prefer clear mode-entry helpers over generic chained advancement for gameplay transitions.

### Data-driven content

Content files under `content/` define the current slice:

- `regions.json`
- `locations.json`
- `units.json`
- `enemy_groups.json`
- `quests.json`

These files provide the bounded playable-slice content used by the current milestone path.

## Current controls

Controls may evolve by milestone, but the current slice supports:

- `Enter`: confirm / advance in relevant contexts
- `1`: add 1 minute in location-style interactions
- `T`: add 15 minutes for travel/testing flow where enabled
- `P`: apply wake-up penalty for testing
- `F1`: toggle debug overlay
- `F5`: save to `saves/slot_1.json`
- `F9`: load from `saves/slot_1.json`

## Build

```bash
cmake -S . -B build -G Ninja
cmake --build build
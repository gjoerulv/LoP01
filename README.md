# Project Ashvale (LoP01) - Vertical Slice Bootstrap

This repository now contains a **minimal playable scaffold** for the first vertical slice:

- C++20
- raylib
- CMake
- nlohmann/json content loading
- save/load scaffolding
- Catch2 tests for pure logic

## Scope in this bootstrap

This is not the full game. It intentionally includes only:

- an app loop
- a simple game state machine scaffold
- JSON content definitions and loader
- save/load persistence scaffold
- logic tests for core rules

## Proposed folder structure

- `src/app`
  - app bootstrap and main loop
  - state-driven update/draw shell
- `src/core`
  - pure logic and foundational systems
  - day/time rules
  - save/load JSON serialization
- `src/data`
  - JSON content repository loading starter files
- `src/gameplay`
  - game session/state scaffolding for gameplay flow
- `content`
  - data-driven JSON content definitions
- `tests`
  - pure logic tests (no raylib dependency)
- `docs`
  - game vision and design/rules references

## Proposed gameplay/data architecture

### Runtime flow

- `App` owns the high-level run loop and rendering/input glue.
- `GameSession` owns gameplay-facing state mode transitions and player-facing values.
- `GameClock` owns day/time progression logic.
- `SaveGameRepository` handles save/load to JSON files.
- `ContentRepository` loads static JSON content into in-memory documents.

### State scaffold included

Current scaffold contains these required modes:

- `title`
- `opening_sequence`
- `overworld_selection`
- `overworld_mode`
- `location_mode`
- `battle_mode`

The scaffold intentionally transitions with simple key input so each mode exists and can be extended without rewriting architecture.

### Data-driven content

Starter JSON files are under `content/`:

- `regions.json`
- `locations.json`
- `units.json`
- `enemy_groups.json`
- `quests.json`

These provide v0 placeholder content compatible with the target direction.

## Controls (current scaffold)

- `Enter`: advance to next state
- `1`: add 1 minute (location-style small action)
- `T`: add 15 minutes (travel step)
- `P`: apply wake-up penalty (time to 11:00 + -1000 gold, floor 0)
- `F5`: save to `saves/slot_1.json`
- `F9`: load from `saves/slot_1.json`

## Build

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

Run app:

```bash
./build/ashvale_slice
```

Run tests:

```bash
ctest --test-dir build --output-on-failure

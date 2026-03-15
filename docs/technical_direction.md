# Technical Direction

## Stack

- C++20
- raylib
- CMake
- nlohmann/json
- Catch2

## Architectural principles

- Prefer a simple game state machine
- Keep systems explicit rather than overly abstract
- Keep data-driven content in JSON
- Separate rules from presentation
- Make pure logic testable without raylib dependency where possible

## Proposed module layout

- src/app
  - main loop
  - app bootstrap
  - screen/state switching

- src/core
  - time
  - random
  - ids
  - save/load

- src/data
  - json loading
  - content repositories
  - definitions for units, heroes, locations, items, encounters

- src/gameplay
  - overworld
  - locations
  - party
  - battle
  - quests
  - penalties
  - progression

- src/ui
  - menus
  - battle HUD
  - tooltips
  - overlays

## Implementation guidance

- Do not build a generic engine
- Do not build full editor tooling
- Do not add networking
- Do not add mod support yet
- Keep the battle simulator mostly independent from rendering
- Favor compileable, testable milestones

# Technical Direction

## Stack

- C++20
- raylib
- CMake
- nlohmann/json
- Catch2

## Architectural principles

- Prefer a simple, explicit gameplay state machine
- Keep systems explicit rather than overly abstract
- Keep content data-driven in JSON
- Separate gameplay rules from presentation
- Keep rendering models and mappers separate from gameplay state
- Make pure logic testable without raylib dependency where possible

## Module layout

- `src/app`
  - main loop
  - application shell
  - mode transitions
  - input/update/draw integration

- `src/core`
  - time
  - random
  - ids
  - save/load

- `src/data`
  - JSON loading
  - content repositories
  - content definitions and typed gameplay-facing data mapping where needed

- `src/gameplay`
  - overworld
  - locations
  - party
  - battle
  - quests
  - penalties
  - progression
  - gameplay session/controllers

- `src/rendering`
  - renderers
  - render models
  - mappers
  - HUD / overlays

## Implementation guidance

- Do not build a generic engine
- Do not build full editor tooling
- Do not add networking
- Do not add mod support yet
- Keep the battle simulator mostly independent from rendering
- Prefer explicit mode-entry helpers over generic chained transitions for gameplay flow
- Favor compilable, testable milestones
- Extend the current playable slice incrementally instead of broadening scope prematurely
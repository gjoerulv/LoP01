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
- Preserve strong separation of concerns: input, gameplay, mapping, and rendering should not collapse into one layer

## Module layout

- `src/app`
  - main loop
  - application shell
  - mode transitions
  - input/update/draw integration
  - controllers and render-model mappers

- `src/core`
  - time
  - ids
  - save/load

- `src/data`
  - JSON loading
  - content repositories
  - content definitions and typed gameplay-facing data mapping where needed

- `src/gameplay`
  - overworld and world-state rules
  - locations
  - party
  - battle
  - quests
  - penalties
  - progression
  - gameplay session

- `src/rendering`
  - renderers
  - render models
  - HUD / overlays

## Content architecture guidance

- Treat authored JSON as static source-of-truth content, not mutable runtime state.
- Use stable ids and explicit fields so future tooling can create/update/delete content safely.
- Prefer typed gameplay-facing definitions rather than passing loosely shaped JSON through gameplay systems.
- Keep content semantics in data whenever practical instead of hiding them in renderer or input code.
- Extend schemas incrementally and keep migrations obvious.

## Performance and correctness guidance

- Favor RAII and clear ownership to avoid leaks.
- Avoid raw owning pointers unless ownership is truly external and obvious.
- Do not parse content repeatedly in the frame loop.
- Avoid unnecessary per-frame allocations and string churn in hot paths.
- Avoid blocking work in the main loop.
- Keep input handling responsive and separate from rendering.
- Keep rendering lightweight and avoid moving gameplay decisions into render code.

## Implementation guidance

- Do not build a generic engine.
- Do not build full editor tooling yet.
- Do not add networking.
- Do not add mod support yet.
- Keep the battle simulator mostly independent from rendering.
- Prefer explicit mode-entry helpers over generic chained transitions for gameplay flow.
- Favor compilable, testable milestones.
- Extend the current playable slice incrementally instead of broadening scope prematurely.
- When adding service/economy systems, prefer content-driven schemas and lightweight runtime state over broad service frameworks.

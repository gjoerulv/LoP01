# Project-wide Copilot instructions

You are working in a 2D single-player game project written in C++20 using raylib and CMake.

## High-level goals

- Continue evolving the existing playable slice through small, complete milestones.
- Prioritize maintainable gameplay code over clever abstractions.
- Prefer deterministic, testable logic.
- Keep rendering and UI simple while gameplay systems mature.
- Use placeholder assets and data-driven content.

## Design pillars

- Overworld flow inspired by Heroes of Might and Magic 2/3
- Town and dungeon exploration inspired by SNES-era Final Fantasy
- True turn-based CTB combat inspired by Final Fantasy X
- Cozy progression and restoration elements inspired by farming/life-sim games
- Tone: fantasy + light dystopia + cozy restoration

## Technical rules

- Use C++20.
- Use raylib for graphics/input/audio.
- Use CMake.
- Prefer plain classes and clear ownership.
- Avoid premature ECS or overengineering.
- Keep gameplay systems separated:
  - time/day system
  - overworld system
  - location system
  - battle system
  - content loading
  - save/load
  - quest/progression flow
- Keep core gameplay logic independent from rendering where practical.
- Put balance values and content definitions in JSON, not hardcoded in gameplay logic.
- Add tests for pure logic wherever feasible.
- Use explicit raw time naming as `minutesIntoSliceDay` and avoid time-string parsing in gameplay rules.

## Combat system implementation

- Follow `docs/combat_rules.md` exactly when implementing combat systems.
- Keep formulas simple and testable.

## Working style

- Before large changes, summarize the plan briefly.
- For ambiguous requirements, choose the simplest implementation that preserves the intended gameplay.
- Document non-obvious design decisions in `README_DECISIONS.md`.
- When creating new systems, provide extension points but do not generalize prematurely.
- Prefer small, complete milestones that build and run.

## Current bounded scope

Keep the playable slice intentionally limited:

- 1 overworld region
- 10 to 20 destinations
- 1 or 2 town-style locations
- 1 home/base
- 1 to 3 dungeon-style locations
- 3 heroes total
- 8 generic unit types
- 8 to 10 enemy groups
- 2 simple quests
- save/load
- placeholder art

## Key docs to follow

Always consult these docs when relevant:

- `README.md`
- `README_DECISIONS.md`
- `docs/game_vision.md`
- `docs/core_loop_rules.md`
- `docs/combat_rules.md`
- `docs/content_scope_v0.md`
- `docs/technical_direction.md`
- `docs/milestone_6_world_identity.md` when working on Milestone 6 tasks

## Document precedence

When documents overlap or conflict, use this order of authority:

1. current codebase
2. active milestone doc
3. `README_DECISIONS.md`
4. core rules / combat rules / technical direction
5. `docs/content_scope_v0.md` as a bounded scope cap only
6. complete-vision notes as incomplete long-term direction
7. archived docs as history only

`docs/content_scope_v0.md` should be used to avoid scope creep, not as a checklist for what is already implemented and not as the primary behavior spec.

## Current milestone baseline

Milestone 5 is complete.

Assume the current baseline already includes:

- explicit `App` / `GameSession` flow
- controller / mapper / renderer split
- typed regions, locations, location scenes, battle scenarios, and quests
- interaction-driven location services
- unified wake-penalty recovery flow
- explicit battle return routing
- minimal typed quest progression
- save/load for current slice state and completed quest ids

Preserve this baseline unless a change is clearly necessary.

## Current milestone focus

The current milestone is Milestone 6: world identity and route rules.

Priorities for the current milestone:

- preserve the existing `App` / `GameSession` and controller / mapper / renderer architecture
- keep mode transitions explicit and easy to follow
- make location services valid per location, not only because of shared scene prototype layout
- make overworld travel more faithful to route/graph rules
- add minimal persistent node/world state only where needed
- keep quests minimal and typed
- keep UI additions lightweight and focused on communicating state clearly

Avoid:

- broad content expansion
- extra regions
- major new combat mechanics
- generic service/event frameworks
- large architectural rewrites

When in doubt, prefer the smallest clean implementation that keeps the playable slice coherent.
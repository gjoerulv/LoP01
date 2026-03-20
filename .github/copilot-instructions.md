# Project-wide Copilot instructions

You are working in a 2D single-player game project written in C++20 using raylib and CMake.

## High-level goals

- Continue evolving the existing playable slice through small, complete milestones.
- Prioritize maintainable gameplay code over clever abstractions.
- Prefer deterministic, testable logic.
- Keep rendering and UI simple while gameplay systems mature.
- Keep the game responsive and performant.
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
  - service/economy state
- Keep core gameplay logic independent from rendering where practical.
- Put balance values and content definitions in JSON, not hardcoded in gameplay logic.
- Add tests for pure logic wherever feasible.
- Use explicit raw time naming as `minutesIntoSliceDay` and avoid time-string parsing in gameplay rules.
- Keep static authored content separate from mutable runtime state.
- Favor RAII and clear ownership to avoid leaks.
- Avoid unnecessary per-frame allocations, repeated content parsing, and blocking work in the main loop.
- Do not mix input logic with rendering code.

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
- a small quest set
- save/load
- placeholder art

## Key docs to follow

Always consult these docs when relevant:

- `README.md`
- `README_DECISIONS.md`
- `docs/game_vision.md`
- `docs/game_vision_complete.md`
- `docs/core_loop_rules.md`
- `docs/combat_rules.md`
- `docs/content_scope_v0.md`
- `docs/technical_direction.md`
- `docs/milestone_7_services_economy_weekly_cadence.md` when working on Milestone 7 tasks

## Document precedence

When documents overlap or conflict, use this order of authority:

1. current codebase
2. active milestone doc
3. `README_DECISIONS.md`
4. `docs/game_vision_complete.md`
5. core rules / combat rules / technical direction
6. `docs/content_scope_v0.md` as a bounded scope cap only
7. archived docs/prompts and archived placeholder vision notes as history only

`docs/content_scope_v0.md` should be used to avoid scope creep, not as a checklist for what is already implemented and not as the primary behavior spec.

## Current milestone baseline

Milestone 6 is complete on the current branch.

Assume the current baseline already includes:

- explicit `App` / `GameSession` flow
- controller / mapper / renderer split
- typed regions, locations, location scenes, battle scenarios, quests, and lightweight world state
- unified wake-penalty recovery flow
- route-aware travel, cutoff-aware travel, and blocker-aware routing
- minimal persistent cleared combat-node state
- minimal typed quest progression tied to world actions
- save/load for current slice state and lightweight world/progression state
- Use the following world hierarchy consistently:
- campaign -> scenario -> world map -> overworld/region -> node -> location

Important travel rules:
- the world map is a scenario-level region selection/planning layer
- the player may consult it at any time
- actual region travel may occur only once per day, before 11:00
- region travel places the player character in the destination region at 11:00
- only the player character travels between regions
- each region preserves its own party state

Preserve this baseline unless a change is clearly necessary.

## Current milestone focus

The active planning target is Milestone 7: home base, services, economy, and weekly cadence.

Priorities for the next milestone:

- preserve the existing `App` / `GameSession` and controller / mapper / renderer architecture
- keep mode transitions explicit and easy to follow
- make home base feel like the free-rest safe hub of the slice
- make inns meaningful as paid rest locations
- make recruit locations content-driven with offers, quantities, and weekly refresh
- keep new service/economy data editor-friendly and strongly content-driven
- keep UI additions lightweight and focused on communicating cost, quantity, and refresh state
- maintain responsiveness, performance, and ownership clarity as systems deepen

Avoid:

- broad content expansion disconnected from the milestone
- extra regions
- major new combat mechanics
- generic service/event frameworks
- large architectural rewrites
- building the editor itself during this milestone

When in doubt, prefer the smallest clean implementation that keeps the playable slice coherent and moves the game toward a stronger service/economy identity.

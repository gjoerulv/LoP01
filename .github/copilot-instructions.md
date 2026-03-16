# Project-wide Copilot instructions

You are working in a 2D single-player game project written in C++20 using raylib and CMake.

## High-level goals

- Build a playable vertical slice first, not a full game.
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
- Keep core gameplay logic independent from rendering where practical.
- Put balance values and content definitions in JSON, not hardcoded in gameplay logic.
- Add tests for pure logic wherever feasible.

## Combat System Implementation

- Follow docs/combat_rules.md exactly when implementing combat systems.
- Keep formulas simple and testable.

## Working style

- Before large changes, summarize the plan briefly.
- For ambiguous requirements, choose the simplest implementation that preserves the intended gameplay.
- Document non-obvious design decisions in README_DECISIONS.md.
- When creating new systems, provide extension points but do not generalize prematurely.
- Prefer small, complete milestones that build and run.

## Current scope

Target only a vertical slice with:
- 2 overworld regions
- 10 to 20 destinations
- 2 towns
- 2 inns
- 1 home/base
- 2 to 3 mines with 5 prototype floors
- 3 heroes
- 8 generic unit types
- 6 enemy groups
- 2 to 3 shops
- 2 to 3 recruit points
- 2 simple quests
- save/load
- placeholder art

## Key docs to follow

Always consult these docs when relevant:
- docs/game_vision.md
- docs/core_loop_rules.md
- docs/combat_rules.md
- docs/content_scope_v0.md
- docs/technical_direction.md

## Current milestone focus

The project is currently in the first visual gameplay pass.

Priorities for this milestone:
- improve presentation
- reduce debug-screen dominance
- create dedicated renderers for each game mode
- improve readability and contextual controls
- preserve existing gameplay logic
- For location-screen polish, keep logic unchanged and focus on presentation-only improvements, ensuring all UI elements are visible within game resolution.

Avoid expanding the project with major new mechanics until the current systems are visually represented and playable.
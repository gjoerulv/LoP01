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
  - roster/party management state
- Keep core gameplay logic independent from rendering where practical.
- Put balance values and content definitions in JSON, not hardcoded in gameplay logic.
- Add tests for pure logic wherever feasible.
- Use explicit raw time naming as `minutesIntoSliceDay` and avoid time-string parsing in gameplay rules.
- Keep static authored content separate from mutable runtime state.
- Favor RAII and clear ownership to avoid leaks.
- Avoid unnecessary per-frame allocations, repeated content parsing, and blocking work in the main loop.
- Do not mix input logic with rendering code.

## Combat system implementation

- Follow `docs/combat_rules.md` exactly when implementing or changing combat systems.
- Treat the current battle model as settled baseline unless a task explicitly reopens it.
- Battle is static formation CTB, not free-movement combat.
- Use effective row depth for battle calculations where the rules call for it.
- Leaders are hero-only.
- Keep formulas simple, readable, and testable.
- Preserve readability-first combat UX: turn-order preview, min/max damage, and min/max KO preview where applicable.

## Working style

- Before large changes, summarize the plan briefly.
- For ambiguous requirements, choose the simplest implementation that preserves the intended gameplay.
- Document non-obvious design decisions in `README_DECISIONS.md`.
- When creating new systems, provide extension points but do not generalize prematurely.
- Prefer small, complete milestones that build and run.
- When work touches vision-level behavior, update the relevant docs alongside the code or note the required doc follow-up explicitly.

## Current bounded scope

Keep the playable slice intentionally limited:

- 1 overworld region
- a small set of destinations and authored locations
- 1 home/base
- a small roster and enemy-group pool appropriate for a vertical slice
- a small quest/progression set
- save/load
- placeholder art

Use content files and current code as the source of truth for exact counts and currently available content. Do not hardcode old milestone assumptions about roster totals, encounter counts, or service counts into new plans.

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
- the active milestone doc, if one exists for the task at hand

Archived milestone docs, archived prompts, and archived placeholder notes are history only unless a task explicitly asks to consult them.

## Document precedence

When documents overlap or conflict, use this order of authority:

1. current codebase for already implemented behavior
2. explicit current-task requirements from the user
3. active milestone doc for the task, if one exists
4. `README_DECISIONS.md`
5. `docs/game_vision_complete.md`
6. `docs/core_loop_rules.md` and `docs/combat_rules.md`
7. `docs/technical_direction.md`
8. `docs/content_scope_v0.md` as a bounded-scope cap only
9. archived docs/prompts as historical context only

`docs/content_scope_v0.md` should be used to avoid scope creep, not as a checklist for what is already implemented and not as the primary behavior spec.

## Current baseline

M8 is complete on the current branch.

Assume the current baseline already includes:

- explicit `App` / `GameSession` flow
- controller / mapper / renderer split
- typed regions, locations, location scenes, battle scenarios, quests, and service definitions
- unified wake-penalty recovery flow
- route-aware travel, cutoff-aware travel, and blocker-aware routing
- minimal persistent cleared combat-node state
- minimal typed quest progression tied to world actions
- save/load for current slice state and lightweight world/progression/service state
- Home Base free rest and free once-per-day travel prep
- Old Inn paid rest
- Recruit Post weekly recruit stock and refresh behavior
- Supply Cart paid same-day travel prep fallback
- app-layer service prompt formatting and UI-light readability improvements
- canonical roster stack/slot model
- persistent owned-roster plus active-party state
- battle quantity persistence and battle write-back into roster state
- active party size of 5, with Leader inside the 5
- leader/aura baseline in place
- player team requiring a legal leader
- enemy teams optionally having a leader
- player-character seeding into owned roster and active-party legality protections
- player-character recovery to 1 HP on allied win if KO'd
- KO non-player heroes leaving the party on allied win if not revived before battle end

Use the following hierarchy as long-term project vocabulary and design direction:

- campaign -> scenario -> world map -> overworld/region -> node -> location

The current codebase is still a bounded single-region slice and does not yet implement the full world-map / cross-region travel layer.

## Current planning posture

There is no new implementation milestone locked yet beyond M8.

Until a new milestone is explicitly chosen:

- preserve the current post-M8 baseline
- treat `docs/combat_rules.md` and `docs/game_vision_complete.md` as the authoritative design baseline for battle and high-level gameplay vision
- do not reopen settled battle rules unless explicitly requested
- prefer vision tightening, bounded milestone planning, and small consistency cleanups over broad new feature work
- keep future milestone proposals tightly scoped to the current single-region vertical slice

## Avoid

- broad content growth disconnected from the current milestone or task
- extra regions or world-map systems unless explicitly requested
- major combat redesign unless the task explicitly reopens battle rules
- generic inventory/equipment/event/service frameworks before they are needed
- large architectural rewrites
- premature editor tooling
- mixing input logic with rendering code
- speculative campaign-scale systems that bypass the current slice

When in doubt, prefer the smallest clean implementation that preserves the existing vertical-slice foundation and strengthens authored progression, consequence, and Home Base identity.

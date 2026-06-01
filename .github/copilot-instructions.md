# Project-wide Copilot instructions

Use these rules when working in this repository.

## High-level goals

- Preserve a small, explicit, maintainable vertical slice.
- Keep gameplay logic deterministic, testable, and readable.
- Extend the game through bounded milestones, not broad speculative systems.
- Prefer clear architecture and content-driven rules over generic frameworks.
- Treat the repository and active design docs as the source of truth.

## Terminology source of truth

Before making terminology-sensitive changes, read `docs/terminology_map.md`.

That file defines the current intended design terms and explains how they map to any legacy runtime, content, or serialized names still present in the repository. Prefer the terminology in `docs/terminology_map.md` for design discussion, documentation updates, and new work unless the task is explicitly a legacy-compatibility change.

## Design pillars

- strong Scenario and Region identity
- readable tactical CTB combat
- meaningful route / time / service planning
- durable party consequence
- restoration and safe-anchor fantasy
- explicit, data-driven gameplay rules

## Technical rules

- Keep gameplay rules out of rendering and input layers.
- Keep input polling / translation in `src/app`, not in renderers.
- Keep renderers focused on drawing from view/state models.
- Keep pure logic pure where practical.
- Prefer explicit state and explicit transitions over hidden chaining.
- Preserve clear ownership and leak-resistant C++ code.
- Avoid unnecessary allocations and repeated parsing in the main loop.
- Keep authored static content separate from runtime mutable state.
- Prefer incremental schema evolution over ad hoc special cases.
- Prefer Catch2 tests for pure logic and mapper behavior over rendering/input tests.

## Combat system implementation

Battle assumptions are settled unless the user explicitly reopens them:

- battle is static-formation CTB, not grid combat
- teams field up to 5 active units
- the Leader position is inside the 5, not an extra slot
- only hero units can be leaders
- targeting is free; row depth affects agility penalty rather than target legality
- agility penalty uses the target's effective row depth
- leader aura is a baseline hard rule
- hero HP persists
- generic stack HP resets
- stack counts persist
- MP persists for all units
- battle UI should favor readable turn-order preview and min/max outcome visibility over exposing hidden math

Detailed combat rules live in `docs/combat_rules.md`.

## Working style

- Make the smallest clean change that solves the task.
- Prefer narrow, test-backed iterations over broad rewrites.
- Call out doc/code mismatches explicitly.
- If a design area is ambiguous, tighten docs/vision before implementing broad behavior.
- Do not silently invent large systems when the vision is underspecified.
- When changing durable behavior, update the relevant docs/tests in the same pass.
- Do not ask the user to restate an accepted plan as a prompt; wait for the next explicit implementation instruction.

## Current bounded scope

The current codebase is a post-M11-e bounded single-Region slice.

That means:

- enemy teams can exist, move, occupy Region nodes, block travel, be shown on the Region map, be engaged through configured contact battles, and be cleared by victory
- enemy-team runtime state, including alliances, persists through save/load
- typed event actions exist for `spawnTeam`, `removeTeam`, and `changeAlliance`
- the full World Map / cross-Region rules are not implemented
- campaign carry-over is not implemented
- final out-of-battle party-management UX is not implemented
- inventory, artifacts, recipes, skill/status depth, fog/scouting, advanced AI economy, and sabotage systems are not implemented

Use the settled terminology and design direction consistently in planning and docs:

- Campaign -> Scenario -> World Map -> Region -> node -> Location
- use `Region` as the main design term instead of `overworld`
- `traveling party` = active party + reserve
- stored units are distinct from reserve
- all traveling heroes cross Regions; traveling generic units do not unless stored
- region hero offerings reroll on Region entry from heroes who are not traveling, stored, or temporarily unavailable

## Key docs to follow

Read these before planning or making broad changes:

1. `README.md`
2. `README_DECISIONS.md`
3. `docs/implementation_roadmap.md`
4. `docs/game_vision.md`
5. `docs/game_shell_flow.md`
6. `docs/presentation_game_feel.md`
7. `docs/core_loop_rules.md`
8. `docs/combat_rules.md`
9. `docs/scenario_authoring.md`
10. `docs/validation_system.md`
11. `docs/content_schema.md`
12. `docs/terminology_map.md`
13. milestone-specific doc/prompt if the task is tied to one
14. `docs/technical_direction.md`
15. `docs/content_scope_v1.md` as a scope cap, not as a checklist of implemented behavior

If those sources disagree:

- prefer the current codebase for already implemented behavior
- prefer explicit current-task requirements from the user for the active task
- prefer the most recently settled design docs for long-term design intent
- flag the mismatch explicitly rather than guessing

## Document precedence

Use this order when there is ambiguity:

1. current codebase for already implemented behavior
2. explicit current-task requirements from the user
3. active milestone doc/prompt for the task, if one exists
4. `README_DECISIONS.md`
5. `docs/implementation_roadmap.md` for sequencing and “not yet” boundaries
6. `docs/game_vision.md`
7. `docs/game_shell_flow.md`
8. `docs/presentation_game_feel.md`
9. `docs/core_loop_rules.md` and `docs/combat_rules.md`
10. `docs/technical_direction.md`
11. `docs/content_scope_v1.md` as a bounded-scope cap only
12. `docs/terminology_map.md` as the source for terminology conflicts
13. archived docs/prompts as historical context only

`docs/content_scope_v1.md` should be used to avoid scope creep, not as a checklist for what is already implemented and not as the primary behavior spec.

## Current baseline

M11-e is complete on the current branch.

Assume the current baseline already includes:

- explicit `App` / `GameSession` flow
- controller / mapper / renderer split
- typed Regions, Locations, battle scenarios, Services, quests, and content definitions
- route-aware and blocker-aware Region travel inside the current single-Region slice
- save/load for current slice state plus lightweight world/progression/service state
- canonical roster stack/slot model
- persistent owned roster plus active-party state
- battle quantity persistence and battle write-back into roster state
- active party size of 5, with Leader inside the 5
- leader / aura baseline
- player team requiring a legal leader
- enemy teams optionally having a leader
- player-character seeding into the human team's traveling party
- player-character recovery to 1 HP at battle end if KO'd
- KO non-player heroes leaving the party on allied win if not revived before battle end
- typed event foundation with story flags and fired-event persistence
- enemy-team Region-layer foundation
- hostile occupation blocking
- preview/confirmed-travel alignment for hostile occupation
- visible hostile-occupation marker in Region rendering
- hostile contact battle through configured node `battleScenarioId`
- exact-team clearing by team color after hostile-contact victory
- enemy-team save/load runtime state, including alliances
- `spawnTeam`, `removeTeam`, and `changeAlliance` event actions with explicit failure behavior

The current codebase is still a bounded single-Region slice and does not yet implement the full World Map / cross-Region travel layer.

## Current planning posture

Current implementation sequencing lives in `docs/implementation_roadmap.md`. Use the roadmap's **Current Next Milestone** section for the next proposed implementation slice.

At the post-M11-e baseline:

- preserve the current single-Region slice
- treat Phase 4 / M12-a scenario outcome rules as the next likely step unless the user explicitly chooses a different path
- do not reopen settled battle rules unless explicitly requested
- prefer bounded milestone planning and small consistency cleanups over broad new feature work
- keep future milestone proposals tightly scoped and compatible with the current vertical slice unless the user explicitly widens scope
- respect player-character rules as defined in `docs/core_loop_rules.md`, `docs/combat_rules.md`, and `docs/content_schema.md`
- use `docs/implementation_roadmap.md` for milestone order and explicit “not yet” boundaries

## Avoid

- broad content growth disconnected from the current milestone or task
- extra Regions or World Map systems unless explicitly requested
- major combat redesign unless the task explicitly reopens battle rules
- generic inventory/equipment/event/service frameworks before they are needed
- large architectural rewrites
- premature editor tooling
- mixing input logic with rendering code
- speculative campaign-scale systems that bypass the current slice
- silently ignoring event/action failures
- debug fallback battles as real content behavior
- adding new build/toolchain discovery instructions to docs unless the user asks for build docs specifically

When in doubt, prefer the smallest clean implementation that preserves the existing vertical-slice foundation and strengthens authored progression, consequence, and safe-anchor identity.

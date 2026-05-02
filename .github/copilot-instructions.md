# Project-wide Copilot instructions

Use these rules when working in this repository.

## High-level goals

- Preserve a **small, explicit, maintainable vertical slice**.
- Keep gameplay logic **deterministic, testable, and readable**.
- Extend the game through **bounded milestones**, not broad speculative systems.
- Prefer **clear architecture and content-driven rules** over generic frameworks.
- Treat the repository and the active design docs as the source of truth.

## Terminology source of truth

Before making terminology-sensitive changes, read `docs/terminology_map.md`.

That file defines the current intended design terms and explains how they map to any legacy runtime, content, or serialized names still present in the repository. Prefer the terminology in `docs/terminology_map.md` for design discussion, documentation updates, and new work unless the task is explicitly a legacy-compatibility change.


## Design pillars

- strong scenario and region identity
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

## Combat system implementation

Battle assumptions now treated as settled unless the user explicitly reopens them:

- battle is static formation CTB, not grid combat
- teams field up to 5 active units
- the Leader position is inside the 5, not an extra slot
- only hero units can be leaders
- targeting is free; row depth affects agility penalty rather than target legality
- agility penalty uses the target's effective row depth
- leader aura is a baseline hard rule
- hero HP persists, generic stack HP resets, stack counts persist, and MP persists for all units
- battle UI should favor readable turn-order preview and min/max outcome visibility over exposing hidden math

Detailed combat rules live in `docs/combat_rules.md`.

## Working style

- Make the smallest clean change that solves the task.
- Prefer narrow, test-backed iterations over broad rewrites.
- Call out doc/code mismatches explicitly.
- If a design area is still ambiguous, tighten the docs/vision before implementing broad behavior.
- Do not silently invent large systems when the vision is underspecified.
- When changing durable behavior, update the relevant docs/tests in the same pass.

## Current bounded scope

The current codebase is still a bounded **single-region slice**.

That means:
- do not assume the full World Map / cross-region rules are implemented
- do not assume campaign carry-over is implemented
- do not assume the final out-of-battle party-management UX exists yet
- do not assume all long-term service/storage/recruitment rules already exist in runtime code

However, the long-term terminology and design direction are now more settled and should be used consistently in planning/docs:
- Campaign -> Scenario -> World Map -> Region -> node -> Location
- use `Region` as the main design term instead of `overworld`
- `traveling party` = active party + reserve
- stored units are distinct from reserve
- all traveling heroes cross regions; traveling generic units do not unless stored
- region hero offerings reroll on region entry from heroes who are not traveling, stored, or temporarily unavailable

## Key docs to follow

Read these before planning or making broad changes:

1. `README.md`
2. `README_DECISIONS.md`
3. `docs/game_vision.md`
4. `docs/combat_rules.md`
5. milestone-specific doc/prompt if the task is tied to one
6. `docs/technical_direction.md`
7. `docs/content_scope_v0.md` as a scope cap, not as a checklist of implemented behavior

If those sources disagree:
- prefer the **current codebase** for already implemented behavior
- prefer the **most recently settled design docs** for long-term design intent
- flag the mismatch explicitly rather than guessing

## Document precedence

Use this order when there is ambiguity:

1. current codebase for already implemented behavior
2. explicit current-task requirements from the user
3. active milestone doc for the task, if one exists
4. `README_DECISIONS.md`
5. `docs/game_vision.md`
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
- typed Regions, Locations, battle scenarios, Services, quests, and content definitions
- route-aware and blocker-aware Region travel inside the current single-Region slice
- save/load for current slice state plus lightweight world/progression/service state
- canonical roster stack/slot model
- persistent owned roster plus active-party state
- battle quantity persistence and battle write-back into roster state
- active party size of 5, with Leader inside the 5
- leader / aura baseline in place
- player team requiring a legal leader
- enemy teams optionally having a leader
- player-character seeding into owned roster and active-party legality protections
- player-character recovery to 1 HP on allied win if KO'd
- KO non-player heroes leaving the party on allied win if not revived before battle end
- post-M8 design clarification across:
  - World Map / Region terminology
  - single-purpose Region nodes
  - enemy-team competition and sabotage
  - event-driven progression
  - quest services / victory / defeat / guidance
  - economy / trader / mines / artifacts / items

The current codebase is still a bounded single-region slice and does not yet implement the full World-Map / cross-region travel layer.

## Current planning posture

There is no new implementation milestone locked yet beyond M8.

Until a new milestone is explicitly chosen:

- preserve the current post-M8 baseline
- treat `docs/combat_rules.md`, `docs/game_vision.md`, `docs/core_loop_rules.md`, and `README_DECISIONS.md` as the authoritative design baseline
- do not reopen settled battle rules unless explicitly requested
- prefer vision tightening, bounded milestone planning, and small consistency cleanups over broad new feature work
- keep future milestone proposals tightly scoped and compatible with the current single-region vertical slice, unless the user explicitly chooses to widen scope
- when discussing future world-map/region/party systems, use the settled terminology and party/storage model rather than older region-local-party assumptions

## Avoid

- broad content growth disconnected from the current milestone or task
- extra regions or world-map systems unless explicitly requested
- major combat redesign unless the task explicitly reopens battle rules
- generic inventory/equipment/event/service frameworks before they are needed
- large architectural rewrites
- premature editor tooling
- mixing input logic with rendering code
- speculative campaign-scale systems that bypass the current slice

When in doubt, prefer the smallest clean implementation that preserves the existing vertical-slice foundation and strengthens authored progression, consequence, and safe-anchor identity.

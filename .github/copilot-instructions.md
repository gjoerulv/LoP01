# Project-wide Copilot instructions

Use these rules when working in this repository.

## High-level goals

- Preserve a small, explicit, maintainable vertical slice.
- Keep gameplay logic deterministic, testable, and readable.
- Extend the game through bounded milestones, not broad speculative systems.
- Prefer clear architecture and content-driven rules over generic frameworks.
- Treat the repository and the active design docs as the source of truth.

## Current baseline

The current codebase should be treated as a post-M17 bounded multi-Region, multi-Scenario vertical slice.

Completed foundations include:

- explicit `App` / `GameSession` flow;
- controller / mapper / renderer split;
- typed Regions, Locations, Services, quests, events, units, battles, items, artifacts, World Map, scenarios, campaigns, resources, owned services, mine outputs, unit mine-production passives, and trader ownership curves;
- route-aware Region travel and hostile-occupation blocking;
- enemy-team Region-layer foundation through the practical M11-e slice;
- deterministic scenario outcome rules with authored victory and defeat conditions;
- persistent roster, active party, reserve, battle write-back, and save/load;
- inventory and artifact foundation, including per-hero equipment and battle stat bonuses;
- team Energy pool with daily reset, spend/recover primitives, save/load, and HUD exposure;
- minimal World Map region-to-region travel from authored exit nodes;
- minimal Campaign System with thin scenarios, transition graph, explicit carry-over, and campaign selection;
- owned-service/economy foundation with resource pool, owned-service runtime state, stack-backed stationing, day-boundary mine payout, trader ownership tiers, authored/default trader curves, validation, and tests.

Do not describe the project as post-M8, post-M11, post-M16, or a single-Region-only slice. Those were older baselines.

## Current planning posture

Current implementation sequencing lives in `docs/implementation_roadmap.md`. The next planned milestone is M18: Passive Effect Spine, unless the user explicitly redirects.

M18 should generalize only the passive/effect seams that have immediate consumers. It must not become a broad skill tree, broad status system, full AI economy, full storage overhaul, or large content expansion.

## Terminology source of truth

Before making terminology-sensitive changes, read `docs/terminology_map.md`.

Use current design terminology in discussion, docs, and new code where practical:

- Campaign
- Scenario
- World Map
- Region
- node
- Location
- Service
- Scenario Info screen
- Adventure button strip
- traveling party
- player character
- Scenario Region Context

Runtime code and serialized values may still contain legacy names such as `overworld`, `overworld_mode`, or `overworld_selection`. Treat those as compatibility names, not current design language.

Do not reintroduce old terms as current design truth.

## Design pillars

- strong scenario and region identity;
- readable tactical CTB combat;
- meaningful route, time, Energy, service, and resource planning;
- durable party consequence;
- restoration and safe-anchor fantasy;
- explicit, data-driven gameplay rules.

## Technical rules

Read `docs/technical_direction.md` before architecture or system work.

- Keep gameplay rules out of rendering and input layers.
- Keep input polling and translation in `src/app`, not in renderers.
- Keep renderers focused on drawing from view/state models.
- Keep pure logic pure where practical.
- Prefer explicit state and explicit transitions over hidden chaining.
- Preserve clear ownership and leak-resistant C++ code.
- Avoid unnecessary allocations, repeated parsing, repeated graph rebuilds, and per-frame scans.
- Keep authored static content separate from runtime mutable state.
- Prefer incremental schema evolution over ad hoc special cases.
- Do not add generic engine infrastructure before the current milestone needs it.

## Source comments

Production source comments should document durable contracts, not milestone bookkeeping. Prefer no comment over a comment that merely says which milestone or phase introduced code.

Use comments for:

- non-obvious invariants;
- correctness, security, data-integrity, save/load, or compatibility traps;
- performance-sensitive choices;
- deliberate limitations that future changes might accidentally violate.

Avoid comments such as `M17 Phase 3a:` in production source unless they are temporary and removed before merge. Put milestone context in docs, decisions, commits, prompts, and tests instead.

Test comments are acceptable when they explain a non-obvious regression or scenario intent.

## Combat system implementation

Battle assumptions are settled unless the user explicitly reopens them:

- battle is static formation CTB, not grid combat;
- teams field up to 5 active units;
- the Leader position is inside the 5, not an extra slot;
- only hero units can be leaders;
- targeting is free;
- row depth affects agility penalty rather than target legality;
- agility penalty uses the target's effective row depth;
- leader aura is a baseline hard rule;
- hero HP persists, generic stack HP resets, stack counts persist, and MP persists for all units;
- battle UI should favor readable turn-order preview and min/max outcome visibility over exposing hidden math.

Detailed combat rules live in `docs/combat_rules.md`.

## Owned service and economy rules

When touching owned services or economy systems, follow `docs/core_loop_rules.md`, `docs/content_schema.md`, `docs/content_scope_v1.md`, and `docs/implementation_roadmap.md`.

Settled rules:

- Mines can be owned and generate passive daily resources for the owning team.
- Stationed guards can increase mine output only through explicit passive skills/effects.
- Mine production passives do not stack.
- For each owned service instance and output resource, only the strongest applicable stationed passive counts.
- Ties do not stack; `+2` and `+2` still gives only `+2`.
- Heroes and generic units are both valid stationed units if they have the applicable passive.
- Trader services can be owned: Trading Post, Market, Freelancer's Guild, Black Market.
- Ownership benefits are per service type.
- Owning more Markets affects Market pricing only; owning more Trading Posts affects Trading Post rates only.
- Ownership benefits apply only when the owning team uses a service of the same type that it owns.
- Allied-owned services do not count.
- Ownership tiers cap at 8 owned services of the same type.
- Resource exchange uses an authored matrix per tier where supported.
- Other services use service-type-specific authored/default curves.
- Ownership does not bypass lock, destruction, hostile occupation, stock, eligibility, story, or service availability rules.

The current runtime implements the narrow foundation for those rules. Do not assume full trader UI, item-market transactions, AI economy, ownership-transfer loops, or a broad passive/skill system exist.

## Key docs to follow

Read these before planning or making broad changes:

1. `README.md`
2. `README_DECISIONS.md`
3. `docs/implementation_roadmap.md`
4. `docs/content_scope_v1.md`
5. `docs/technical_direction.md`
6. `docs/game_vision.md`
7. `docs/game_shell_flow.md`
8. `docs/presentation_game_feel.md`
9. `docs/core_loop_rules.md`
10. `docs/combat_rules.md`
11. `docs/scenario_authoring.md`
12. `docs/validation_system.md`
13. `docs/content_schema.md`
14. `docs/terminology_map.md`
15. milestone-specific docs or prompts if the task is tied to one.

Archived docs, including `docs/content_scope_v0.md.archived` and `docs/implementation_roadmap.md.00.archived`, are historical context only. Do not use archived files as active scope caps, current implementation baselines, or behavior specs.

## Document precedence

When sources disagree:

1. current codebase for already implemented behavior;
2. explicit current-task requirements from the user;
3. active milestone doc for the task, if one exists;
4. `docs/implementation_roadmap.md` for sequencing and explicit not-yet boundaries;
5. `docs/content_scope_v1.md` for current content-scope limits;
6. `README_DECISIONS.md` for durable decisions;
7. `docs/game_vision.md`;
8. `docs/game_shell_flow.md`;
9. `docs/presentation_game_feel.md`;
10. `docs/core_loop_rules.md` and `docs/combat_rules.md`;
11. `docs/technical_direction.md`;
12. `docs/content_schema.md` and `docs/validation_system.md`;
13. `docs/terminology_map.md` for terminology conflicts;
14. archived docs/prompts as historical context only.

If the conflict affects behavior, stop and report it instead of guessing.

## Working style

- Make the smallest clean change that solves the task.
- Prefer narrow, test-backed iterations over broad rewrites.
- Move faster than the early M12-M17 micro-slices only when the phase remains coherent and testable.
- Call out doc/code mismatches explicitly.
- If a design area is still ambiguous, tighten the docs/vision before implementing broad behavior.
- Do not silently invent large systems when the vision is underspecified.
- When changing durable behavior, update the relevant docs/tests in the same pass.
- Avoid demo-specific source branches; prove systems through generic data and tests.
- Preserve save/load compatibility unless the task explicitly includes migration work.

## Avoid

- broad content growth disconnected from the current milestone;
- extra campaign/scenario/world content volume before system foundations need it;
- major combat redesign unless the task explicitly reopens battle rules;
- generic inventory/equipment/event/service frameworks beyond current milestone needs;
- large architectural rewrites;
- premature editor tooling;
- mixing input logic with rendering code;
- speculative campaign-scale systems that bypass the current slice;
- performance-hostile patterns such as repeated per-frame content scans, repeated parsing, avoidable large copies, or unnecessary graph rebuilds;
- stale, redundant, or milestone-specific source comments.

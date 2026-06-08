# Project-wide Copilot instructions

Use these rules when working in this repository.

## High-level goals

- Preserve a small, explicit, maintainable vertical slice.
- Keep gameplay logic deterministic, testable, and readable.
- Extend the game through bounded milestones, not broad speculative systems.
- Prefer clear architecture and content-driven rules over generic frameworks.
- Treat the repository and the active design docs as the source of truth.

## Current baseline

The current codebase should be treated as a **post-M22** bounded multi-Region, multi-Scenario vertical slice. Completed foundations include battle, roster, save/load, content validation, typed events, enemy teams, scenario outcomes, a dedicated Scenario Result screen, inventory/artifacts, Energy, World Map, Campaign, owned-service/economy, the narrow unit passive-effect spine, Trading Post transaction rules/APIs, bounded Trading Post interaction flow, and Scenario-authored player economy/service start state.

Do not describe the project as post-M8, post-M11, post-M16, post-M17, post-M18, post-M19, post-M20, post-M21, or a single-Region-only slice. Those were older baselines.

## Current milestone

Current implementation sequencing lives in `docs/implementation_roadmap.md`.

Latest completed milestone: **M22 — Scenario Result Presentation Flow**.

Next selected milestone (planned, not yet implemented): **M23 — Owned Service Claiming and Contesting Foundation** (see `docs/implementation_roadmap.md` §4). Do not assume any milestone beyond M23; the one after it should begin with a fresh audit of active roadmap/docs/source.

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

Production source comments should document durable contracts, not milestone bookkeeping. Prefer no comment over a comment that merely says which milestone or phase introduced code. Use comments for non-obvious invariants, correctness traps, data-integrity/save-load traps, compatibility behavior, performance-sensitive choices, or deliberate limitations. Avoid comments such as `M22 Phase 1:` in production source unless they are temporary and removed before merge. Test comments are acceptable when they explain a non-obvious regression or scenario intent.

## Economy, Trading Post, passive-effect, Scenario start-state, and result-flow rules

When touching owned services, economy systems, unit passive effects, Trading Post interaction/transactions, Scenario start-state, or Scenario result presentation, follow `docs/core_loop_rules.md`, `docs/scenario_authoring.md`, `docs/content_schema.md`, `docs/content_scope_v1.md`, and `docs/implementation_roadmap.md`.

Settled rules:

- Mines can be owned and generate passive daily resources for the owning team.
- Stationed guards can increase mine output only through explicit `mine_production` passive effects.
- Mine production passives do not stack; strongest-only per service instance and output resource.
- `leader_energy` passive effects apply only from the current leader and feed the daily Energy passive term.
- Artifact `statBonus` remains on the artifact battle-stat path; artifact Energy and item effects are deferred.
- Trader-service ownership benefits are per service type and only apply when using an owned same-type service.
- Ownership does not bypass lock, destruction, hostile occupation, stock, eligibility, story, or service availability rules.
- Trading Post transactions are exposed through a bounded Location-mode interaction.
- Resource and Gold mutation must route through existing GameSession resource APIs.
- The 20-minute Trading Post visit cost is charged once on exit only if at least one trade succeeded.
- Scenario `playerStart` can author starting Gold, non-Gold resources, and initial player-owned service state.
- `playerStart.gold` and top-level `startGold` are aliases; authoring both is invalid.
- Scenario start-state applies to runtime `GameSession` state at Scenario start. It must not mutate content definitions.
- Scenario Result mode presents latched deterministic outcomes and next-step text before campaign/terminal progression continues.
- Scenario Result mode is transient; save/load is suppressed while it is active.

Do not assume Market/Black Market/Freelancer's Guild behavior, full item-market transactions, AI economy, ownership-transfer loops, broad Scenario/team/roster authoring, broad passive/skill systems, campaign branch-choice UI, result scores/rewards/fanfare, or post-victory event chains exist.

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

Archived docs are historical context only. Do not use archived files as active scope caps, current implementation baselines, or behavior specs.

## Working style

- Make the smallest clean change that solves the task.
- Prefer narrow, test-backed iterations over broad rewrites.
- Move faster than early micro-slices only when the phase remains coherent and testable.
- Call out doc/code mismatches explicitly.
- If a design area is still ambiguous, tighten the docs/vision before implementing broad behavior.
- When changing durable behavior, update relevant docs/tests in the same pass.
- Avoid demo-specific source branches; prove systems through generic data and tests.
- Preserve save/load compatibility unless the task explicitly includes migration work.

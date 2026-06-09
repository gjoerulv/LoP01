# Project-wide Copilot instructions

Use these rules when working in this repository.

## High-level goals

- Preserve a small, explicit, maintainable vertical slice.
- Keep gameplay logic deterministic, testable, and readable.
- Extend the game through bounded milestones, not broad speculative systems.
- Prefer clear architecture and content-driven rules over generic frameworks.
- Treat the repository and the active design docs as the source of truth.

## Current baseline

The current codebase should be treated as a **post-M25** bounded multi-Region, multi-Scenario vertical slice.

Completed foundations include battle, roster, save/load, content validation, typed events, runtime enemy-team spawning, scenario outcomes, a dedicated Scenario Result screen, inventory/artifacts, Energy, World Map, Campaign, owned-service/economy, the narrow unit passive-effect spine, Trading Post transaction rules/APIs, bounded Trading Post interaction flow, Scenario-authored player economy/service start state, in-play owned-service claiming/contesting after defeating hostile guards, v1 strategic-economy proof content, and player-facing mine stationing/unstationing.

Do not describe the project as post-M24 or earlier. Those were older baselines.

## Current milestone

Current implementation sequencing lives in `docs/implementation_roadmap.md`.

Latest completed milestone: **M25 — Player-facing Service Stationing Flow**.

Current next milestone: **not yet selected** (see `docs/implementation_roadmap.md` §5 for candidates). Active scope cap: `docs/content_scope_v2.md`.

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

Production source comments should document durable contracts, not milestone bookkeeping. Prefer no comment over a comment that merely says which milestone or phase introduced code. Use comments for non-obvious invariants, correctness traps, data-integrity/save-load traps, compatibility behavior, performance-sensitive choices, or deliberate limitations. Avoid comments such as `M25 Phase 1:` in production source unless they are temporary and removed before merge.

Test comments are acceptable when they explain a non-obvious regression or scenario intent.

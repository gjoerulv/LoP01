# Project-wide Copilot instructions

Use these rules when working in this repository.

## High-level goals

- Preserve a small, explicit, maintainable vertical slice.
- Keep gameplay logic deterministic, testable, and readable.
- Extend the game through bounded milestones, not broad speculative systems.
- Prefer clear architecture and content-driven rules over generic frameworks.
- Treat the repository and the active design docs as the source of truth.

## Current baseline

The current codebase should be treated as a **post-M28** bounded multi-Region, multi-Scenario vertical slice.

Completed foundations include battle, roster, save/load, content validation, typed events, runtime enemy-team spawning, scenario outcomes, a dedicated Scenario Result screen, inventory/artifacts, Energy, World Map, Campaign, owned-service/economy, the narrow unit passive-effect spine, Trading Post transaction rules/APIs, bounded Trading Post interaction flow, Scenario-authored player economy/service start state, in-play owned-service claiming/contesting after defeating hostile guards, v1 strategic-economy proof content, player-facing mine stationing/unstationing, general player-side owned-service claiming on legal node entry, a bounded read-only owned-service overview / strategic service readout panel, and a bounded unit-storage foundation (store/retrieve at an owned storage service).

Do not describe the project as post-M27 or earlier. Those were older baselines.

## Current milestone

Current implementation sequencing lives in `docs/implementation_roadmap.md`.

Latest completed milestone: **M28 — Storage Foundation**.

Current selected milestone: **M29 — Cross-Region Generic Unit Preservation / Travel Warning** (planned, not implemented).

Active scope cap: `docs/content_scope_v2.md`.

M28 shipped a bounded **Storage** foundation — a placement bucket DISTINCT from M25 mine stationing (storage cap 7, units persist/retrieve; "garrison" is not a separate system, it is M25's stationed guards). Owned non-Player-Character stacks store/retrieve at a player-owned `Storage`-kind service behind explicit `GameSession` methods, preserving the one-place-at-a-time invariant; additive `stored_units` save (no schema bump); `home_base_storage` is player-owned via `playerStart`; the M27 overview shows a read-only `Stored n/7` row. Do not treat M29 generic-unit travel loss, storage/garrison **defense** (gate defense, stationed-defender combat, storage loss/capture), enemy-side capture, or service destruction/restoration as implemented.

## M29 planning boundary

M29 should connect Storage to the final cross-Region travel rule: warn before Region-to-Region travel would lose generic stacks in the traveling party, remove only those traveling generics on confirmed travel, and leave stored units untouched. Do not add remote storage management, service defense/capture/loss, enemy-side capture, or broad shell/World Map rewrites under M29.

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

Use comments for non-obvious invariants, correctness traps, data-integrity/save-load traps, compatibility behavior, performance-sensitive choices, or deliberate limitations. Avoid comments such as `M25 Phase 1:` in production source unless they are temporary and removed before merge. Test comments are acceptable when they explain a non-obvious regression or scenario intent.

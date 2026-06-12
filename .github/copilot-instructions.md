# Project-wide Copilot instructions

Use these rules when working in this repository.

## High-level goals

- Preserve a small, explicit, maintainable vertical slice.
- Keep gameplay logic deterministic, testable, and readable.
- Extend the game through bounded milestones, not broad speculative systems.
- Prefer clear architecture and content-driven rules over generic frameworks.
- Treat the repository and the active design docs as the source of truth.

## Current baseline

The current codebase should be treated as a **post-M29** bounded multi-Region, multi-Scenario vertical slice.

Completed foundations include battle, roster, save/load, content validation, typed events, runtime enemy-team spawning, scenario outcomes, a dedicated Scenario Result screen, inventory/artifacts, Energy, World Map, Campaign, owned-service/economy, the narrow unit passive-effect spine, Trading Post transaction rules/APIs, bounded Trading Post interaction flow, Scenario-authored player economy/service start state, in-play owned-service claiming/contesting after defeating hostile guards, v1 strategic-economy proof content, player-facing mine stationing/unstationing, general player-side owned-service claiming on legal node entry, a bounded read-only owned-service overview / strategic service readout panel, a bounded unit-storage foundation, and cross-Region generic-unit travel loss with explicit warning/confirmation.

Do not describe the project as post-M28 or earlier except in historical/archive context.

## Current milestone

Current implementation sequencing lives in `docs/implementation_roadmap.md`.

Latest completed milestone: **M29 — Cross-Region Generic Unit Preservation / Travel Warning**.

The next milestone is **not yet selected**. Use `docs/implementation_roadmap.md` §4/§5 as the source of truth.

Active scope cap: `docs/content_scope_v2.md`.

M29 implemented the final cross-Region generic-unit travel consequence: confirmed World Map travel removes only traveling generic stacks, where "traveling" means slotted active/reserve stacks. Stored and stationed stacks survive Region travel with their refs intact. Heroes and the Player Character travel. The warning is a two-stage confirm on the World Map screen using `GameSession::PreviewRegionTravelGenericLosses()` as the pure at-risk read; `TravelToRegion` remains the single confirmed-loss mutation owner. No new GameMode or schema bump was added.

M29 deliberately did **not** add remote storage management, generic auto-storage, travel hard-blocks, service defense/capture/loss, enemy-side Region travel, enemy-side capture, service destruction/restoration, or a broad World Map/shell rewrite.

M28 shipped a bounded **Storage** foundation — a placement bucket distinct from M25 mine stationing (storage cap 7, units persist/retrieve; "garrison" is not a separate system, it is M25's stationed guards). Owned non-Player-Character stacks store/retrieve at a player-owned `Storage` service behind explicit `GameSession` methods, preserving one-place-at-a-time; additive `stored_units` save, no schema bump. `LocationServiceKind::Storage` is not in `IsOwnableServiceKind`; storage claimability remains deferred.

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

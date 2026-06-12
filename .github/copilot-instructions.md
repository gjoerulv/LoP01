# Project-wide Copilot instructions

Use these rules when working in this repository.

## High-level goals

- Preserve a small, explicit, maintainable vertical slice.
- Keep gameplay logic deterministic, testable, and readable.
- Extend the game through bounded milestones, not broad speculative systems.
- Prefer clear architecture and content-driven rules over generic frameworks.
- Treat the repository and the active design docs as the source of truth.

## Current baseline

The current codebase should be treated as a **post-M30** bounded multi-Region, multi-Scenario vertical slice.

Completed foundations include battle, roster, save/load, content validation, typed events, runtime enemy-team spawning, scenario outcomes, a dedicated Scenario Result screen, inventory/artifacts, Energy, World Map, Campaign, owned-service/economy, the narrow unit passive-effect spine, Trading Post transaction rules/APIs, bounded Trading Post interaction flow, Scenario-authored player economy/service start state, in-play owned-service claiming/contesting after defeating hostile guards, v1 strategic-economy proof content, player-facing mine stationing/unstationing, general player-side owned-service claiming on legal node entry, a bounded read-only owned-service overview / strategic service readout panel, a bounded unit-storage foundation, cross-Region generic-unit travel loss with explicit warning/confirmation, and the M30 contested-infrastructure loop: deterministic service defense with stationed/stored defenders, storage loss with Temporarily Unavailable heroes, enemy-side service capture pressure, opt-in service destruction/restoration, and a persisted service event log.

Do not describe the project as post-M29 or earlier except in historical/archive context.

## Current milestone

Current implementation sequencing lives in `docs/implementation_roadmap.md`.

Latest completed milestone: **M30 — v2 Completion: Contested Infrastructure, Service State, and Closure Audit**.

`docs/content_scope_v2.md` is complete and ready to archive. The next milestone is **not yet selected**; a `docs/content_scope_v3.md` should exist before one is chosen. Use `docs/implementation_roadmap.md` §4/§5 as the source of truth.

M30 turned the service/storage/stationing loop into a contested infrastructure loop. Service attacks are node-level against player-owned attackable services (mine/trader/storage kinds; never Rest/Shop/Recruit/Muster; never the arrival node). Player absent: the pure deterministic `ServiceDefenseRules` strength comparison (defender wins ties; repelled attacker team defeated; winning attacker captures every eligible service and occupies the node). Player party on the node: the existing interactive battle surface decides via `ApplyServiceDefenseVictory`/`Defeat`. Capture resolves placed stacks atomically — generics dismissed, heroes Temporarily Unavailable (`unavailable_heroes`, weekly return-to-reserve at a day start with a free slot), Player Character never placed/lost/TU, refs cleared in the same mutation, immediate ownership transfer. Enemy pressure runs in `ProcessEnemyPhase` (same node or adjacent, patrol-radius legal, current region only, deterministic targeting, authored `enemyGroupId` strength). Destruction/restoration is opt-in (`destroyable` + validated `restore_cost`; 1000 Energy + 1 hour; restoration queues and completes at next day start before payout; destroy-again cancels). All M30 save fields are additive — no schema bump.

M30 deliberately did **not** add full-simulation defense battles or any battle AI (the strength comparison is a documented stand-in; do not invent a second battle engine), a shared hero pool (the weekly reserve return is the stand-in), enemy-side destruction/sabotage, enemy-vs-enemy service contention, AI economy, remote service management, or a final service-management UI. These are v3 candidates.

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

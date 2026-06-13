# Project-wide Copilot instructions

Use these rules when working in this repository.

## High-level goals

- Preserve a small, explicit, maintainable vertical slice.
- Keep gameplay logic deterministic, testable, and readable.
- Extend the game through bounded milestones, not broad speculative systems.
- Prefer clear architecture and content-driven rules over generic frameworks.
- Treat the repository and the active design docs as the source of truth.

## Current baseline

The current codebase should be treated as a **post-M31** bounded multi-Region, multi-Scenario vertical slice.

Completed foundations include battle, roster, save/load, content validation, typed events, runtime enemy-team spawning, scenario outcomes, a dedicated Scenario Result screen, inventory/artifacts, Energy, World Map, Campaign, owned-service/economy, the narrow unit passive-effect spine, Trading Post transaction rules/APIs, bounded Trading Post interaction flow, Scenario-authored player economy/service start state, player-facing mine stationing/unstationing, Storage, cross-Region generic-unit travel loss with explicit warning/confirmation, the M30 contested-infrastructure loop (deterministic service defense with stationed/stored defenders, storage loss with Temporarily Unavailable heroes, enemy-side service capture pressure, opt-in service destruction/restoration, persisted service event log), and the M31 shell entry flow (main menu, New Game Campaign/Standalone Scenario selection behind a validation/playability gate, bounded single-save Continue, `GameSession::StartStandaloneScenario`).

Do not describe the project as post-M30 or earlier except in historical/archive context.

## Current milestone

Current implementation sequencing lives in `docs/implementation_roadmap.md`.

Active scope: `docs/content_scope_v3.md`.

Latest completed milestone: **M31 — Shell Entry + Scenario/Campaign Selection**.

The next milestone is **not yet selected**. Use `docs/implementation_roadmap.md` §4/§5 as the source of truth.

M31 shipped the first bounded shell/menu and Campaign/Standalone Scenario selection path: `GameMode::Title` hosts an App-local, never-persisted screen state machine; starts go through `GameSession::StartCampaign`/`StartStandaloneScenario` behind a validation gate; Continue is a bounded single-save load with safe failure. It deliberately did not add character creation, settings/mods, save-slot metadata, Scenario Region Context, fog/scouting, threat preview, item economy, or AI economy; starts use the prebuilt default Player Character and keep the M16 roster/clock start-state semantics (see the roadmap's documented limitations).

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

Production source comments should document durable contracts, not milestone bookkeeping.

Prefer no comment over a comment that merely says which milestone or phase introduced code. Use comments for non-obvious invariants, correctness traps, data-integrity/save-load traps, compatibility behavior, performance-sensitive choices, or deliberate limitations.

Test comments are acceptable when they explain a non-obvious regression or scenario intent.

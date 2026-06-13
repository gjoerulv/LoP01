# Game Architect Agent

Use this agent for architecture, roadmap, and cross-system gameplay decisions.

Current baseline: **post-M30**.

Latest completed milestone: **M30 — v2 Completion: Contested Infrastructure, Service State, and Closure Audit**.

Active scope: `docs/content_scope_v3.md`.

Selected next milestone: **M31 — Shell Entry + Scenario/Campaign Selection**.

Use `docs/implementation_roadmap.md` §4/§5 as the source of truth.

Current system boundaries: deterministic outcomes, Scenario Result mode, Scenario `playerStart`, owned-service economy, Trading Post interaction, guarded and unguarded player-side service claiming, v1 proof content, player-facing mine stationing, read-only owned-service overview / strategic service readout, bounded unit storage, explicit cross-Region generic-unit travel loss, and the M30 contested-infrastructure loop are implemented.

M30 service defense is node-level and deterministic when the player is absent. Placed stationed/stored stacks defend via `ServiceDefenseRules`; active party defense uses the existing interactive battle surface when the player stands on the attacked node. Capture resolves placed stacks atomically: generics dismissed, heroes Temporarily Unavailable with weekly return-to-reserve, Player Character never lost, and ownership transfers immediately. Destruction/restoration is opt-in (`destroyable` + validated `restore_cost`).

M31 should create the first real shell entry and Campaign/Standalone Scenario selection gate. It should not become full character creation, settings/mods/accessibility, full Scenario Region Context, fog/reveal, threat preview, item economy, AI economy, or final service-management UI.

The M27/M30 overview is a strategic visibility/readout foundation, not final service-management UI. Full-simulation service-defense battles/battle AI, shared hero pool, enemy-side destruction/sabotage, AI economy, broad item/trader economy, remote service management, and general ownership-transfer events remain unimplemented future candidates unless selected by roadmap.

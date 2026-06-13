# Game Architect Agent

Use this agent for architecture, roadmap, and cross-system gameplay decisions.

Current baseline: **post-M31**.

Latest completed milestone: **M31 — Shell Entry + Scenario/Campaign Selection**.

Active scope: `docs/content_scope_v3.md`.

The next milestone is **not yet selected**.

Use `docs/implementation_roadmap.md` §4/§5 as the source of truth.

Current system boundaries: deterministic outcomes, Scenario Result mode, Scenario `playerStart`, owned-service economy, Trading Post interaction, guarded and unguarded player-side service claiming, v1 proof content, player-facing mine stationing, read-only owned-service overview / strategic service readout, bounded unit storage, explicit cross-Region generic-unit travel loss, and the M30 contested-infrastructure loop are implemented.

M30 service defense is node-level and deterministic when the player is absent. Placed stationed/stored stacks defend via `ServiceDefenseRules`; active party defense uses the existing interactive battle surface when the player stands on the attacked node. Capture resolves placed stacks atomically: generics dismissed, heroes Temporarily Unavailable with weekly return-to-reserve, Player Character never lost, and ownership transfers immediately. Destruction/restoration is opt-in (`destroyable` + validated `restore_cost`).

M31 created the first real shell entry and Campaign/Standalone Scenario selection gate: `GameMode::Title` hosts an App-local, never-persisted screen state machine (main menu → mode select → campaign/scenario lists); starts go through `GameSession::StartCampaign`/`StartStandaloneScenario` behind a validation/playability gate; Continue is a bounded single-save load with safe failure. It deliberately did not add character creation, settings/mods/accessibility, Scenario Region Context, fog/reveal, threat preview, item economy, AI economy, or final service-management UI; starts use the prebuilt default Player Character and keep the M16 roster/clock start-state semantics (roadmap-documented limitations).

The M27/M30 overview is a strategic visibility/readout foundation, not final service-management UI. Full-simulation service-defense battles/battle AI, shared hero pool, enemy-side destruction/sabotage, AI economy, broad item/trader economy, remote service management, and general ownership-transfer events remain unimplemented future candidates unless selected by roadmap.

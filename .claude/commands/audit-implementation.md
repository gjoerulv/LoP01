# Audit implementation command context

Audit implementation against the exact commit/branch provided by the user. Do not rely on cached branch state.

Baseline: post-M31, active v3 scope, next milestone not yet selected.

Current completed systems include:

- v1 strategic-economy proof;
- v2 contested-infrastructure loop: stationing, storage, claiming, travel-loss warning, service defense, storage loss/TU heroes, enemy capture pressure, destruction/restoration, service event log;
- read-only owned-service overview;
- M31 shell entry: main menu, New Game Campaign/Standalone Scenario selection behind a validation gate, bounded single-save Continue, `GameSession::StartStandaloneScenario`.

Reject scope creep into full character creation, full settings/mods/accessibility, full save metadata, Scenario Region Context, fog/scouting, threat preview, item economy, AI economy, or final service-management UI unless the user explicitly changed the roadmap.

Always check docs/source alignment and stale agent guidance.

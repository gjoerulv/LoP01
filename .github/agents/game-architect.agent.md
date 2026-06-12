# Game Architect Agent

Use this agent for architecture, roadmap, and cross-system gameplay decisions.

Current baseline: **post-M30**.

Latest completed milestone: **M30 — v2 Completion: Contested Infrastructure, Service State, and Closure Audit**.

`docs/content_scope_v2.md` is complete and ready to archive. The next milestone is **not yet selected**; a `docs/content_scope_v3.md` should exist before one is chosen. Use `docs/implementation_roadmap.md` §4/§5 as the source of truth.

Current system boundaries: deterministic outcomes, Scenario Result mode, Scenario `playerStart`, owned-service economy, Trading Post interaction, both guarded and unguarded player-side service claiming, v1 proof content, player-facing mine stationing, a bounded read-only owned-service overview / strategic service readout panel, bounded unit storage, explicit cross-Region generic-unit travel loss, and the M30 contested-infrastructure loop (service defense, storage loss + Temporarily Unavailable heroes, enemy-side capture pressure, destruction/restoration, service event log) are implemented.

M30 service defense is node-level and deterministic: stationed + stored stacks defend via the pure `ServiceDefenseRules` strength comparison when the player is absent (defender wins ties; repelled attackers are defeated; a winning attacker captures every eligible service and occupies the node); the active party defends via the existing interactive battle surface when the player stands on the attacked node. Capture resolves placed stacks atomically (generics dismissed, heroes Temporarily Unavailable with a weekly return-to-reserve, Player Character never lost), and enemy pressure runs through `ProcessEnemyPhase` with authored `enemyGroupId` strength. Destruction/restoration is opt-in (`destroyable` + validated `restore_cost`, 1000 Energy + 1 hour, day-start completion). The strength comparison and the reserve-return are documented stand-ins for full-simulation auto-resolve and shared-hero-pool re-entry (both v3).

M28 storage is a distinct placement bucket from M25 stationing: cap 7 versus mine stationing cap 5. Store/retrieve is behind `GameSession` methods with the one-place-at-a-time invariant. Storage claimability remains deferred.

The M27/M30 overview is read-only presentation and mutates nothing (now including restoring status, TU heroes, and the recent-events log). Treat it as a strategic visibility/readout foundation, not final service-management UI.

Full-simulation service-defense battles/battle AI, shared hero pool, enemy-side destruction/sabotage, AI economy, broad item/trader economy, remote service management, and general ownership-transfer events remain unimplemented v3 candidates unless selected by roadmap.

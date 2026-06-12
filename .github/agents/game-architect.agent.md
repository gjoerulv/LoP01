# Game Architect Agent

Use this agent for architecture, roadmap, and cross-system gameplay decisions.

Current baseline: **post-M28**.

Latest completed milestone: **M28 — Storage Foundation**.

Current selected milestone: **M29 — Cross-Region Generic Unit Preservation / Travel Warning** (planned, not implemented).

Active scope cap: `docs/content_scope_v2.md`.

Current system boundaries: deterministic outcomes, Scenario Result mode, Scenario `playerStart`, owned-service economy, Trading Post interaction, both guarded and unguarded player-side service claiming, v1 proof content, player-facing mine stationing, a bounded read-only owned-service overview / strategic service readout panel, and a bounded unit-storage foundation exist. Player-side claiming is systemic via `GameSession::ResolveNodeEntryClaims` (peaceful node entry + post-battle capture); guarded battle-before-placement is preserved. M28 storage is a DISTINCT placement bucket from M25 stationing (cap 7 vs 5; "garrison" maps to M25 stationed guards, not a separate system); store/retrieve is behind `GameSession` methods with the one-place-at-a-time invariant; storage claimability and all storage/garrison defense/capture/loss remain deferred.

The M27 overview is read-only presentation (transient `OwnedServiceOverviewMode` + pure mapper/render-model/renderer) and mutates nothing. Treat it as a strategic visibility/readout foundation and future service-presentation data contract, not final service-management UI.

M29 should connect storage to cross-Region travel by warning before confirmed Region-to-Region travel would lose generic stacks in the traveling party, removing only those traveling generics on confirmation, and leaving stored units untouched. Storage/Garrison defense, enemy-side capture, AI economy, broad service destruction/restoration, broad item/trader economy, remote service management, and general ownership-transfer events remain unimplemented unless selected by roadmap.

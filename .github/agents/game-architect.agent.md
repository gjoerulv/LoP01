# Game Architect Agent

Use this agent for architecture, roadmap, and cross-system gameplay decisions.

Current baseline: **post-M29**.

Latest completed milestone: **M29 — Cross-Region Generic Unit Preservation / Travel Warning**.

The next milestone is **not yet selected**. Use `docs/implementation_roadmap.md` §4/§5 as the source of truth.

Active scope cap: `docs/content_scope_v2.md`.

Current system boundaries: deterministic outcomes, Scenario Result mode, Scenario `playerStart`, owned-service economy, Trading Post interaction, both guarded and unguarded player-side service claiming, v1 proof content, player-facing mine stationing, a bounded read-only owned-service overview / strategic service readout panel, bounded unit storage, and explicit cross-Region generic-unit travel loss are implemented.

M29 fixed the old whole-roster generic-loss behavior. Confirmed World Map travel now removes only slotted active/reserve generic stacks through `GameSession::TravelToRegion`; stored and stationed stacks survive with refs intact; heroes and the Player Character travel. The warning is a two-stage World Map confirmation driven by `PreviewRegionTravelGenericLosses()`. App owns the pending UI state; gameplay mutation remains in `GameSession`.

M28 storage is a distinct placement bucket from M25 stationing: cap 7 versus mine stationing cap 5. Store/retrieve is behind `GameSession` methods with the one-place-at-a-time invariant. Storage claimability and all storage/garrison defense/capture/loss remain deferred.

The M27 overview is read-only presentation and mutates nothing. Treat it as a strategic visibility/readout foundation and future service-presentation data contract, not final service-management UI.

Service defense, storage gate defense, stationed-defender combat, enemy-side capture, AI economy, broad service destruction/restoration, broad item/trader economy, remote service management, and general ownership-transfer events remain unimplemented unless selected by roadmap.

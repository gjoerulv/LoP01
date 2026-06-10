# Gameplay instructions

- Treat the project as post-M27.
- Current gameplay foundations include deterministic scenario outcomes, Scenario Result presentation, Scenario `playerStart`, owned-service economy, Trading Post interaction, guarded and unguarded player-side service claiming, v1 strategic-economy proof content, player-facing mine stationing/unstationing, and a bounded read-only owned-service overview / strategic service readout panel.
- The next milestone is not yet selected (see `docs/implementation_roadmap.md` §5).
- M27 owned-service overview is READ-ONLY presentation (transient `OwnedServiceOverviewMode`, opened with `O` from Region mode): a pure mapper/render-model/renderer over existing `GameSession` accessors. It must not mutate ownership, stationing, or payout; stationing stays reachable through the mine Location-zone interaction. `GameSession::PreviewMineDailyOutput` reuses the exact payout rules and must equal the payout delta for a payable mine.
- Treat the M27 overview as a strategic visibility/readout foundation, not final service-management UI. Do not add remote stationing, Storage/Garrison, service repair/destruction, ownership transfer, or other service-management actions to it unless a scoped roadmap milestone explicitly selects that work.
- Ownership and guarding are related but separate: an owned service does not have to be guarded, and a guarded service is only one capture path.
- Claiming is systemic: legally entering an unguarded node claims its eligible ownable services via `GameSession::ResolveNodeEntryClaims` (the single claim path; `ClaimContestedServicesAtNode` is a back-compat alias), wired into `App::OnDestinationArrived` and the post-battle victory path. It is a no-op while the node is hostile-occupied and skips player-owned/allied services (re-entry never clears the player's stationed units).
- Hostile-occupied travel may start battle before the player team is placed on the target node. Preserve that behavior; victory resolves capture for that node exactly once, and the player does not move onto it or spend extra travel/Energy/time.
- Claimed services mutate runtime owned-service state only; content definitions must not be mutated.
- `mine_production` is player-facing: M25 added a bounded stationing flow at player-owned mines (physical one-place-at-a-time placement behind `GameSession`, mines only). Stationing is guard/worker capacity only — not Storage/Garrison.
- Do not add enemy-side capture, AI economy, service destruction/restoration, Market/Black Market/Freelancer behavior, Storage/Garrison, or broader v2 systems without an explicit roadmap milestone.
- Gameplay-rule changes must be tested in pure/unit tests where practical and integration/end-to-end tests where the rule crosses systems.

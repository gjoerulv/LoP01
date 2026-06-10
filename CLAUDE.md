# Claude Guidance for Ashvale

## Current baseline

Treat the repository as a **post-M27** C++20 / raylib / CMake game project.

Completed foundations include battle, roster, save/load, Region/Location flow, content validation, typed events, runtime enemy-team spawning, scenario outcomes, a dedicated Scenario Result screen, inventory/artifacts, Energy, World Map, Campaign, owned-service/economy systems, the narrow unit passive-effect spine, Trading Post transaction rules/APIs, bounded Trading Post interaction flow, Scenario-authored player economy/service start state, in-play owned-service claiming/contesting after defeating hostile guards, v1 strategic-economy proof content, player-facing mine stationing/unstationing, general player-side owned-service claiming on legal node entry, and a bounded read-only owned-service overview / strategic service readout panel.

Latest completed milestone: **M27 — Owned Service Overview / Strategic Service Readout**.

Active scope cap: **`docs/content_scope_v2.md`**.

Current selected milestone: **not yet selected**. Candidate v2 directions are listed in `docs/implementation_roadmap.md` §5 and `docs/content_scope_v2.md` §5. Do not treat enemy-side capture, service destruction/restoration, Storage/Garrison, or other v2 expansion items as already implemented.

## Required reading

Before architecture, roadmap, economy, Scenario, content, UI, or rendering work, read:

1. `README.md`
2. `README_DECISIONS.md`
3. `docs/implementation_roadmap.md`
4. `docs/content_scope_v2.md`
5. `docs/technical_direction.md`
6. `docs/game_vision.md`
7. `docs/game_shell_flow.md`
8. `docs/presentation_game_feel.md`
9. `docs/core_loop_rules.md`
10. `docs/scenario_authoring.md`
11. `docs/content_schema.md`
12. `docs/validation_system.md`
13. `docs/terminology_map.md`

Archived files, including `docs/content_scope_v1.md` once archived by the user, are historical context only. Do not use archived roadmap/scope files as current requirements.

## Working rules

- Keep implementation slices narrow, test-backed, and aligned with the active roadmap.
- If docs and source disagree, stop and report the mismatch.
- Keep gameplay rules out of rendering/input layers.
- Keep authored static content separate from runtime mutable state.
- Preserve save/load compatibility unless the task explicitly includes migration work.
- Avoid per-frame scans, repeated content parsing, graph rebuilds, large needless copies, and hidden nested scans.
- Avoid demo-specific source branches; prove systems through generic data and tests.
- Do not introduce broad frameworks before a scoped consumer needs them.

## Source comments

Production source comments should document durable contracts, not milestone bookkeeping. Avoid comments such as `M25 Phase 1:` in production source.

Use comments only for non-obvious invariants, validation traps, save/load contracts, compatibility behavior, performance-sensitive choices, or deliberate limitations. Test comments are acceptable when they explain non-obvious regression intent.

## Current settled system boundaries

- Scenario outcome rules are deterministic and latched through `GameSession`; presentation uses the dedicated transient Scenario Result mode.
- Scenario Result mode is not normal save/load progression. Save/load is suppressed while it is active, and accidental serialized `scenario_result` mode self-heals to Region mode.
- `playerStart` is the Scenario-authored surface for starting Gold, non-Gold resources, and initial player-owned service state.
- `playerStart.gold` is an alias for legacy top-level `startGold`; authoring both is invalid.
- Scenario start-state applies to runtime `GameSession` state when a Scenario starts; it is not persisted back into content.
- Gold remains a single source of truth through the existing `gold_` / `ResourceType` delegation path.
- Runtime owned-service state is mutable. Shipped code proves both guarded-service claiming (after defeating a hostile guard) and general player-side claiming on legal node entry.
- Owned services do not have to be guarded. The guarded-capture path is not a universal guard requirement.
- M26 added the peaceful/unguarded player-side claim path: legally entering a node claims its eligible ownable services via `GameSession::ResolveNodeEntryClaims` (the single claim path used for both peaceful entry and post-battle capture; `ClaimContestedServicesAtNode` is a back-compat alias).
- `ResolveNodeEntryClaims` is a no-op while the node is hostile-occupied, skips player-owned/allied services (so re-entry never clears the player's stationed units), and mutates runtime `OwnedServiceSaveState` only. The App wires it in `OnDestinationArrived` (peaceful) and the post-battle victory path; no save schema bump.
- Hostile-occupied travel may start battle before the moving player team is placed on the destination node. This is intended final-direction behavior, preserved by M26; the guarded node is claimed once after victory and the player does not move onto it or spend extra travel/Energy/time.
- Claiming mutates runtime owned-service state only; content definitions are never mutated.
- M26 is not enemy-side capture, service destruction/restoration, Storage/Garrison, or a general ownership-transfer event system.
- World-map-arrival and location-mode-entry claiming are out of M26 scope (only intra-region travel arrival claims).
- M27 added a bounded, READ-ONLY owned-service overview / strategic service readout: a transient `GameMode::OwnedServiceOverviewMode` opened with `O` from Region mode, rendered by a pure `OwnedServiceOverviewModelMapper` → render-model → `OwnedServiceOverviewRenderer`.
- The M27 panel lists only player-owned services with location/region, kind, owner/status, stationed `count/5` + unit names, a daily-output preview, and Trading Post tier. It mutates nothing (no ownership/stationing/payout changes); M25 stationing stays reachable through the mine Location-zone interaction, not the panel.
- The M27 overview is an early strategic visibility surface and read-model foundation. It is not the final service-management UI and must not grow into remote stationing, storage, garrison management, repair/destruction, or ownership-transfer UI without a scoped milestone.
- `OwnedServiceOverviewMode` is never persisted (`ToString` is diagnostic-only; `FromString("owned_service_overview")` self-heals to `RegionMode`; App suppresses save/load while open); no schema bump.
- `GameSession::PreviewMineDailyOutput(serviceId)` is a pure read used by the overview; it reuses the exact payout rules (base + strongest-only stationed `mine_production`) and equals the daily payout delta for a payable mine. It does not apply the payability gate (lock/destroy/occupation) — that is shown as status, not folded into the number. `ApplyDailyMinePayout` is unchanged.
- Allied ownership does not grant player benefits and is not claimable in the current player-side claim path.

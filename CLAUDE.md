# Claude Guidance for Ashvale

## Current baseline

Treat the repository as a **post-M30** C++20 / raylib / CMake game project.

Completed foundations include battle, roster, save/load, Region/Location flow, content validation, typed events, runtime enemy-team spawning, scenario outcomes, a dedicated Scenario Result screen, inventory/artifacts, Energy, World Map, Campaign, owned-service/economy systems, the narrow unit passive-effect spine, Trading Post transaction rules/APIs, bounded Trading Post interaction flow, Scenario-authored player economy/service start state, in-play owned-service claiming/contesting after defeating hostile guards, v1 strategic-economy proof content, player-facing mine stationing/unstationing, general player-side owned-service claiming on legal node entry, a bounded read-only owned-service overview / strategic service readout panel, a bounded unit-storage foundation (store/retrieve at an owned storage service), cross-Region generic-unit travel loss with an explicit warning/confirmation (traveling generics only; stored/stationed stacks survive), and the M30 contested-infrastructure loop: deterministic service defense with stationed/stored defenders, storage loss with Temporarily Unavailable heroes, enemy-side service capture pressure, opt-in service destruction/restoration, and a persisted service event log with overview presentation.

Latest completed milestone: **M30 — v2 Completion: Contested Infrastructure, Service State, and Closure Audit**.

`docs/content_scope_v2.md` is **complete and ready to archive**. The next milestone is **not yet selected**; create `docs/content_scope_v3.md` before selecting one. Do not treat the documented v3 candidates — full-simulation service-defense battles/battle AI, shared hero pool with TU pool re-entry, enemy-side destruction/sabotage, AI economy, item economy, fog-of-war, shell flow, final service-management UI — as already implemented.

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
- M28 added a bounded **Storage** foundation, a DISTINCT placement concept from M25 stationing (pressure-tested against `core_loop_rules` §4/§21 + `game_vision` §5: "garrison" is not a separate final-vision system — it is M25's stationed guards). Storage holds owned non-Player-Character stacks at a player-owned `Storage`-kind service, capacity **7** (`StorageRules::kMaxStoredUnitsPerService`, separate from the mine cap of 5). Units persist (don't travel) and are retrievable. Defense/capture/loss/garrison combat are deferred.
- Storage uses its own `core::StoredUnitSaveState` + additive `OwnedServiceSaveState.storedUnits` (`stored_units`; absent → empty; no schema bump). Mutations live only behind `GameSession` (`TryStoreStackAtService`, `TryRetrieveStackFromService`, `CanStore…`/`CanOpenStorageAtService`, `EligibleStorageStackIds`, `NormalizeStoredUnits`). Store requires the stack be slotted (so a stationed stack can't be stored and a stored stack can't be stationed — automatic cross-exclusion); retrieve returns the same stack id to reserve, fails atomically when reserve is full, and heals a corrupt slotted+stored double-placement by dropping only the stored ref. One-place-at-a-time and no-duplication/no-loss invariants span active/reserve/stationed/stored.
- `LocationServiceKind::Storage` is intentionally NOT in `IsOwnableServiceKind`, so M26 claiming is unchanged; storage claimability is deferred. The one shipped `home_base_storage` service is player-owned via `playerStart` (the `playerStart` ownable-kind check was extended to accept storage). The M27 overview shows a read-only `Stored n/7` row for storage services; no management actions there.
- M29 implemented the final cross-Region generic-unit travel consequence. The at-risk set is exactly the traveling party: slotted (active/reserve) generic stacks. `GameSession::PreviewRegionTravelGenericLosses()` is the pure at-risk read, and the confirmed-travel removal inside `GameSession::TravelToRegion` deletes exactly that set, so M25-stationed and M28-stored stacks survive Region travel with their refs intact. (This deliberately fixed the M15-era removal, which scanned the whole roster and would have deleted placed stacks once M25/M28 existed.) Heroes (category Hero/Leader) travel; the Player Character is additionally excluded by an explicit `isPlayerCharacter` guard.
- The M29 warning is a two-stage confirm on the existing World Map screen: confirming a legal destination while at-risk generics exist shows a text block listing per-stack `Nx Name` losses; a second confirm commits `TravelToRegion`, while cancel/selection-change dismisses the warning and keeps the screen open so units can be stored first. A no-loss party travels on the first confirm. The pending flag is App-local UI state, never persisted, reset on screen entry/`ResetTransientModeState`; no new GameMode and no schema bump. `TravelToRegion` remains the single loss-resolution owner; direct callers bypass the UI warning by design.
- M29 did not add remote storage management, generic auto-storage, travel hard-blocks, service defense/capture/loss, enemy-side capture, enemy Region travel, or World Map/shell rewrites beyond the confirmation block.
- M30 made infrastructure contested. Service attacks are NODE-level: every eligible player-owned service at the node (attackable kinds = Mine, the four trader kinds, Storage via `economy::ServiceKindIsAttackable`; never Rest/Shop/Recruit/Muster; never the arrival node; never locked/destroyed services) resolves together through `GameSession::ResolveServiceAttack`. Player absent: the pure deterministic `ServiceDefenseRules` strength comparison (unit power = attack + defense + maxHp; stack power × quantity; defender wins ties) — defenders hold ⇒ the attacker team is defeated; attacker wins (or no defenders) ⇒ every eligible service is captured and the attacker occupies the node. Player party standing on the node: the existing interactive battle surface decides, reported back via `ApplyServiceDefenseVictory`/`ApplyServiceDefenseDefeat`. This strength comparison is a documented stand-in for final full-simulation auto-resolve (v3 battle-depth) — there is NO battle AI in the codebase; do not invent a second battle engine.
- Capture resolves placed stacks atomically in `ApplyServiceCaptureAtNode`: generic defender stacks are dismissed (erased), hero defender stacks become Temporarily Unavailable, refs are cleared in the same mutation (no dangling refs, no duplicates, no transfer to the enemy), and `ownerTeamColor` transfers immediately. The Player Character can never be placed, lost, or TU (M25/M28 gates plus an explicit capture-time guard).
- The minimal TU pipeline is additive save state (`unavailable_heroes`: unitId + becameUnavailableDay + returnDay). TU heroes own no roster stack (hidden from all placement surfaces automatically) and return to the player's reserve at a day start ≥ returnDay (weekly, `economy::kUnavailableHeroReturnDays`) when a slot is free — a documented stand-in for shared-hero-pool re-entry (v3). PC entries are healed away on load.
- Enemy pressure lives in `ProcessEnemyPhase`: a player-hostile team standing on or adjacent to an eligible node (patrol-radius enforced, current region only) attacks it instead of patrolling; first target by sorted node id; fixed color order. Teams carry `enemyGroupId` (runtime + additive save + optional `spawnTeam` arg) resolving attack strength from `enemy_groups.json` via the GameSession enemy-group catalog. A player-present target yields a `service_attack_pending_battle` result; the App starts the location's authored battle scenario (missing scenario ⇒ the attack stalls with a status line, mirroring the hostile-travel pattern). Allied teams never attack player services; enemy capture targets player-owned services only (enemy-vs-enemy contention is v3).
- M30 destruction/restoration is opt-in: authored `destroyable` flag + mandatory `restore_cost` (validated: destroyable ⇔ non-empty valid cost). Destroy requires Region mode, standing on the unoccupied node, no placed units at the service, 1000 Energy + 1 hour; destroying a destroyed+queued service cancels the queued restoration at full cost (§20). Restore requires standing on the node, spends the authored cost (no Energy/time), queues, and completes at the next day start BEFORE that day's mine payout. The player surface is a bounded two-press `K` maintenance action in Region mode. Mine payout semantics are unchanged.
- Service events are logged to a bounded persisted `service_event_log` (capped at 24, newest last, additive save; entries use service/location ids). The M27 overview additionally shows a "Destroyed — restoring" status, a Temporarily Unavailable section, and the recent-events section; it remains read-only.
- M30 did NOT implement: full-simulation service-defense battles or any battle AI, partial defender losses, shared hero pool, enemy-side destruction/sabotage/restoration, enemy-vs-enemy service contention, AI economy, item economy, fog-of-war, shell flow, remote management from the overview, or a final service-management UI. These are v3 candidates in `docs/implementation_roadmap.md` §5.
- Allied ownership does not grant player benefits and is not claimable in the current player-side claim path.

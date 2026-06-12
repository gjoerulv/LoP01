# Ashvale Implementation Roadmap

## Context

The current codebase is a **post-M29** bounded multi-Region, multi-Scenario vertical slice.

The v1 strategic-economy proof is complete: the shipped slice and tests exercise Scenario-authored player economy/service start state, owned services, mine payout, narrow unit passive effects, authored Trading Post trade data, guarded and unguarded service claiming, Scenario Result presentation, Campaign progression, and player-facing mine stationing.

The active scope cap is `docs/content_scope_v2.md`. Archived docs, including `docs/content_scope_v0.md.archived`, older `docs/implementation_roadmap.md.*.archived` files, and `docs/content_scope_v1.md` once archived by the user, are historical context only.

## 1. Current implementation baseline

Current stable foundation:

- explicit `App` / `GameSession` flow;
- controller / mapper / renderer split;
- battle engine, CTB, static formation, leader aura, deterministic damage, and battle write-back;
- persistent roster, active/reserve party, mustering, and save/load;
- daily clock, Region travel, wake/recovery penalty, and basic services;
- team Energy pool with daily-starting formula, spend/recover primitives, day-rollover reset, save/load, snapshot/HUD exposure, and current-leader `leader_energy` contribution;
- JSON content loading through `ContentRepository`;
- content validation foundation;
- typed event foundation, including runtime `spawnTeam` creation/reactivation, `removeTeam`, and `changeAlliance`;
- enemy-team Region-layer foundation;
- scenario outcome foundation;
- dedicated Scenario Result mode with outcome label, reason, next-step text, Continue handling, transient save/load policy, mapper, renderer, and tests;
- inventory and artifact foundation with equipped-artifact battle stat bonuses;
- minimal World Map;
- minimal Campaign System;
- owned-service/economy foundation with resources, owned-service runtime state, mine outputs, stack-backed stationing, daily mine payout, trader ownership tiers, authored/default trader curves, validation, and proof tests;
- passive-effect spine foundation with canonical unit `passive_effects`, legacy `mine_production_passive` authoring compatibility, `mine_production` effects, and `leader_energy` effects;
- Trading Post transaction foundation with pure quote rules, `GameSession` transaction APIs, service-specific use/ownership gating, tier-0 fallback/default behavior, Gold delegation, validation, and end-to-end tests;
- Trading Post interaction flow with a bounded Location-mode service interaction, buy/sell/barter modes, live prompt feedback, per-visit time cost, and a small authored playable Home Base Trading Post;
- Scenario-authored player economy/service start state through `playerStart`, including starting Gold, non-Gold resources, and initial player-owned service state applied at Scenario start;
- owned-service claiming/contesting foundation: defeating a hostile team occupying/guarding a node can claim eligible ownable services at that node for the player;
- v1 strategic-economy proof content: shipped `playerStart`, shipped `leader_energy`, shipped `mine_production` authoring, authored Trading Post curve data, guarded Steel Mine claim proof, and tests proving the play-reachable chain plus runtime-stationed mine-production boost;
- player-facing mine stationing flow: a bounded, text-prompt interaction at player-owned mines that stations/unstations/splits eligible owned stacks behind explicit `GameSession` methods (physical one-place-at-a-time placement, Player-Character excluded, up to 5 per mine, no schema bump), making `mine_production` visible in normal play;
- general player-side owned-service claiming: legally entering an unguarded node claims its eligible ownable services immediately via `GameSession::ResolveNodeEntryClaims` (the single claim path, with `ClaimContestedServicesAtNode` as a back-compat alias), wired into `App::OnDestinationArrived` and the post-battle victory path; guarded battle-before-placement preserved; idempotent re-entry never clears the player's stationed units; no schema bump; an unguarded Copper Mine proves the peaceful path in shipped content;
- owned-service strategic readout: a bounded, read-only overview panel (transient `OwnedServiceOverviewMode`, opened with `O` from Region mode) listing player-owned services with location/region, kind, owner/status (locked/destroyed/occupied), stationed `count/5` + unit names for mines, daily output preview (base + strongest-only `mine_production` via `GameSession::PreviewMineDailyOutput`, matching payout), Trading Post ownership tier, and (M28) stored `count/7` + unit names for storage — assembled by a pure mapper/render-model/renderer, mutating nothing; never persisted (`FromString` self-heals to Region); no schema bump;
- storage foundation: a bounded unit-storage placement distinct from M25 stationing. Owned non-Player-Character stacks can be stored at and retrieved from a player-owned storage service (cap 7) behind explicit `GameSession` methods (`TryStoreStackAtService`, `TryRetrieveStackFromService`, `CanStore…`/`CanOpenStorageAtService`, `EligibleStorageStackIds`, `NormalizeStoredUnits`), preserving the one-place-at-a-time stack invariant (store requires slotted → automatic cross-exclusion with stationing; retrieve returns the same stack id to reserve, fails atomically when reserve is full, and heals corrupt double-placement). Additive `stored_units` save field (no schema bump); a `home_base_storage` service is authored and player-owned via `playerStart`; a bounded text-prompt `StorageInteraction` opens from the Home Base storage zone. Defense/capture/loss/garrison remain deferred;
- cross-Region generic-unit preservation / travel warning: confirmed World Map travel removes only traveling (slotted) generic stacks via `GameSession::TravelToRegion` — heroes and the Player Character travel, while M25-stationed and M28-stored stacks stay behind with refs intact (fixing the M15-era removal that scanned the whole roster). A pure `PreviewRegionTravelGenericLosses()` read drives a two-stage World Map confirmation listing per-stack `Nx Name` losses (cancel keeps the screen open so units can be stored first; a no-loss party travels on the first confirm); App-local pending state, no new GameMode, no schema bump.

Still incomplete or intentionally deferred:

- Storage defense / garrison combat and the defensible-asset system (storage gate defense, stationed-defender combat, storage loss/capture) — the storage *placement* foundation shipped in M28; defense remains deferred;
- victory event chains;
- richer scenario outcome condition leaves;
- per-team / multi-human scenario outcome tracking;
- softlock / victory-reachability proof validation;
- advanced enemy AI economy/service use;
- enemy recruitment and sabotage/destruction/restoration loops;
- enemy/AI capture of player-owned services;
- ownership transfer UI and full service destruction/restoration loops;
- fog/visibility per team;
- leader item/artifact Energy bonus seam;
- broad passive skill trees, status effects, active abilities, and artifact/item effect execution beyond currently implemented narrow paths;
- item use, food consumption, cooking, recipes, seeds, ingredients;
- artifact combination and artifact-handler services;
- battle `Item` command and item use in battle;
- Market / Black Market / Freelancer's Guild item economy beyond M17 ownership-tier foundation;
- broad trader inventory browsing UI beyond the current bounded Trading Post interaction;
- HUD/raylib inventory rendering and inventory render-model;
- event-driven Region unlock;
- per-Region world/enemy state partitioning;
- authored starting roster / hero-pool support;
- full per-Scenario content directories and Scenario Region Contexts;
- general team-definition authoring;
- full campaign branching-choice UI;
- full shell/menu/character-creation/load/settings flow.

## 2. Known doc/code gaps and debt

| # | Issue | Action |
|---|-------|--------|
| 1 | Team Energy has an implemented leader passive term (`leader_energy`) and a still-deferred leader item/artifact Energy term. | Keep `leader_energy` on the current unit passive spine. Do not fake item/artifact Energy until an item/artifact effect milestone exists. |
| 2 | `ContentRepository` loads only content kinds with C++ struct definitions. Recipes, full Scenario Region Contexts, and several long-term authored structures still do not exist. | Add structs/loaders only when a scoped phase requires them. |
| 3 | `docs/game_shell_flow.md` specifies the full shell flow, while code still focuses on the playable slice and direct mode transitions. | Gap, not conflict. Full shell remains deferred. |
| 4 | `docs/validation_system.md` specifies a broader three-level validation model than is currently implemented. | Expand validation only when a phase requires it. |
| 5 | `docs/combat_rules.md` specifies timed status effects and broader command depth. | Gap; acceptable until skill/status phases. |
| 6 | Player color is still effectively fixed as `Green` in several Region/enemy-team/outcome/economy/start-state/claiming paths. | Known debt. Do not fix opportunistically unless introducing a real player-team identity model. |
| 7 | `scenario_outcome.json` remains a bounded-slice authoring file, while `ScenarioDefinition` supports only a subset of long-term Scenario authoring. | Intentional sequencing. Expand Scenario authoring through scoped milestones. |
| 8 | Unit `passive_effects` support only `mine_production` and `leader_energy`; artifact `statBonus` remains on the artifact path. | Intentional M18 scope. Do not fold artifact/item/status behavior into the unit passive spine without a scoped milestone. |
| 9 | Trading Post interaction is implemented as a bounded text-prompt service flow, not a full shop/inventory UI. | Gap, not conflict. Build broader trader UI only in a scoped UI/economy milestone. |
| 10 | Scenario `playerStart` covers economy/service start state only; authored starting roster, full team definitions, item/artifact start state, and `unlockedRegions` overrides are intentionally absent. | Gap, not conflict. Add only when a scoped milestone needs them. |
| 11 | Scenario Result mode presents deterministic outcome and next step, but not scores, rewards, branching choices, fanfare, or post-victory event chains. | Intentional M22 scope. Add only through future scoped milestones. |
| 12 | Owned-service claiming covers both player-side guarded capture (M23) and general player-side claiming on legal node entry (M26). It does not implement enemy-side capture, sabotage, or destruction/restoration. | Resolved by M26. Enemy-side capture and destruction/restoration remain deferred; do not add them without a scoped milestone. |
| 13 | `mine_production` is implemented, content-authored, and player-facing: M25 added a bounded stationing flow at player-owned mines. | Resolved in M25. Stationing stays guard/worker capacity only; do not let it grow into Storage/Garrison, stationed-defender combat, or enemy-side capture without a scoped milestone. |
| 14 | M27 implements an owned-service overview/readout, not the final service-management UI. | Treat it as a strategic visibility foundation and data contract for future Region map overlays, selected-node panels, Adventure-strip strategic screens, and later Storage/Garrison/service-defense work. Do not turn it into remote stationing, ownership transfer, repair, storage, or garrison management without a scoped milestone. |

No true design contradictions are currently known. Remaining gaps are implementation sequencing issues.

## 3. Completed implementation phases

- **Phase 1 — Content Validation System:** foundation implemented; broader validation model still expandable.
- **Phase 2 — Minimal Typed Event Foundation:** foundation implemented.
- **Phase 3 — Enemy Teams on Region Layer:** practical Region-layer enemy-team slice completed by M11-e.
- **Phase 4 — Victory and Defeat Conditions:** M12 complete.
- **Phase 5 — Inventory and Artifacts:** M13 complete.
- **Phase 6 — Energy Pool:** M14 complete; current-leader `leader_energy` passive term filled by M18. Item/artifact Energy remains deferred.
- **Phase 7 — World Map Layer:** M15 complete.
- **Phase 8 — Campaign System:** M16 complete.
- **Phase 9 — Owned Services and Economy Foundation:** M17 complete.
- **Phase 10 — Passive Effect Spine:** M18 complete.
- **Phase 11 — Service Economy Expansion:** M19 complete.
- **Phase 12 — Trading Post Interaction Flow:** M20 complete.
- **Phase 13 — Scenario Economy Start-State Authoring Foundation:** M21 complete.
- **Phase 14 — Scenario Result Presentation Flow:** M22 complete.
- **Phase 15 — Owned Service Claiming and Contesting Foundation:** M23 complete.
- **Phase 16 — v1 Strategic-Economy Proof Content:** M24 complete.
- **Phase 17 — Player-facing Service Stationing Flow:** M25 complete.
- **Phase 18 — General Owned-Service Claiming Semantics:** M26 complete.
- **Phase 19 — Owned Service Overview / Strategic Service Readout:** M27 complete.
- **Phase 20 — Storage Foundation:** M28 complete.
- **Phase 21 — Cross-Region Generic Unit Preservation / Travel Warning:** M29 complete.

## 4. Current next milestone

Latest completed milestone: **M29 — Cross-Region Generic Unit Preservation / Travel Warning**.

Active scope cap: **`docs/content_scope_v2.md`**.

The next milestone is **not yet selected**. See §5 for candidates; service defense remains the strongest likely successor because M25/M28/M29 have created stationed and stored units that the player now deliberately leaves behind, but they still do not defend anything.

### M29 — Cross-Region Generic Unit Preservation / Travel Warning (complete)

**Audit finding resolved:** a crude generic-loss-on-travel already existed since M15, but it scanned the **whole roster**, so once M25 stationing and M28 storage existed it would also have deleted placed stacks (their refs then silently healed away on load). M29's core fix restricts the at-risk set to the traveling party.

**Delivered:** the at-risk set is exactly the slotted (active + reserve) generic stacks. A pure `GameSession::PreviewRegionTravelGenericLosses()` read returns per-stack `{stackId, unitId, quantity}` entries; `GenericTravelingPartyUnitCount()` is its quantity sum; and the confirmed-travel removal inside `TravelToRegion` deletes exactly the previewed set, so the warning the player confirms is the loss that happens. Heroes (category Hero/Leader) travel; the Player Character is additionally protected by an explicit `isPlayerCharacter` guard; M25-stationed and M28-stored stacks survive with refs intact and remain retrievable after returning (proven end-to-end against shipped content: store at `home_base_storage` → travel to `riverside_vale` → return → retrieve).

The warning is a two-stage confirm on the existing World Map screen: confirming a legal destination while at-risk generics exist shows a bounded text block ("Confirm travel to X?", one `Nx Name` line per stack, stored/stationed-safe note); a second confirm commits travel, while cancel/selection-change dismisses the warning and keeps the screen open so units can be stored first. A no-loss party travels on the first confirm; illegal destinations keep reporting their commit-time block reason without the warning. The pending flag is App-local UI state (reset on screen entry, load, and `ResetTransientModeState`), with a stale-pending guard so a confirm never commits travel to a destination other than the warned one. No new GameMode, no schema bump, no content changes; `WorldMapController` stays a pure input→intent function and `TravelToRegion` remains the single loss-resolution owner (direct callers bypass the UI warning by design).

**Boundaries preserved:** no remote storage management, no generic auto-storage, no travel hard-block, no enemy-side Region travel, no Storage defense/capture/loss, no service destruction/restoration, no per-Scenario content partitioning, no shell/menu rewrite.

### M28 — Storage Foundation (complete)

**Design call (pressure-tested against the final-vision docs):** "Storage" and "Garrison" are NOT the same concept. The final-vision rules (`core_loop_rules` §4/§21, `game_vision` §5) specify **Storage** as a concrete 7-slot per-service unit store (units persist in-Region, don't travel, retrievable, defensible gate); "garrison" is not a separate system — it is the **stationed guards** concept already implemented by M25 for mines. M28 therefore built **Storage** as a distinct placement bucket and deferred all garrison/defense/capture/loss.

**Delivered:** a distinct `core::StoredUnitSaveState` + additive `OwnedServiceSaveState.storedUnits` (`stored_units`, no schema bump); a pure `StorageRules` module (`kMaxStoredUnitsPerService = 7`); `GameSession` store/retrieve API mirroring M25 stationing (whole-stack only, PC excluded, active-pull leader guard, same-stack-id retrieve to reserve with atomic reserve-full fail and corrupt double-placement healing, `NormalizeStoredUnits` on load); a new `LocationServiceKind::Storage` + `IsStorageService`; a bounded text-prompt `StorageInteraction` reached from a Home Base storage zone; an authored `home_base_storage` service player-owned via `playerStart` (the `playerStart` ownable check was extended for storage; `IsOwnableServiceKind`/M26 claiming left unchanged, so storage claimability is deferred); and a read-only `Stored n/7` row in the M27 overview. Cross-exclusion with mine stationing is automatic (both require the stack to be slotted). M25/M26/M27 behavior and tests are unchanged.

### M27 — Owned Service Overview / Strategic Service Readout (complete)

**Delivered:** a bounded, read-only owned-service overview reached with `O` from Region mode. It is hosted by a transient `GameMode::OwnedServiceOverviewMode` mirroring `ScenarioResultMode` (never persisted; `ToString` is diagnostic-only and `FromString` self-heals to `RegionMode`; App suppresses save/load while open; no schema bump).

A pure `OwnedServiceOverviewModelMapper` assembles a render-model from existing `GameSession` read accessors — listing only player-owned services with location/region, kind, owner/status (Owned, Locked, Destroyed, Unavailable: occupied), stationed `count/5` + unit names for mines, a daily-output preview, and Trading Post ownership tier — and an `OwnedServiceOverviewRenderer` draws it.

The new `GameSession::PreviewMineDailyOutput(serviceId)` reuses the exact payout building blocks (base parse + strongest-only stationed passives + `ComputeMineDailyOutput`) so the preview equals the daily payout delta for a payable mine; `ApplyDailyMinePayout` is unchanged. The panel is read-only — no ownership, stationing, or payout mutation; M25 stationing stays reachable through the mine Location-zone interaction.

**Positioning:** M27 satisfies the final-vision requirement that service ownership, status, stationed units, and economic effects be strategically legible. The exact `O`-key full-screen panel is an early inspection surface and render-model/data-contract foundation, not necessarily the final service UI. Future milestones may reuse the same information in Region map icons, selected-node panels, Adventure-button screens, or Storage/Garrison/service-defense views.

Tests cover the mode persistence policy, preview==payout consistency, and the mapper (owned-only rows, mine preview/stationing, trader tier, locked/destroyed status without mutation, empty state).

### M26 — General Owned-Service Claiming Semantics (complete)

**Delivered:** a single node-entry claim resolver `GameSession::ResolveNodeEntryClaims(nodeId)` (with `ClaimContestedServicesAtNode` retained as a back-compat alias) reused by both peaceful legal node entry and post-battle guarded capture; the App wires it into `OnDestinationArrived` (after node-entry events, so event-spawned guards block the claim) and the hostile-victory path.

Legally entering an unguarded node claims its eligible ownable services immediately; hostile-occupied nodes still start battle before placement and claim once after victory. Idempotent re-entry skips player-owned/allied services so the player's stationed units are never cleared. Runtime `OwnedServiceSaveState` only; no save schema bump. An unguarded **Copper Mine** was authored to prove the peaceful claim → station → payout loop in shipped content.

Tests cover peaceful claim, hostile-block, idempotent stationing-preserve, locked/destroyed/non-ownable/other-node exclusion, alias parity, save/load, and the shipped-content Copper/Steel mine paths.

### M25 — Player-facing Service Stationing Flow (complete)

**Goal:** Make the existing stationed-unit service path reachable through gameplay.

The player can station and unstation eligible owned units at eligible owned services, starting with mines, so `mine_production` becomes visible in normal play rather than only through tests/save-data injection.

**Delivered:** a pure `StationingRules` legality module; `GameSession` mutation methods (`TryStationStackAtService`, `TryStationSplitAtService`, `TryUnstationStackFromService`, `CanStationStackAtService`, `EligibleStationingStackIds`) enforcing physical one-place-at-a-time placement, same-stack-id unstation into reserve with atomic-fail when reserve is full, generic-stack split, Player-Character exclusion, active-party leader-guard on active pulls, and capacity 5; a bounded text-prompt `StationingInteraction` reached from the mine Location-zone dispatch; and tests covering pure rules, mutations, save/load round-trip (no schema bump), the interaction, and an end-to-end payout boost reached through the mutation API.

**Rationale:** v1 proved the strategic-economy loop, but the `mine_production` half of the passive system was not player-facing. Runtime state, save/load, payout calculation, and content-authored `mine_production` already existed. M25 exposed that existing path through one bounded interaction without building the full Storage/Garrison system.

**Explicit non-goals preserved after M25:**

- No full Storage/Garrison service kind was added in M25; Storage placement was later delivered by M28, while stationed-defender combat and service-defense semantics remain deferred.
- No stationed defenders, combat defense, capacity/loss framework, or service siege system.
- No enemy-side stationing or AI economy.
- No enemy capture of player-owned services.
- No starting-roster authoring or broad team authoring.
- No broad inventory/trader UI rewrite.
- No new passive kinds.

## 5. Candidate directions after M29

These are candidates/sequence notes, not blanket commitments:

1. ~~**M29 — Cross-Region Generic Unit Preservation / Travel Warning.**~~ Delivered by **M29**: traveling generic stacks are lost only after an explicit warning/confirmation, while stored/stationed units survive Region change.
2. ~~**Storage/Garrison Foundation.**~~ Storage placement delivered by **M28** (7-slot store/retrieve at an owned storage service, distinct from M25 stationed guards). The remaining defense work — storage gate defense, stationed-defender combat, storage loss/capture — stays deferred until service-defense rules are selected.
3. ~~**Owned Service Overview / Strategic Service Readout.**~~ Delivered by **M27** as a read-only overview panel and future service-presentation data contract.
4. **Service defense / stationed-defender resolution.** Natural after M29 if the next goal is contested infrastructure: storage gate defense, mine/stationed-guard defense, and attack/defense battle resolution. Do this before enemy-side capture/destruction if those systems need defenders.
5. **Service destruction / restoration slice.** Add a narrow destruction/restoration loop only after service-defense semantics exist.
6. **Enemy-side capture pressure.** Let non-player teams contest or capture player-owned services only after service defense and stationing rules are established.
7. **Inventory render-model / HUD presentation.** Inventory/artifacts exist and are stored/persisted, but there is no render-model or HUD surface.
8. **Campaign branch-choice presentation.** When a scenario has multiple `nextScenarioIds`, present a player-facing choice. Struct support exists, but `CampaignProgressionRules` resolves the first entry only.
9. **Market / Black Market / Freelancer's Guild behavior.** Builds on trader ownership tiers, but risks item-economy sprawl unless tightly scoped.
10. **Scenario Region Context / per-scenario content partitioning.** Useful if upcoming authored content needs scenario-specific Region/enemy/service state rather than global content.
11. **Scenario result polish extensions.** Scores, rewards, fanfare, animations, or post-victory event chains can build on M22, but should not be bundled unless a milestone explicitly selects one narrow result-extension slice.

After M29, prefer the highest-value direct continuation from the final-vision docs rather than system excitement. Service defense is the strongest likely successor because M25/M28/M29 have created stationed and stored units the player now deliberately leaves behind, but they still do not defend anything.

# Ashvale Implementation Roadmap

## Context

The previous roadmap was archived after M29. This file is the active implementation roadmap that finished the v2 scope in `docs/content_scope_v2.md`.

The current codebase is a **post-M30** bounded multi-Region, multi-Scenario vertical slice. The v1 strategic-economy proof and the v2 contested-infrastructure loop are complete: stationing, storage, claiming, cross-Region loss warnings, service defense, storage loss with Temporarily Unavailable heroes, enemy-side capture pressure, destruction/restoration, and the service-state readout/log all work together in shipped content and tests.

The active scope cap is `docs/content_scope_v2.md`, which M30 completed. **v2 is complete and ready to archive**: archive `docs/content_scope_v2.md` and create `docs/content_scope_v3.md` before selecting the next milestone (§6). Archived docs, including `docs/content_scope_v0.md.archived`, older `docs/implementation_roadmap.md.*.archived` files, and `docs/content_scope_v1.md.archived`, are historical context only.

M30 was an umbrella completion milestone, not permission to implement the entire final game; its deliberate simplifications are documented in §4 and its deferred systems in §5.

---

## 1. Current implementation baseline

Current stable foundation:

- explicit `App` / `GameSession` flow;
- controller / mapper / renderer split;
- battle engine, CTB, static formation, leader aura, deterministic damage, and battle write-back;
- persistent roster, active/reserve party, mustering, and save/load;
- daily clock, Region travel, World Map travel, wake/recovery penalty, and basic services;
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
- Trading Post interaction flow with a bounded Location-mode service interaction, buy/sell/barter modes, live prompt feedback, per-visit time cost, and an authored playable Home Base Trading Post;
- Scenario-authored player economy/service start state through `playerStart`, including starting Gold, non-Gold resources, and initial player-owned service state applied at Scenario start;
- owned-service claiming/contesting foundation: guarded capture after hostile victory and peaceful legal node-entry claiming through `GameSession::ResolveNodeEntryClaims`;
- v1 strategic-economy proof content: shipped `playerStart`, shipped `leader_energy`, shipped `mine_production` authoring, authored Trading Post curve data, guarded Steel Mine claim proof, and tests proving the play-reachable chain plus runtime-stationed mine-production boost;
- player-facing mine stationing flow: bounded text-prompt interaction at player-owned mines; physical one-place-at-a-time stack placement; Player Character excluded; cap 5; split stationing for generics; no schema bump;
- general player-side owned-service claiming: legal unguarded node-entry claims eligible ownable services; guarded battle-before-placement preserved; Copper Mine proves the peaceful path;
- owned-service strategic readout: bounded read-only overview panel opened with `O` from Region mode, showing owned services, location/region, kind, status, stationed units, mine payout preview, Trading Post tier, and stored-unit state;
- Storage foundation: storage service kind, 7-slot per-service stored-unit placement distinct from stationing, `StoredUnitSaveState`, additive `stored_units`, Home Base storage service, store/retrieve interaction, and same-stack-id retrieve into reserve;
- cross-Region generic-unit preservation / travel warning: confirmed World Map travel removes only traveling active/reserve generic stacks; heroes/Player Character travel; stored and stationed stacks stay behind; two-stage World Map warning lists the at-risk stacks;
- service defense / stationed-defender resolution (M30): node-level attacks against player-owned attackable services (mines, traders, storage) resolved by the pure deterministic `ServiceDefenseRules` strength comparison when the player is absent — defenders hold ties, a repelled attacker team is defeated, a winning attacker captures every eligible service at the node and occupies it; the player-present case routes through the existing interactive battle surface (`ApplyServiceDefenseVictory`/`Defeat` callbacks);
- storage loss + minimal Temporarily Unavailable hero pipeline (M30): captured services resolve their placed stacks atomically — generics dismissed, heroes enter the TU list (`unavailable_heroes`, additive save) with a weekly return-to-reserve at day start when a slot is free; the Player Character can never be placed, lost, or TU;
- enemy-side service capture pressure (M30): `ProcessEnemyPhase` lets hostile teams attack player-owned service nodes (same node or adjacent, patrol-radius legal, never the arrival node, current region only) with deterministic first-target selection; teams carry an authored `enemyGroupId` (save + `spawnTeam` arg) resolving their attack strength from `enemy_groups.json`;
- service destruction/restoration (M30): authored opt-in `destroyable` flag + mandatory validated `restore_cost`; destruction requires standing on the unoccupied node, costs 1000 Energy + 1 hour, and is blocked by placed units; restoration spends the authored cost, queues, and completes at next day start before that day's payout; destroying again cancels the queue (§20); bounded two-press `K` maintenance action in Region mode;
- service-state presentation (M30): bounded persisted service event log (capped, additive `service_event_log`), enemy-phase status lines, and M27 overview extensions (restoring status, Temporarily Unavailable section, recent-events section);
- M30 content proof: a player-owned direct storage depot (`river_depot`, playerStart-owned, raid event pressure), a destroyable copper mine with authored restore cost, and enemy groups on the authored raider spawns — proven end-to-end against shipped content.

---

## 2. Known doc/code gaps and debt

| # | Issue | Action |
|---|-------|--------|
| 1 | Team Energy has an implemented leader passive term (`leader_energy`) and a still-deferred leader item/artifact Energy term. | Keep `leader_energy` on the current unit passive spine. Do not fake item/artifact Energy until an item/artifact effect milestone exists. |
| 2 | `ContentRepository` loads only content kinds with C++ struct definitions. Recipes, full Scenario Region Contexts, and several long-term authored structures still do not exist. | Add structs/loaders only when a scoped phase requires them. M30 added only the additive `destroyable`/`restore_cost` service fields, no new content kinds. |
| 3 | `docs/game_shell_flow.md` specifies the full shell flow, while code still focuses on the playable slice and direct mode transitions. | Gap, not conflict. Full shell remains deferred (v3 candidate). |
| 4 | `docs/validation_system.md` specifies a broader three-level validation model than is currently implemented. | M30 added the destroyable/restore-cost validation contract only. The broader model remains expandable. |
| 5 | `docs/combat_rules.md` specifies timed status effects and broader command depth. | Gap; acceptable until skill/status phases. |
| 6 | Player color is still effectively fixed as `Green` in several Region/enemy-team/outcome/economy/start-state/claiming paths. | Known debt. The M30 service-defense paths use `PlayerColor()` rather than literals, but older paths keep the hardcode. Fix only with a real player-team identity model. |
| 7 | `scenario_outcome.json` remains a bounded-slice authoring file, while `ScenarioDefinition` supports only a subset of long-term Scenario authoring. | Intentional sequencing. M30 did not expand Scenario authoring (the proof reuses `playerStart`). |
| 8 | Unit `passive_effects` support only `mine_production` and `leader_energy`; artifact `statBonus` remains on the artifact path. | Do not fold artifact/item/status behavior into the unit passive spine without a scoped milestone. |
| 9 | Trading Post interaction is implemented as a bounded text-prompt service flow, not a full shop/inventory UI. | Gap, not conflict. Build broader trader UI only in a scoped v3 milestone. |
| 10 | Scenario `playerStart` covers economy/service start state only; authored starting roster, full team definitions, item/artifact start state, and `unlockedRegions` overrides are intentionally absent. | Gap, not conflict. The M30 proof was authored with `playerStart` alone; add the rest only when a scoped milestone needs them. |
| 11 | Scenario Result mode presents deterministic outcome and next step, but not scores, rewards, branching choices, fanfare, or post-victory event chains. | Left for v3 (candidate list). |
| 12 | Owned-service claiming covers both player-side guarded capture and general player-side claiming. It does not implement enemy-side capture, sabotage, or destruction/restoration. | Resolved by M30: enemy-side capture pressure, service defense, and player-facing destruction/restoration shipped. Enemy-side destruction/sabotage and AI restoration remain v3. |
| 13 | `mine_production` is implemented, content-authored, and player-facing. | Unchanged by M30: payout semantics intact; stationed units now also defend, but production stays strongest-only. |
| 14 | M27 implements an owned-service overview/readout, not the final service-management UI. | M30 extended the readout with restoring status, TU heroes, and a recent-events log. It remains read-only; remote management stays out of scope. |
| 15 | Storage gate defense, storage-loss consequences, and the minimal Temporarily Unavailable pipeline shipped in M30. | Resolved by M30: stored stacks defend their node, storage loss dismisses generics and makes heroes Temporarily Unavailable (weekly reserve return as the pool stand-in). Shared hero pool re-entry is v3. |
| 16 | World Map generic-loss warning makes Storage strategically necessary. | Resolved by M30: placed (stationed/stored) units now defend their services through the deterministic service-defense resolver. |

No true design contradictions are currently known. Remaining gaps are implementation sequencing issues.

---

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
- **Phase 22 — v2 Completion: Contested Infrastructure, Service State, and Closure Audit:** M30 complete.

---

## 4. Current next milestone

Latest completed milestone: **M30 — v2 Completion: Contested Infrastructure, Service State, and Closure Audit**.

`docs/content_scope_v2.md` is **complete and ready to archive**. The next milestone is **not yet selected** — per §6, archive v2 and create `docs/content_scope_v3.md` first. §5 lists the candidates; service-defense depth (full-simulation auto-resolve / battle AI) and the shared hero pool are the most direct continuations of what M30 bounded.

### M30 — v2 Completion: Contested Infrastructure, Service State, and Closure Audit (complete)

**Goal met:** the service/storage/stationing loop is now a contested infrastructure loop. Placed units strategically matter: stationed and stored stacks defend their nodes, enemy teams legally pressure player-owned services, losses resolve without dangling refs, destroyed services can be restored, and the player can see all of it.

#### Settled pressure-test answers

1. **Defensible Service:** a player-owned service of an attackable kind — Mine, the four trader kinds, or Storage — at a Region node (`ServiceKindIsAttackable`). Attacks are node-level: every eligible service at the node resolves together, mirroring node-keyed occupation/claiming. Rest/Shop/Recruit/Muster interactions are never systemically attackable; arrival nodes are protected. There is no separate "direct Region service" type in code — services bind to Locations placed 1:1 on Region nodes, and the authored `destroyable` flag is the §20 "Region Services only" gate.
2. **Defender roster:** player absent → the union of stationed + stored stacks at the node's eligible services; player party standing on the node → the active party via the existing interactive battle surface (§21); no defenders + player absent → capture without battle (mirror of peaceful claiming).
3. **Battle path:** player-involved defense reuses the existing battle flow (location `battleScenarioId`, hostile-contact pattern, existing victory/defeat write-back; `ApplyServiceDefenseVictory`/`Defeat` report the result). Absent-player defense uses the pure deterministic `ServiceDefenseRules` strength comparison (unit power = attack + defense + maxHp; stack power × quantity; defender wins ties). No second battle engine; no battle AI exists, so full-simulation auto-resolve is explicitly v3.
4. **Refs after battle:** one atomic `GameSession` mutation (`ApplyServiceCaptureAtNode`) resolves losing defenders — generic stacks erased, hero stacks erased + appended to the Temporarily Unavailable list, refs cleared in the same pass; the Player Character is excluded by the M25/M28 placement gates plus an explicit capture-time guard. Winning defenders are untouched.
5. **Enemy capture:** immediate `ownerTeamColor` transfer for all eligible services at the node (Mine/traders/Storage). Storage capture = storage loss. Inherited refs are resolved, never transferred. Enemy-side destruction is deferred (capture already denies payouts); the reclaim loop is the existing M26 guarded capture.
6. **AI pressure path:** one new deterministic `ProcessEnemyPhase` action — a player-hostile team standing on or adjacent to an eligible node (patrol-radius legal, current region, never the arrival node) attacks it instead of patrolling; first target by sorted node id; fixed color order. Teams carry an authored `enemyGroupId` for strength. No AI economy, no pathing.
7. **Content proof:** shipped `river_depot` (direct storage, playerStart-owned, storage zone, defense battle scenario, raid-warning spawn event), destroyable `copper_mine_svc` with authored `restore_cost`, `enemyGroupId` on the authored raider spawns, and end-to-end tests driving store → pressure → hold/loss → TU → return → claim → destroy → queue → restore against the real content directory.

#### Documented M30 simplifications (deliberate, final-rule-compatible stand-ins)

- Absent-player defense is a deterministic strength comparison, not a full battle simulation; the winning side survives intact and the losing side is eliminated (no partial losses). Full-sim auto-resolve belongs to the v3 battle-depth candidate.
- A repelled attacker team is defeated/deactivated outright.
- Temporarily Unavailable heroes return to the **player's reserve** after `kUnavailableHeroReturnDays` (7) at a day start with a free reserve slot. This stands in for shared-hero-pool re-entry, which does not exist yet (v3).
- Enemy pressure targets **player-owned** services only and captures rather than destroys; enemy-vs-enemy contention and AI sabotage/restoration are v3.
- Player-present defense battles use the location's authored battle scenario as the attacker roster — the same simplification the existing hostile-contact flow makes.
- The destruction/restoration trigger surface is a bounded two-press `K` action in Region mode, not a final service-management UI.
- Service event log lines use service/location ids; the overview rows keep display names.

#### Save/load

All M30 state is additive, no schema bump: `OwnedServiceSaveState.restoration_queued`, `EnemyTeamSaveState.enemy_group_id`, `SaveData.unavailable_heroes`, `SaveData.service_event_log` (capped, re-trimmed and PC-healed on load).

#### Tests

`ServiceDefenseTests` (rules, resolver outcomes, TU pipeline + weekly return, PC guards, save/load), `EnemyServicePressureTests` (phase targeting, patrol/arrival/alliance/region gates, pending player battle, spawnTeam group authoring), `ServiceDestructionTests` (gates, costs, payout interaction, queue/cancel/day-start completion, validation contract), overview mapper presentation tests, and `ContestedInfrastructureContentTests` (real-content proof loop). All prior M25/M26/M27/M28/M29 suites remain green.

---

## 5. Candidate directions after v2 / future v3

These are not v2 commitments. After M30, create a new active content scope before selecting one.

Direct continuations of the M30 simplifications (see §4):

1. **Service-defense battle depth.** Replace the deterministic strength comparison with full-simulation auto-resolve (requires a battle AI that can drive both sides), partial defender losses, attacker retreat instead of elimination, and threat preview / redo / manual-defense selection per `core_loop_rules` §16/§21.
2. **Shared hero pool and Temporarily Unavailable re-entry.** Hero recruitment from a scenario hero pool, TU heroes returning to the pool (not the player's reserve), and hero-availability competition between teams.
3. **Enemy-side destruction, sabotage, and restoration.** AI teams destroying/restoring services per personality (Warrior sabotage, Builder restore), enemy-vs-enemy service contention, and enemy use of captured services.
4. **Service-management presentation.** Region-map service-state overlays, selected-node panels, display-name event logs, and the final service-management UI (the M27/M30 overview is a read-model foundation, not the end state).

Larger systems (unchanged from the pre-M30 candidate list):

5. **Full enemy AI economy and equipment behavior.** Trading Post, Market, Black Market, Freelancer's Guild, artifact combination, cooking, service use, and resource strategy across AI teams.
6. **Full item economy and item-use systems.** Items, seeds, ingredients, food, consumables, field use, battle `Item` command, and broader item-effect execution.
7. **Artifact inventory/equipment UI and artifact-combination services.** Inventory presentation can start narrow, but broad artifact management belongs in its own scope.
8. **Fog-of-war, scouting, reveal, and enemy inspection.** Includes team-specific explored/revealed state, scouting levels, visibility, and threat/inspection detail.
9. **Full shell/menu/character creation/load/settings flow.** Implement according to `docs/game_shell_flow.md` when the project is ready for shell-level polish.
10. **Scenario Region Context / per-Scenario content partitioning.** Useful when authored content needs scenario-specific Region/enemy/service state rather than global content.
11. **Campaign branch-choice and richer Scenario Result presentation.** Branch-choice UI, scores, rewards, fanfare, post-victory event chains, and scenario-transition carry-over policy.
12. **Advanced battle command depth.** Timed status effects, active abilities, spells, broader MP/item use, auto-combat controls, threat preview, and redo/manual battle flow.
13. **Editor tooling.** Only after content schema and validation needs justify it.

---

## 6. Roadmap discipline

- M30 is complete: archive `docs/content_scope_v2.md` (user action) and create a new `docs/content_scope_v3.md` before selecting the next milestone.
- Do not keep extending v2; new infrastructure work belongs in the v3 scope.
- Milestone-agnostic docs remain source-of-truth for final rules:
  - `docs/game_vision.md`
  - `docs/technical_direction.md`
  - `docs/core_loop_rules.md`
  - `docs/combat_rules.md`
  - `docs/game_shell_flow.md`
  - `docs/presentation_game_feel.md`
  - `docs/scenario_authoring.md`
  - `docs/content_schema.md`
  - `docs/validation_system.md`
  - `docs/terminology_map.md`
- Update milestone-agnostic docs only when the actual long-term rule, schema, terminology, or architectural direction changes.

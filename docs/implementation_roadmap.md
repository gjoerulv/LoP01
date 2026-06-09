# Ashvale Implementation Roadmap

## Context

The current codebase is a **post-M26** bounded multi-Region, multi-Scenario vertical slice. The v1 strategic-economy proof is complete: the shipped slice and tests exercise Scenario-authored player economy/service start state, owned services, mine payout, narrow unit passive effects, authored Trading Post trade data, guarded and unguarded service claiming, Scenario Result presentation, Campaign progression, and player-facing mine stationing.

Archived docs, including `docs/content_scope_v0.md.archived`, older `docs/implementation_roadmap.md.*.archived` files, and `docs/content_scope_v1.md` once archived by the user, are historical context only. The active scope cap is now `docs/content_scope_v2.md`.

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
- general player-side owned-service claiming: legally entering an unguarded node claims its eligible ownable services immediately via `GameSession::ResolveNodeEntryClaims` (the single claim path, with `ClaimContestedServicesAtNode` as a back-compat alias), wired into `App::OnDestinationArrived` and the post-battle victory path; guarded battle-before-placement preserved; idempotent re-entry never clears the player's stationed units; no schema bump; an unguarded Copper Mine proves the peaceful path in shipped content.

Still incomplete or intentionally deferred:

- Storage/Garrison service kind and defensible-asset system;
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
- generic origin-storage across Regions;
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

## 4. Current next milestone

Latest completed milestone: **M26 — General Owned-Service Claiming Semantics**.

Active scope cap: **`docs/content_scope_v2.md`**.

The next milestone is **not yet selected**. Candidate v2 directions are in §5 below and in `docs/content_scope_v2.md` §5; the natural successors are a Storage/Garrison foundation or an owned-service management/presentation view now that stationing and ownership-claiming semantics are proven.

### M26 — General Owned-Service Claiming Semantics (complete)

**Delivered:** a single node-entry claim resolver `GameSession::ResolveNodeEntryClaims(nodeId)` (with `ClaimContestedServicesAtNode` retained as a back-compat alias) reused by both peaceful legal node entry and post-battle guarded capture; the App wires it into `OnDestinationArrived` (after node-entry events, so event-spawned guards block the claim) and the hostile-victory path. Legally entering an unguarded node claims its eligible ownable services immediately; hostile-occupied nodes still start battle before placement and claim once after victory. Idempotent re-entry skips player-owned/allied services so the player's stationed units are never cleared. Runtime `OwnedServiceSaveState` only; no save schema bump. An unguarded **Copper Mine** was authored to prove the peaceful claim → station → payout loop in shipped content. Tests cover peaceful claim, hostile-block, idempotent stationing-preserve, locked/destroyed/non-ownable/other-node exclusion, alias parity, save/load, and the shipped-content Copper/Steel mine paths.

### M26 — General Owned-Service Claiming Semantics (original plan)

**Goal:** Make player-side owned-service claiming behave like the intended systemic Region rule rather than only the current guarded-battle proof path.

The player should claim eligible ownable services when their team legally enters a node and no hostile guard/occupier blocks capture. Guarded or hostile-occupied nodes still start battle before the player team is placed on the node; after victory, the capture/arrival resolution for that node should run exactly once. M26 should preserve the guarded Steel Mine path while adding the missing peaceful/unguarded claim path.

**Rationale:** M23 proved guarded service claiming, and M25 made claimed mine ownership meaningful through stationing. Manual M25 testing exposed that claiming is still effectively wired through hostile-battle victory. That is valid for guarded mines, but it is not the general ownership rule. Closing this gap now is higher value than adding Storage/Garrison because it stabilizes the ownership semantics that later management, defense, and capture systems will build on.

**Intended rule model:**

- If the player legally enters a node containing claimable ownable services and the node is not blocked by a hostile guard/occupier, claim eligible services immediately as part of node-entry resolution.
- If a hostile guard/occupier blocks entry, battle starts before the player team is moved to the target node. This is intentional final-direction behavior.
- If the player wins that battle, resolve the target node's capture/arrival outcome once: claim eligible services, clear appropriate guard/contest state, and move/settle the player according to existing travel semantics.
- If the player loses or retreats, do not claim the service and do not enter the guarded node.
- Event-driven ownership transfer remains future work. M26 is systemic player-side claiming, not a custom `changeOwnership` event action.

**Narrow acceptance criteria:**

- A reusable `GameSession`-level claim/entry path handles both unguarded node-entry claiming and post-battle guarded claiming without duplicating claim logic in `App`.
- Unguarded / peaceful claimable services at a legally entered node become player-owned at the correct time.
- Guarded service claiming still requires defeating the hostile guard/occupier first.
- Battle may still start before the moving team is placed on the hostile-occupied destination.
- Post-battle victory does not double-claim, double-run arrival side effects, double-spend travel/Energy/time, or incorrectly run enemy phase twice.
- Claiming mutates runtime `OwnedServiceSaveState` only; authored content definitions are never mutated.
- Claimed services preserve existing M25 stationing invariants: ownership transfer must not duplicate stationed units, leak inherited stationed refs incorrectly, or invalidate save/load.
- Tests cover pure claim rules where practical, normal arrival claiming, guarded victory claiming, loss/no-claim behavior, save/load compatibility, and the Steel Mine manual-play path.

**Explicit non-goals:**

- No enemy-side capture of player-owned services.
- No service destruction, restoration, sabotage, siege, or stationed-defender combat.
- No Storage/Garrison service kind or garrison-management UI.
- No new resource economy, item economy, or trader behavior.
- No broad ownership-transfer UI.
- No `changeOwnership` / event-driven ownership action unless the implementation audit proves a tiny helper is unavoidable for tests; even then, do not expose it as a general authoring system in M26.
- No player-team identity refactor beyond the minimum needed to preserve current Green-player behavior.
- No full Scenario Region Context or per-Region state partitioning.

**Likely first slice:** audit `ClaimContestedServicesAtNode`, `ServiceClaimRules`, Region travel arrival, hostile-contact battle resolution, and service ownership tests. Then introduce a single node-entry claim resolver behind `GameSession` and wire it into the normal arrival path before broad App cleanup.

**Risk notes:** the main risk is double-running arrival/capture side effects. Do not fix this by spreading ownership mutations through `App`. Keep claim mutation centralized, explicit, and test-backed. Be especially strict about hostile-contact victory: battle-before-placement is correct, but victory should not leave the ownership flow dependent on a custom one-off battle hook forever.

### M25 — Player-facing Service Stationing Flow (complete)

**Goal:** Make the existing stationed-unit service path reachable through gameplay. The player can station and unstation eligible owned units at eligible owned services, starting with mines, so `mine_production` becomes visible in normal play rather than only through tests/save-data injection.

**Delivered:** a pure `StationingRules` legality module; `GameSession` mutation methods (`TryStationStackAtService`, `TryStationSplitAtService`, `TryUnstationStackFromService`, `CanStationStackAtService`, `EligibleStationingStackIds`) enforcing physical one-place-at-a-time placement, same-stack-id unstation into reserve with atomic-fail when reserve is full, generic-stack split, Player-Character exclusion, active-party leader-guard on active pulls, and capacity 5; a bounded text-prompt `StationingInteraction` reached from the mine Location-zone dispatch; and tests covering pure rules, mutations, save/load round-trip (no schema bump), the interaction, and an end-to-end payout boost reached through the mutation API.

**Rationale:** v1 proved the strategic-economy loop, but the `mine_production` half of the passive system was not player-facing. Runtime state, save/load, payout calculation, and content-authored `mine_production` already existed. M25 exposed that existing path through one bounded interaction without building the full Storage/Garrison system.

**Explicit non-goals preserved after M25:**

- No full Storage/Garrison service kind yet.
- No stationed defenders, combat defense, capacity/loss framework, or service siege system.
- No enemy-side stationing or AI economy.
- No enemy capture of player-owned services.
- No starting-roster authoring or broad team authoring.
- No broad inventory/trader UI rewrite.
- No new passive kinds.

## 5. Candidate directions after M26

These are candidates, not selected commitments:

1. **Storage/Garrison Foundation.** Add a bounded storage/garrison service or interaction after M25 proves stationing semantics and M26 stabilizes ownership claiming.
2. **Owned Service Presentation / Management View.** Show owned services, stationed units, expected mine output, and ownership status through a dedicated model/renderer.
3. **Service destruction / restoration and enemy-side capture.** The broader contesting loop after M23/M25/M26. Deferred until stationing/defense semantics exist.
4. **Inventory render-model / HUD presentation.** Inventory/artifacts exist and are stored/persisted, but there is no render-model or HUD surface.
5. **Campaign branch-choice presentation.** When a scenario has multiple `nextScenarioIds`, present a player-facing choice. Struct support exists, but `CampaignProgressionRules` resolves the first entry only.
6. **Market / Black Market / Freelancer's Guild behavior.** Builds on trader ownership tiers, but risks item-economy sprawl unless tightly scoped.
7. **Scenario Region Context / per-scenario content partitioning.** Useful if upcoming authored content needs scenario-specific Region/enemy/service state rather than global content.
8. **Scenario result polish extensions.** Scores, rewards, fanfare, animations, or post-victory event chains can build on M22, but should not be bundled unless a milestone explicitly selects one narrow result-extension slice.

The next selected milestone after M26 should be narrow, testable, and justified by current gameplay value rather than system excitement.

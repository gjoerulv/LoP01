# Ashvale Implementation Roadmap

## Context

The current codebase is a **post-M24** bounded multi-Region, multi-Scenario vertical slice.

The v1 strategic-economy proof is complete: the shipped slice and tests now exercise Scenario-authored player economy/service start state, owned services, mine payout, narrow unit passive effects, authored Trading Post trade data, guarded service claiming, Scenario Result presentation, and Campaign progression.

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
- Trading Post transaction foundation with pure quote rules, GameSession transaction APIs, service-specific use/ownership gating, tier-0 fallback/default behavior, Gold delegation, validation, and end-to-end tests;
- Trading Post interaction flow with a bounded Location-mode service interaction, buy/sell/barter modes, live prompt feedback, per-visit time cost, and a small authored playable Home Base Trading Post;
- Scenario-authored player economy/service start state through `playerStart`, including starting Gold, non-Gold resources, and initial player-owned service state applied at Scenario start;
- owned-service claiming/contesting foundation: defeating a hostile team occupying/guarding a node can claim eligible ownable services at that node for the player;
- v1 strategic-economy proof content: shipped `playerStart`, shipped `leader_energy`, shipped `mine_production` authoring, authored Trading Post curve data, guarded Steel Mine claim proof, and tests proving the play-reachable chain plus runtime-stationed mine-production boost;
- player-facing mine stationing flow: a bounded, text-prompt interaction at player-owned mines that stations/unstations/splits eligible owned stacks behind explicit `GameSession` methods (physical one-place-at-a-time placement, Player-Character excluded, up to 5 per mine, no schema bump), making `mine_production` visible in normal play.

Still incomplete or intentionally deferred:

- Storage/Garrison service kind and defensible-asset system;
- victory event chains;
- richer scenario outcome condition leaves;
- per-team / multi-human scenario outcome tracking;
- softlock / victory-reachability proof validation;
- advanced enemy AI economy/service use;
- enemy recruitment and sabotage/destruction/restoration loops;
- enemy/AI capture of player-owned services;
- unguarded / peaceful ownership-claim interactions beyond the guarded-capture path;
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
| 12 | Owned-service claiming is player-side guarded capture only. It does not implement enemy-side capture, peaceful/unguarded claiming, sabotage, or destruction/restoration. | Intentional M23 scope. Expand only through future scoped milestones. |
| 13 | `mine_production` is implemented, content-authored, and now player-facing: M25 added a bounded stationing flow at player-owned mines. | Resolved in M25. Stationing stays guard/worker capacity only; do not let it grow into Storage/Garrison, stationed-defender combat, or enemy-side capture without a scoped milestone. |

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

## 4. Current next milestone

Latest completed milestone: **M25 — Player-facing Service Stationing Flow**.

Active scope cap: **`docs/content_scope_v2.md`**.

The next milestone is **not yet selected**. Candidate v2 directions are in §5 below and in `docs/content_scope_v2.md` §5; the natural successor is a bounded Storage/Garrison foundation or an owned-service management view, now that stationing semantics are proven.

### M25 — Player-facing Service Stationing Flow  (complete)

**Goal:** Make the existing stationed-unit service path reachable through gameplay. The player can station and unstation eligible owned units at eligible owned services, starting with mines, so `mine_production` becomes visible in normal play rather than only through tests/save-data injection.

**Delivered:** a pure `StationingRules` legality module; `GameSession` mutation methods (`TryStationStackAtService`, `TryStationSplitAtService`, `TryUnstationStackFromService`, `CanStationStackAtService`, `EligibleStationingStackIds`) enforcing physical one-place-at-a-time placement, same-stack-id unstation into reserve with atomic-fail when reserve is full, generic-stack split, Player-Character exclusion, active-party leader-guard on active pulls, and capacity 5; a bounded text-prompt `StationingInteraction` reached from the mine Location-zone dispatch; and tests covering pure rules, mutations, save/load round-trip (no schema bump), the interaction, and an end-to-end payout boost reached through the mutation API.

**Rationale:** v1 proved the strategic-economy loop, but the `mine_production` half of the passive system is still not player-facing. Runtime state, save/load, payout calculation, and content-authored `mine_production` already exist. M25 should expose that existing path through one bounded interaction without building the full Storage/Garrison system.

**Narrow acceptance criteria:**

- A player-facing interaction path allows stationing an eligible owned unit at an eligible player-owned service, initially a mine.
- The interaction preserves the stack-backed invariant: stationed units must correspond to owned units under the current roster model.
- The player can unstation or replace stationed units without duplicating or losing units.
- Stationed `mine_production` affects mine payout through the existing strongest-only rules; no payout rewrite.
- Save/load preserves stationed assignments through existing owned-service serialization; no schema bump unless the implementation audit proves one is unavoidable.
- The interaction is bounded and text-prompt based, following existing service interaction patterns.
- Tests cover pure rules where applicable, GameSession mutation behavior, save/load, and an end-to-end payout proof through player-facing stationing.

**Explicit non-goals:**

- No full Storage/Garrison service kind yet.
- No stationed defenders, combat defense, capacity/loss framework, or service siege system.
- No enemy-side stationing or AI economy.
- No enemy capture of player-owned services.
- No starting-roster authoring or broad team authoring.
- No broad inventory/trader UI rewrite.
- No new passive kinds.
- No v2 scope expansion beyond this selected stationing milestone.

**Likely first slice:** audit the current roster/owned-service/stationed-unit model, then add a pure stationing legality/mutation rule and tests before App interaction wiring. If the existing stack-backed invariant cannot be preserved with current roster APIs, stop and revise the plan before coding UI.

**Risk notes:** the main risk is accidental unit duplication/loss. Do not mutate stationed units directly from the App layer. Keep mutations behind explicit `GameSession` methods. Do not let service stationing become Storage/Garrison by stealth.

## 5. Candidate directions after M25

These are candidates, not selected commitments:

1. **Storage/Garrison Foundation.** Add a bounded storage/garrison service or interaction after M25 proves stationing semantics. This is the most natural v2 successor if stationing holds up.
2. **Owned Service Presentation / Management View.** Show owned services, stationed units, expected mine output, and ownership status through a dedicated model/renderer.
3. **Service destruction / restoration and enemy-side capture.** The broader contesting loop after M23/M25. Deferred until stationing/defense semantics exist.
4. **Inventory render-model / HUD presentation.** Inventory/artifacts exist and are stored/persisted, but there is no render-model or HUD surface.
5. **Campaign branch-choice presentation.** When a scenario has multiple `nextScenarioIds`, present a player-facing choice. Struct support exists, but `CampaignProgressionRules` resolves the first entry only.
6. **Market / Black Market / Freelancer's Guild behavior.** Builds on trader ownership tiers, but risks item-economy sprawl unless tightly scoped.
7. **Scenario Region Context / per-scenario content partitioning.** Useful if upcoming authored content needs scenario-specific Region/enemy/service state rather than global content.
8. **Scenario result polish extensions.** Scores, rewards, fanfare, animations, or post-victory event chains can build on M22, but should not be bundled unless a milestone explicitly selects one narrow result-extension slice.

The next selected milestone after M25 should be narrow, testable, and justified by current gameplay value rather than system excitement.

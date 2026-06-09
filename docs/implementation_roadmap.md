# Ashvale Implementation Roadmap

## Context

The current codebase is a **post-M23** bounded multi-Region, multi-Scenario vertical slice.

The stable foundation now includes battle, roster, save/load, Region/Location flow, content validation, typed events, runtime enemy-team spawning, deterministic scenario outcomes, a dedicated Scenario Result screen, inventory and artifacts, the team Energy pool, a minimal World Map layer, a minimal Campaign System, owned-service/economy foundation, a narrow unit passive-effect spine, a headless Trading Post transaction layer, a bounded playable Trading Post interaction flow, Scenario-authored player economy/service start state, and in-play owned-service claiming/contesting after defeating hostile guards.

Archived docs, including `docs/content_scope_v0.md.archived` and older `docs/implementation_roadmap.md.*.archived` files, are historical context only. The active scope cap remains `docs/content_scope_v1.md` until a deliberate v2 scope is selected.

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
- owned-service claiming/contesting foundation: defeating a hostile team occupying/guarding a node can claim eligible ownable services at that node for the player; claimed services feed existing mine payout, trader tier, Trading Post use, and save/load systems.

Still incomplete or intentionally deferred:

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

## 4. Current next milestone

Latest completed milestone: **M23 — Owned Service Claiming and Contesting Foundation**.

No next milestone is currently selected.

The next planning pass should perform a **v1 completion audit** before selecting another implementation milestone. The likely question is whether `docs/content_scope_v1.md` is now complete enough to archive and replace with `content_scope_v2.md`, or whether one final v1 cleanup milestone is still needed.

Do not assume M24. Do not create v2 scope automatically. First audit the active docs/source against `docs/content_scope_v1.md` and `docs/game_vision.md`.

## 5. Candidate directions after M23

These are candidates, not selected commitments:

1. **v1 completion audit and v2 scope planning.** Determine whether the compact strategic-economy proof is complete enough to archive `content_scope_v1.md` and define `content_scope_v2.md`. This should be the next planning step before implementation work.
2. **Service destruction / restoration and enemy-side capture.** The broader contesting loop after M23: occupying-team service destruction/restoration (`core_loop_rules.md` §20) and enemy/AI capture of player services. Deferred — enemy-side capture needs AI economy; both are beyond M23's narrow player-side claim path.
3. **Campaign branch-choice presentation.** When a scenario has multiple `nextScenarioIds`, present a player-facing choice. Struct support exists, but `CampaignProgressionRules` resolves the first entry only, and large campaign branching UI is out of v1 content scope.
4. **Inventory render-model / HUD presentation.** Inventory/artifacts exist and are stored/persisted, but there is no render-model or HUD surface. Independent, pure-presentation, no logic change.
5. **Market / Black Market / Freelancer's Guild behavior.** Builds on trader ownership tiers, but risks item-economy sprawl unless tightly scoped; currently out of v1 content scope.
6. **Scenario Region Context / per-scenario content partitioning.** Useful if upcoming authored content needs scenario-specific Region/enemy/service state rather than global content. No current authored-content pressure; out of v1 content scope.
7. **Scenario result polish extensions.** Scores, rewards, fanfare, animations, or post-victory event chains can build on M22, but should not be bundled unless a milestone explicitly selects one narrow result-extension slice.

The next selected milestone should be narrow, testable, and justified by current gameplay value rather than system excitement.

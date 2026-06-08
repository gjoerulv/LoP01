# Ashvale Implementation Roadmap

## Context

The current codebase is a **post-M21** bounded multi-Region, multi-Scenario vertical slice.

The stable foundation now includes battle, roster, save/load, Region/Location flow, content validation, typed events, enemy teams on the Region layer, deterministic scenario outcomes, inventory and artifacts, the team Energy pool, a minimal World Map layer, a minimal Campaign System, owned-service/economy foundation, a narrow unit passive-effect spine, a headless Trading Post transaction layer, a bounded playable Trading Post interaction flow, and Scenario-authored player economy/service start state.

Archived docs, including `docs/content_scope_v0.md.archived` and older `docs/implementation_roadmap.md.*.archived` files, are historical context only. The active scope cap is `docs/content_scope_v1.md`.

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
- typed event foundation;
- enemy-team Region-layer foundation;
- scenario outcome foundation;
- inventory and artifact foundation with equipped-artifact battle stat bonuses;
- minimal World Map;
- minimal Campaign System;
- owned-service/economy foundation with resources, owned-service runtime state, mine outputs, stack-backed stationing, daily mine payout, trader ownership tiers, authored/default trader curves, validation, and proof tests;
- passive-effect spine foundation with canonical unit `passive_effects`, legacy `mine_production_passive` authoring compatibility, `mine_production` effects, and `leader_energy` effects;
- Trading Post transaction foundation with pure quote rules, GameSession transaction APIs, service-specific use/ownership gating, tier-0 fallback/default behavior, Gold delegation, validation, and end-to-end tests;
- Trading Post interaction flow with a bounded Location-mode service interaction, buy/sell/barter modes, live prompt feedback, per-visit time cost, and a small authored playable Home Base Trading Post;
- Scenario-authored player economy/service start state through `playerStart`, including starting Gold, non-Gold resources, and initial player-owned service state applied at Scenario start.

Still incomplete or intentionally deferred:

- polished scenario-end/result screen flow;
- victory event chains;
- richer scenario outcome condition leaves;
- per-team / multi-human scenario outcome tracking;
- softlock / victory-reachability proof validation;
- advanced enemy AI economy/service use;
- enemy recruitment and sabotage/destruction/restoration loops;
- ownership transfer/contesting UI and full service destruction/restoration loops;
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
| 6 | Player color is still effectively fixed as `Green` in several Region/enemy-team/outcome/economy/start-state paths. | Known debt. Do not fix opportunistically unless introducing a real player-team identity model. |
| 7 | `scenario_outcome.json` remains a bounded-slice authoring file, while `ScenarioDefinition` supports only a subset of long-term Scenario authoring. | Intentional sequencing. Expand Scenario authoring through scoped milestones. |
| 8 | Unit `passive_effects` support only `mine_production` and `leader_energy`; artifact `statBonus` remains on the artifact path. | Intentional M18 scope. Do not fold artifact/item/status behavior into the unit passive spine without a scoped milestone. |
| 9 | Trading Post interaction is implemented as a bounded text-prompt service flow, not a full shop/inventory UI. | Gap, not conflict. Build broader trader UI only in a scoped UI/economy milestone. |
| 10 | Scenario `playerStart` covers economy/service start state only; authored starting roster, full team definitions, item/artifact start state, and `unlockedRegions` overrides are intentionally absent. | Gap, not conflict. Add only when a scoped milestone needs them. |

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

## 4. Current next milestone

Latest completed milestone: **M21 — Scenario Economy Start-State Authoring Foundation**.

Selected next milestone (planned, not yet implemented): **M22 — Scenario Result Presentation Flow**.

- **Goal:** Add a dedicated player-facing scenario-end result step that presents the deterministic outcome (Victory/Defeat and reason) and the immediate next step (advance to next scenario, campaign complete, campaign failed, or standalone end) before control resumes, replacing the current single-line HUD status append.
- **Rationale:** Scenario outcome computation (M12) and campaign progression (M16) are complete and deterministic, but the only player-facing surface is one line appended to the shared HUD status text. This is the one obviously thin spot in an otherwise complete loop. It is presentation-only over stable model state: low architecture risk, high clarity value, no new systems, and no content-scope change.
- **Acceptance criteria (narrow):**
  - a new result `GameMode` value entered when an outcome latches, before campaign auto-advance / input-freeze;
  - a result view-model mapped from the existing `ScenarioOutcome` (state, reason) plus campaign context (next scenario id, campaign completed, campaign failed, or no campaign), with no new gameplay logic;
  - a dedicated result renderer drawing the outcome label, reason, next-step line, and a Continue affordance;
  - Continue advances exactly as today through the existing campaign-transition / terminal path; outcome evaluation, campaign progression, carry-over, and save format are unchanged;
  - tests cover mapper construction for Victory, Defeat, campaign-complete, campaign-failed, and standalone-scenario cases, plus mode entry/exit, with existing outcome/campaign tests still passing.
- **Non-goals:**
  - no campaign branch-choice (multiple `nextScenarioIds` still resolves to the first; this stays a candidate);
  - no score/stats/rewards/fanfare/animation beyond outcome, reason, and next-step;
  - no inventory/reward display, no shell/menu integration, no post-victory event chains;
  - no change to outcome evaluation, campaign progression, carry-over, or save/load format;
  - no content-scope change and no new authored content.
- **Likely first slice:** add the result `GameMode` value and a minimal pass-through Result mode that renders the already-latched outcome on its own screen with a Continue input that triggers the existing campaign-progress path; then extract a proper result view-model, mapper, renderer, and tests.
- **Risk notes:**
  - insert the new mode at the correct scenario-end seam without double-latching or skipping the existing input-freeze / campaign-advance path; keep the single-latch invariant;
  - cover both entry points: victory (enemy cleared, return to Region) and defeat (battle defeat with wake penalty/recover, and condition-defeat);
  - standalone (non-campaign) scenarios end with no next, so the result mode must handle "no next" gracefully;
  - do not entangle result presentation with the known fixed-as-`Green` player-color debt (gap #6).

After M22, the next planning pass should re-audit and select from the candidates below. Do not assume M23 without that audit.

## 5. Candidate directions after M22

Scenario result / victory presentation flow is now selected as **M22** (see §4). The directions below remain candidates, not selected commitments:

1. **Campaign branch-choice presentation.** The natural successor to the M22 result seam: when a scenario has multiple `nextScenarioIds`, present a player-facing choice. Struct support already exists, but `CampaignProgressionRules` currently resolves the first entry only, and `docs/content_scope_v1.md` lists large campaign branching UI as out of v1 scope. Select only once authored content needs a real branch; pair it with a narrow `content_scope_v1` update.
2. **Inventory render-model / HUD presentation.** Inventory/artifacts exist and are stored/persisted, but there is no render-model or HUD surface. Independent, pure-presentation, no logic change; a reasonable successor once the result flow is polished.
3. **Service ownership transfer / claiming loop.** Builds on owned services and Scenario start-state, but should not start until the desired contest/claim interaction is clearly scoped. Sprawl risk; no consumer yet.
4. **Market / Black Market / Freelancer's Guild behavior.** Builds on trader ownership tiers, but risks item-economy sprawl unless tightly scoped; currently out of v1 content scope.
5. **Scenario Region Context / per-scenario content partitioning.** Useful if upcoming authored content needs scenario-specific Region/enemy/service state rather than global content. No current authored-content pressure; out of v1 content scope.

The next selected milestone should be narrow, testable, and justified by current gameplay value rather than system excitement.

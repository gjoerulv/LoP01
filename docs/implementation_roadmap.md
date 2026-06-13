# Ashvale Implementation Roadmap

## Context

The previous roadmap finished the v2 scope. The current codebase is a **post-M30** bounded multi-Region, multi-Scenario vertical slice.

The v1 strategic-economy proof and the v2 contested-infrastructure loop are complete: stationing, storage, claiming, cross-Region loss warnings, service defense, storage loss with Temporarily Unavailable heroes, enemy-side capture pressure, destruction/restoration, and the service-state readout/log all work together in shipped content and tests.

`docs/content_scope_v2.md` is complete and should be archived by the user. The active scope cap is now `docs/content_scope_v3.md`.

v3 is about **scenario readiness, player information, and authored progression**. The first selected v3 milestone is **M31 — Shell Entry + Scenario/Campaign Selection**.

---

## 1. Current implementation baseline

Current stable foundation:

- explicit `App` / `GameSession` flow;
- controller / mapper / renderer split;
- battle engine, CTB, static formation, leader aura, deterministic damage, and battle write-back;
- persistent roster, active/reserve party, mustering, storage, stationing, and save/load;
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
- owned-service/economy foundation with resources, owned-service runtime state, mine outputs, stack-backed stationing, storage, daily mine payout, trader ownership tiers, authored/default trader curves, validation, and proof tests;
- passive-effect spine foundation with canonical unit `passive_effects`, legacy `mine_production_passive` authoring compatibility, `mine_production` effects, and `leader_energy` effects;
- Trading Post transaction foundation and bounded Trading Post interaction flow;
- Scenario-authored player economy/service start state through `playerStart`;
- guarded and unguarded player-side owned-service claiming through `GameSession::ResolveNodeEntryClaims`;
- player-facing mine stationing flow;
- owned-service strategic readout opened with `O` from Region mode;
- Storage foundation: `Storage` service kind, 7-slot per-service stored-unit placement, `StoredUnitSaveState`, additive `stored_units`, Home Base storage service, and same-stack-id retrieve into reserve;
- cross-Region generic-unit preservation / travel warning: confirmed World Map travel removes only traveling active/reserve generic stacks; heroes/Player Character travel; stored and stationed stacks stay behind; two-stage World Map warning lists the at-risk stacks;
- service defense / stationed-defender resolution: node-level attacks against player-owned attackable services resolved by deterministic `ServiceDefenseRules` when the player is absent and by the existing interactive battle surface when the player stands on the attacked node;
- storage loss + minimal Temporarily Unavailable hero pipeline: captured services resolve placed stacks atomically; generics dismissed; heroes enter `unavailable_heroes` and return weekly to reserve when possible; Player Character can never be placed, lost, or TU;
- enemy-side service capture pressure through `ProcessEnemyPhase`, patrol-radius/current-region/arrival-node/alliance gates, and authored `enemyGroupId` strength;
- opt-in service destruction/restoration with `destroyable`, validated `restore_cost`, Energy/time costs, day-start restoration completion, and bounded `K` maintenance action;
- persisted bounded `service_event_log`, enemy-phase status lines, overview restoring status, TU heroes section, and recent-events section;
- M30 content proof: player-owned `river_depot` storage gate, destroyable copper mine, raider pressure events, and end-to-end tests.

---

## 2. Known doc/code gaps and debt

| # | Issue | Action |
|---|-------|--------|
| 1 | Team Energy has an implemented leader passive term (`leader_energy`) and a still-deferred leader item/artifact Energy term. | Keep `leader_energy` on the current unit passive spine. Do not fake item/artifact Energy until an item/artifact effect milestone exists. |
| 2 | `ContentRepository` loads only content kinds with C++ struct definitions. Recipes, full Scenario Region Contexts, full shell metadata, and several long-term authored structures still do not exist. | Add structs/loaders only when a scoped phase requires them. M31 may add shell/selection-facing metadata only if needed for real selection/validation. |
| 3 | `docs/game_shell_flow.md` specifies the full shell flow, while code still focuses on the playable slice and direct mode transitions. | v3 gap. M31 is selected to implement the first bounded shell entry and content-selection path. |
| 4 | `docs/validation_system.md` specifies a broader three-level validation model than is currently implemented. | v3 gap. M31 should add only the validation/playability gate needed for shell selection. Broader release validation belongs to a later v3 closure milestone. |
| 5 | `docs/combat_rules.md` specifies timed status effects and broader command depth. | Gap; acceptable until a scoped battle-depth milestone. |
| 6 | Player color is still effectively fixed as `Green` in several Region/enemy-team/outcome/economy/start-state/claiming paths. | Known debt. Fix only with a real player-team identity/scenario-start model. |
| 7 | Scenario/Campaign authoring is thinner than final docs: no full Scenario Region Context, no full starting roster/hero pool, no banned-content list, limited branch/carry-over policy. | v3 target. Do not overbuild in M31 unless required for shell selection safety. |
| 8 | Unit `passive_effects` support only `mine_production` and `leader_energy`; artifact `statBonus` remains on the artifact path. | Do not fold artifact/item/status behavior into the unit passive spine without a scoped milestone. |
| 9 | Trading Post interaction is implemented as a bounded text-prompt service flow, not a full shop/inventory UI. | Gap, not conflict. Build broader trader UI only in a scoped future milestone. |
| 10 | Scenario `playerStart` covers economy/service start state only; authored starting roster, full team definitions, item/artifact start state, and `unlockedRegions` overrides are intentionally absent. | v3 candidate after M31, likely Scenario Context + Start-State Authoring. |
| 11 | Scenario Result mode presents deterministic outcome and next step, but not scores, rewards, branching choices, fanfare, or post-victory event chains. | v3 candidate after shell/selection and scenario context. |
| 12 | M30's absent-player service defense is deterministic strength comparison, not full-simulation auto-resolve. | v3 candidate: threat preview + auto-resolve foundation with battle AI or battle-rule-aligned simulation. |
| 13 | Temporarily Unavailable heroes return directly to reserve after a weekly delay, standing in for shared hero-pool re-entry. | v3 candidate: shared hero pool / TU re-entry once scenario roster/hero-pool authoring exists. |
| 14 | Enemy pressure captures player-owned services only; enemy-side destruction/sabotage/restoration and enemy-vs-enemy contention remain absent. | Future scope. Do not add before scenario visibility/selection and threat systems can explain it. |
| 15 | Owned-service overview/readout is not the final service-management UI. | Keep read-only unless a scoped milestone selects service-management presentation/actions. |
| 16 | Fog/reveal/scouting/enemy inspection rules are documented but mostly unimplemented. | v3 candidate after M31/M32. |

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

`docs/content_scope_v2.md` is complete and should be archived. Active planning now uses `docs/content_scope_v3.md`.

Selected next milestone: **M31 — Shell Entry + Scenario/Campaign Selection** *(planned)*.

### M31 — Shell Entry + Scenario/Campaign Selection (planned)

**Goal:** implement the first real shell entry and content-selection gate so authored Campaigns/Scenarios can be started safely through player-facing flow instead of direct/dev-like runtime transitions.

M31 should be ambitious enough to create a useful shell foundation, but not a full final shell.

#### Required scope

M31 should implement:

1. **Boot/title/main-menu flow**
   - A title or main entry mode that can be reached on startup.
   - Main options at minimum: Continue, New Game, Load Game, Settings/Mods/Credits disabled or placeholder where appropriate, Quit if platform-safe.
   - Controller/keyboard-friendly command mapping using existing input patterns.

2. **New Game selection path**
   - Game Mode Selection with at least Campaign and Standalone Scenario.
   - Tutorial may be shown if there is a designated content entry; otherwise hidden/disabled.
   - PvP hidden/disabled.

3. **Campaign / Standalone Scenario selection**
   - Lists installed/loaded Campaigns and Scenarios from current content.
   - Standalone Scenario selection shows only entries that are intended to be standalone-selectable, or uses an explicit bounded fallback if the current schema lacks that field.
   - Rows show player-facing name/description plus validation/playability status where available.
   - Invalid/unplayable content is disabled or blocked with a clear reason.

4. **Validation/playability gate**
   - Starting selected content must run or reuse the appropriate validation checks.
   - Invalid content must not be started silently.
   - Dev-facing raw reports can remain out of scope, but player-facing reason strings must be clear enough to avoid black-box failure.

5. **Start handoff**
   - Starting a selected Scenario/Campaign should transition into the existing Scenario runtime path without bypassing `playerStart`, save/load, content validation, or Scenario outcome assumptions.
   - If character creation is not implemented, use an explicit prebuilt/default Player Character path and document it as an M31 limitation.

6. **Continue / Load Game foundation**
   - Continue should attempt to load the most recent valid save if the current save system exposes that safely; otherwise show a clear disabled/placeholder reason.
   - Load Game should show at least a bounded list/placeholder path that does not unsafe-load incompatible content.
   - Do not implement full save-slot metadata/mod compatibility unless required for a safe M31 proof.

7. **Tests and presentation**
   - Use mapper/render-model/renderer patterns where practical.
   - Add tests for shell state transitions, selection filtering, validation blocking, start handoff, and safe disabled states.

#### Explicitly out of M31

- full character creation;
- full settings/options UI;
- full Mods menu / load-order management;
- full accessibility/video/audio/controls settings;
- saved character templates;
- full save-slot browser metadata;
- full content-package/mod compatibility model;
- scenario-region partitioning unless a tiny schema addition is absolutely required to select/start content safely;
- broad UI skinning/polish beyond readable shell screens;
- new gameplay systems unrelated to starting/choosing content.

#### Acceptance criteria

- The game can boot into a shell/menu path.
- The player can choose New Game → Campaign or Standalone Scenario.
- Invalid/unplayable entries are not silently started.
- At least one shipped Scenario/Campaign can be started from shell selection and reaches the existing playable runtime.
- Continue/Load behavior is safe and either functional or clearly disabled with reason.
- Save/load compatibility remains intact.
- Existing M25–M30 gameplay tests remain green.
- Active docs/agent guidance are updated only after implementation is complete.

---

## 5. Candidate directions after M31 / future v3 milestones

These are v3 candidates, not current commitments. Select one only after auditing the post-M31 codebase.

1. **Scenario Context + Start-State Authoring.** Add the smallest Scenario Region Context / content-partitioning and authored start-state expansion needed to prevent global-content leakage and support scenario-specific rosters, services, resources, regions, and hero pools.
2. **Fog, Reveal, and Enemy Visibility Foundation.** Persistent reveal/explored state, basic scouting, visible enemy estimates, and read-model/UI support without full hidden-information AI.
3. **Threat Preview + Auto-Resolve Foundation.** Cheap threat preview for Region/service battles and battle-rule-aligned auto-resolve that can eventually replace M30's deterministic absent-player strength comparison.
4. **Quest / Guidance / Journal Foundation.** Visible objectives, guidance, event-updated goals, and victory/defeat explanation tied to authored Scenarios.
5. **v3 Content Proof + Release Validation Pass.** A small authored Scenario/Campaign proof using v3 systems plus release-readiness validation checks.

Larger systems that remain future candidates unless explicitly selected:

- full enemy AI economy and equipment behavior;
- enemy-side destruction/sabotage/restoration and enemy-vs-enemy service contention;
- full item economy and item-use systems;
- artifact inventory/equipment UI and artifact-combination services;
- full final fog/scouting polish;
- complete shell settings/mods/accessibility flow;
- campaign branch-choice, rewards, scores, fanfare, and rich Scenario Result presentation;
- advanced battle command depth, timed status effects, active abilities, spells, auto-combat controls, redo/manual battle flow;
- editor tooling.

---

## 6. Roadmap discipline

- v2 is complete. Do not keep extending v2.
- `docs/content_scope_v3.md` is the active scope cap.
- M31 is selected. Do not implement later v3 candidates inside M31 unless directly required for M31's safe shell/content-selection path.
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

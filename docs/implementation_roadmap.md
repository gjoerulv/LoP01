# Ashvale Implementation Roadmap

## Context

The previous roadmap completed the v2 scope and started v3. The current codebase is a **post-M32** bounded multi-Region, multi-Scenario vertical slice.

The v1 strategic-economy proof and the v2 contested-infrastructure loop are complete: stationing, storage, claiming, cross-Region loss warnings, service defense, storage loss with Temporarily Unavailable heroes, enemy-side capture pressure, destruction/restoration, and the service-state readout/log all work together in shipped content and tests. `docs/content_scope_v2.md` is historical and should stay archived. The active scope cap is `docs/content_scope_v3.md`.

v3 is about **scenario readiness, player information, and authored progression**. The completed v3 milestones so far are **M31 — Shell Entry + Scenario/Campaign Selection** and **M32 — Scenario Context, Start-State Authoring, and Visibility Foundation**.

The selected next milestone is **M33 — Threat Preview + Battle-Rule-Aligned Auto-Resolve Foundation**.

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
- M30 content proof: player-owned `river_depot` storage gate, destroyable copper mine, raider pressure events, and end-to-end tests;
- M31 shell entry flow: `GameMode::Title`, bounded App-local shell screen state, Continue/New Game/Quit, Campaign and Standalone Scenario selection, validation/playability gate, and single-save Continue through `saves/slot_1.json`;
- M32 Scenario Context + start-state authoring + visibility foundation: an optional authored scenario `regions` context (empty => all Regions, backward compatible) that filters the World Map read model and `TravelToRegion`; an optional `playerStart.roster` (active/reserve unit-id/quantity entries) with validation for unit existence, hero uniqueness, hero quantity, active/reserve capacity, active-leader presence, and exactly-one Player Character; a clean scenario-start reset (unconditional clock→day 1 and reveal seed; roster + inventory/equipment + TU heroes + service log reset only when a roster is authored, then overridden by campaign carry-over); a persistent per-Region HoMM-style reveal layer (radius-2 BFS seed around the start node and start-owned-service nodes, extended on movement and World Map arrival, saved/loaded); and reveal-gated enemy visibility with a bounded presence estimate in the Region read model/renderer.

---

## 2. Known doc/code gaps and debt

| # | Issue | Action |
|---|-------|--------|
| 1 | Team Energy has an implemented leader passive term (`leader_energy`) and a still-deferred leader item/artifact Energy term. | Keep `leader_energy` on the current unit passive spine. Do not fake item/artifact Energy until an item/artifact effect milestone exists. |
| 2 | `ContentRepository` loads only content kinds with C++ struct definitions. Recipes, full Scenario Region Contexts, full shell metadata, and several long-term authored structures still do not exist. | Add structs/loaders only when a scoped phase requires them. |
| 3 | `docs/game_shell_flow.md` specifies the full shell flow, while code implements only the bounded M31 entry/selection/load slice. | M31 reduced the gap. Settings/mods, save metadata, character creation, rich Load Game browser, and full campaign presentation remain future scope. |
| 4 | `docs/validation_system.md` specifies a broader three-level validation model than is currently implemented. | M31 added a shell validation/playability gate. Broader release validation belongs to a later v3 closure milestone. |
| 5 | `docs/combat_rules.md` specifies timed status effects and broader command depth. | Gap; acceptable until a scoped battle-depth milestone. |
| 6 | Player color is still effectively fixed as `Green` in several Region/enemy-team/outcome/economy/start-state/claiming paths. | Known debt. Fix only with a real player-team identity/scenario-start model. |
| 7 | Scenario/Campaign authoring is thinner than final docs. M32 added the smallest Scenario Context (an authored `regions` boundary) and an authored starting roster. Still absent: full Scenario Region Context (per-Region variable/flag/override passing), per-scenario content directories, authored hero pool, banned-content list, richer branch/carry-over policy. | Partially addressed by M32. Remaining items are future scope. |
| 8 | Unit `passive_effects` support only `mine_production` and `leader_energy`; artifact `statBonus` remains on the artifact path. | Do not fold artifact/item/status behavior into the unit passive spine without a scoped milestone. |
| 9 | Trading Post interaction is implemented as a bounded text-prompt service flow, not a full shop/inventory UI. | Gap, not conflict. Build broader trader UI only in a scoped future milestone. |
| 10 | Scenario `playerStart` now also authors a starting roster (`playerStart.roster`) alongside economy/service start state. Authored hero pool, item/artifact start-state, and per-scenario `unlockedRegions` overrides remain absent. M32 resets inventory/equipment to empty on an authored-roster start (no positive item/artifact authoring), so a clean item/artifact start-state is the only safe subset added. | Roster done by M32. Hero pool and authored item/artifact start-state remain future. |
| 11 | Scenario Result mode presents deterministic outcome and next step, but not scores, rewards, branching choices, fanfare, or post-victory event chains. | Future v3 candidate after Scenario Context and quest/guidance foundations. |
| 12 | M30's absent-player service defense is deterministic strength comparison, not full-simulation auto-resolve. | Selected next: M33 should add threat preview and battle-rule-aligned auto-resolve foundations without inventing a second battle system. |
| 13 | Temporarily Unavailable heroes return directly to reserve after a weekly delay, standing in for shared hero-pool re-entry. | Broad hero recovery/recruitment remains future scope. |
| 14 | Enemy pressure captures player-owned services only; enemy-side destruction/sabotage/restoration and enemy-vs-enemy contention remain absent. | Future scope. Do not add before scenario visibility/selection and threat systems can explain it. |
| 15 | Owned-service overview/readout is not the final service-management UI. | Keep read-only unless a scoped milestone selects service-management presentation/actions. |
| 16 | M32 implemented the bounded fog/reveal foundation: persistent per-Region radius-2 reveal seeded at start and extended on movement/World Map arrival, with reveal-gated enemy presence + a bounded estimate in the Region read model. Full scouting ranges/passives, line-of-sight geometry, reveal services, quantity/level-band inspection depth, and hidden-information AI remain unimplemented. | Foundation done by M32. Polish/inspection depth/AI knowledge are future scope. |
| 17 | M32 fixed the start-state leakage for scenarios that author start-state: clock resets to day 1 unconditionally, and roster + inventory/equipment + TU heroes + service log reset on an authored-roster start so a second New Game does not inherit the previous run. A scenario that authors no roster still keeps the prebuilt default roster/inventory (the documented M16/M31 default-roster path, relied on by v1/v2 economy-proof content tests). | Resolved by M32 for authored-roster scenarios; default-roster scenarios intentionally unchanged. |

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
- **Phase 23 — Shell Entry + Scenario/Campaign Selection:** M31 complete.
- **Phase 24 — Scenario Context, Start-State Authoring, and Visibility Foundation:** M32 complete.

---

## 4. Current next milestone

Latest completed milestone: **M32 — Scenario Context, Start-State Authoring, and Visibility Foundation**.

`docs/content_scope_v2.md` is historical/archived. Active planning uses `docs/content_scope_v3.md`.

Selected next milestone: **M33 — Threat Preview + Battle-Rule-Aligned Auto-Resolve Foundation**.

### M33 — Threat Preview + Battle-Rule-Aligned Auto-Resolve Foundation (planned)

**Goal:** make Region/service conflict outcomes understandable before commitment and start replacing M30's deterministic absent-player service-defense stand-in with a battle-rule-aligned auto-resolve foundation.

M33 should improve player-facing decision quality and system consistency. The player should get a cheap, bounded preview before dangerous Region/service battles, and absent-player or AI-vs-AI service conflicts should resolve through a deterministic auto-resolve path that is derived from the existing battle model where practical. Do **not** invent a second battle engine.

#### Required M33 deliverables

1. **Threat preview model**
   - Add a cheap threat preview for player-relevant Region/service fights before committing to travel or service defense where the attacker/defender are knowable under the M32 reveal rules.
   - Preview must be bounded and player-facing: show relative danger / rough outcome band, not exact hidden debug stats unless the information is already visible.
   - Respect reveal/visibility: unrevealed or hidden enemies should not leak exact composition through preview.
   - Keep preview read-only. It must not mutate battle, roster, service, or enemy-team state.

2. **Battle-rule-aligned auto-resolve foundation**
   - Introduce a deterministic auto-resolve service or rules module that uses the same durable unit stats and battle assumptions as the existing battle system where practical.
   - It may remain simplified, but it must be closer to battle rules than M30's raw strength comparison and must have tests proving stable outcomes.
   - Use it for absent-player service-defense resolution first, replacing or wrapping `ServiceDefenseRules` where scoped and safe.
   - Do not use this milestone to build full battle AI, action scripting, redo/manual choice, auto-combat controls, or timed status effects.

3. **Integration with service defense and Region travel**
   - Service attacks against player-owned mines/storage/direct services should use the new auto-resolve path when the player is absent.
   - Player-present service defense should continue using the existing interactive battle surface unless a safe preview-only hook is added.
   - Region travel into hostile nodes should surface a preview/warning when the hostile force is visible/known enough.
   - Existing M25-M32 semantics must remain intact: stationing/storage placement, generic travel loss, reveal-gated enemy visibility, service capture/loss/destruction/restoration, and save/load.

4. **Presentation/read-model support**
   - Extend existing Region/World Map/service read models enough to show a bounded threat preview where the player can act on it.
   - Do not build a full final combat-planning UI.
   - Use display names and player-facing bands. Avoid id-heavy debug strings.
   - Keep the UI compatible with M32 fog/reveal: unknown enemies should remain unknown.

5. **Validation and tests**
   - Add focused tests for preview calculation, visibility gating, auto-resolve determinism, service-defense integration, and no unintended roster/service mutation during preview.
   - Add end-to-end tests showing that absent-player service defense uses the new path and preserves stack/save invariants.
   - Keep existing battle, service defense, storage, stationing, World Map, reveal, shell, and save/load tests green.

#### Explicit M33 out of scope

- full battle AI;
- player-command automation / auto-combat controls;
- redo/manual battle selection;
- timed status effects, spells, broad skill trees, or advanced command depth;
- full final fog/scouting polish;
- exact hidden enemy composition leak on unrevealed nodes;
- full service-management UI;
- enemy-side economy expansion;
- enemy-side destruction/sabotage/restoration beyond existing M30 capture pressure;
- quest/guidance/journal foundation;
- release-validation/content-proof closure;
- save schema migration unless a focused audit proves it unavoidable.

#### M33 acceptance criteria

M33 is complete when:

- visible/known hostile Region/service threats expose a bounded preview before player commitment;
- hidden/unrevealed threats do not leak exact composition through preview;
- absent-player service-defense resolution uses a deterministic battle-rule-aligned auto-resolve path instead of the old raw strength-only comparison;
- preview is pure/read-only and tested not to mutate runtime state;
- auto-resolve preserves roster, stationing, storage, service ownership/loss, TU, and save/load invariants;
- existing M25-M32 tests remain green;
- `docs/implementation_roadmap.md` is updated to mark M33 complete only after tests/manual validation pass.

---

## 5. Candidate directions after M33 / future v3 milestones

These are v3 candidates, not current commitments. Select one only after auditing the post-M33 codebase.

1. **Quest / Guidance / Journal Foundation.** Visible objectives, guidance, event-updated goals, and victory/defeat explanation tied to authored Scenarios.
2. **v3 Content Proof + Release Validation Pass.** A small authored Scenario/Campaign proof using v3 systems plus release-readiness validation checks.

Larger systems that remain future candidates unless explicitly selected:

- full enemy AI economy and equipment behavior;
- enemy-side destruction/sabotage/restoration and enemy-vs-enemy service contention;
- full item economy and item-use systems;
- artifact inventory/equipment UI and artifact-combination services;
- full final fog/scouting polish beyond the M32 foundation;
- complete shell settings/mods/accessibility flow;
- character creation and full hero-pool/recruitment UI;
- save-slot metadata and full Load Game browser;
- campaign branch-choice, rewards, scores, fanfare, and rich Scenario Result presentation;
- advanced battle command depth, timed status effects, active abilities, spells, auto-combat controls, redo/manual battle flow;
- editor tooling.

---

## 6. Roadmap discipline

- v2 is complete. Do not keep extending v2.
- `docs/content_scope_v3.md` is the active scope cap.
- M31 and M32 are complete.
- M33 is selected and planned.
- Do not bundle future candidates into M33 unless the active scope is revised deliberately.

Milestone-agnostic docs remain source-of-truth for final rules:

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

Update milestone-agnostic docs only when the actual long-term rule, schema, terminology, or architectural direction changes.

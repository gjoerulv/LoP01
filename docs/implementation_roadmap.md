# Ashvale Implementation Roadmap

## Context

The previous roadmap finished the v2 scope. The current codebase is a **post-M30** bounded multi-Region, multi-Scenario vertical slice.

The v1 strategic-economy proof and the v2 contested-infrastructure loop are complete: stationing, storage, claiming, cross-Region loss warnings, service defense, storage loss with Temporarily Unavailable heroes, enemy-side capture pressure, destruction/restoration, and the service-state readout/log all work together in shipped content and tests.

`docs/content_scope_v2.md` is complete and should be archived by the user. The active scope cap is now `docs/content_scope_v3.md`.

v3 is about **scenario readiness, player information, and authored progression**. The first completed v3 milestone is **M31 — Shell Entry + Scenario/Campaign Selection**.

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
| 2 | `ContentRepository` loads only content kinds with C++ struct definitions. Recipes, full Scenario Region Contexts, full shell metadata, and several long-term authored structures still do not exist. | Add structs/loaders only when a scoped phase requires them. M31 added no new content kinds; shell selection reuses the existing definitions. |
| 3 | `docs/game_shell_flow.md` specifies the full shell flow, while code still focuses on the playable slice and direct mode transitions. | Reduced by M31: bounded shell entry, content selection, the validation gate, and single-save Continue shipped. The remaining shell flow (settings/mods, save metadata, character creation, Load Game browser) stays future scope. |
| 4 | `docs/validation_system.md` specifies a broader three-level validation model than is currently implemented. | Reduced by M31: the shell runs a global content-error gate plus per-entry reference checks before any start. Broader release validation belongs to a later v3 closure milestone. |
| 5 | `docs/combat_rules.md` specifies timed status effects and broader command depth. | Gap; acceptable until a scoped battle-depth milestone. |
| 6 | Player color is still effectively fixed as `Green` in several Region/enemy-team/outcome/economy/start-state/claiming paths. | Known debt. Fix only with a real player-team identity/scenario-start model. |
| 7 | Scenario/Campaign authoring is thinner than final docs: no full Scenario Region Context, no full starting roster/hero pool, no banned-content list, limited branch/carry-over policy. | v3 target. M31 did not overbuild here; this is the natural next-milestone candidate. |
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
- **Phase 23 - Shell Entry + Scenario/Campaign Selection:** M31 complete.

---

## 4. Current next milestone

Latest completed milestone: **M31 — Shell Entry + Scenario/Campaign Selection**.

`docs/content_scope_v2.md` is complete and should be archived. Active planning uses `docs/content_scope_v3.md`.

The next milestone is **not yet selected**. See §5 for candidates; Scenario Context + Start-State Authoring is the most direct continuation (the shell now exposes the M16-era start-state limitations documented below).

### M31 — Shell Entry + Scenario/Campaign Selection (complete)

**Goal met:** the game boots into a real shell entry path. Authored Campaigns and Standalone Scenarios are chosen through player-facing selection with a validation/playability gate, Continue is a bounded single-save load, and invalid content or failed loads keep the player in the shell with readable reasons.

#### Delivered

- **Boot/main menu:** `GameMode::Title` hosts the shell; the App-local screen state machine (MainMenu → GameModeSelect → CampaignSelect/ScenarioSelect) is never persisted. Main menu: **Continue / New Game / Quit** with per-item enabled state and a bounded status line. Settings/Mods/Credits/Tutorial/PvP are hidden entirely (no real systems/content behind them yet) rather than shown as dead entries.
- **New Game selection:** Game Mode Selection lists Campaign and Standalone Scenario (rows disabled with reasons when no content of that kind is installed). Campaign rows show authored name + description; scenario rows show authored name plus a generated "Starts in <Region>." context line. Ids appear nowhere as primary text. The standalone list enforces `standaloneSelectable` (`game_shell_flow.md` §9/§11) — `scenario_second` stays campaign-only.
- **Validation/playability gate:** a global content gate (load failure, or any `ContentValidator` error, counted once at boot) blocks every start and disables every row with the same reason; cheap per-entry reference checks (`app::shell::ShellSelectionRules` — campaign start scenario resolves; scenario start region/node resolve) disable individual rows with their own reasons. The gate re-runs at confirm time, so invalid content is never silently started. Raw validation reports stay dev-facing (§12/§26).
- **Start handoff:** campaign starts use the existing `GameSession::StartCampaign`; standalone starts use the new `GameSession::StartStandaloneScenario` (campaign-state cleared, same `TransitionToScenario` chokepoint: scenario-local reset, `playerStart`, outcome selection, RegionMode). Nothing bypasses save/load, validation, or outcome assumptions.
- **Continue:** bounded single-save path (`saves/slot_1.json`). Missing file ⇒ disabled item + "No save found" on confirm; unreadable/incompatible file ⇒ readable failure with no session mutation (`LoadFromFile` is exception-safe and returns nullopt before any state is touched). Load Game is intentionally not a separate entry — there is exactly one save, so it would duplicate Continue. Quicksave (F5) is suppressed at the shell so a fresh session can never overwrite a real save; quickload (F9) remains.
- **Presentation/tests:** the shell reuses the existing controller/mapper/renderer pattern — the pure `CampaignController` navigates every list (now honoring Up/Down as its footer always promised), `ShellModelMapper` builds all screen models, and the generic `CampaignSelectModel`/`TitleScreenModel` gained enabled/status fields. Tests cover the rules, the mappers, controller navigation, standalone start semantics, and save/load round-trips.

#### Documented M31 limitations (deliberate)

- **No character creation:** every start uses the existing prebuilt default Player Character path (the M9-era startup roster with `hero_player`). This is the explicit prebuilt/default path the milestone allows; full creation flow is a future v3 candidate.
- **New Game start-state inherits the M16 semantics:** `TransitionToScenario` resets gold/resources/services/flags/outcomes/quests/enemy teams per `playerStart`, but the roster, clock, and inventory continue from the current session (there is no scenario-authored roster yet). Starting a New Game after finishing a previous run in the same process therefore keeps that run's roster/day. This is the pre-existing campaign-start behavior, now surfaced through the shell; the Scenario Context + Start-State milestone owns the real fix.
- **Single-save Continue:** no save slots, no save metadata (version/content ids), no grouped Load Game browser (`game_shell_flow.md` §14/§17 remain future scope).
- **Legacy compatibility:** `GameMode::CampaignSelectMode` and `OpeningSequence` remain functional for old saves but are no longer entered by the shell. The old cancel-at-title direct dev start was removed from player input; `GameSession::AdvanceMode` remains as the session-level dev/test seam.

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
- M31 is complete. Select the next v3 milestone from this roadmap's candidates after a code/doc audit; do not bundle later candidates into unscoped work.
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

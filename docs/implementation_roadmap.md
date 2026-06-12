# Ashvale Implementation Roadmap

## Context

The previous roadmap was archived after M29. This file is the new active implementation roadmap for finishing the active v2 scope in `docs/content_scope_v2.md`.

The current codebase is a **post-M29** bounded multi-Region, multi-Scenario vertical slice. The v1 strategic-economy proof is complete, and v2 has already delivered the major player-facing infrastructure-control foundations: player-facing mine stationing, systemic player-side service claiming, owned-service overview/readout, Storage, and cross-Region generic-unit loss warnings.

The active scope cap is still `docs/content_scope_v2.md`. Archived docs, including `docs/content_scope_v0.md.archived`, older `docs/implementation_roadmap.md.*.archived` files, and `docs/content_scope_v1.md.archived`, are historical context only.

M30 is intentionally an umbrella completion milestone. It should finish the remaining v2 infrastructure-control loop, then leave the project ready to archive `docs/content_scope_v2.md` and create a future v3 scope. Do **not** treat M30 as permission to implement the entire final game.

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
- cross-Region generic-unit preservation / travel warning: confirmed World Map travel removes only traveling active/reserve generic stacks; heroes/Player Character travel; stored and stationed stacks stay behind; two-stage World Map warning lists the at-risk stacks.

Still incomplete or intentionally deferred before v2 can close:

- service defense / stationed-defender resolution;
- direct-storage gate defense and storage loss consequences;
- service destruction/restoration;
- enemy-side capture or denial pressure against owned services;
- Temporarily Unavailable hero pipeline needed by storage-loss and other hero-dismissal rules;
- service-state presentation for capture, defense, destruction, restoration, and loss events;
- v2 closure audit to decide what remains for v3 rather than leaving ambiguous candidate lists behind.

---

## 2. Known doc/code gaps and debt

| # | Issue | Action |
|---|-------|--------|
| 1 | Team Energy has an implemented leader passive term (`leader_energy`) and a still-deferred leader item/artifact Energy term. | Keep `leader_energy` on the current unit passive spine. Do not fake item/artifact Energy until an item/artifact effect milestone exists. |
| 2 | `ContentRepository` loads only content kinds with C++ struct definitions. Recipes, full Scenario Region Contexts, and several long-term authored structures still do not exist. | Add structs/loaders only when a scoped phase requires them. M30 should not create broad content directories unless service-defense/destruction authoring proves it necessary. |
| 3 | `docs/game_shell_flow.md` specifies the full shell flow, while code still focuses on the playable slice and direct mode transitions. | Gap, not conflict. Full shell remains deferred unless M30 needs only a tiny warning/result surface. |
| 4 | `docs/validation_system.md` specifies a broader three-level validation model than is currently implemented. | M30 may add validation only for service-defense/destruction/storage-loss authoring that it introduces. |
| 5 | `docs/combat_rules.md` specifies timed status effects and broader command depth. | Gap; acceptable until skill/status phases. |
| 6 | Player color is still effectively fixed as `Green` in several Region/enemy-team/outcome/economy/start-state/claiming paths. | Known debt. Do not fix opportunistically unless M30 needs a real player-team identity model. Prefer color-parametric helper APIs where touched. |
| 7 | `scenario_outcome.json` remains a bounded-slice authoring file, while `ScenarioDefinition` supports only a subset of long-term Scenario authoring. | Intentional sequencing. M30 should not expand Scenario authoring except if v2 closure needs a narrow proof scenario. |
| 8 | Unit `passive_effects` support only `mine_production` and `leader_energy`; artifact `statBonus` remains on the artifact path. | Do not fold artifact/item/status behavior into the unit passive spine during M30. |
| 9 | Trading Post interaction is implemented as a bounded text-prompt service flow, not a full shop/inventory UI. | Gap, not conflict. M30 should not build a broad shop UI unless explicitly kept to a tiny service-state proof. |
| 10 | Scenario `playerStart` covers economy/service start state only; authored starting roster, full team definitions, item/artifact start state, and `unlockedRegions` overrides are intentionally absent. | Add only if M30's v2-completion proof cannot be authored otherwise. |
| 11 | Scenario Result mode presents deterministic outcome and next step, but not scores, rewards, branching choices, fanfare, or post-victory event chains. | Leave for v3 unless M30's closure audit explicitly selects a tiny branch-choice or result-extension proof. |
| 12 | Owned-service claiming covers both player-side guarded capture and general player-side claiming. It does not implement enemy-side capture, sabotage, or destruction/restoration. | M30 should address the narrow enemy-side/service-defense/destruction path needed for v2 infrastructure pressure. |
| 13 | `mine_production` is implemented, content-authored, and player-facing. | Do not change payout semantics during M30. Service defense may use stationed units defensively, but production rules remain strongest-only and unchanged. |
| 14 | M27 implements an owned-service overview/readout, not the final service-management UI. | M30 may extend the readout/logs with defense/destruction/restoration state. Do not turn it into remote management. |
| 15 | Storage placement exists, but direct-storage gate defense, storage-loss consequences, and Temporarily Unavailable heroes are not implemented. | M30 should implement the minimal final-rule-compatible version if service defense includes storage gates. |
| 16 | World Map generic-loss warning now makes Storage strategically necessary, but stored/stationed units still cannot defend anything. | M30 should make placed units strategically matter through service defense. |

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

---

## 4. Current next milestone

Latest completed milestone: **M29 — Cross-Region Generic Unit Preservation / Travel Warning**.

Active scope cap: **`docs/content_scope_v2.md`**.

Selected next milestone: **M30 — v2 Completion: Contested Infrastructure, Service State, and Closure Audit**.

M30 is planned, not implemented.

M30 should complete the active v2 scope by turning the already-built service/storage/stationing loop into a contested infrastructure loop. It must directly serve the milestone-agnostic docs and should be ambitious enough to close v2, but it must not silently expand into the full final game.

### M30 — v2 Completion: Contested Infrastructure, Service State, and Closure Audit (planned)

#### Goal

Complete `docs/content_scope_v2.md` by implementing the remaining narrow infrastructure-control consequences that make owned Services, stationed units, stored units, and enemy pressure form a coherent playable loop.

M30 should finish v2 through these deliverables:

1. **Service defense / stationed-defender resolution**
   - Implement a narrow resolver for attacks against owned Region Services that have defenders.
   - Mines may use stationed units as defenders where applicable.
   - Direct storage Services may use stored units as defenders, following the final storage-gate rule.
   - If the traveling party is present on the attacked storage node, use the active party for the defense battle as the final docs specify.
   - AI-vs-AI or enemy-vs-absent-service battles may auto-resolve without interrupting the player, but must be deterministic and testable.
   - Player-involved defense should use the existing battle/result surfaces where practical; do not build the full threat-preview/redo/manual-battle system unless the current battle flow already supports the needed path safely.

2. **Storage loss and Temporarily Unavailable hero foundation**
   - If a direct storage defense is lost, generic stored units at that storage are dismissed.
   - Stored heroes become Temporarily Unavailable through a minimal explicit runtime/save pipeline.
   - The Player Character must never enter the Temporarily Unavailable path.
   - Temporarily Unavailable heroes should remain hidden/unavailable until the defined return timing. Use the simplest final-rule-compatible weekly return if implementing return is required; otherwise clearly keep return timing bounded and tested.
   - Do not implement broad hero pool/recruitment redesign beyond what storage-loss requires.

3. **Enemy-side service capture pressure**
   - Let enemy teams contest or capture player-owned Region Services only through legal Region-layer action and the defense resolver.
   - Ownership transfers immediately when capture succeeds, matching the final ownership rule.
   - Do not allow enemy teams to enter Locations or use Location-only service interactions.
   - Do not implement a full AI economy. Use a narrow priority/action path that can attack/capture/deny strategically relevant owned Services when legal.
   - Preserve allied ownership boundaries: allies do not automatically share ownership or benefits.

4. **Service destruction / restoration slice**
   - Implement the minimal systemic destruction/restoration model for Region Services only.
   - A service must be explicitly destroyable before it can be destroyed.
   - Destruction requires legal occupation and the specified Energy/time cost.
   - Restoration uses resources, queues restoration, and completes at next day start unless destroyed again before restoration completes.
   - Guarded services may not be destroyed until guardians/defenders are defeated.
   - Do not apply systemic destruction to ordinary Location-mode service interactions unless a service is authored as a Region Service / direct Service node.

5. **Service-state presentation and logs**
   - Extend the existing overview/readout, Region-node status, or bounded notifications so the player can understand:
     - owned service captured/lost;
     - service attacked/defended;
     - service destroyed/restoring/restored;
     - storage lost and heroes becoming Temporarily Unavailable;
     - stationed/stored defenders participating or being lost.
   - Use existing render-model/mapper/renderer patterns.
   - Do not build a full final service-management UI.

6. **Minimal content proof**
   - Add only the content needed to prove the loop:
     - at least one direct Region storage Service or storage gate if Home Base storage is Location-only and cannot prove direct gate defense;
     - at least one destroyable owned Region Service or mine;
     - an enemy team that can legally pressure the service without requiring a full AI economy;
     - tests and one manual path proving attack/defense/capture/destruction/restoration.
   - Do not create a large campaign, full economy simulation, editor tooling, or broad Scenario Region Context system.

7. **v2 closure audit**
   - At the end of M30, update this roadmap and `docs/content_scope_v2.md` to state that v2 is complete and ready to archive.
   - Move unfinished large systems into a future v3 candidate list rather than leaving them as ambiguous v2 promises.
   - Do not rewrite milestone-agnostic docs unless M30 exposes an actual source-of-truth contradiction.

#### Required planning pressure-tests before implementation

Claude/Fable must explicitly answer these before coding:

1. **What is a defensible Service in current code and content?**
   - Distinguish direct Region Services, Location Services, mines, and storage gates.
   - Do not treat every Location service as systemically attackable.

2. **What exact defender roster is used?**
   - Mine: stationed units only, or stationed units plus occupying team if present?
   - Direct storage: stored units when the owner is absent; active party if the traveling party is present on the node.
   - Empty storage/mine: define whether capture/destruction succeeds without battle.

3. **What battle path is used?**
   - Reuse existing battle simulation/write-back where possible.
   - For absent-player service defense, prefer deterministic auto-resolve and log the outcome.
   - Do not invent a second battle engine.

4. **How are stored/stationed refs updated after battle?**
   - No dangling refs.
   - No duplicate stacks.
   - No Player Character loss.
   - Stored hero loss uses the Temporarily Unavailable path if storage-loss is implemented.

5. **What does enemy capture mean?**
   - Which service kinds transfer ownership?
   - Which service kinds can be destroyed instead?
   - How does capture interact with stationed/stored defenders?
   - Are inherited stationed/stored refs cleared, transferred, dismissed, or preserved? Pick explicit rules and test them.

6. **What is the minimal AI pressure path?**
   - Enemy teams should be able to threaten infrastructure, but M30 must not become a full AI economy/minimax project.
   - Use deterministic, medium-depth, bounded action selection.

7. **What content proves completion without turning v2 into a campaign rewrite?**
   - Prefer one tight proof loop over many shallow systems.

#### Hard constraints

- Do not implement full AI economy.
- Do not implement full item economy, crafting, cooking, recipes, seeds, or broad item-use systems.
- Do not implement full shell/menu/character-creation/load/settings flow.
- Do not implement full fog-of-war / scouting.
- Do not implement full Scenario Region Context / per-Scenario content directories unless M30 proves it is unavoidable.
- Do not implement broad trader-service families unless one narrow service is required for the infrastructure proof.
- Do not implement remote stationing/storage management from the overview.
- Do not mutate authored content definitions during gameplay.
- Keep runtime state in save/runtime structures.
- Keep mutations behind `GameSession` or dedicated gameplay services, not direct `App` edits.
- Preserve M25/M28 one-place-at-a-time stack placement invariants.
- Preserve M29 cross-Region generic-loss semantics.
- Preserve save/load compatibility unless a scoped migration is explicitly justified.
- Keep UI bounded and inspectable.

#### Exact source areas to inspect when planning M30

- `src/gameplay/GameSession.h`
- `src/gameplay/GameSession.cpp`
- service ownership / claim / station / storage methods
- enemy-team Region action phase
- battle setup, auto-resolve, and battle write-back
- `src/core/SaveGame.h`
- `src/core/SaveGame.cpp`
- `src/data/definitions/LocationServiceDefinition.h`
- content loading and validation around service definitions
- Region node/service content files
- `src/app/App.cpp`
- owned-service overview mapper/renderer
- World Map / Region render models and notifications
- tests around service claiming, stationing, storage, generic travel loss, enemy teams, battle, save/load, and content validation

#### Expected tests

M30 should include focused tests for:

- service-defense rule selection;
- mine stationed-defender battle/auto-resolve path;
- direct-storage defense path;
- storage-loss consequences, including generic dismissal and stored hero Temporarily Unavailable behavior;
- Player Character cannot be stored/lost/TU through service defense;
- enemy capture transfers ownership only when legal;
- destroyed/restoring/restored service state, day rollover, and availability gates;
- no dangling stationed/stored refs after capture/destruction/loss;
- no duplicate or lost unintended roster stacks;
- save/load round-trip for new service-defense/destruction/TU state;
- overview/log/status presentation of service attack, capture, destruction, restoration, and TU events;
- real-content proof loop;
- all M25/M26/M27/M28/M29 tests remain green.

#### Manual validation target

A successful M30 manual proof should look like this:

1. Player owns a mine and/or direct storage gate.
2. Player leaves generic units stationed/stored there deliberately.
3. Enemy pressure reaches the service.
4. If defenders exist, a service-defense result is resolved and reported.
5. If the defender wins, ownership/state remains stable.
6. If the attacker wins, ownership/loss/destruction consequences resolve without dangling refs.
7. Destroyable service can be destroyed and later restored through the bounded rules.
8. Overview/readout/logs show what happened.
9. Save/reload preserves the result.

M30 is complete only when this proof works and `docs/content_scope_v2.md` can be marked complete/ready to archive.

---

## 5. Candidate directions after v2 / future v3

These are not v2 commitments. After M30, create a new active content scope before selecting one.

1. **Full enemy AI economy and equipment behavior.** Trading Post, Market, Black Market, Freelancer's Guild, artifact combination, cooking, service use, and resource strategy across AI teams.
2. **Full item economy and item-use systems.** Items, seeds, ingredients, food, consumables, field use, battle `Item` command, and broader item-effect execution.
3. **Artifact inventory/equipment UI and artifact-combination services.** Inventory presentation can start narrow, but broad artifact management belongs in its own scope.
4. **Fog-of-war, scouting, reveal, and enemy inspection.** Includes team-specific explored/revealed state, scouting levels, visibility, and threat/inspection detail.
5. **Full shell/menu/character creation/load/settings flow.** Implement according to `docs/game_shell_flow.md` when the project is ready for shell-level polish.
6. **Scenario Region Context / per-Scenario content partitioning.** Useful when authored content needs scenario-specific Region/enemy/service state rather than global content.
7. **Campaign branch-choice and richer Scenario Result presentation.** Branch-choice UI, scores, rewards, fanfare, post-victory event chains, and scenario-transition carry-over policy.
8. **Advanced battle command depth.** Timed status effects, active abilities, spells, broader MP/item use, auto-combat controls, threat preview, and redo/manual battle flow.
9. **Editor tooling.** Only after content schema and validation needs justify it.

---

## 6. Roadmap discipline

- `docs/content_scope_v2.md` remains the active scope cap until M30 is complete.
- When M30 completes, archive `docs/content_scope_v2.md` and create a new `docs/content_scope_v3.md` before selecting the next milestone.
- Do not keep extending v2 indefinitely.
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

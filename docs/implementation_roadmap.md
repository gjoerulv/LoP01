# Ashvale Implementation Roadmap

## Context

The current codebase is a post-M16 bounded multi-Region, multi-Scenario vertical slice.

The stable foundation now includes battle, roster, save/load, basic Region/Location flow, content validation foundation, typed events, the practical Phase 3 enemy-team Region-layer slice, deterministic scenario outcome evaluation, a minimal inventory + artifact-equipping layer, the team Energy pool, a minimal World Map layer, and a minimal Campaign System.

This roadmap works from foundations outward:

1. validation;
2. typed event/condition core;
3. enemy teams on the Region layer;
4. victory/defeat;
5. inventory and artifacts;
6. Energy;
7. World Map;
8. Campaign;
9. owned services and economy.

`docs/content_scope_v1.md`, `docs/technical_direction.md`, `docs/presentation_game_feel.md`, and `docs/game_shell_flow.md` are active guidance documents. Respect them when their subject matter is touched. Do not pre-implement shell screens, presentation behavior, or large content growth beyond what the current milestone requires.

Archived docs, including `docs/content_scope_v0.md.archived` and `docs/implementation_roadmap.md.00.archived`, are historical context only.

---

## 1. Current implementation baseline

Current stable foundation:

- explicit `App` / `GameSession` flow;
- controller / mapper / renderer split;
- battle engine, CTB, static formation, leader aura, deterministic damage, and battle write-back;
- persistent roster, active/reserve party, mustering, and save/load;
- daily clock, Region travel, wake/recovery penalty, and basic services;
- team Energy pool with daily-starting formula, spend/recover primitives, day-rollover reset, save/load, legacy-save recompute, snapshot exposure, and HUD exposure;
- JSON content loading through `ContentRepository`;
- content validation foundation;
- typed event foundation with optional `events.json`, typed conditions/actions, story flags, fired event ids, validation, and integration tests;
- enemy-team Region-layer foundation with runtime state, deterministic patrol movement, hostile occupation, travel blocking, contact battle, exact team clearing, mutation event actions, save/load, and tests;
- scenario outcome foundation with pure rules, authored/default victory, authored defeat, defeat priority, latching, save/load, ordered integration hooks, placeholder feedback, and tests;
- inventory and artifact foundation with item/artifact definitions, unsupported-effect validation, team-shared inventories, per-hero equipment, equip/unequip methods, grant/remove event actions, equipped-artifact battle stat bonuses, save/load, authored proof content, and tests;
- minimal World Map with region entries, adjacency, exit-node-gated travel, 1000 Energy spend, before-11:00 departure gate, next-day 11:00 arrival, generic-unit loss warning/removal, unlocked-region save/load, model/controller/renderer, authored proof content, and tests;
- minimal Campaign System with thin scenario and campaign definitions, transition graph, allow-list carry-over, campaign runtime state, scenario transition chokepoint, outcome-based advancement, save/load, presence-gated campaign selection, authored proof content, and tests.

Still incomplete or intentionally deferred:

- polished scenario-end/result screen flow;
- victory event chains;
- richer scenario outcome condition leaves;
- per-team / multi-human scenario outcome tracking;
- softlock / victory-reachability proof validation;
- advanced enemy AI economy/service use;
- enemy recruitment and sabotage/destruction/restoration loops;
- fog/visibility per team;
- Energy formula leader passive/item/artifact bonus seams beyond the current zero-valued arguments;
- item use, food consumption, cooking, recipes, seeds, ingredients;
- artifact combination and artifact-handler services;
- battle `Item` command and item use in battle;
- battle spoils transfer, gold steal, consumable steal;
- `teamHasItem` / `teamHasArtifact` condition leaves;
- Market / Black Market / Trading Post / Freelancer's Guild item economy;
- HUD/raylib inventory rendering and inventory render-model;
- event-driven region unlock;
- per-region world/enemy state partitioning;
- generic origin-storage across Regions;
- full per-Scenario `ScenarioDefinition` content kind;
- authored starting rosters and full campaign branching-choice UI;
- full shell/menu/character-creation/load/settings flow.

---

## 2. Doc/code conflicts and known debt

| # | Issue | Action |
|---|-------|--------|
| 1 | Team Energy pool is implemented, but leader passive and leader item/artifact bonus terms are zero-valued seams. | Close the seams when passive skills and artifact/item Energy effects exist. Do not fake the values in content or UI. |
| 2 | `ContentRepository` loads only content kinds with C++ struct definitions. Items, artifacts, World Map, thin scenarios, and campaigns have loaders; recipes and full per-Scenario Region Contexts do not. | No conflict. Add structs/loaders only when a scoped phase requires them. |
| 3 | `docs/game_shell_flow.md` specifies the full shell flow, while code still focuses on the playable slice and direct mode transitions. | Gap, not conflict. Full shell remains deferred. |
| 4 | `docs/validation_system.md` specifies a broader three-level validation model than is currently implemented. | Expand validation only when a phase requires it. |
| 5 | `docs/combat_rules.md` specifies timed status effects and broader command depth. | Gap; acceptable until skill/status phases. |
| 6 | Player color may still be hardcoded as `Green` in some Region/enemy-team/outcome paths. | Known debt. Do not fix opportunistically unless introducing a real player-team identity model. |
| 7 | Hostile contact requires a configured node `battleScenarioId`. | Intentional rule. Missing encounters produce diagnostics, not fallback battles. Future validation should enforce this for hostile-contact-capable nodes. |
| 8 | `scenario_outcome.json` is a bounded-slice authoring file, not the full `ScenarioDefinition` outcome model. | Intentional M12 compromise. Full Scenario authoring comes later. |
| 9 | Owned-service and economy docs are more advanced than runtime. | This is the next intended implementation frontier. M17 should implement the narrow foundation only. |

No true design contradictions are currently known. Remaining gaps are implementation sequencing issues.

---

## 3. Completed implementation phases

### Phase 1 — Content Validation System

**Status:** Foundation implemented; broader validation model still expandable.

Goal: prevent structurally invalid authored content from reaching playable state; unblock safe content authoring.

Future expansion remains allowed when host systems require additional validators.

### Phase 2 — Minimal Typed Event Foundation

**Status:** Foundation implemented.

Goal: typed events, conditions, and actions exist in C++ and evaluate deterministically.

Implemented foundation includes event definitions, triggers, typed condition/action evaluation, story flag persistence, one-shot fired guards, priority ordering, enemy-team mutation actions, validation, save/load, and tests.

### Phase 3 — Enemy Teams on Region Layer

**Status:** Practical Phase 3 slice completed by M11-e.

Goal: enemy teams spawn, move, act, occupy nodes, block player interaction, and can be cleared through contact battle.

Completed baseline includes enemy runtime state, fixed-color-order phase, patrol movement, hostile occupation, hostile markers, configured hostile-contact battles, exact-team clearing, mutation event actions, persistence, and tests.

Deferred beyond Phase 3: advanced AI economy/service use, enemy recruitment, sabotage/destruction/restoration loops, cross-Region enemy persistence/travel, fog/visibility/scouting, full enemy inspection UI, and full auto-resolve.

### Phase 4 — Victory and Defeat Conditions

**Status:** M12 complete.

Goal: scenarios end with deterministic authored or default win/loss outcomes. Condition evaluation reuses the typed condition evaluator from Phase 2.

Completed baseline includes pure outcome rules, default victory fallback, authored victory/defeat conditions, default-victory override, defeat priority, latching, save/load, placeholder feedback, ordered Region arrival hooks, and tests.

Deferred: full result screen, rewards/carry-over beyond campaign foundation, victory event chains, richer condition leaves, diplomacy UI, advanced AI, fog/scouting, and polished presentation.

### Phase 5 — Inventory and Artifacts

**Status:** M13 complete.

Goal: items and artifacts exist in runtime state; heroes equip artifacts; equipped-artifact stat bonuses flow into battle stats; inventory and equipment persist through save/load.

Completed baseline includes item/artifact definitions, optional loaders, unsupported-effect validation, team-shared inventories, per-hero equipment, equip/unequip, event grant/remove actions, battle stat bonuses, save/load, authored proof content, and tests.

Deferred: item use, recipes, cooking, food effects, artifact combination, trader-service item economy, item/artifact condition leaves, battle spoils/steal, rarity enforcement, inventory UI, per-region/per-storage inventory, and validation softlock checks.

### Phase 6 — Energy Pool

**Status:** M14 complete.

Goal: the traveling party has a shared Energy pool used by strategic travel and future services.

Completed baseline includes daily-starting Energy, spend/recover primitives, reset through the day-rollover chokepoint, save/load, legacy recompute, snapshot/HUD exposure, and tests.

Deferred: leader passive bonus and leader item/artifact bonus terms.

### Phase 7 — World Map Layer

**Status:** M15 complete.

Goal: multiple Regions per Scenario; player selects destination Region from a World Map screen opened from an authored Region exit node.

Completed foundation includes `WorldMapDefinition`, optional loader, validation, pure travel rules, `GameSession` World Map state, unlocked-region persistence, exit-node gating, Energy spending, before-11:00 gate, next-day arrival, generic traveling-party unit loss, controller/model/renderer, authored proof content, and tests.

Deferred: event-driven region unlock, per-region enemy/world state partitioning, generic storage services, reachability/softlock validation graph proofs, start-of-day event firing on arrival, route-quality/Energy modifiers, and World Map UI polish.

### Phase 8 — Campaign System

**Status:** M16 complete.

Goal: scenarios sequence with authored carry-over rules; campaign selection is added to shell flow.

Completed foundation includes thin `ScenarioDefinition`, `CampaignDefinition`, optional loaders, validation, progression rules, carry-over rules, `GameSession` campaign runtime, ordered scenario transition chokepoint, outcome-based advancement, additive save/load, presence-gated campaign selection, HUD campaign status, authored proof content, and tests.

Deferred: full per-Scenario content kind, per-scenario region partitioning, authored starting rosters, campaign branching-choice UI, full character creation/load/settings shell, localized text objects, and mod overrides.

---

## 4. Current next milestone

Latest completed milestone: **M16 — Campaign System foundation**.

### Next planned milestone: M17 — Owned Services and Economy Foundation

M17 is the highest-value next milestone because it connects the already-existing strategic layers:

- Regions and Services;
- enemy-team occupation and future ownership contesting;
- resources;
- Energy/logistics pressure;
- inventory/artifact/economy follow-ups;
- stationed units and future passive skills;
- campaign/scenario content goals from `docs/content_scope_v1.md`.

M17 should establish the smallest coherent owned-service/economy foundation. It should not become a broad economy sim.

Recommended M17 scope:

1. Owned service runtime state and save/load.
2. Content/schema support for owned service defaults needed by the slice.
3. Mine/resource-service passive daily output for the owning team.
4. Stationed-guard data seam needed for resource-production passives.
5. Explicit passive-effect representation only as narrow as needed for production modifiers.
6. Strongest-only non-stacking production bonus calculation per owned service instance and output resource.
7. Trader-service ownership tier calculation by service type, capped at 8.
8. Authored/default service-type curves, including an authored Trading Post exchange matrix when scoped.
9. Validation for references, illegal stacking assumptions, invalid tier curves, unknown service types, invalid resources, and save/runtime invariants.
10. Pure tests first; integration/end-to-end proof second.

M17 non-goals:

- full passive skill tree;
- full AI economy;
- full market item economy;
- broad trader UI polish;
- generic origin-storage across Regions;
- full per-Scenario Region Contexts;
- large content expansion;
- procedural economy;
- full service destruction/restoration loop;
- broad combat or inventory redesign.

After M17, likely follow-ups are:

- M18 Passive Effect Spine, if M17 proves the passive seam needs generalization;
- M19 Service Economy Expansion, for richer trader-service behavior;
- later item-use, recipes/cooking, artifact combination, and inventory UI milestones.

---

## 5. Acceptance checks per phase

| Phase | Acceptance check |
|-------|------------------|
| 1 | Implemented validation rules have Catch2 coverage; malformed JSON surfaces typed messages with correct paths; valid content loads without blocking messages. |
| 2 | Authored event fires on trigger, evaluates a condition, executes an action, persists story flags/fired event ids, and does not re-fire one-shot events. |
| 3 | Enemy teams move/occupy on the Region layer; hostile occupation blocks use/travel where intended; contact battle clears the exact occupying team; event actions mutate teams; save/load round-trips enemy-team state. |
| 4 | Default/authored victory and authored defeat evaluate deterministically; defeat wins priority; default victory is suppressed by authored victory list; outcome latches and persists. |
| 5 | Item/artifact content loads and validates; equip/unequip obeys ownership/slot rules; equipped artifacts affect battle stats without mutating persistent unit definitions; save/load round-trips inventory/equipment. |
| 6 | Energy computes, spends, recovers, resets, clamps, persists, and displays consistently; illegal negative spend fails loudly. |
| 7 | World Map opens from exit nodes, shows unlocked Regions, enforces path/departure/Energy legality, spends Energy, arrives correctly, warns/removes generic travelers, and persists unlocked/current region state. |
| 8 | Campaign selection is presence-gated; scenarios sequence; carry-over allow-list is applied; disallowed state is absent after transition; Energy recomputes after carry-over; defeat fails run; final victory completes campaign. |
| 9 | Owned service state persists; owned mines pay daily resources; stationed production passives use strongest-only non-stacking semantics; trader ownership tiers are per service type and capped; ownership never bypasses locks, destruction, occupation, stock, eligibility, story, or availability rules. |

---

## 6. Tests needed

All currently completed phase test suites have shipped.

For M17, expected test families:

- owned service content/loader tests;
- owned service validation tests;
- pure mine-output calculation tests;
- pure strongest-passive selection tests;
- save/load round-trip tests for owned service runtime state;
- day-rollover payout integration tests;
- trader tier calculation tests;
- ownership-rule integration tests proving allied-owned/enemy-owned/neutral services do not count;
- regression tests proving ownership does not bypass locked/destroyed/occupied/unavailable service rules.

Continue using Catch2 and prefer pure-logic tests over rendering/input tests.

---

## 7. Explicit not-yet boundaries

These are specified at design level but remain out of scope until explicitly selected:

- full passive skill tree UI;
- skill/status combat system expansion;
- advanced enemy AI economy/service use;
- enemy recruitment and sabotage loops;
- fog of war per team;
- full Location expansion / multi-screen Locations;
- full game shell;
- presentation and audio implementation beyond scoped UI work;
- mod loading;
- Energy leader-bonus seams beyond narrow M17 needs;
- cooking / recipes / food effects / item use / battle `Item` command;
- artifact combination;
- full Market / Black Market / Trading Post / Freelancer's Guild item economy beyond M17 ownership-tier foundation;
- `teamHasItem` / `teamHasArtifact` condition leaves;
- HUD/raylib inventory rendering;
- PvP mode;
- tutorial system;
- designer editor tooling.

# Ashvale Implementation Roadmap

## Context

The current codebase is a post-M18 bounded multi-Region, multi-Scenario vertical slice. The stable foundation now includes battle, roster, save/load, basic Region/Location flow, content validation, typed events, enemy teams on the Region layer, deterministic scenario outcomes, inventory and artifacts, the team Energy pool, a minimal World Map layer, a minimal Campaign System, owned-service/economy foundation, and a narrow passive-effect spine for unit-driven mine production and leader Energy.

This roadmap works from foundations outward:

1. validation;
2. typed event/condition core;
3. enemy teams on the Region layer;
4. victory/defeat;
5. inventory and artifacts;
6. Energy;
7. World Map;
8. Campaign;
9. owned services and economy;
10. passive/effect generalization when a narrow seam needs to become reusable;
11. service-economy expansion when ownership tiers need player-facing transactions.

Archived docs, including `docs/content_scope_v0.md.archived` and `docs/implementation_roadmap.md.00.archived`, are historical context only.

## 1. Current implementation baseline

Current stable foundation:

- explicit `App` / `GameSession` flow;
- controller / mapper / renderer split;
- battle engine, CTB, static formation, leader aura, deterministic damage, and battle write-back;
- persistent roster, active/reserve party, mustering, and save/load;
- daily clock, Region travel, wake/recovery penalty, and basic services;
- team Energy pool with daily-starting formula, spend/recover primitives, day-rollover reset, save/load, snapshot/HUD exposure, and leader passive Energy contribution through `leader_energy` unit passive effects;
- JSON content loading through `ContentRepository`;
- content validation foundation;
- typed event foundation;
- enemy-team Region-layer foundation;
- scenario outcome foundation;
- inventory and artifact foundation with equipped-artifact battle stat bonuses;
- minimal World Map;
- minimal Campaign System;
- owned-service/economy foundation with resources, owned-service runtime state, mine outputs, stack-backed stationing, daily mine payout, trader ownership tiers, authored/default trader curves, Trading Post exchange matrices, validation, and proof tests;
- passive-effect spine foundation with canonical unit `passive_effects`, legacy `mine_production_passive` authoring compatibility, typed validation, M17 mine-production behavior preserved, and leader `leader_energy` feeding the daily Energy passive term.

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
- Market / Black Market / Trading Post / Freelancer's Guild item economy beyond M17 ownership-tier foundation;
- HUD/raylib inventory rendering and inventory render-model;
- event-driven region unlock;
- per-region world/enemy state partitioning;
- generic origin-storage across Regions;
- full per-Scenario `ScenarioDefinition` content kind;
- authored starting rosters and full campaign branching-choice UI;
- full shell/menu/character-creation/load/settings flow.

## 2. Doc/code conflicts and known debt

| # | Issue | Action |
|---|-------|--------|
| 1 | Team Energy pool has an implemented leader passive term (`leader_energy`) and a still-deferred leader item/artifact Energy term. | Keep `leader_energy` unit effects on the current passive spine. Do not fake item/artifact Energy until an item/artifact effect milestone exists. |
| 2 | `ContentRepository` loads only content kinds with C++ struct definitions. Items, artifacts, World Map, thin scenarios, campaigns, owned-service economy definitions, trader curves, and unit passive effects have loaders; recipes and full per-Scenario Region Contexts do not. | No conflict. Add structs/loaders only when a scoped phase requires them. |
| 3 | `docs/game_shell_flow.md` specifies the full shell flow, while code still focuses on the playable slice and direct mode transitions. | Gap, not conflict. Full shell remains deferred. |
| 4 | `docs/validation_system.md` specifies a broader three-level validation model than is currently implemented. | Expand validation only when a phase requires it. |
| 5 | `docs/combat_rules.md` specifies timed status effects and broader command depth. | Gap; acceptable until skill/status phases. |
| 6 | Player color may still be hardcoded as `Green` in some Region/enemy-team/outcome/economy paths. | Known debt. Do not fix opportunistically unless introducing a real player-team identity model. |
| 7 | `scenario_outcome.json` is a bounded-slice authoring file, not the full `ScenarioDefinition` outcome model. | Intentional M12 compromise. Full Scenario authoring comes later. |
| 8 | Unit `passive_effects` support only `mine_production` and `leader_energy`; artifact `statBonus` remains on the artifact path. | Intentional M18 scope. Do not fold artifact/item/status behavior into the unit passive spine without a scoped milestone. |

No true design contradictions are currently known. Remaining gaps are implementation sequencing issues.

## 3. Completed implementation phases

### Phase 1 — Content Validation System

**Status:** Foundation implemented; broader validation model still expandable.

### Phase 2 — Minimal Typed Event Foundation

**Status:** Foundation implemented.

### Phase 3 — Enemy Teams on Region Layer

**Status:** Practical Phase 3 slice completed by M11-e.

### Phase 4 — Victory and Defeat Conditions

**Status:** M12 complete.

### Phase 5 — Inventory and Artifacts

**Status:** M13 complete.

### Phase 6 — Energy Pool

**Status:** M14 complete; leader passive seam filled by M18.

Completed baseline includes daily-starting Energy, spend/recover primitives, reset through the day-rollover chokepoint, save/load, legacy recompute, snapshot/HUD exposure, tests, and M18 leader passive Energy contribution through the current leader's `leader_energy` passive effects.

Deferred: leader item/artifact Energy bonus term.

### Phase 7 — World Map Layer

**Status:** M15 complete.

### Phase 8 — Campaign System

**Status:** M16 complete.

### Phase 9 — Owned Services and Economy Foundation

**Status:** M17 complete.

Completed foundation includes:

- strict resource type support with Gold delegated to the existing gold source of truth;
- owned-service runtime state and additive save/load;
- validation that service ids are unique and each location is placed in at most one Region node;
- mine/resource service kinds and authored mine outputs;
- pure mine-production rules with strongest-only, non-stacking per-resource passives;
- stack-backed stationing refs, normalization, and stale-reference rejection;
- day-boundary mine payout through the existing `AdvanceClock` chokepoint;
- trader service types, per-type ownership-tier calculation, and service-specific benefit gate;
- authored/default trader curves and Trading Post exchange matrices;
- validation and proof tests.

Deferred beyond M17: trader UI, actual player-facing trader transaction flow, broad item-market behavior, AI economy/service use, ownership transfer/contesting loops, full service destruction/restoration loop, full passive skill trees, generic origin-storage across Regions, and broad content expansion.

### Phase 10 — Passive Effect Spine

**Status:** M18 complete.

Completed foundation includes:

- canonical `UnitDefinition::passiveEffects` runtime representation;
- strict loading/validation for `passive_effects` arrays and entries;
- legacy `mine_production_passive` JSON authoring compatibility, converted at load into canonical `mine_production` effects;
- mixed legacy + canonical passive authoring rejected as ambiguous;
- `mine_production` effects feeding the existing strongest-only/non-stacking mine-output path;
- `leader_energy` effects feeding the daily Energy leader passive bonus term;
- cross-consumer isolation tests;
- end-to-end tests for canonical and legacy authoring paths;
- concise schema/rules/validation docs.

Deferred beyond M18: artifact Energy, item effects, artifact special effects beyond existing `statBonus`, battle statuses, active abilities, skill-tree UI, and broad effect dispatch.

## 4. Current next milestone

Latest completed milestone: **M18 — Passive Effect Spine**.

### Next planned milestone: M19 — Service Economy Expansion

M19 is the likely next milestone because M17 established owned trader-service tiers and authored/default curves, but intentionally deferred player-facing service transactions and richer trader-service behavior.

M19 should connect the owned-service economy foundation to a narrow, test-backed service-transaction layer without becoming a full item economy or broad shop UI milestone.

Recommended M19 scope:

1. Audit current trader-service rules, curves, Trading Post matrices, resource pool, ownership gates, service availability gates, and existing service interaction flow.
2. Define the smallest service-transaction model needed for one or two trader service types, preferably pure rules first.
3. Preserve M17 ownership semantics: benefits apply only when using a same-type service the team owns, and ownership never bypasses lock, destruction, hostile occupation, stock, eligibility, story, or availability rules.
4. Keep resource exchange and price/discount calculations pure and validation-backed.
5. Add minimal integration proof; defer broad UI unless the slice explicitly needs a basic interaction path.

M19 non-goals:

- full item-market economy;
- full shop UI and inventory browsing UI;
- AI economy/service use;
- storage overhaul;
- ownership transfer/contesting loops;
- skill-tree or status-effect expansion;
- content-volume growth disconnected from service-transaction proof.

## 5. Acceptance checks per phase

| Phase | Acceptance check |
|-------|------------------|
| 1 | Implemented validation rules have Catch2 coverage; malformed JSON surfaces typed messages with correct paths; valid content loads without blocking messages. |
| 2 | Authored event fires on trigger, evaluates a condition, executes an action, persists story flags/fired event ids, and does not re-fire one-shot events. |
| 3 | Enemy teams move/occupy on the Region layer; hostile occupation blocks use/travel where intended; contact battle clears the exact occupying team; event actions mutate teams; save/load round-trips enemy-team state. |
| 4 | Default/authored victory and authored defeat evaluate deterministically; defeat wins priority; default victory is suppressed by authored victory list; outcome latches and persists. |
| 5 | Item/artifact content loads and validates; equip/unequip obeys ownership/slot rules; equipped artifacts affect battle stats without mutating persistent unit definitions; save/load round-trips inventory/equipment. |
| 6 | Energy computes, spends, recovers, resets, clamps, persists, displays consistently, applies current-leader `leader_energy` passive effects, and still leaves item/artifact Energy as a deferred seam. |
| 7 | World Map opens from exit nodes, shows unlocked Regions, enforces path/departure/Energy legality, spends Energy, arrives correctly, warns/removes generic travelers, and persists unlocked/current region state. |
| 8 | Campaign selection is presence-gated; scenarios sequence; carry-over allow-list is applied; disallowed state is absent after transition; Energy recomputes after carry-over; defeat fails run; final victory completes campaign. |
| 9 | Owned service state persists; owned mines pay daily resources; stationed production passives use strongest-only non-stacking semantics; trader ownership tiers are per service type and capped; ownership never bypasses locks, destruction, occupation, stock, eligibility, story, or availability rules. |
| 10 | Passive/effect spine keeps existing M17 behavior valid, supports only active consumers (`mine_production` and `leader_energy`), rejects unsupported/malformed authoring, and does not disturb artifact/item effect paths. |
| 11 | Service Economy Expansion, if selected, proves player-facing service transactions use ownership tiers/curves correctly without broad item-economy or UI sprawl. |

## 6. Tests needed

All currently completed phase test suites have shipped.

For M19, expected test families depend on the accepted scope but should likely include:

- pure trader/service transaction rule tests;
- service-specific ownership gate tests;
- Trading Post exchange matrix resolution tests;
- resource spend/receive tests using the existing resource pool and Gold delegation;
- validation tests for authored transaction/curve data;
- integration proof for a minimal service transaction flow.

Continue using Catch2 and prefer pure-logic tests over rendering/input tests.

## 7. Explicit not-yet boundaries

These are specified at design level but remain out of scope until explicitly selected:

- full passive skill tree UI;
- skill/status combat system expansion;
- advanced enemy AI economy/service use;
- enemy recruitment and sabotage loops;
- ownership transfer/contesting UI and full service destruction/restoration loops;
- fog of war per team;
- full Location expansion / multi-screen Locations;
- full game shell;
- presentation and audio implementation beyond scoped UI work;
- mod loading;
- cooking / recipes / food effects / item use / battle `Item` command;
- artifact combination;
- full Market / Black Market / Trading Post / Freelancer's Guild item economy beyond the M17 ownership-tier foundation;
- artifact Energy and artifact special effects beyond current `statBonus` until explicitly promoted;
- `teamHasItem` / `teamHasArtifact` condition leaves;
- HUD/raylib inventory rendering;
- PvP mode;
- tutorial system;
- designer editor tooling.

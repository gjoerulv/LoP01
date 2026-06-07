# Ashvale Implementation Roadmap

## Context

The current codebase is a post-M20 bounded multi-Region, multi-Scenario vertical slice.

The stable foundation now includes battle, roster, save/load, Region/Location flow, content validation, typed events, enemy teams on the Region layer, deterministic scenario outcomes, inventory and artifacts, the team Energy pool, a minimal World Map layer, a minimal Campaign System, owned-service/economy foundation, a narrow unit passive-effect spine, a headless Trading Post transaction layer, and a bounded playable Trading Post interaction flow.

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
- Trading Post interaction flow with a bounded Location-mode service interaction, buy/sell/barter modes, live prompt feedback, per-visit time cost, and a small authored playable Home Base Trading Post.

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
- full per-Scenario `ScenarioDefinition` content kind;
- authored starting rosters and full campaign branching-choice UI;
- full shell/menu/character-creation/load/settings flow.

## 2. Known doc/code gaps and debt

| # | Issue | Action |
|---|-------|--------|
| 1 | Team Energy has an implemented leader passive term (`leader_energy`) and a still-deferred leader item/artifact Energy term. | Keep `leader_energy` on the current unit passive spine. Do not fake item/artifact Energy until an item/artifact effect milestone exists. |
| 2 | `ContentRepository` loads only content kinds with C++ struct definitions. Recipes, full Scenario Region Contexts, and several long-term authored structures still do not exist. | Add structs/loaders only when a scoped phase requires them. |
| 3 | `docs/game_shell_flow.md` specifies the full shell flow, while code still focuses on the playable slice and direct mode transitions. | Gap, not conflict. Full shell remains deferred. |
| 4 | `docs/validation_system.md` specifies a broader three-level validation model than is currently implemented. | Expand validation only when a phase requires it. |
| 5 | `docs/combat_rules.md` specifies timed status effects and broader command depth. | Gap; acceptable until skill/status phases. |
| 6 | Player color may still be hardcoded as `Green` in some Region/enemy-team/outcome/economy paths. | Known debt. Do not fix opportunistically unless introducing a real player-team identity model. |
| 7 | `scenario_outcome.json` remains a bounded-slice authoring file, while thin `ScenarioDefinition` supports only a subset of long-term Scenario authoring. | Intentional sequencing. Expand Scenario authoring through scoped milestones. |
| 8 | Unit `passive_effects` support only `mine_production` and `leader_energy`; artifact `statBonus` remains on the artifact path. | Intentional M18 scope. Do not fold artifact/item/status behavior into the unit passive spine without a scoped milestone. |
| 9 | Trading Post interaction is implemented as a bounded text-prompt service flow, not a full shop/inventory UI. | Gap, not conflict. Build broader trader UI only in a scoped UI/economy milestone. |

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

## 4. Current next milestone

Latest completed milestone: **M20 — Trading Post Interaction Flow**.

Selected next milestone: **M21 — Scenario Economy Start-State Authoring Foundation**.

### Why M21 now

The highest-value next step is not another trader-service expansion, full marketplace, UI polish pass, or AI economy. The project now has enough economy/service systems that the bigger bottleneck is authored scenario setup: designers still cannot cleanly define the player's initial economy/service-control state for a scenario. That pushes work back toward hardcoded defaults, test-only save setup, or demo-specific branches.

M21 should make the start of a Scenario more content-driven without attempting the full long-term ScenarioDefinition model.

### M21 goal

Add a narrow, validated, content-authored player start-state surface for economy and service-control setup.

The selected scope is:

- scenario-authored starting Gold and non-Gold resources for the player;
- scenario-authored initial owned-service state for the player, using existing owned-service runtime fields where legal;
- scenario-authored World Map unlocked Region overrides only if they are needed to avoid hardcoded setup for the bounded slice;
- application of that authored start state when a Scenario starts or a new session is initialized;
- additive compatibility with existing thin `ScenarioDefinition` fields such as `startRegionId`, `startNodeId`, and legacy `startGold`.

M21 is not the full Scenario authoring system. It is the smallest step that lets authored content initialize the economy/service systems already proven by M17-M20.

### M21 constraints

Do not include:

- authored starting roster or hero-pool system;
- full team definitions;
- full per-Scenario content directories;
- Scenario Region Contexts;
- campaign branching-choice UI;
- full shell/menu/load-slot flow;
- ownership transfer/claiming/contesting UI;
- enemy AI economy/service use;
- item/inventory/artifact start-state authoring unless explicitly selected;
- trader-service behavior beyond the implemented Trading Post surface;
- passive/effect expansion.

Preserve:

- save/load compatibility;
- existing `startGold` behavior or a clearly validated migration/alias rule;
- Gold single-source-of-truth and resource pool semantics;
- owned-service validation invariants: service ids are instance keys, and each location is placed in at most one Region node;
- all M17-M20 tests and runtime behavior.

### Recommended M21 phases

#### M21 Phase 1 — Scenario start-state schema, loader, and validation

Define a narrow authored start-state shape on thin `ScenarioDefinition`, for example:

```json
"playerStart": {
  "gold": 2500,
  "resources": [
    { "resource": "Wood", "amount": 5 },
    { "resource": "Stone", "amount": 3 }
  ],
  "ownedServices": [
    { "serviceId": "home_base_trading_post" }
  ],
  "unlockedRegions": ["ashvale_meadow"]
}
```

Exact field names may change during implementation, but the model must stay narrow.

Validation should reject:

- invalid resource names;
- negative starting resources or Gold;
- duplicate resource entries;
- invalid service ids;
- duplicate owned service entries;
- service ids that cannot be legal owned-service instances under the M17 invariants;
- invalid Region ids in unlocked Region overrides;
- ambiguous authoring such as both legacy `startGold` and `playerStart.gold` with conflicting values.

No runtime behavior change in this phase unless the loader/validator structure requires a minimal adapter.

#### M21 Phase 2 — Apply authored player start state at Scenario start

Apply the validated start state through existing `GameSession` APIs/state:

- Gold through the existing Gold source of truth;
- non-Gold resources through the resource pool;
- owned services through existing owned-service runtime state;
- World Map unlocked Regions through existing unlocked-region state, only if Phase 1 selected that field.

Do not write directly to private containers when an existing API exists. Do not introduce a second start-state source.

Expected tests:

- new Scenario with `playerStart.resources` starts with the authored resource counts;
- legacy Scenario with `startGold` still starts with the expected Gold;
- Scenario with authored owned Trading Post starts with player ownership and the existing Trading Post interaction receives ownership-tier benefits;
- absent `playerStart` preserves current defaults;
- save/load after start round-trips the resulting runtime state without adding new save fields.

#### M21 Phase 3 — End-to-end proof content and docs

Add the smallest authored proof content needed to show that Scenario start state can drive the current economy/service systems without hardcoded setup.

Docs to update only if implementation finalizes field names or semantics:

- `docs/content_schema.md` for the exact `playerStart` shape;
- `docs/scenario_authoring.md` for the current authored Scenario subset;
- `docs/validation_system.md` for new validation errors;
- `README_DECISIONS.md` only if a real design decision is made.

Do not rewrite broad vision docs as a changelog.

### M21 acceptance check

M21 is accepted when a thin Scenario can author the player's starting Gold/resources and initial owned-service state, validation rejects malformed/ambiguous start-state content, the authored state is applied through existing runtime APIs at Scenario start, existing saves remain compatible, and at least one end-to-end test proves authored start state affects a real existing system such as Trading Post ownership tier or resource availability.

## 5. Future candidate milestones after M21

Do not start these without a fresh planning audit and explicit user selection.

Potential directions:

- authored starting rosters / player team definition;
- scenario result screen and victory event chains;
- richer scenario outcome condition leaves such as ownership or item/artifact conditions;
- ownership claiming/transfer/contesting flow;
- service destruction/restoration/sabotage loops;
- Market / Black Market / Freelancer's Guild transaction behavior;
- inventory rendering / item-use / recipe systems;
- enemy AI economy/service use;
- campaign branching-choice UI;
- broader shell/menu/load-slot flow.

## 6. Tests needed

All completed phase test suites have shipped. For M21, define tests from the selected scope and prefer:

- pure loader/validator tests for start-state authoring;
- GameSession/scenario-start integration tests for runtime application;
- end-to-end proof tests for authored start state affecting existing economy/service behavior;
- regression tests proving legacy `startGold` and absent `playerStart` behavior remain stable.

Continue using Catch2 and prefer pure logic/controller tests over rendering/input tests.

## 7. Explicit not-yet boundaries

These remain out of scope until explicitly selected:

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
- Market / Black Market / Freelancer's Guild item economy beyond the current ownership-tier foundation;
- full shop/inventory UI beyond the bounded Trading Post interaction proof;
- artifact Energy and artifact special effects beyond current `statBonus` until explicitly promoted;
- `teamHasItem` / `teamHasArtifact` condition leaves;
- HUD/raylib inventory rendering;
- PvP mode;
- tutorial system;
- designer editor tooling.

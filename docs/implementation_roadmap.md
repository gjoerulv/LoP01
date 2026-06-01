# Ashvale Implementation Roadmap

## Context

The current codebase is a post-M16 bounded multi-Region, multi-Scenario vertical slice. The stable foundation now includes battle, roster, save/load, basic Region/Location flow, content validation foundation, typed events, the practical Phase 3 enemy-team Region-layer slice, deterministic scenario outcome evaluation, a minimal inventory + artifact-equipping layer (items, unequipped artifacts, per-hero equipment slots, and equipped-artifact stat bonuses applied at battle setup), the team Energy pool (daily-starting formula, spend/recover, day-rollover reset), a minimal World Map layer (exit-node-gated region-to-region travel on the Energy pool, with generic-unit loss-with-warning), and a minimal Campaign System (thin authored scenarios sequenced by a campaign transition graph with explicit allow-list carry-over and a presence-gated selection screen).

The roadmap works from foundations outward:

1. validation
2. typed event/condition core
3. enemy teams on the Region layer
4. victory/defeat
5. inventory and artifacts
6. World Map
7. Campaign

`docs/presentation_game_feel.md` and `docs/game_shell_flow.md` are active guidance documents and must be respected when their subject matter is touched. They are not implementation priorities until shell, UI, and presentation phases are explicitly in scope. Earlier phases should not pre-implement shell screens or presentation behavior beyond what those phases strictly require.

---

## 1. Current Implementation Baseline

Current stable foundation:

- explicit `App` / `GameSession` flow
- controller / mapper / renderer split
- battle engine, CTB, static formation, leader aura, deterministic damage, and battle write-back
- persistent roster, active/reserve party, mustering, and save/load
- daily clock, Region travel, wake/recovery penalty, and basic services
- team Energy pool (M14): daily-starting formula `1000 + lowestPartyAgility*100 + leaderPassive + leaderItem` with leader passive/item terms as zero-valued seams; auto-reset on day rollover via a single clock chokepoint; `CanSpendEnergy`/`TrySpendEnergy`/`RecoverEnergy` primitives (negative spend fails loudly, recover clamps to daily max); save/load with legacy-save recompute; exposed in `SessionSnapshot` and the HUD model
- JSON content loading through `ContentRepository`
- content validation foundation
- typed event foundation:
  - optional `events.json` loading
  - duplicate id / priority validation
  - `EventDefinitions` accessor
  - `GameSession` initialization
  - camelCase field parsing
  - unknown condition/action type validation
  - story flags / fired event ids in save/load
  - start-of-day notification
  - region-node-entry notification
  - enemy-team mutation action validation for `spawnTeam`, `removeTeam`, and `changeAlliance`
  - integration tests
- typed `EnemyGroupDefinition` composition loading
- `EnemyTeamState` runtime fields:
  - `teamColor`
  - `nodeId`
  - `personality`
  - `aggression`
  - `patrol`
  - `alliances`
  - `active`
  - `energy`
  - `cooldownExpiresAtMinutes`
- `GameSession` enemy-team container and fixed-color-order enemy phase
- `FindHopCount` BFS primitive in `RegionTravelRules`
- Region-travel enemy-phase hook in `App::UpdateRegionMode`
- deterministic patrol step with patrol-radius enforcement
- hostile-occupation blocking:
  - `arrivalNodeId` on `RegionDefinition`
  - `IsBlockedByHostileOccupation()` pure-logic header
  - `GameSession::HostileOccupiedNodeIds()` query
  - wired in `App::UpdateRegionMode` with arrival-node exemption
  - Catch2 test coverage
- post-M11-e enemy-team Region-layer features:
  - Region preview and confirmed travel use the same hostile-occupation truth
  - hostile-occupied nodes are exposed in the Region render model
  - hostile-occupied nodes are drawn with a visible danger marker
  - hostile-occupied destinations can start contact battles using the node's configured `battleScenarioId`
  - hostile contact has no debug battle fallback; missing encounter config produces a diagnostic and leaves the enemy team uncleared
  - victory clears the exact engaged team by team color
  - enemy-team runtime state persists through save/load, including mutable alliances
  - `spawnTeam`, `removeTeam`, and `changeAlliance` event actions exist with explicit failure behavior
  - enemy-phase trigger expectations are documented in code and covered by tests
- M12 scenario outcome foundation:
  - pure `ScenarioOutcomeRules` evaluator
  - default victory fallback when no authored victory condition exists
  - authored victory conditions through `content/scenario_outcome.json`
  - authored defeat conditions through `content/scenario_outcome.json`
  - defeat priority when defeat and victory both match in the same evaluation
  - non-empty `victoryConditions` disables the default victory fallback entirely
  - `ScenarioOutcomeDefinition` content struct reusing the typed `EventCondition` tree
  - optional `content/scenario_outcome.json` loaded via `ContentRepository`
  - `changeAlliance` event action requires explicit `add` arg — no silent default
  - latched terminal outcome on `GameSession` survives save/load and is not re-evaluated away on load
  - outcome checks at relevant boundaries: end of `FireMatchingEvents`, `ProcessEnemyPhase`, and `ClearEnemyTeamByColor`
  - `App::UpdateRegionMode` enforces the documented order: travel apply → quest notify → `regionNodeEntry` events → outcome check #1 → enemy phase → outcome check #2 → battle/Location only if ongoing
  - placeholder status feedback: `Victory!` / `Defeat.` plus matched reason appended to `statusMessage_`
  - authored demo content: `evt_cleanse_at_sunken_ruin` sets `ashvale_cleansed` and matches victory; `evt_trap_at_clocktower` sets `ashvale_lost` and matches defeat
  - Catch2 coverage for pure outcome rules, authored content load/validation, ordered integration hooks, default-victory override semantics, and save/load persistence
- M13 inventory and artifact foundation:
  - `data::ItemDefinition` with subtype enum (`consumable | quest | seed | ingredient | food | material`), `stackCap`, `baseValue`
  - `data::ArtifactDefinition` with `allowedSlots`, `rarity` (free-form string), `tier`, `baseValue`, `combinable`, `statBonuses`
  - only `statBonus` artifact effects implemented (Attack / Defense / Magic / Resistance); any other authored effect type is an explicit `ARTIFACT_EFFECT_TYPE_UNSUPPORTED` validation error
  - any authored item `effects` field is an explicit `ITEM_EFFECTS_UNSUPPORTED` validation error (no item effects are implemented in M13)
  - optional `content/items.json` and `content/artifacts.json` loaders, reload-safe (cleared at the top of `LoadFromDirectory`)
  - `GameSession` runtime layer: team-shared `items_`, team-shared **unequipped** `artifacts_`, per-hero `heroEquipment_` with five slots (1 Attack + 1 Defense + 3 Misc)
  - equipped-artifact ownership invariant: equipped artifacts live only in `HeroEquipmentState`, never simultaneously in `artifacts_`
  - `TryEquipArtifact` / `UnequipArtifact` `GameSession` methods (not event actions) with explicit failure on illegal slot, allowedSlots mismatch, missing inventory copy, hero not on team, or slot already occupied
  - four typed event actions `giveItem` / `takeItem` / `giveArtifact` / `takeArtifact` with the same explicit-failure semantics as `giveResource`/`takeResource`
  - `takeArtifact` does **not** auto-unequip — it only removes from the unequipped inventory and fails explicitly when only equipped copies exist
  - equipped artifact `statBonus` values added to per-battle hero `attack` / `defense` / `magic` / `resistance` via `PlayerBattleEntry` carried by `ActiveBattleStackEntry`; persistent `UnitDefinition` is never mutated; battle damage seed unchanged; enemy units never receive bonuses
  - save/load round-trip for `items`, `artifacts` (unequipped only), `heroEquipment`; legacy saves without these keys load as empty inventories (no `schemaVersion` bump)
  - authored demo content: `content/items.json` (consumable ration + quest token), `content/artifacts.json` (combinable Attack `artifact_iron_sword` + non-combinable Misc `artifact_journeyman_charm`), `evt_supply_cart_pickup` event on the `supply_cart` node grants the ration and the iron sword on first arrival
  - Catch2 coverage for content load + validation, pure stack-cap / consumable-duplicate rules, equip/unequip rules, all four event-action contracts, save/load round-trip + legacy compatibility, effective-stat inspection through `BattleFactory`, and an end-to-end test that loads real `content/` and drives a session through `supply_cart`

Still incomplete / intentionally deferred:

- proper scenario-end transitions beyond placeholder behavior
- victory event chains
- polished result screen / presentation flow
- richer outcome condition leaves such as hero alive, route state, ownership, time limits, unit counts, and Region revealed
- per-team / multi-human scenario outcome tracking
- softlock / victory-reachability proof validation
- advanced personality/aggression-driven AI beyond the current basic patrol/phase foundation
- enemy recruitment and advanced AI service use
- enemy sabotage/destruction/restoration loops
- fog/visibility per team
- Energy formula leader-bonus seams (`Y` leader passive-skill bonus, `Z` leader equipped-item/artifact bonus) — the team Energy pool itself ships in M14; only these two terms remain zero
- item use, food consumption, cooking, recipes, seeds, ingredients
- artifact combination and artifact-handler services
- battle `Item` command and item use in battle
- battle spoils transfer, gold steal, consumable steal
- `teamHasItem` / `teamHasArtifact` condition leaves
- Market / Black Market / Trading Post / Freelancer's Guild item economy
- HUD / raylib inventory rendering and inventory render-model
- event-driven region unlock, per-region world/enemy state, and generic origin-storage (World Map region-to-region travel itself shipped in M15)
- full per-Scenario `ScenarioDefinition` content kind (world map / region contexts / hero pools / banned skills / resource defaults), per-scenario region partitioning, and authored starting rosters (the minimal Campaign System — thin scenarios, transition graph, allow-list carry-over — shipped in M16)
- full shell/menu/character-creation/load/settings flow

---

## 2. Doc/Code Conflicts and Known Debt

| # | Issue | Action |
|---|-------|--------|
| 1 | The team Energy pool (`docs/core_loop_rules.md` §6) is implemented as of M14: state, daily-starting formula, spend/recover, day-rollover auto-reset, save/load, and `SessionSnapshot`/HUD exposure. The remaining gap is the formula's leader-bonus terms (`Y` leader passive-skill bonus, `Z` leader equipped-item/artifact bonus), which are zero-valued seams pending the skill system and an artifact/item Energy-effect type. | Close the seams when those systems land: a passive-skill Energy bonus (skill system milestone) and an artifact/item `energyBonus` effect (inventory-effects milestone) feed `ComputeDailyStartingEnergy`'s `leaderPassiveEnergyBonus` / `leaderItemEnergyBonus` arguments, which are already wired through at 0. |
| 2 | `ContentRepository` loads only what has C++ struct definitions. `ItemDefinition`, `ArtifactDefinition`, `WorldMapDefinition` (M13 / M15), and the thin `ScenarioDefinition` + `CampaignDefinition` (M16) now have load paths. `RecipeDefinition` and the full per-Scenario `ScenarioDefinition` (world map, region contexts, hero pools, etc.) remain doc-specified without loaders. | No conflict. Each future phase adds the relevant structs and load calls. Do not add speculative stubs. |
| 3 | `docs/game_shell_flow.md` specifies full shell flow. Code still focuses on the playable slice and direct mode transitions. | Gap, not conflict. Full shell remains deferred. |
| 4 | `docs/validation_system.md` specifies a broader three-level validation model than is currently implemented. | Continue expanding validation only when a phase requires it. |
| 5 | `docs/combat_rules.md` specifies timed status effects and broader combat command depth. Current battle code tracks a smaller baseline. | Gap; acceptable until skill/status phases. |
| 6 | Player color is still hardcoded as `"Green"` in some Region/enemy-team/outcome paths. | Known debt. Do not fix opportunistically unless introducing a real player-team identity model. |
| 7 | Hostile contact requires a configured node `battleScenarioId`. | Intentional post-M11-e rule. Missing encounters produce diagnostics, not fallback battles. Future validation should enforce this for hostile-contact-capable nodes. |
| 8 | `scenario_outcome.json` is a single bounded-slice authoring file, not a full `ScenarioDefinition`. | Intentional M12 compromise. Full Scenario/Campaign authoring comes later. |

No true design contradictions are currently known. Remaining gaps are implementation sequencing issues.

---

## 3. Implementation Phases

### Phase 1 — Content Validation System

**Status:** Foundation implemented; broader validation model still expandable.

**Goal:** Prevent structurally invalid authored content from reaching playable state; unblock safe content authoring.

Scope from `docs/validation_system.md`:

- `src/data/ContentValidator.h/cpp`
- `ValidationMessage` structure with severity, code, path, message, and optional suggestion/related data
- rules for schema identity, required field presence, node uniqueness, adjacency references, arrival node, quest references, service legality, and related content references
- validation levels: `SaveLevel`, `PlayableLevel`, `ReleaseLevel`

Out of scope for the initial validation foundation:

- full victory-path reachability graph analysis
- full UI display of validation messages
- release-level warning suppression UX

---

### Phase 2 — Minimal Typed Event Foundation

**Status:** Foundation implemented.

**Goal:** Typed events, conditions, and actions exist in C++ and evaluate deterministically.

Implemented/expected foundation:

- `EventDefinition` and `EventDefinitions`
- event triggers including `startOfDay`, `regionNodeEntry`, and initial progression triggers
- condition evaluator with typed leaf conditions and composition
- action executor with explicit failure behavior
- one-shot fired event guard
- priority ordering
- story flag persistence
- fired event persistence
- integration tests
- event validation recognizes implemented enemy-team mutation actions: `spawnTeam`, `removeTeam`, and `changeAlliance`

Future expansion remains allowed when host systems require additional triggers or action sinks.

---

### Phase 3 — Enemy Teams on Region Layer

**Status:** Practical Phase 3 slice completed by M11-e.

**Goal:** Enemy teams spawn, move, act, occupy nodes, block player interaction, and can be cleared through contact battle.

Completed baseline:

- enemy team runtime state and container
- fixed-color-order enemy phase
- patrol-bounded movement
- hostile occupation query
- hostile occupation travel blocking with arrival-node exemption
- preview/confirm alignment for hostile occupation
- Region render-model hostile occupation flag
- visible hostile marker in `RegionRenderer`
- hostile contact battle using configured node `battleScenarioId`
- no debug fallback for hostile contact
- exact-team clearing by team color on victory
- enemy-team save/load runtime state, including alliances
- `spawnTeam`, `removeTeam`, and `changeAlliance` event actions
- explicit failure behavior for event mutation actions
- tests for movement, occupation, preview, event actions, persistence, and exact clear behavior

Deferred beyond Phase 3:

- advanced AI economy/service use
- enemy recruitment
- sabotage/destruction/restoration loops
- cross-Region enemy persistence and travel
- fog/visibility/scouting
- full enemy army inspection UI
- full auto-resolve

---

### Phase 4 — Victory and Defeat Conditions

**Status:** M12-a / M12-b / M12-c complete.

**Goal:** Scenarios end with deterministic authored or default win/loss outcomes. Condition evaluation reuses the typed condition evaluator from Phase 2.

Completed foundation:

- pure `ScenarioOutcomeRules`
- default victory fallback when no authored victory condition exists
- authored victory and defeat conditions in optional `content/scenario_outcome.json`
- default-victory override rule: a non-empty `victoryConditions` list disables default victory entirely
- defeat priority: if defeat and victory both match in the same evaluation, defeat wins
- outcome latching on `GameSession`
- save/load persistence for latched outcomes
- placeholder App feedback for victory/defeat
- outcome evaluation after relevant state changes:
  - hostile contact victory
  - `regionNodeEntry` event mutations
  - enemy phase
  - start-of-day event mutations through the shared event path
  - relevant Region-mode state changes
- tests for default victory, authored victory, authored defeat, no-outcome states, ordered arrival hooks, and save/load persistence

Outcome evaluation order for Region travel arrival:

1. Apply travel time and destination change.
2. Apply quest destination notification.
3. Fire `regionNodeEntry` events.
4. Evaluate scenario outcome.
5. Run enemy phase.
6. Evaluate scenario outcome again.
7. Continue to node battle / Location transition only if no scenario outcome fired.

Reason: `regionNodeEntry` events may remove, spawn, or change hostile teams. If those events satisfy victory or defeat conditions, the scenario should end before enemy phase gets another action. Enemy phase can also create an outcome in future richer systems, so it keeps a second boundary check after the current patrol-only phase.

Out of scope:

- full result screen
- campaign progression
- rewards/carry-over
- victory event chains
- polished victory/defeat presentation
- richer outcome condition leaves
- auto-resolve
- diplomacy UI
- advanced enemy AI
- fog/scouting
- inventory/equipment

---

### Phase 5 — Inventory and Artifacts

**Status:** M13-a / M13-b / M13-c complete. Foundation usable; richer follow-ups deferred per the explicit list below.

**Goal:** Items and artifacts exist in runtime state; heroes equip artifacts; equipped-artifact stat bonuses flow into battle stats; inventory and equipment persist through save/load.

Completed foundation:

- `ItemDefinition` (subtype enum, `stackCap`, `baseValue`) and `ArtifactDefinition` (`allowedSlots`, `rarity` string, `tier`, `baseValue`, `combinable`, `statBonuses`)
- optional `content/items.json` and `content/artifacts.json` loaders with explicit validation errors for unsupported item `effects` and unsupported artifact effect types
- reload-safe optional-loader state (cleared at the top of `LoadFromDirectory`)
- team-shared `items_` (consumable max-1 enforced at runtime; non-consumables stack to authored `stackCap`)
- team-shared **unequipped** `artifacts_` (stack cap 999)
- per-hero `HeroEquipmentState` with five slots (1 Attack + 1 Defense + 3 Misc); equipped-artifact ownership invariant — equipped artifacts live only in `HeroEquipmentState`, never simultaneously in `artifacts_`
- `TryEquipArtifact` / `UnequipArtifact` `GameSession` methods with explicit failure modes
- four typed event actions `giveItem` / `takeItem` / `giveArtifact` / `takeArtifact` with explicit-failure semantics; `takeArtifact` does **not** auto-unequip
- equipped artifact `statBonus` values added to per-battle hero stats through `ActiveBattleStackEntry` → `PlayerBattleEntry` → `BattleFactory::BuildBattleUnit`; persistent `UnitDefinition` is never mutated; enemy units never receive bonuses; deterministic damage seed unchanged
- save/load round-trip for items, unequipped artifacts, and hero equipment; legacy saves without these keys load as empty inventories with no `schemaVersion` bump
- authored demo content (`item_traveler_ration`, `item_ashvale_token`, `artifact_iron_sword`, `artifact_journeyman_charm`, `evt_supply_cart_pickup`)
- Catch2 coverage across `InventoryContentTests`, `ArtifactContentTests`, `ContentRepositoryReloadTests`, `InventoryRulesTests`, `ArtifactRulesTests`, `InventoryEventActionsTests`, `InventorySaveGameTests`, `BattleArtifactStatTests`, and `InventoryEndToEndTests`

Deferred from M13 (not part of Phase 5's M13 slice; future milestones):

- leader-item / leader-passive Energy bonus terms (the team Energy pool itself shipped in M14; only the formula's `Y`/`Z` seams remain — see §2 debt #1)
- `RecipeDefinition`, cooking, food effects, seeds, ingredients
- artifact combination, combination recipes, artifact-handler services
- item use / consumption (battle `Item` command, field-use food, consumable use)
- Market / Black Market / Trading Post / Freelancer's Guild item economy
- `teamHasItem` / `teamHasArtifact` condition leaves (deferred because the typed condition evaluator is shared with `ScenarioOutcomeRules`)
- battle spoils transfer, gold steal, consumable steal
- mod overrides for items/artifacts
- `Ultimate`-rarity enforcement (rarity stays a free-form string until the enum is pinned down)
- HUD / raylib inventory rendering and inventory render-model
- per-region / per-storage inventory (team-shared only)
- validation softlock checks (e.g. "quest-important artifact can be discarded")

---

### Phase 6 — World Map Layer

**Status:** M15-a / M15-b / M15-c complete (minimal slice). Multiple Regions per Scenario with exit-node-gated region-to-region travel on the M14 Energy pool.

**Goal:** Multiple Regions per Scenario; player selects destination Region from a World Map screen opened from an authored Region exit node.

Completed foundation (M15):

- `data::WorldMapDefinition` (id, name, region entries with `unlocked` + `exitNodeIds`, adjacency pairs) — travel metadata only; arrival nodes stay in `RegionDefinition.arrivalNodeId`
- optional `content/world_map.json` loader (reload-safe; absent = single-Region scenario) with structural validation (`WORLDMAP_ENTRY_DUPLICATE`, `WORLDMAP_REGION_UNKNOWN`, `WORLDMAP_ARRIVAL_NODE_MISSING`, `WORLDMAP_ARRIVAL_NODE_UNKNOWN`, `WORLDMAP_EXIT_NODE_UNKNOWN`, `WORLDMAP_ADJACENCY_UNKNOWN_ENTRY`)
- pure `gameplay/worldmap/WorldMapTravelRules` (`EvaluateWorldMapTravel`, BFS region hop count over unlocked adjacency; reasons: AlreadyHere / DestinationLocked / NoPath / PastDepartureDeadline / InsufficientEnergy; `NotAtExitNode` is session-only)
- `GameSession` World Map state: `SetWorldMap` (seeds persisted `unlockedRegionIds_`), `SetRegionCatalog`, `IsRegionUnlocked`, `CanOpenWorldMapHere`, `EnterWorldMapMode`, `TravelToRegion` (exit-node gate → `TrySpendEnergy(1000)` → drop generic traveling-party units → advance clock to next-day 11:00 → switch region + `RegionDefinition.arrivalNodeId`), generic-loss count/removal
- save/load for `unlockedRegionIds` (absent → keep authored seed; no `schemaVersion` bump); current region/arrival node already persisted
- before-11:00 start gate (`IsBeforeRegionTransferDeadline`) and arrive-at-11:00 (`days*1200 + 300 - minutes` via the M14 `AdvanceClock` chokepoint, which refreshes Energy on the arrival day)
- `WorldMapController` / `WorldMapModelMapper` / minimal `WorldMapRenderer`; opened on demand (input `M`) from an exit node; opening sequence drops straight into Region mode
- authored proof content (`riverside_vale` second Region + `content/world_map.json`) and `WorldMapEndToEndTests`
- `WorldMapTravelRulesTests`, `WorldMapContentTests`, `WorldMapTravelTests`, `WorldMapControllerTests`, `WorldMapEndToEndTests`

Generic-unit handling: M15 implements generic-unit **travel loss/removal with a warning** (generics in the traveling party are dropped on departure; heroes persist). Preserving generics via origin-Region storage is future work.

Deferred from M15 (future milestones): event-driven region unlock (`unlockRegion`), per-region enemy-team / world-state partitioning, generic storage services, reachability/softlock validation graph proofs, start-of-day event firing on arrival, route-quality/terrain Energy modifiers, and World Map UI polish.

---

### Phase 7 — Campaign System

**Status:** M16-a / M16-b / M16-c complete (minimal slice). Authored scenarios sequence into a campaign with explicit allow-list carry-over; a minimal campaign-selection entry point is added to the shell.

**Goal:** Scenarios sequence with authored carry-over rules; campaign selection is added to shell flow.

Completed foundation (M16):

- thin `data::ScenarioDefinition` (start region/optional start node, optional `startGold`, optional inline victory/defeat conditions, `standaloneSelectable`) + optional `content/scenarios.json` loader with `SCENARIO_*` validation. Inline outcome by JSON key presence; absent ⇒ the global `scenario_outcome.json` fallback (M12 unchanged)
- `data::CampaignDefinition` (transition graph of scenario entries + `nextScenarioIds`, `campaignFlags`, explicit allow-list `CarryOverRule`s) + optional `content/campaigns.json` loader with `CAMPAIGN_*` reference validation against the loaded scenarios
- pure `gameplay/campaign/CampaignProgressionRules` (`ResolveNextScenarioId`: victory-only, linear) and `gameplay/campaign/CampaignCarryover` (`CampaignCarrySet` domain snapshot + `ApplyCarryOver`; player hero always retained; carryRoster/Items/Artifacts/Gold + named story-flag allow-list)
- `GameSession` campaign runtime: global vs active outcome-definition separation; scenario/campaign catalogs with id→index maps; the single ordered `TransitionToScenario` chokepoint (reset → set scenario → seed defaults → apply carry-over → recompute Energy LAST → clear latch → RegionMode); `StartCampaign`, `AdvanceCampaignOnVictory`, `ResolveCampaignAfterOutcome` (victory advances, defeat ⇒ `Failed`, final victory ⇒ `Completed`); campaign flags persist across scenarios while scenario story flags reset except named carries
- additive save/load for campaign id / current scenario / completed scenarios / campaign flags / state (legacy saves load as no campaign; no `schemaVersion` bump)
- presence-gated `CampaignSelectMode` + `CampaignController` / `CampaignModelMapper` / `CampaignSelectRenderer`; Title routes to selection when campaigns exist (standalone still reachable), HUD shows a campaign/scenario status line
- authored proof content: `content/scenarios.json` (`scenario_intro` → `scenario_second`), `content/campaigns.json` (`campaign_ashvale` with a carry rule), and `evt_secure_vale_market`
- Catch2 coverage: `ScenarioContentTests`, `CampaignContentTests`, `CampaignProgressionRulesTests`, `CampaignCarryoverTests`, `CampaignTransitionTests`, `CampaignSaveGameTests`, `CampaignEndToEndTests`, `CampaignControllerTests`, `CampaignStartupTests`

Deferred from M16 (future milestones): full per-Scenario `ScenarioDefinition` content kind (world map, region contexts, hero pools, banned skills/artifacts, resource defaults), per-scenario region partitioning and per-region enemy/world-state, authored starting roster / party setup, campaign branching-choice UI, full character creation / load UI / autosave slots / settings / save grouping, localized text objects, and mod overrides.

---

## 4. Current Next Milestone

### Next: Phase 7 follow-ups or Phase 8

Latest completed milestone: **M16 — Phase 7 / Campaign (minimal slice)**.

M16 shipped the minimal Campaign System across three tight slices:
- **M16-a** — thin `ScenarioDefinition` + `CampaignDefinition` content kinds, optional `content/scenarios.json` / `content/campaigns.json` loaders with `SCENARIO_*` / `CAMPAIGN_*` validation, and the pure `CampaignProgressionRules` + `CampaignCarryover` (`CampaignCarrySet` domain snapshot).
- **M16-b** — `GameSession` campaign runtime: global/active outcome-definition separation, scenario/campaign id→index catalogs, the ordered `TransitionToScenario` chokepoint (Energy recomputed last), `StartCampaign` / `AdvanceCampaignOnVictory` / `ResolveCampaignAfterOutcome`, and additive campaign save/load.
- **M16-c** — presence-gated `CampaignSelectMode` + `CampaignController` / `CampaignModelMapper` / `CampaignSelectRenderer`, Title routing (standalone still reachable), HUD campaign status, authored proof content (`campaign_ashvale`: `scenario_intro` → `scenario_second`), and docs cleanup.

See §3 Phase 7 for the full feature list and M16 deferrals.

**Next milestone** — either Phase 7 follow-ups (the full per-Scenario `ScenarioDefinition` content kind with world map / region contexts / hero pools, per-scenario region partitioning, authored starting rosters, campaign branching, fuller shell/character-creation), or a previously deferred M15 follow-up (event-driven region unlock via an `unlockRegion` event action, per-region enemy/world state, generic origin-storage, World Map UI polish).

The earlier M13 follow-ups also remain deferred: cooking, recipes, food effects, item use (battle `Item` command and field-use), artifact combination, the trader-service item economy, battle spoils/steal, `teamHasItem`/`teamHasArtifact` condition leaves, `Ultimate`-rarity enforcement, and HUD/raylib inventory rendering. The Energy formula's leader passive/item bonus terms remain zero-valued seams (see §2 debt #1).

---

## 5. Acceptance Checks Per Phase

| Phase | Acceptance Check |
|-------|------------------|
| 1 | `ContentValidatorTests` pass for implemented validation rules; malformed JSON surfaces typed error messages with correct paths; valid content loads without blocking messages. |
| 2 | Authored event fires on trigger, evaluates a condition, executes an action, persists story flags/fired event ids, and does not re-fire one-shot events. |
| 3 | Enemy team spawns/moves/occupies on the Region layer; hostile occupation blocks player use; hostile marker is visible; contact battle can clear the exact occupying team; event actions can spawn/remove/change alliances; save/load round-trips enemy-team runtime state. |
| 4 | If no authored victory condition exists, default victory fires when all hostile enemy teams are defeated/removed/allied; authored victory conditions can end the scenario and suppress default victory; authored defeat conditions can end the scenario; defeat wins over victory; no-outcome states remain playable; Region arrival checks outcome after `regionNodeEntry` events and again after enemy phase before continuing to follow-up transitions; latched outcomes survive save/load. |
| 5 | Hero equips artifact via `TryEquipArtifact` and the artifact moves from the team's unequipped inventory into the hero slot; equipped-artifact stat bonuses flow into per-battle hero `attack` / `defense` / `magic` / `resistance` at battle construction without mutating persistent `UnitDefinition`; save/load round-trips items, unequipped artifacts, and hero equipment; legacy saves without M13 keys load as empty inventories. (Combine recipe and leader-item Energy bonus criteria are explicitly deferred to a later cooking/combination/Energy milestone.) |
| 6 | World Map (opened from an authored Region exit node) shows unlocked Regions; travel costs 1000 Energy (`TrySpendEnergy`); start-before-11:00 legality is enforced; travel arrives at 11:00 on the correct day-based duration; generic traveling-party units are dropped with a warning (heroes persist); current region + unlocked set round-trip through save/load. (Generic origin-storage preservation is deferred.) |
| 7 | **(M16 met)** Two scenarios play sequentially; carry-over allow-list is applied; disallowed state is absent after transition. Covered end-to-end by `CampaignEndToEndTests` + `CampaignTransitionTests` (carried roster/gold present, non-carried roster/flags absent, Energy recomputed after carry, latch cleared); defeat ends the run (`Failed`), final victory ⇒ `Completed`. |

---

## 6. Tests Needed

All currently-planned phase test suites have shipped. Future per-Scenario / fuller-shell work will add its own suites when scoped.

Phase 7 Campaign test coverage shipped in M16: `ScenarioContentTests`, `CampaignContentTests`, `CampaignProgressionRulesTests`, `CampaignCarryoverTests`, `CampaignTransitionTests`, `CampaignSaveGameTests`, `CampaignEndToEndTests`, `CampaignControllerTests`, `CampaignStartupTests`.

Phase 6 World Map test coverage shipped in M15: `WorldMapTravelRulesTests`, `WorldMapContentTests`, `WorldMapTravelTests`, `WorldMapControllerTests`, `WorldMapEndToEndTests`.

Existing tests already cover the Phase 1-5 foundation, including M12 scenario outcome rules (pure rules, content load/validation, integration hooks, save/load) and M13 inventory + artifact rules (content load/validation with explicit unsupported-effect errors, optional-loader reload safety, stack-cap / consumable-duplicate runtime rules, equip/unequip rules, all four event-action contracts, save/load round-trip + legacy compatibility, effective-stat inspection through `BattleFactory`, and an end-to-end test that loads real `content/` and drives a session through `supply_cart`). Continue using Catch2 and prefer pure-logic tests over rendering or input tests.

---

## 7. Explicit "Not Yet" Boundaries

These are specified at design level in the docs but remain out of scope until explicitly selected:

- **Skill system / status effects** — duration-by-affected-unit's-own-turns model specified in `docs/combat_rules.md`; no implementation phase yet.
- **Enemy recruitment and advanced AI service use** — specified in `docs/core_loop_rules.md`; deferred beyond Phase 3.
- **Enemy sabotage** — service destruction, storage attacks, and restoration loops are deferred beyond Phase 3.
- **Fog of war per-team** — HoMM-like reveal model specified in `docs/core_loop_rules.md`; depends on enemy teams and later scouting/UI work.
- **Location expansion / multi-screen Locations** — specified in `docs/game_vision.md`, `docs/content_schema.md`, and `docs/scenario_authoring.md`; not in current slice.
- **Full game shell** — Character Creation, Load UI, autosave slots, Settings; fully specified in `docs/game_shell_flow.md`; partially addressed later, full shell deferred.
- **Presentation and audio implementation** — tone, animation, music, and feedback layering are active guidance but not an implementation priority until relevant UI/presentation phases.
- **Mod loading** — `content/mods/` path and override-by-kind+id are specified in `docs/content_schema.md`; no loader phase yet.
- **Energy formula leader-bonus seams** — the team Energy pool shipped in M14 (state, daily formula, spend/recover, auto-reset, save/load, snapshot/HUD). What remains deferred is only the formula's leader passive-skill (`Y`) and leader equipped-item/artifact (`Z`) bonus terms, currently zero-valued seams pending the skill system and an artifact/item Energy-effect type.
- **Cooking / recipes / food effects / item use / battle `Item` command** — `RecipeDefinition`, ingredient consumption, food effects, party-menu cooking, and the battle `Item` action all defer to future milestones. M13 grants and removes items but does not consume or use them.
- **Artifact combination** — `ArtifactCombinationRecipeDefinition`, artifact-handler services, the irreversible 2-into-1 fusion flow.
- **Market / Black Market / Trading Post / Freelancer's Guild item economy** — `docs/core_loop_rules.md` §23 trader services are their own milestone.
- **`teamHasItem` / `teamHasArtifact` condition leaves** — deferred so scenario outcome semantics are not silently broadened (the typed condition evaluator is shared between events and `ScenarioOutcomeRules`).
- **HUD / raylib inventory rendering** — render-model exposure and visual drawing.
- **PvP mode** — specified as hidden-until-implemented in `docs/game_shell_flow.md`; explicitly deferred.
- **Tutorial** — authored content type depends on event system and a separate authoring effort.
- **Designer editor tool** — future scope only.

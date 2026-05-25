# Ashvale Implementation Roadmap

## Context

The current codebase is a post-M12 bounded single-Region vertical slice. The stable foundation now includes battle, roster, save/load, basic Region/Location flow, content validation foundation, typed events, the practical Phase 3 enemy-team Region-layer slice, and deterministic scenario outcome evaluation.

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
- daily clock, Energy, Region travel, wake/recovery penalty, and basic services
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
- inventory, artifacts, recipes, and item use
- World Map and cross-Region travel
- Campaign carry-over
- full shell/menu/character-creation/load/settings flow

---

## 2. Doc/Code Conflicts and Known Debt

| # | Issue | Action |
|---|-------|--------|
| 1 | Energy formula includes leader item bonus; artifacts/items are not implemented yet, so the bonus is always zero. | Accept for now. Add or keep a code comment that leader item bonus is zero until artifact/item runtime exists. |
| 2 | `ContentRepository` loads only what has C++ struct definitions. `ItemDefinition`, `ArtifactDefinition`, `RecipeDefinition`, `WorldMapDefinition`, and `CampaignDefinition` are specified in docs but not all have load paths yet. | No conflict. Each future phase adds the relevant structs and load calls. Do not add speculative stubs. |
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

**Goal:** Items and artifacts exist in runtime state; heroes equip artifacts; combination recipes work; leader item bonus closes the Energy formula gap.

Scope:

- `ItemDefinition`
- `ArtifactDefinition`
- `RecipeDefinition`
- content loading for items/artifacts/recipes
- team-global inventory
- per-hero artifact slots
- leader artifact bonuses wired into Energy formula and battle stats
- save/load inventory state
- `InventoryTests.cpp` and `ArtifactTests.cpp`

---

### Phase 6 — World Map Layer

**Goal:** Multiple Regions per Scenario; player selects destination Region from World Map screen.

Scope:

- `WorldMapDefinition`
- World Map controller/renderer for the existing `WorldMapMode` enum slot
- unlocked Region visibility
- Region-to-Region travel legality
- 1000 Energy one-time travel cost
- travel must begin before 11:00
- day-based travel duration
- arrival at 11:00
- generics remain in origin Region unless future storage/carry rules allow otherwise
- `WorldMapTravelTests.cpp`

---

### Phase 7 — Campaign System

**Goal:** Scenarios sequence with authored carry-over rules; campaign selection is added to shell flow.

Scope:

- `CampaignDefinition`
- campaign transition graph
- `CampaignCarryover`
- explicit allow-list carry-over rules
- Campaign → Scenario transitions
- partial shell entry point for campaign selection
- `CampaignCarryoverTests.cpp`

Full character creation, load UI, autosave slots, and settings remain out of scope unless explicitly selected.

---

## 4. Current Next Milestone

### M13 — Phase 5 candidate: Inventory and Artifacts

Latest completed milestone: **M12-c — Scenario Outcome content proof and end-to-end demo**.

M12 closed the Phase 4 scenario-outcome foundation: pure rules, save/load-aware latching, ordered integration hooks in the Region travel flow, and authored demo content driving both an authored victory and an authored defeat path. The single-Region slice can now end deterministically.

Future scenario-outcome work is deferred, not part of M12:

- richer condition leaves: hero alive, route destroyed, ownership, time limits, unit counts, Region revealed, etc.
- victory event action chains
- polished result screen, transition flow, campaign hand-off
- per-team / multi-human outcome tracking
- softlock / reachability proofs in validation

Recommended next major milestone is **M13 / Phase 5: Inventory and Artifacts**, unless a small post-M12 cleanup is selected first.

---

## 5. Acceptance Checks Per Phase

| Phase | Acceptance Check |
|-------|------------------|
| 1 | `ContentValidatorTests` pass for implemented validation rules; malformed JSON surfaces typed error messages with correct paths; valid content loads without blocking messages. |
| 2 | Authored event fires on trigger, evaluates a condition, executes an action, persists story flags/fired event ids, and does not re-fire one-shot events. |
| 3 | Enemy team spawns/moves/occupies on the Region layer; hostile occupation blocks player use; hostile marker is visible; contact battle can clear the exact occupying team; event actions can spawn/remove/change alliances; save/load round-trips enemy-team runtime state. |
| 4 | If no authored victory condition exists, default victory fires when all hostile enemy teams are defeated/removed/allied; authored victory conditions can end the scenario and suppress default victory; authored defeat conditions can end the scenario; defeat wins over victory; no-outcome states remain playable; Region arrival checks outcome after `regionNodeEntry` events and again after enemy phase before continuing to follow-up transitions; latched outcomes survive save/load. |
| 5 | Hero equips artifact; stat bonus affects relevant calculations; combine recipe produces output item; save/load round-trips inventory; leader item bonus updates Energy display. |
| 6 | World Map shows Regions; travel costs 1000 Energy; start-before-11:00 legality is enforced; travel arrives at 11:00 on the correct day; generics remain in origin Region unless later rules change this. |
| 7 | Two scenarios play sequentially; carry-over allow-list is applied; disallowed state is absent after transition. |

---

## 6. Tests Needed

Future test suites still needed for upcoming phases:

- `InventoryTests.cpp`, `ArtifactTests.cpp` — Phase 5
- `WorldMapTravelTests.cpp` — Phase 6
- `CampaignCarryoverTests.cpp` — Phase 7

Existing tests already cover much of the Phase 1-4 foundation, including M12 scenario outcome rules, authored outcome content, integration hooks, and save/load persistence. Continue using Catch2 and prefer pure-logic tests over rendering or input tests.

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
- **Cooking / crafting economy** — recipe types require inventory first.
- **PvP mode** — specified as hidden-until-implemented in `docs/game_shell_flow.md`; explicitly deferred.
- **Tutorial** — authored content type depends on event system and a separate authoring effort.
- **Designer editor tool** — future scope only.

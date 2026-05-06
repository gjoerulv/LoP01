# Ashvale Implementation Roadmap

## Context
Post-M8 baseline is stable: battle engine, persistent roster, single-region playable slice,
save/load, basic location/service interactions, time/travel rules, quest tracking. The core
design documentation is broad and stable for the currently identified major systems. Remaining
design gaps should be treated as explicit clarification tasks, not guessed during
implementation. The roadmap works from foundations outward: validation → typed event/condition
core → enemy teams (which immediately draw on event types) → victory/defeat → inventory →
World Map → Campaign.

**Presentation and shell guidance:** `docs/presentation_game_feel.md` and
`docs/game_shell_flow.md` are active guidance documents and must be respected when their
subject matter is touched. However, they are not implementation priorities until shell, UI, and
presentation phases are explicitly in scope. Work in earlier phases (validation, events, enemy
teams, victory/defeat) should not pre-implement shell screens or presentation behaviors beyond
what those phases strictly require.

---

## 1. Current Implementation Baseline

**Solid (production quality):**
- Battle engine: static formation CTB, 5 units, Leader aura, deterministic damage
  (`src/gameplay/battle/`)
- Persistent roster: Active(5) + Reserve(7), save/load, muster interaction
  (`src/gameplay/`, `src/core/SaveGame`)
- Time & travel: daily clock, Energy formula, region node travel, dead-end rules
  (`src/core/GameClock`, `src/gameplay/region/`)
- Location services: rest, shop/travel-prep, recruit, quest, muster
  (`src/gameplay/location/LocationServiceRules`)
- Quest state: objective types BringResource, ClearCombatNode, ReachNode
  (`src/gameplay/quests/QuestState`)
- Content loading: JSON → typed C++ definitions (`src/data/ContentRepository`); save-game schema versioning exists in `src/core/SaveGame`
- Tests: 17 test files covering core rules; Catch2 infrastructure in place

**Present but incomplete against specification:**
- `EnemyGroupDefinition` struct loads basic fields; the full AI model (personality types,
  aggression levels, action-budget cadence, four-priority pipeline) is fully specified in
  `docs/core_loop_rules.md` §12–18 and `README_DECISIONS.md` §93–102 but not yet implemented
  in C++
- `WorldMapMode` exists as an enum value in `App.h`; the World Map layer (controller,
  renderer, definition struct) is specified in `docs/game_vision.md`, `docs/core_loop_rules.md`,
  and `docs/content_schema.md` but not yet implemented
- Quest event wiring exists for three objective types; the full typed event system (triggers,
  eligibility, conditions with `all`/`any`/`not` composition, typed actions, If/Else branches)
  is fully specified in `docs/content_schema.md` §20–25 and `docs/scenario_authoring.md`
  §12–19 but no C++ structs or engine exist

**Specified in docs; C++ structs and logic not yet written:**
- Validation system — three levels (Save / Playable / Release), severity codes,
  path-qualified messages: `docs/validation_system.md`
- Enemy team systemic AI and Region-layer behavior:
  `docs/core_loop_rules.md` §12–18, `README_DECISIONS.md` §93–102
- Victory and defeat condition evaluation (OR-based, default "defeat all enemy teams"):
  `docs/core_loop_rules.md` §35–36, `README_DECISIONS.md` §44–45 and §160
- Full typed event system:
  `docs/content_schema.md` §20–25, `docs/scenario_authoring.md` §12–19
- `ItemDefinition`, `ArtifactDefinition`, `RecipeDefinition`, `WorldMapDefinition`,
  `CampaignDefinition`: `docs/content_schema.md`
- Skill system / status effects (duration tracked by affected unit's own turns):
  `docs/combat_rules.md`

---

## 2. Doc/Code Conflicts to Resolve First

| # | Issue | Action |
|---|-------|--------|
| 1 | Energy formula includes leader item bonus; no artifacts implemented yet, so bonus is always zero. | Accept; add a code comment "leader item bonus = 0 until ArtifactDefinition implemented." No fix needed yet. |
| 2 | `ContentRepository` loads only what has C++ struct definitions; `EventDefinition`, `ItemDefinition`, `ArtifactDefinition`, `RecipeDefinition`, `WorldMapDefinition` are all fully specified in `docs/content_schema.md` but have no load path yet. | No conflict. Each phase adds the relevant struct and load call. Do not add speculative stubs. |
| 3 | `docs/game_shell_flow.md` specifies a full shell (Title → Main Menu → Character Creation → Load Game, autosave slots, Settings). Code has only a `TitleRenderer` stub and direct mode transitions. | Gap, not a conflict. Phase 6 implements the in-game World Map layer. Full shell flow is not needed before Phase 7 Campaign and later shell/UI work. |
| 4 | `docs/validation_system.md` specifies three-level validation with severity codes; `ContentRepository` does no validation gating today. | Fix in Phase 1. |
| 5 | `docs/combat_rules.md` specifies timed status effects tracked by affected unit's own turns; current battle code tracks HP/MP/KO/defending only. | Gap; acceptable until skill system. Note for a future phase. |
| 6 | Wake penalty (−1000 gold) is implemented in `GameClock`; verify deduction is wired to `GameSession` gold state end-to-end. | Verify at start of Phase 2; one-line fix if not wired. |

No true design contradictions found. All gaps are implementation-only.

---

## 3. Implementation Phases

### Phase 1 — Content Validation System
**Goal:** Prevent structurally invalid authored content from reaching playable state; unblock
safe content authoring.

Scope (from `docs/validation_system.md`):
- `src/data/ContentValidator.h/cpp` — `ValidationMessage` struct (severity enum, code string,
  dot-path, message, optional suggestion), `ContentValidator` class returning a message list
- Rules: schema identity checks aligned with `schemaVersion`, `kind`, and `id`; required field
  presence; node id uniqueness per Region; adjacency references resolve; one main
  node-content item max; arrival flag present on at least one node; quest objective
  node/location references resolve; service type fields legal
- Three validation levels: `SaveLevel`, `PlayableLevel`, and `ReleaseLevel`.
- M9 implements the level model and message severities; Release-level warning acknowledgment/suppression is deferred.
- `ContentRepository::Load()` calls validator; returns messages alongside loaded data; caller
  decides gate level
- `tests/ContentValidatorTests.cpp` — one test per rule, valid and invalid JSON fixtures

Out of scope: event validation, victory-path reachability graph analysis, UI display of
validation messages.

**Files:** `src/data/ContentValidator.h/cpp`, `src/data/ContentRepository.h/cpp`,
`tests/ContentValidatorTests.cpp`

---

### Phase 2 — Minimal Typed Event Foundation
**Goal:** The typed condition/action vocabulary specified in `docs/content_schema.md` §20–25
and `docs/scenario_authoring.md` §12–19 exists in C++ and evaluates correctly. This is the
shared foundation for enemy team authoring, victory/defeat conditions, and authored scenario
progression — all three depend on the same condition types and action types.

Scope:
- `src/gameplay/events/EventDefinition.h` — C++ structs mirroring the schema: `EventTrigger`
  enum (values drawn from `docs/content_schema.md`; initial set covers `regionNodeEntry`,
  `startOfDay`, `locationCollision`, `locationConfirm`, `neutralEncounterDefeated`,
  `serviceUsed`, `serviceDestroyed`, `questCompletion`), `EventEligibility`, typed
  `EventCondition` (with `all`/`any`/`not` composition; leaf condition types aligned with
  `docs/content_schema.md`), typed `EventAction` (values must align with
  `docs/content_schema.md`, covering resource grant/take actions, story-flag actions,
  showMessage, battle/team actions, victory/defeat triggers, AI/personality changes, and
  alliance changes), `EventBranch` (If/Else arrays)
- `src/gameplay/events/EventEngine.h/cpp` — condition evaluator, action executor, one-shot
  guard (fired events not re-fired), automatic priority ordering, cycle detection at load time
- `ContentRepository` extended to load `EventDefinition` from authored JSON
- `GameSession` wires `regionNodeEntry` and `startOfDay` triggers; other triggers wired as
  their host systems are built
- `src/core/SaveGame` extended for story flags and one-shot fired-event tracking
- `tests/EventEngineTests.cpp` — trigger evaluation, condition leaf types, `all`/`any`/`not`
  composition, If/Else branching, one-shot guard, cycle detection rejection

Out of scope: `showGuidance`, complex dialogue sequencing, all rendering of event dialogue
(presentation deferred to `docs/presentation_game_feel.md` guidance).

**Files:** `src/gameplay/events/EventDefinition.h`,
`src/gameplay/events/EventEngine.h/cpp`, `src/data/ContentRepository.h/cpp`,
`src/core/SaveGame.h/cpp`, `tests/EventEngineTests.cpp`

---

### Phase 3 — Enemy Teams on Region Layer
**Goal:** Enemy teams spawn, move, act, and occupy nodes according to the AI model in
`docs/core_loop_rules.md` §12–18 and `README_DECISIONS.md` §93–102. Enemy team authoring
(spawn, personality changes, alliance changes) draws on typed event actions established in
Phase 2.

Scope:
- `src/gameplay/EnemyTeamState.h/cpp` — position, personality (Warrior/Builder/Explorer),
  aggression (Berserk → Pacifist), patrol radius, alliance table
- Action cadence: one action per enemy phase; acts after each player action that costs time;
  fixed color order
- AI priority pipeline: victory condition check → survival → logistics → personality goal
- Actions in this phase: move toward goal node, occupy adjacent location nodes (making them
  inaccessible to player), use region services (rest), idle
- Occupation shown with visual indicator in `RegionRenderer`
- Enemy team event actions defined in Phase 2 (spawn, personality change, alliance change)
  wired to `EnemyTeamState`
- `EnemyGroupDefinition` extended to load personality, aggression, patrol radius, starting
  node
- `tests/EnemyTeamTests.cpp` — spawn, movement rules, occupation, inaccessibility,
  color-order action sequencing

Out of scope: enemy recruitment and advanced AI service use (specified in
`docs/core_loop_rules.md`; deferred beyond Phase 3), cross-Region enemy persistence. UI
presentation of enemy team identity deferred to `docs/presentation_game_feel.md` guidance.

**Files:** `src/gameplay/EnemyTeamState.h/cpp`, `src/app/RegionController.h/cpp`,
`src/rendering/RegionRenderer.h/cpp`, `src/data/ContentRepository.h/cpp` (extended
EnemyGroupDefinition load), `tests/EnemyTeamTests.cpp`

---

### Phase 4 — Victory and Defeat Conditions
**Goal:** Scenarios end with authored win/loss outcomes. Condition evaluation reuses the typed
condition evaluator from Phase 2; no new condition vocabulary needed.

Scope (from `docs/core_loop_rules.md` §35–36, `README_DECISIONS.md` §44–45 and §160):
- `src/gameplay/VictoryDefeatRules.h/cpp` — OR-based victory set, sequential defeat set;
  default victory = all enemy teams defeated; condition types drawn from the Phase 2 condition
  evaluator
- Evaluated after each phase (player action, enemy action, day transition)
- On victory: signal `GameSession` → scenario-end transition; on defeat: defeat transition
- `tests/VictoryDefeatTests.cpp` — default victory, authored OR-victory set,
  first-matching-defeat-condition triggers loss

Presentation of victory/defeat screens deferred to `docs/game_shell_flow.md` and
`docs/presentation_game_feel.md` guidance; transitions use placeholder screens for now.

**Files:** `src/gameplay/VictoryDefeatRules.h/cpp`, `src/app/GameSession.h/cpp`,
`tests/VictoryDefeatTests.cpp`

---

### Phase 5 — Inventory and Artifacts
**Goal:** Items and artifacts exist in runtime state; heroes equip artifacts; combination
recipes work; leader item bonus closes the Energy formula gap from Section 2, item #1.

Scope (from `docs/content_schema.md`):
- C++ structs: `ItemDefinition`, `ArtifactDefinition` (slot: Attack/Defense/Misc×3, stat
  bonuses), `RecipeDefinition` (2 inputs → 1 output)
- `ContentRepository` extended to load all three
- `src/gameplay/Inventory.h/cpp` — team-global item store, per-hero artifact slots
- Leader artifact bonuses wired into Energy formula and battle stats
- `src/core/SaveGame` extended for inventory state
- `tests/InventoryTests.cpp`, `tests/ArtifactTests.cpp`

**Files:** `src/data/ItemDefinition.h`, `src/data/ArtifactDefinition.h`,
`src/data/RecipeDefinition.h`, `src/gameplay/Inventory.h/cpp`, `src/core/SaveGame.h/cpp`,
`tests/InventoryTests.cpp`, `tests/ArtifactTests.cpp`

---

### Phase 6 — World Map Layer
**Goal:** Multiple Regions per Scenario; player selects destination Region from World Map
screen.

Scope (from `docs/game_vision.md`, `docs/core_loop_rules.md`, and `docs/content_schema.md`;
presentation rules from `docs/presentation_game_feel.md` apply here):
- `WorldMapDefinition` C++ struct and `ContentRepository` load
- `WorldMapController.h/cpp` and `WorldMapRenderer.h/cpp` implementing the `WorldMapMode`
  enum slot already in `App.h`
- Reuse Region HUD components (top info bar, bottom resource bar) as specified in
  `docs/game_vision.md` and `docs/core_loop_rules.md`
- Only unlocked Regions visible; warn before travel that generics remain in origin Region
- 1000 Energy one-time cost at travel start; travel must begin before 11:00; travel duration is day-based and arrival is always 11:00
- `tests/WorldMapTravelTests.cpp`

**Files:** `src/data/WorldMapDefinition.h`, `src/app/WorldMapController.h/cpp`,
`src/rendering/WorldMapRenderer.h/cpp`, `src/app/App.h/cpp`,
`tests/WorldMapTravelTests.cpp`

---

### Phase 7 — Campaign System
**Goal:** Scenarios sequence with authored carry-over rules; campaign selection added to shell
flow.

Scope:
- `CampaignDefinition` struct and load
- `src/gameplay/CampaignCarryover.h/cpp` — explicit allowed list (heroes, items, gold cap,
  story flags); disallowed values stripped at transition
- Campaign → Scenario transitions with carry-over applied to save state
- Shell flow entry point: Title → Main Menu → Campaign selection (partial shell; full
  Character Creation and Load Game screens specified in `docs/game_shell_flow.md` remain out
  of scope for this phase)
- `tests/CampaignCarryoverTests.cpp`

**Files:** `src/data/CampaignDefinition.h`, `src/gameplay/CampaignCarryover.h/cpp`,
`tests/CampaignCarryoverTests.cpp`

---

## 4. First Small Milestone

**M9 — Content Validation (Phase 1)**

Deliverable: `ContentValidator` runs at `ContentRepository::Load()` time and returns typed
`ValidationMessage` objects before any game state is constructed.

Scope:
1. `src/data/ContentValidator.h` — `Severity` enum (Error/Warning/Info),
   `ValidationMessage` struct, `ContentValidator` class
2. `src/data/ContentValidator.cpp` — 10–15 rules: schema identity checks aligned with
   `schemaVersion`, `kind`, and `id`; required field presence; node id uniqueness per Region;
   adjacency link resolution; one main node-content item max; arrival flag present; quest
   objective references resolve; service type field legality
3. `ContentRepository::Load()` calls validator and surfaces message list; callers decide gate
   level
4. `tests/ContentValidatorTests.cpp` — one positive and one negative test per rule

Out of scope for M9: UI display of messages, event validation, victory-path reachability,
Release-level acknowledgment flow.

---

## 5. Acceptance Checks Per Phase

| Phase | Acceptance Check |
|-------|-----------------|
| 1 | `ContentValidatorTests` all pass; malformed JSON surfaces typed Error messages with correct dot-path; valid content loads without messages |
| 2 | Authored event fires on trigger, evaluates a condition, executes an action; story flag persists across save/load; one-shot event does not re-fire |
| 3 | Enemy team spawns at authored start node, moves one step per player turn, occupies an adjacent node; occupied node is inaccessible to player in UI; spawn event action creates a new team at runtime |
| 4 | Authored victory condition fires scenario-end when met; default "all enemy teams defeated" works with no authored condition; first matching defeat condition triggers defeat transition |
| 5 | Hero equips artifact, stat bonus reflected in battle damage; combine recipe produces output item; save/load round-trips inventory; leader item bonus updates Energy display |
| 6 | World Map shows regions, travel costs 1000 Energy, start-before-11:00 legality is enforced, arrives at 11:00 on the correct day, generics stay in origin Region |
| 7 | Two scenarios play sequentially; carry-over list applied; disallowed items absent after transition |

---

## 6. Tests Needed

- `ContentValidatorTests.cpp` — Phase 1
- `EventEngineTests.cpp` — Phase 2 (trigger, condition composition, If/Else, one-shot, cycle
  detection)
- `EnemyTeamTests.cpp` — Phase 3 (movement, occupation, action cadence, color order)
- `VictoryDefeatTests.cpp` — Phase 4
- `InventoryTests.cpp`, `ArtifactTests.cpp` — Phase 5
- `WorldMapTravelTests.cpp` — Phase 6
- `CampaignCarryoverTests.cpp` — Phase 7

All follow existing Catch2 patterns. No rendering or input tests; pure logic only.

---

## 7. Explicit "Not Yet" Boundaries

These are **specified at design level in the docs** but out of scope for this roadmap until explicitly requested:
requested:

- **Skill system / status effects** — duration-by-affected-unit's-own-turns model specified
  in `docs/combat_rules.md`; no implementation phase yet
- **Enemy recruitment and advanced AI service use** — specified in
  `docs/core_loop_rules.md`; deferred beyond Phase 3
- **Enemy sabotage** (service destruction, storage attacks) — specified in
  `docs/core_loop_rules.md`; deferred beyond Phase 3
- **Fog of war per-team** — HoMM-like reveal model specified in `docs/core_loop_rules.md`;
  depends on enemy teams (Phase 3+)
- **Location expansion / multi-screen Locations** — specified in `docs/game_vision.md`,
  `docs/content_schema.md`, and `docs/scenario_authoring.md`; not in current slice
- **Full game shell** (Character Creation, Load UI, autosave slots, Settings) — fully
  specified in `docs/game_shell_flow.md`; partially addressed in Phase 7, full shell deferred
- **Presentation and audio implementation** — tone, animation, music, feedback layering fully
  specified in `docs/presentation_game_feel.md`; active guidance but not an implementation
  priority until Phase 6+ shell and UI work
- **Mod loading** — `content/mods/<modName>/` path and override-by-kind+id specified in
  `docs/content_schema.md`; no loader phase yet
- **Cooking / crafting economy** — recipe types specified in `docs/content_schema.md`;
  requires inventory (Phase 5) first, then a separate phase
- **PvP mode** — specified as hidden-until-implemented in `docs/game_shell_flow.md`;
  explicitly deferred
- **Tutorial** — authored content type specified in `docs/game_shell_flow.md`; depends on
  event system (Phase 2+) and a separate authoring effort
- **Designer editor tool** — referenced in `docs/scenario_authoring.md`; future scope, out of
  plan

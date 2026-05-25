# Scenario Authoring and Validation

This document defines the intended long-term authoring model for Ashvale Scenarios, Regions, Locations, Services, events, quests, victory/defeat conditions, teams, items, artifacts, recipes, and validation.

This is a design and tooling guide. Detailed validation levels, severities, gates, and validator categories live in `docs/validation_system.md`. Detailed content data shapes and schema conventions live in `docs/content_schema.md`.

This document does not replace:

- `docs/game_vision.md`
- `docs/core_loop_rules.md`
- `docs/combat_rules.md`
- `README_DECISIONS.md`
- `docs/terminology_map.md`
- `docs/validation_system.md`
- `docs/content_schema.md`

Use this document when designing content schemas, designer tools, validation rules, scenario files, and AI-agent tasks that modify authored content.

---

## 1. Authoring philosophy

Ashvale should be heavily data-driven.

The following should be editable through a designer tool eventually:

- Campaigns
- Scenarios
- World Maps
- Regions
- Locations
- nodes
- routes
- Services
- teams
- quests
- events
- victory conditions
- defeat conditions
- resource amounts, costs, pickups, payouts, and economy settings
- units
- items
- artifacts
- recipes
- artifact-combination recipes

The resource enum itself is code/schema-defined. Designers configure how resources are used, awarded, spent, traded, and produced; they do not add arbitrary new resource types unless the resource enum is explicitly expanded.

Code defines:

- allowed system types
- runtime behavior
- formulas
- validation rules
- legal enum values
- engine-level constraints

Content configures those systems within legal limits. Content should not be able to invent arbitrary new mechanics without code support.

---

## 2. Validation philosophy

Designer-authored content should be strict enough to prevent invalid content from being played. However, validation must not prevent a designer from saving unfinished work.

### Save versus play

A designer may save an invalid or incomplete map for later work. An invalid map should not be playable in the game.

### Harsh versus invalid

Validation must distinguish between:

- **invalid content**
  - broken references
  - impossible required structures
  - invalid service settings
  - missing arrival node
  - missing required content ids
  - no valid win path where one is required
- **harsh but legal content**
  - no safe anchor
  - scarce resources
  - brutal enemy advantage
  - hidden or obscure victory path
  - one missed opportunity causing defeat
  - no forgiving recovery path

Harsh Scenarios are legal if they are structurally valid. The designer is responsible for making harsh content fair or fun.

Current implementation note: not all long-term validation examples are implemented yet. M12 validates the current authored outcome shape structurally and through tests, but does not attempt full softlock proofing or complete instant-win/loss analysis.

---

## 3. Content hierarchy

The intended authoring hierarchy is:

- **Campaign**
  - Scenario sequence
  - branches
  - transition rules
  - carry-over rules
- **Scenario**
  - World Map
  - Regions
  - hero pool
  - resource defaults
  - victory conditions
  - defeat conditions
  - global events
  - teams
  - Scenario-specific variables and flags
- **World Map**
  - Region list
  - Region visibility / unlock state
  - manually authored Region adjacency
- **Region**
  - nodes
  - routes
  - arrival node
  - Services
  - neutral encounters
  - pickups
  - Region events
  - ownership
  - initial reveal
  - teams present in that Region
- **Location**
  - screens / maps
  - event sprites
  - event triggers
  - service calls
  - collision / layer behavior
  - visual state changes
- **Content libraries**
  - units
  - heroes
  - generic units
  - items
  - artifacts
  - recipes
  - Services
  - event actions
  - condition types
  - templates

Standalone Scenarios are legal. Campaigns are not required.

Current bounded-slice exception: M12 does not introduce the full top-level `ScenarioDefinition` content kind. The current outcome authoring surface is the single optional `content/scenario_outcome.json` file.

---

## 4. Reusable Regions

Regions may be reusable across Scenarios. A World Map links to one or more Regions. A Scenario may define variables, flags, rules, or overrides that make a reused Region behave differently in that Scenario.

This allows shared authored spaces without requiring every Scenario to duplicate Region data. Validation must evaluate a Region in the context of the Scenario that uses it.

---

## 5. World Map authoring

World Map adjacency is manually authored. The designer defines which Regions connect to which other Regions. The World Map should not rely on automatic polygon-border detection as the source of truth.

A Scenario may start with multiple Regions defined, with some locked or hidden. Locked Regions are not shown to the player until unlocked. A Region may exist and remain non-enterable forever. This is legal if it is intentional and not required for victory.

World Map authoring is future scope. The current playable slice is single-Region.

---

## 6. Region authoring

Each Region should define at minimum:

- id
- display name
- authored description
- Region size label
- initial unlocked / enterable state
- arrival node
- nodes
- routes
- reveal defaults
- Services
- neutral encounters / temporary contents where authored
- ownership state where applicable
- initial teams / stationed guards where applicable

Every Region must contain at least one node. Every playable Region must have exactly one valid arrival node unless a later design explicitly supports multiple arrival nodes.

### Region size

Region size is derived from node count. The intended labels are:

- Tiny
- Small
- Medium
- Large
- Huge
- Gigantic

The exact thresholds can be tuned later.

---

## 7. Node authoring

Nodes are fundamentally empty travel points. Avoid treating “node type” as the primary design truth. A node's gameplay behavior is determined mostly by its **node content** and attached events.

### Node content

A node may contain at most one main content item.

Examples:

- resource pickup
- artifact pickup
- Service
- neutral enemy
- one-time special content

A node may not contain both a resource pickup and a neutral hostile encounter.

### Events on nodes

Events may be attached to nodes. A designer should be able to attach or edit node events easily, for example through a node context menu such as “Edit event...”. A node event may optionally take priority over the node's normal content if authored to do so.

### Region node-entry event

A **Region node-entry event** triggers when an eligible team arrives on the node. This is distinct from Location-mode collision or confirm-button events.

Current implementation supports `regionNodeEntry` and uses it for the M12 authored victory/defeat demo.

---

## 8. Blocker behavior

A blocker is usually behavior created by content, not an intrinsic node category.

Examples of blocker behavior include:

- gate service
- quest gate
- neutral enemy blocking traversal
- hostile team occupation
- authored service rule
- other content that prevents passage until resolved

Validation should reason about blockers based on the content that creates them.

---

## 9. Route authoring

Routes connect nodes inside a Region.

A route should define:

- source node
- destination node
- road flag
- terrain enum

Travel time and Energy cost are computed from route quality and distance.

### Route state

Routes may be:

- visible / active
- hidden
- destroyed
- restored

Events may eventually destroy, restore, reveal, or activate routes. Events do not create entirely new route definitions from nothing at runtime. If a route should appear later, it should exist as hidden or inactive authored data.

---

## 10. Service authoring

Region Services are predefined typed Services with editable legal settings. Service definitions are data-based and may be modded or overridden where legal. Code still defines the supported service type list and runtime behavior.

### Service defaults

Each service type should have global defaults. Defaults live in data and can be created or edited with the designer tool. Defaults are global, not per Scenario, although individual placed Services may override allowed fields.

### Service validation

Validation should be strict and service-type-specific.

Examples:

- storage capacity must be valid
- sealed hero service must reference a valid Sealed hero
- market stock must reference valid items
- black market stock must reference valid artifacts
- mine resource type must be valid
- quest service must contain a valid quest chain if quests are authored
- recruitment service must reference valid unit ids
- artifact handler must reference valid artifact-combination rules
- farming service must have valid seed/fertilizer/output settings

### Service visibility

A Region Service becomes visible when its node is revealed.

### Service state

Events may eventually change Service state, including:

- destroyed
- restored
- settings override

Events may override authored service settings where such overrides are legal.

---

## 11. Location service calls

Location-mode service use is event-driven. A Location event may call a reusable service definition. Location service calls use the same service flow as predefined Region Services where allowed.

The exact allowed list can be finalized later. This keeps Location interactions flexible without creating separate incompatible service systems.

---

## 12. Event object structure

An Event object should include:

- id
- trigger type
- eligibility
- conditions
- optional priority
- optional message / portrait / image
- action chain
- repeatability
- state flags

Events are data-authored and should be editable through the designer tool.

---

## 13. Event triggers

Currently implemented trigger categories include:

- Region node-entry event
- start-of-day automatic event
- Location collision event
- Location confirm-button event
- neutral encounter defeated
- service used
- service destroyed
- quest completion

More triggers may be added later, but they should remain typed and explicit.

---

## 14. Event eligibility

Eligibility defines who may trigger or participate in an event.

Supported eligibility checks include:

- team color
- human / AI
- time window
- required hero
- combinations of the above

Do not use eligibility for general world-state requirements. Those belong in conditions.

---

## 15. Event conditions

Conditions define whether the actual requirement is satisfied.

### Current implementation

The current shared condition evaluator supports:

- `always`
- `teamHasResource`
- `teamHasHero`
- `storyFlagSet`
- `all`
- `any`
- `not`

These are the only condition leaves M12 outcome authoring should rely on.

### Future condition targets

Future event, quest, victory, and defeat conditions may include checks such as:

- team has item
- team has artifact
- team owns node or Service
- team defeated another team
- day / week / month range
- quest state
- Region revealed
- unit count
- Service state
- route state
- Scenario variable state

Those leaves are design targets only until code and validation implement them.

Quest objectives, event conditions, victory conditions, and defeat conditions should share the same broad typed condition model where practical.

---

## 16. Event actions

Event actions are the typed effects executed by an event.

### Current implementation

The current event system recognizes these action types:

- `showMessage`
- `giveResource`
- `takeResource`
- `setStoryFlag`
- `clearStoryFlag`
- `if`
- `spawnTeam`
- `removeTeam`
- `changeAlliance`

The action list should remain explicit and typed. Implemented action handlers must fail explicitly when required context or required arguments are missing.

### Future action targets

Future phases may add:

- give/take item
- give/take artifact
- give troops
- recover team
- grant experience
- grant skill
- remove skill
- kill / remove unit
- start fight
- change ownership
- unlock/lock Region
- destroy/restore Service
- update guidance
- trigger victory/defeat event chains
- build/restore/upgrade Location service
- change AI personality/aggression/patrol
- destroy/restore route
- reveal/activate hidden route

These are not current action types unless code and validation explicitly support them.

---

## 17. Event branches

Events should support nested If / Else branches. Branches may go as deep as the designer needs. Branching is preferred over optional action flags.

Example:

```text
If traveling party has room:
  Show "Jon joins you."
  Add Jon to traveling party.
Else:
  Show "Your party is full."
```

This gives designers explicit control over fallback behavior.

---

## 18. Event action execution model

Event action chains are **non-atomic ordered actions**.

Rules:

- actions execute strictly in authored order
- previous successful actions are not rolled back if a later action fails
- there is no automatic refund
- designers are responsible for safe event flow
- validation and runtime guardrails should help catch mistakes

This is **Model A: non-atomic ordered actions**.

### Runtime event-action failure

Event actions should not fail during normal intended play.

Designers should use:

- eligibility
- conditions
- If / Else branches
- validation

to prevent illegal actions from being reached.

If an action still fails at runtime:

- the game should fail safely
- previous successful actions are not rolled back
- the player should see a clear popup/log message when reasonable
- the reason should be included when possible
- developer/debug logs should include enough detail to fix the authored content

Examples:

- “Could not add Jon to party.”
- “Could not add item: stack is full.”
- “Could not take resource: not enough Wood.”

### Take actions

Actions that take resources or items must re-check availability at execution time. If unavailable:

- fail hard
- do not clamp
- do not allow negative resources

### Give actions and overflow

Give actions must obey inventory and roster rules. If receiving rules cap or reject excess:

- the system rule applies
- excess may be discarded or capped where that is the defined rule
- the player should receive popup/log feedback when reasonable

---

## 19. Event repeatability and priority

Manual events are one-shot by default. A manual event may be marked repeatable. Repeatable manual events do not require a cooldown by default.

Automatic events may:

- run at start of day
- repeat every N days
- run when conditions are true

When multiple automatic events are eligible at the same time, use authored priority. The designer tool should allow moving automatic events up/down in priority. Newly created automatic events should be added to the bottom by default.

---

## 20. Quest service authoring

A quest service is a Service containing a quest chain.

A quest service may contain:

- zero quests
- one quest
- several quests in order

### Quest entry structure

A quest entry should define:

- starting message
- progress message
- completion message with Yes / No
- objective
- completion event action chain
- eligibility
- repeatability where applicable

Quest-service objectives use the same broad typed condition structure as events. Quest completion actions are event actions.

### Empty quest service

A quest service may have zero quests. If it has no authored quests, it shows a default empty / abandoned message unless overridden.

### Completion by other teams

If another team completes a quest that is visible in the player's quest log, the player's entry becomes failed by default.

---

## 21. Victory and defeat authoring

Victory and defeat conditions are Scenario-level rules. They are separate from quests, even when they reference quest-service completion.

### Current M12 behavior

The current implementation accepts a single optional `content/scenario_outcome.json`. Per-Scenario authoring through a full top-level `ScenarioDefinition` content kind is not yet introduced; the single file covers the bounded slice.

Both `victoryConditions` and `defeatConditions` reuse the same `EventCondition` tree shape as events. See `docs/content_schema.md` for the exact file shape.

Supported condition leaves are exactly:

- `always`
- `teamHasResource`
- `teamHasHero`
- `storyFlagSet`

Supported composites are:

- `all`
- `any`
- `not`

Authored victory conditions, when present, **disable default victory entirely**. To use the default “all hostile teams defeated/removed/allied” rule, leave `victoryConditions` empty or omit the file.

Defeat conditions are OR-based. If both a defeat and a victory condition match in the same evaluation, **defeat wins** — consistent with §36 of `core_loop_rules.md`.

Latched outcomes are persistent. Once victory or defeat is latched, later state changes and save/load should not re-evaluate it away.

### Long-term victory model

Long-term victory conditions are OR-based. Each victory condition may eventually define:

- condition
- eligible teams
- victory event actions where authored

Satisfying any one victory condition wins the Scenario for that team.

If no authored victory condition exists, the Scenario falls back to the default victory condition:

- defeat/remove/ally all hostile enemy teams

If no authored victory condition exists and the default condition cannot make sense, future validation should report a structural problem. Current M12 validation does not attempt full reachability or all impossible-victory proofs.

### Long-term defeat model

Long-term defeat conditions are OR-based. Each defeat condition may eventually define:

- condition
- affected team(s)
- defeat event actions where authored

If any defeat condition becomes true for a team, that team loses.

### Instant win/loss validation

Future validation should detect obvious instant win or instant loss states where practical. M12 does not implement full instant-win/loss or softlock proofing.

### Future condition leaves

Richer leaves such as hero alive, route state, ownership, time limits, unit counts, Region revealed, party eliminated, or player leader dead are explicit future work. Do not add them casually to make one scenario demo richer.

---

## 22. Team and AI authoring

A team definition may include:

- color
- human / AI
- starting Region
- starting node
- active party
- reserve
- starting resources
- inventory
- artifacts
- hero stats and skills
- owned Services
- alliance state
- AI personality
- AI aggression
- patrol radius
- initial revealed nodes

Validation should eventually enforce:

- max 8 teams per Region
- unique colors where required
- valid starting node
- valid active party
- valid leader rules
- valid alliance references
- valid AI settings

### Enemy team templates

Enemy team templates should be based on hero units.

A template includes:

- 1 hero unit
- 1–2 generic unit types
- small stack ranges by default
- personality
- aggression

Enemy team templates do not include artifacts or resources by default.

### Team spawning

Events may spawn teams from templates or mutate existing runtime enemy-team state. Spawned teams may inherit Scenario defaults for AI, personality, resources, or other legal fields in future systems.

Current M12 enemy-team mutation actions are runtime mutations: `spawnTeam`, `removeTeam`, and `changeAlliance`.

---

## 23. Item, artifact, and recipe authoring

### Item definitions

An item definition should include:

- id
- name
- icon
- description
- subtype
- stack cap
- usable context
- effect actions where applicable
- base value

Food is a normal item subtype.

### Food

Food is:

- field-use only
- hero-consumed
- produced by cooking recipes
- defined as an item with food-specific effects and duration

### Cooking recipes

A cooking recipe should define:

- id
- input ingredients
- output food item
- output quantity
- time cost
- required passive skills
- output modifiers from passive skills where applicable

### Artifact definitions

An artifact definition should include:

- id
- name
- allowed slot types
- effects by slot where applicable
- rarity / tier
- base value
- combinable / upgradable flag

Not all artifacts are combinable. Ultimate or final-form artifacts may set combinable / upgradable to false.

### Artifact combination recipes

Artifact combination recipes are always **2 inputs to 1 output**.

Inputs may be:

- two of the exact same artifact
- one artifact plus one other artifact type

Combination recipes define:

- input artifact ids / quantities
- output artifact id
- service restrictions where applicable

Artifact combination recipes are globally fixed, while specific services may deny selected recipes.

---

## 24. Validation errors and warnings

Validation should be strict but useful.

### Errors

Examples of long-term validation errors:

- missing required references
- invalid ids
- no arrival node in a playable Region
- invalid route references
- invalid resource type
- invalid service config
- impossible mandatory victory target
- player team has no legal leader
- invalid active party
- invalid team color setup
- instant unavoidable win/loss at Scenario start
- authored content required for victory cannot exist

Errors prevent play.

Current implementation note: this list includes future validation targets. Current code validates the implemented content systems and should expand only when a phase needs the deeper checks.

### Warnings

Examples of validation warnings:

- no safe anchor
- very harsh economy
- hidden important object
- quest-important artifact can be discarded or consumed
- take-resource action without obvious condition
- give-hero action without apparent capacity branch
- generic unit loss risk not clearly communicated
- non-enterable Region exists forever
- obscure but legal victory path

Warnings do not prevent saving. Some warnings may still prevent marking a Scenario as release-ready if the designer chooses that workflow.

---

## 25. Softlock validation

Validation should eventually include best-effort softlock analysis.

This may include graph analysis for:

- required quest item reachability
- victory target reachability
- required hero availability
- arrival-node validity
- Region path validity
- required Region unlock paths
- route destruction / restoration logic
- service availability needed for required progression

Validation does not need to prove every possible scenario path correct. The goal is to catch likely structural impossibilities and obvious softlocks. Designers remain responsible for the authored experience.

Current implementation note: M12 does not implement full softlock or victory-reachability proofing.

---

## 26. Designer responsibility

The editor should help authors avoid mistakes, but it cannot replace design judgment.

Docs and tools should make clear:

- designers may create harsh or obscure Scenarios
- validation catches structural impossibility, not bad taste
- intentional difficulty is legal
- intentional softlocks should be modeled as defeat conditions, not accidental broken states
- scenario authors are responsible for avoiding accidental unwinnable content

---

## 27. Designer tool workflow

The long-term designer tool should support:

- Campaign editor
- Scenario editor
- World Map editor
- Region editor
- Location editor
- node editor
- route editor
- service property editor
- event pipe editor
- condition editor
- action-chain editor
- quest-service editor
- victory / defeat editor
- team editor
- template editor
- item / artifact / recipe editor
- validation report
- content reference picker
- test-play from selected Scenario / Region / node / day / team

The tool should be designer-friendly, but content should also remain hand-editable where practical. JSON or similar data files should stay reasonably human-readable.

---

## 28. Validation timing

Validation should run:

- on editor save
- on game startup
- in CI / tests
- before marking content playable
- before packaging or release

Invalid content may be saved as work-in-progress. Invalid content should not be playable.

---

## 29. Player character authoring

The player character is authored as a normal unique hero unit with special human-team rules. For human teams, the Team definition owns `playerCharacterHeroId`.

Single-player Scenarios must define exactly one player character for the human team. Multi-human / PvP Scenarios may define one player character per human team.

### Character creation

Before a Scenario or Campaign starts, the player creates the player character. The Scenario provides the stable hero identity, such as `hero_player`.

Character creation fills that identity with:

- name
- sex
- simple graphical representation
- starting stats
- starting skills
- starting preset/template

Starting presets such as Warrior, Builder, and Explorer are presets only. They are not permanent class restrictions.

### Authoring restrictions

The player character:

- must belong to the human team
- must stay in the traveling party
- may be active or reserve
- cannot be stored
- cannot be dismissed
- cannot become Temporarily Unavailable
- cannot be recruitable
- cannot be sealed/frozen
- cannot be neutral
- cannot be AI-owned
- cannot appear in AI templates

The player character is a hero mechanically, but is not part of the recruitable hero pool.

### Events

Events may modify player-character stats, skills, passives, appearance, name, equipment, or other authored properties. Events may not replace the player-character identity with a different hero id during a Scenario.

---

## 30. Agent / implementation guidance

For future implementation and AI-agent work:

- treat authored content as data-driven unless code-level mechanics are required
- keep event types, condition types, and action types explicit
- do not create arbitrary scripting languages unless the design changes
- prefer typed data plus validation over free-form behavior
- treat node behavior as node content plus events, not as a large fixed node-type hierarchy
- validate references aggressively
- let designers save unfinished content, but block invalid content from play
- implement validation as both editor feedback and automated test/CI support
- keep validation best-effort for softlocks; do not promise perfect proof of winnability
- distinguish current implementation from long-term design targets when editing docs
- do not describe future condition leaves/actions as available unless code and validation support them

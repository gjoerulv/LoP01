# Terminology Map

This document defines the current intended project terminology and explains how it maps to any older runtime, content, or serialized names that may still appear in the repository.

Use this file as the terminology source of truth when design docs, AI guidance, source code, content files, and serialized values do not all use the same names.

This is a terminology and interpretation document. It is **not** the full design specification.

Current design truth lives primarily in:

- `docs/game_vision.md`
- `docs/game_shell_flow.md`
- `docs/presentation_game_feel.md`
- `docs/combat_rules.md`
- `docs/core_loop_rules.md`
- `README_DECISIONS.md`
- `docs/scenario_authoring.md`
- `docs/validation_system.md`
- `docs/content_schema.md`

---

## 1. World structure terms

### Campaign
A **Campaign** is a collection of connected Scenarios.

- A Campaign may or may not have a shared story.
- A Campaign may branch.
- Winning a Campaign requires winning the relevant connected Scenarios defined by that Campaign.

### Scenario
A **Scenario** is the top-level authored play unit.

- A Scenario may stand alone.
- A Scenario may also be one chapter of a larger Campaign.
- A Scenario may inherit selected data from a previous Scenario in a Campaign.

### World Map
The **World Map** is the scenario-level Region selection layer.

Use **World Map** when referring to:
- opening the scenario map
- inspecting other Regions
- initiating Region-to-Region travel

### Region
A **Region** is the main in-scenario travel space.

Use **Region** when referring to:
- the node graph the player currently moves on
- node-to-node travel
- Region-layer Services
- enemy-team movement and occupation
- current in-scenario world-state

### Location
A **Location** is an entered place inside a Region.

A Location may contain:
- NPCs
- quests
- multiple screens
- zero or more Services
- dungeon-like content
- safe-anchor behavior

### Service
A **Service** is a functional interaction available either:
- directly in a Region
- or inside a Location

Examples may include:
- recruitment
- storage
- recovery
- supply
- sanctuary
- trader interaction
- mine / owned-resource interaction
- Sealed / Frozen Hero interaction
- quest interaction
- other authored interactions


### Player character
The **player character** is a normal unique hero unit with special human-team rules.

The player character uses a stable hero id such as `hero_player` and is configured through character creation.

The player character must stay in the human team's traveling party, but may be active or reserve.

### Character creation
**Character creation** is the pre-Scenario or pre-Campaign process that fills the stable player-character identity with name, sex, simple appearance, starting stats, starting skills, and starting preset/template.

---

## 2. Region structure terms

### Node
A **node** is a travel point inside a Region graph.

A node's gameplay behavior is determined by its main **node content** and any attached events.

There is **no dedicated permanent combat-node type** in the current design.

### Node content
**Node content** is the main authored content placed on a Region node.

A node may contain at most one main content item, such as:
- resource pickup
- artifact pickup
- Service
- neutral enemy
- one-time special content

Blocker behavior is usually created by content, such as a gate service, neutral enemy, hostile team occupation, or authored rule. It is not primarily a separate node type.

### Route
A **route** is the connection between nodes inside a Region.

A route has authored quality such as:
- road
- rough terrain
- snow
- desert
- meadow
- other authored terrain / route types

Route quality affects:
- travel time
- Energy cost

### Arrival node
An **arrival node** is a flag on a node, not a separate node category.

- Each Region must have an arrival node.
- An arrival node may also be a Location node or single Service node.
- Arrival nodes are protected from enemy spawning and enemy occupation.

### Safe anchor
A **safe anchor** is a Location that provides:
- free rest
- guaranteed rest

A safe anchor is not a universal mandatory structure for every Region or Scenario.

### Dungeon
A **dungeon** is a type of Location.

Enemy teams do not enter Locations, including dungeons.

### Sanctuary
A **sanctuary** is a single Service node on the Region layer where combat cannot be initiated.

- Teams occupying a sanctuary cannot be attacked there.
- Sanctuary is not a blocker-node type.
- Sanctuary may still be destroyable if flagged as destroyable.

### Owned resource service
An **owned resource service** is a capturable Region-layer service that generates resources over time for its owning team.

Mines are the baseline example of an owned resource service.

### Mine
A **mine** is an owned resource service that generates resources daily.

- A mine may be free or guarded.
- A mine may be captured by teams.
- A mine does not inherently block movement.
- A mine may have stationed units.
- Stationed units affect defense, not production.

### Sealed / Frozen Hero service
A **Sealed Hero** or **Frozen Hero** service is a one-time Region Service that frees a hero from a special **Sealed** state.

- Freeing the hero costs time and Energy.
- The action is illegal if the acting team has no room for the hero.
- The freed hero joins the acting team using normal hero-placement rules.
- After use, the node becomes an empty travel node.

### Quest service
A **quest service** is a specific authored Service structure that exposes quests on the map.

A quest service:
- may contain zero, one, or several quests
- exposes at most one currently available quest from its chain at a time
- always resolves through turn-in
- may be blocking, non-blocking, disappearing, or repeatable in the blocker/toll case

### Location-built service
A **Location-built service** is a service created, restored, upgraded, revealed, or enabled through a Location-mode event.

It is not a separate universal construction subsystem.

The trigger may be an NPC, object, shrine, counter, stone, field plot, or other authored Location interactable.

### Farming service
A **farming service** is a service or event-driven service flow that lets a team plant seeds and later collect grown output.

Farming exists in two forms:
- direct Region farming services
- Location farming triggered by events

Region farming is contestable and may be guarded, sabotaged, or stolen from.

### Crop care / watering
**Crop care** or **watering** is the farming support action that improves daily crop growth progress.

It costs time, not resources, and is limited to once per day per farming service.

### Fertilization
**Fertilization** is an optional planting-time farming modifier.

It is chosen when planting, not later, and may affect:
- growth speed
- output amount
- output type

### Artifact handler service
An **artifact handler service** is a service that combines artifacts.

It does not dismantle artifacts, repair artifacts, or perform general crafting.

### Cooking
**Cooking** is the party-menu crafting flow for turning ingredients into food.

Cooking is available outside battle while inside a Scenario whenever the party menu is available.

---

## 3. Party, roster, and inventory terms

### Active party
The **active party** is the current battle-legal party.

- up to 5 units
- exactly one assigned leader
- the units used directly in battle

### Reserve
The **reserve** is the traveling non-active roster.

- up to 7 units
- travels with the player
- may be switched into the active party outside battle

### Traveling party
The **traveling party** is:

- the active party
- plus the reserve

This is the roster that:
- moves with the player inside a Region
- crosses between Regions
- determines the team's shared Energy

### Stored units
**Stored units** are units assigned to a specific storage Service.

- each storage Service has its own independent storage
- each storage has up to 7 slots
- stored units do not travel with the player
- stored units persist in the Region where they are stored

### Temporarily Unavailable
**Temporarily Unavailable** is the current design term for heroes who have been removed from the roster and are not yet back in the available hero pool.

A Temporarily Unavailable hero is:
- not active
- not in reserve
- not stored
- not recruitable until returned to the relevant pool

This term is preferred over older narrow wording such as "recovering" when the state may result from either injury or voluntary dismissal.

### Sealed
**Sealed** is a distinct hero state used for heroes trapped in one-time Sealed / Frozen Hero services.

A Sealed hero:
- is not currently owned by any team
- is not currently available in ordinary hero recruitment
- may be freed by a legal team through the service that contains them

### Item inventory
The **item inventory** is the shared team inventory for non-artifact items.

This includes things such as:
- consumables
- stackable utility / trigger items
- seeds
- ingredients
- food

### Artifact inventory
The **artifact inventory** is the shared team inventory for unequipped artifacts.

Artifacts are not stored in the general item inventory.

### Equipped artifact slots
Heroes equip artifacts into:
- **1 Attack slot**
- **1 Defense slot**
- **3 Misc slots**

Generic units do not equip artifacts.

---

## 4. Team, ownership, and enemy-world terms

### Team color
A **team color** is the primary identifier for a team in world-layer logic.

- The player team also has a color.
- The game supports up to 8 total teams in a Region, including the player.

### Allied
**Allied** teams may share a node and do not attack one another by default.

Allied does **not** automatically grant:
- shared ownership
- shared service-use rights
- shared recruit ownership
- shared storage ownership

### Enemy
**Enemy** teams are teams that may attack each other and contest control directly.

There is no intermediate diplomacy state between allied and enemy.

### Ownership
**Ownership** means that a node or Service belongs to a specific team for control purposes.

Ownership matters for things like:
- mines
- storage gates
- destroyable Region Services
- quest-service competition in some scenarios
- other authored owned assets

Ownership transfers immediately when captured.

### Enemy team
An **enemy team** is an AI-controlled traveling party on the Region layer.

An enemy team:
- moves within a Region
- does not travel between Regions
- does not enter Locations
- may contain heroes and generic units
- may use direct Region Services
- may occupy nodes
- may attack the player
- may attack direct storage gates
- may capture owned nodes and Services
- may destroy destroyable Region Services
- may use eligible quest services

Enemy teams only act while the player is in the same Region.

### Enemy-team personality
An **enemy-team personality** is the broad behavioral profile that shapes systemic priorities.

Baseline personalities are:
- **Warrior**
- **Builder**
- **Explorer**

### Aggression level
An **aggression level** describes how willing an AI team is to initiate fights.

Baseline aggression levels are:
- **Berserk**
- **Reckless**
- **Opportunistic**
- **Careful**
- **Coward**
- **Pacifist**

### Patrol radius
A **patrol radius** is an authored movement constraint for an enemy team.

If patrol is enabled, the team stays within its allowed radius unless specific later rules override it.

### Stationary hostile encounter
A **stationary hostile encounter** is temporary neutral hostile content on a node.

It is not a colored team and not an owned faction asset.

Once cleared, that node becomes an empty travel node.


### AI behavior seed
An **AI behavior seed** is hidden Scenario-start seed data used to make AI world behavior deterministic.

The same Scenario state, same AI seed, and same player actions should produce the same AI world decisions.

### Battle seed
A **battle seed** is deterministic seed data used for battle simulation and damage randomness.

Battle seed behavior is separate from AI world behavior seed data.

### AI knowledge
**AI knowledge** is the set of Region information available to an AI team through that team's own fog-of-war and scouting rules.

AI teams should not act on information outside their AI knowledge.

### Priority pipeline
A **priority pipeline** is the AI action-selection structure where major priorities are considered in order before personality weighting is applied.

The broad order is:
- victory opportunity
- survival / defeat avoidance
- urgent tactical or logistical needs
- personality-shaped goals

### Threat preview
**Threat preview** is the cheap color-coded estimate shown before or around battle selection.

It is not a full battle simulation and does not represent the exact auto-resolve result.

### Auto-resolve result
An **auto-resolve result** is the actual result of an automated backend battle simulation.

It uses the battle rules and AI battle controller rather than a simple threat estimate.

### Auto-combat
**Auto-combat** is toggleable automated control during a manual battle.

It should use the same battle AI controller as auto-resolve.

### Fog of war
**Fog of war** is the per-team revealed/unrevealed map-knowledge system.

Unrevealed areas are hidden.

Revealed areas remain revealed and provide live information.

### Revealed
A node or area is **revealed** once a team has uncovered it through movement, scouting, allied vision, stationed guards, owned services, or reveal services.

Revealed information is live, not stale.

### Scouting
**Scouting** is the capability that increases reveal range and improves enemy-team inspection detail.

At high scouting, full enemy-team details may be inspectable.

### Visible enemy estimate
A **visible enemy estimate** is the default limited information shown for a visible enemy team.

It includes:
- color
- leader
- threat color
- unit quantity ranges
- hero level ranges

---

## 5. Progression and scenario terms

### Quest
A **quest** is a single authored task that a team may pursue.

A quest is not the same thing as a full scenario progression model.

### Objective
An **objective** is a typed requirement used by:
- a quest
- a victory condition
- sometimes other authored progression checks

### Quest chain
A **quest chain** is an ordered list of quests inside one quest service.

Only one quest in that chain is currently available at a time.

### Event
An **event** is the general trigger / condition / effect system that changes world state.

Events are the primary systemic progression engine.

### Event actions
**Event actions** are the typed effects that fire when an event or quest turn-in resolves.

These may include:
- starting a fight
- granting resources
- recovering a team
- granting experience
- granting or removing skills
- killing a unit
- granting troops
- changing alliances
- changing ownership
- unlocking Regions
- spawning or removing teams
- destroying or restoring Services
- triggering victory
- triggering defeat
- setting story flags
- updating guidance

Use **event actions** as the system term rather than “reward” when the effect is broader than a simple benefit.

### Passive effect
A **passive effect** is a typed, always-on modifier authored on a unit definition (`passive_effects`). Each effect has a **kind** naming its consumer; the current kinds are `mine_production` (stationed-unit mine output) and `leader_energy` (current-leader daily Energy). Passive effects are definition-derived and resolved at the day boundary, not stored in save state. Prefer **passive effect** over “skill”/“buff” for this typed unit data.

### Eligibility
**Eligibility** defines who is allowed to participate in or trigger something.

Typical eligibility checks include:
- team color
- human / AI
- time window
- required hero present
- combinations of the above

### Condition
**Condition** defines whether the actual quest, event, victory, or defeat requirement is satisfied.

Eligibility and condition are separate concepts.

### Victory condition
A **victory condition** is a Scenario-level win rule.

- Victory conditions are separate from the quest system.
- A Scenario may have one or more victory conditions.
- Satisfying any one victory condition is enough to win.

### Defeat condition
A **defeat condition** is a Scenario-level loss rule.

- Defeat conditions are separate from the quest system.
- If any defeat condition becomes true, that team loses.

### Guidance text
**Guidance text** is an event-driven directional hint layer.

It is:
- separate from the quest log
- separate from formal victory / defeat conditions
- persistent until changed by another event
- hidable by the player

### Quest log / journal
The **quest log** or **journal** is the player-facing log of discovered quest-service tasks and their visible states.

Typical visible states are:
- **Undiscovered**
- **Visible in log**
- **Completed**
- **Failed**

### Carry-over
**Carry-over** is the set of selected state elements passed from one Scenario to another in a Campaign.

Carry-over is authored per transition from an explicit allowed list.

---

## 6. Economy and resource terms

### Resources
The baseline resource set is:

- **Gold**
- **Wood**
- **Stone**
- **Steel**
- **Fiber**
- **Clay**
- **Gems**

Resources belong to the team globally within a Scenario and are shared across Regions.

### Gold
**Gold** is structurally a normal resource, but it acts as the main universal currency and is usually gathered at a much higher scale than the other resources.

### Trading Post
A **Trading Post** is a specialized trader service used for:
- exchanging resources for other resources
- exchanging resources for gold and vice versa according to the defined trade model
- sending resources to other teams

It does not combine all other merchant functions into one service.

### Market
A **Market** is a specialized trader service used for:
- buying items for gold
- selling items for gold

Item stock is authored and restocks weekly.

### Freelancer’s Guild
A **Freelancer’s Guild** is a specialized service used for selling generic units for gold.

Teams may sell full stacks or partial stacks.

### Black Market
A **Black Market** is a specialized service used for buying artifacts for gold.

Its stock is authored and does not restock by default.

### Gold trade
**Gold trade** is the gold-based exchange model used by Trading Posts.

It is separate from the resource-for-resource barter model.

### Barter
**Barter** is the resource-for-resource exchange model used by Trading Posts.

It is not required to be derived through gold and may use its own punitive default exchange table.

### Base value
A **base value** is the underlying gold reference value used for resource pricing.

Current default base values are:
- Wood / Stone = 100 gold
- Steel / Fiber / Clay = 200 gold
- Gems = 500 gold

### Consumable
A **consumable** is an item used directly for battle, field use, or both.

A team may hold at most one identical consumable type at a time.

### Utility / trigger item
A **utility item** or **trigger item** is a non-consumable item used mainly for quests, events, or authored conditions.

These stack to 999 per identical type by default.

### Seed
A **seed** is a plantable item used through farming services.

Seeds are a distinct item subtype with later agricultural and ingredient use.

### Ingredient
An **ingredient** is an item used to produce food.

Farmed outputs may become ingredients.

### Food
**Food** is an item subtype that only heroes consume.

Food is a field-use layer, not a battle-use item layer.

Food may:
- recover HP/MP
- apply temporary buffs
- support longer-duration non-battle effects

### Recipe
A **recipe** defines the ingredients, time cost, skill requirements, and output of a cooking action.

Recipes are globally visible by default, though the player may filter to show only currently craftable recipes.

### Artifact
An **artifact** is the game's equippable gear layer.

Artifacts are:
- hero-only
- slot-based
- stored in the artifact inventory when unequipped
- potentially stackable by identical type in inventory
- sometimes combinable into stronger artifacts

There is no broader separate “equipment” system outside artifacts.

### Artifact combination
**Artifact combination** is the irreversible process of consuming specific artifacts to create a stronger artifact through a dedicated service.

### Battle spoils
**Battle spoils** are the economic gains from defeating another team in battle.

By default, this includes:
- all defeated-team items
- all defeated-team artifacts
- 1/4 of defeated-team non-gold resources

### Escape
**Escape** is a battle outcome initiated by the current leader where a team flees and later recovers.

The escaping team survives, all heroes and reserve units survive, and active generic units are lost.

### Surrender
**Surrender** is a battle outcome where a team offers gold to the opponent in order to retreat with its full team intact.

Surrender is initiated by the current leader.

The surrendered team later respawns under the normal surrender rules.

---

## 7. Energy terms

### Energy
**Energy** is a shared travel resource belonging to the traveling party.

Units do **not** have individual Energy.

Current intended daily starting Energy is:

`1000 + (lowest traveling-party agility × 100) + leader passive bonus + leader item bonus`

Energy may be restored by:
- rest
- items
- events
- Services

### World Map travel Energy
Region-to-Region travel on the World Map requires:
- **1000 Energy**
- paid once when travel begins

### Service destruction cost
Destroying a destroyable Region Service costs:
- **1000 Energy**
- **1 hour**

### Sealed / Frozen Hero freeing cost
Freeing a Sealed / Frozen Hero costs:
- **500 Energy**
- **1 hour**


---

## 8. UI and information terms

### Adventure button strip
The **Adventure button strip** is the left-side icon menu in Region and World Map UI.

It is not the same thing as the Scenario Info screen.

### Scenario Info screen
The **Scenario Info screen** is the formal screen for Scenario-level rules and information, especially victory and defeat conditions.

### Contextual info panel
The **contextual info panel** is the bottom-left Region / World Map information panel.

It shows selected node or Region information when something is selected, and otherwise may toggle between active-party summary and occupied-node information.

### News popup
A **news popup** is a temporary event or status message shown in the upper-left content area.

News popups are also recorded in the message log / history where appropriate.

### Message log / history
The **message log** or **history** is the persistent record of relevant messages, popups, and guidance changes with date information.

### Threat color
**Threat color** is the UI color used to communicate approximate danger.

It appears on target cursors and hover / inspection panels where appropriate.

### Target preview
**Target preview** is the battle UI information shown before confirming an action against a target.

It may include damage, KO / kill result, and Agility penalty.

### Help window
The **help window** is the battle panel under the CTB bar used for skill, target, and contextual help text.

### Party menu
The **party menu** is the main out-of-battle menu for managing active party, reserve, items, artifacts, cooking, and positions.

### Active container
An **active container** is the current unit-container context for stack operations.

It may be:
- active party
- reserve
- currently opened storage

Generic stack split/merge shortcuts operate only within the current active container.

### Context action
A **context action** is the controller / touch / keyboard equivalent of secondary mouse interaction such as right-click.

It is used for actions like showing requirements, opening unit info, or opening a context menu.

### Context info action
A **context info action** opens detailed information about the selected unit, item, artifact, quest, or other inspected object.

### Select-then-confirm
**Select-then-confirm** is the movement interaction model where the first action selects and previews, and the second confirms.

It applies to Region node travel and World Map Region travel.

### Location event sprite
A **Location event sprite** is a Location-mode sprite tied to an authored event trigger.

It may define interaction behavior, collision, and render layer.

### Show All Recipes
**Show All Recipes** is the cooking UI toggle that reveals recipes even when the team currently lacks requirements.

Unavailable recipes remain disabled and show missing ingredient counts inline.

---

## 9. Authoring and validation terms

### Designer tool
The **designer tool** is the intended long-term editor for creating and modifying authored content such as Scenarios, Regions, Locations, nodes, Services, events, quests, teams, items, artifacts, recipes, and validation reports.

### Content library
A **content library** is a reusable data set for definitions such as units, items, artifacts, Services, recipes, condition types, and action types.

### Event attachment
An **event attachment** is an event connected to a node, Location sprite, Service, or other authored trigger point.

### Region node-entry event
A **Region node-entry event** triggers when an eligible team arrives on a Region node.

### Event branch
An **event branch** is an If / Else structure inside an event action flow.

Branches are used to author conditional outcomes and fallback behavior.

### Event action chain
An **event action chain** is the ordered list of actions executed by an event or quest completion.

Event action chains are non-atomic by current design.

### Non-atomic event execution
**Non-atomic event execution** means that event actions execute in authored order and previous successful actions are not automatically rolled back if a later action fails.

### Runtime event-action failure
A **runtime event-action failure** occurs when an authored event action cannot legally execute at runtime.

This should not happen in normal intended play, but if it does, the game should fail safely and show/log a clear reason when reasonable.

### Validation error
A **validation error** is a structural authoring problem that prevents content from being playable.

### Validation warning
A **validation warning** is a likely issue or design risk that does not necessarily prevent saving or playing.

### Softlock validation
**Softlock validation** is best-effort validation that looks for likely unreachable requirements, impossible victory paths, invalid Region paths, missing required content, or other structural progression traps.

### Playable content
**Playable content** is authored content that passes required validation checks and may be loaded for actual gameplay.

Invalid work-in-progress content may be saved, but should not be playable.


### Content schema
The **content schema** is the authored JSON data model described in `docs/content_schema.md`.

### Scenario Region Context
**Scenario Region Context** is Scenario-provided data used by a reusable Region when loaded in that Scenario.

### Localized text object
A **localized text object** stores player-facing text by language code.

### Mod override identity
**Mod override identity** is the top-level `kind + id` pair used to decide whether mod content overrides existing content.

### Authored initial state
**Authored initial state** is the starting state defined in content files.

### Runtime state
**Runtime state** is playthrough state stored in save data, not authored content.

### Save validation
**Save validation** runs when content is saved and never blocks saving.

### Playable validation
**Playable validation** determines whether content can be played. Errors block play.

### Release validation
**Release validation** determines whether content can be marked release-ready or packaged. Warnings must be resolved, acknowledged, or suppressed with a reason.

### Validation severity
**Validation severity** is the level assigned to a validation message.

Current severities are:
- Error
- Warning
- Info

### Warning suppression
**Warning suppression** is the act of acknowledging a validation warning with a stored reason so release validation may proceed.

### Quick fix
A **quick fix** is an explicit editor suggestion or action that may repair a validation issue.

Quick fixes must be reviewable and should not silently change designer intent.

---

## 10. Legacy runtime and content names

Some older names may still appear in source files, content files, tests, comments, or serialized values.

These names should be interpreted through the current design terminology rather than treated as design truth.

### `overworld`
Legacy design/runtime/content term that should now usually be interpreted as:

- **Region**, when referring to the in-scenario travel layer
- sometimes **World Map**, when referring to the higher-level selection layer

Always use context.

### `overworld_mode`
Legacy serialized or older runtime-facing term corresponding to:

- **Region**
- current runtime mode naming may use **RegionMode**

### `overworld_selection`
Legacy serialized or older runtime-facing term corresponding to:

- **World Map**
- current runtime mode naming may use **WorldMapMode**

### `overworld_destination`
Legacy content key used in some data files.

Interpret this as:
- destination on the Region layer
- usually a Region node or older Region travel destination field depending on context

Do not treat the key name itself as current design terminology.

---

## 11. Leader terminology note

### Current design intent
Current design intent is:

- only **hero units** may be leaders

### Legacy runtime mismatch
Some current runtime/content structures may still contain a separate legacy `Leader` category, such as:
- `UnitCategory::Leader`
- `UnitDefinitionCategory::Leader`
- content values like `"category": "leader"`

Treat that as a legacy implementation detail unless the task is specifically about refactoring that model.

Do **not** expand the old separate-leader-category model in new design work unless the design changes explicitly.

---

## 12. Game shell terms

### Game shell
The **game shell** is the player-facing entry and configuration layer outside active gameplay.

It includes boot flow, title screen, main menu, game mode selection, save/load, settings, mods menu, and dev/debug shell options.

### Game Mode Selection
**Game Mode Selection** is the New Game flow where the player chooses Campaign, Standalone Scenario, Tutorial, or PvP when available.

### Saved character template
A **saved character template** is reusable character-creation convenience data.

It is not Campaign carry-over data.

### Save metadata
**Save metadata** is version/content/mod information stored with a save file so compatibility can be checked before loading.

### Mods menu
The **Mods menu** is the shell screen for installed mod visibility, enabled/disabled state, load order, metadata, and validation health where supported.

---

## 13. Presentation terms

### Game feel
**Game feel** is the moment-to-moment tactile quality of play, including responsiveness, feedback, pacing, and perceived weight of actions.

### Presentation action
A **presentation action** is a typed event action that changes audiovisual presentation, such as playing a stinger, fading the screen, changing music, or showing a portrait.

### Stinger
A **stinger** is a short musical or sound cue used to emphasize a transition, discovery, battle start, result, warning, or story beat.

### Ambience
**Ambience** is background environmental audio or subtle visual motion that supports place identity without becoming the main music or gameplay signal.

---

## 14. Guidance for future work

When writing docs, code comments, prompts, plans, or design notes:

- prefer **World Map** over `overworld_selection`
- prefer **Region** over `overworld` for the in-scenario travel layer
- prefer **Location** for entered places inside a Region
- prefer **Service** for functional interactions
- prefer **traveling party**, **stored units**, **Temporarily Unavailable**, and **Sealed** when those are the correct concepts
- treat enemy-team behavior as **authored setup + systemic behavior**
- treat **events** as the universal progression engine
- treat **quest services** as a specific authored structure, not the whole progression system
- treat **victory conditions** and **defeat conditions** as Scenario-level rules outside the quest system
- keep **eligibility** and **condition** as separate concepts
- treat ownership, sabotage, sanctuary, and service destruction as Region-layer concepts unless explicitly designed otherwise
- treat **artifacts** as the hero equipment layer and **items** as the shared non-artifact inventory layer
- treat **gold trade** and **barter** as related but separate Trading Post systems
- treat Location construction/restoration/upgrade as event-driven world-state, not as one universal hard-wired building subsystem
- treat farming as either a Region service or a Location event-driven flow, depending on authored context
- treat cooking as a party-menu system available outside battle, not as a required world service
- treat artifact handling as artifact combination only unless the design explicitly expands later
- treat AI world behavior as deterministic from Scenario-start AI seed data
- treat AI knowledge as governed by fog of war, not omniscience
- distinguish **threat preview** from the actual **auto-resolve result**
- treat auto-resolve and auto-combat as using the same automated battle controller
- treat scouting as the gate for deeper enemy inspection
- distinguish the **Adventure button strip** from the **Scenario Info screen**
- keep hover information accessible through select/tap/context actions
- avoid UI patterns that rely on mouse-only hover
- use `docs/scenario_authoring.md` for content authoring and designer-tool terminology
- use `docs/validation_system.md` for validation levels, severities, gates, errors, warnings, and reports
- use `docs/content_schema.md` for authored content JSON schema conventions, Scenario Region Context, mod override identity, and runtime-state boundaries
- treat node behavior as node content plus event attachments, not as a broad fixed node-type hierarchy
- treat event action chains as non-atomic ordered flows
- treat Location UI as event-driven sprite interaction, not as a default service-list screen
- treat the player character as a unique hero with special human-team rules; they must remain in the traveling party and cannot be stored, dismissed, recruited, sealed, or AI-owned
- use `docs/presentation_game_feel.md` for moment-to-moment feel, audiovisual tone, transitions, and feedback

When source/runtime compatibility requires older names to remain in place:
- preserve compatibility deliberately
- document the mismatch if it could confuse future work
- do not assume legacy names define current design intent

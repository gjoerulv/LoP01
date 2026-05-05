# Game Vision

This document describes the current intended long-term vision for Project Ashvale.

For terminology, see `docs/terminology_map.md`.

Detailed systemic rules live primarily in:
- `docs/combat_rules.md`
- `docs/core_loop_rules.md`
- `docs/scenario_authoring.md`
- `docs/validation_system.md`
- `docs/content_schema.md`
- `README_DECISIONS.md`

This file stays at the **vision** level. It explains the intended shape of the game and the major layers of play without restating every exact rule and formula.

---

## 1. Core fantasy

Project Ashvale is a turn-based strategy/RPG about surviving, organizing a traveling force, and pushing through hostile territory while building enough strength, allies, resources, and positional advantage to win a Scenario.

The intended experience combines:
- authored world structure
- readable tactical combat
- meaningful roster consequence
- travel and logistics pressure
- enemy-team competition on the Region layer
- strong scenario identity
- event-driven progression
- contested economic pressure

The player should feel like they are:
- moving through dangerous territory
- making meaningful travel and roster decisions
- interacting with authored places and factions
- competing with other active forces in the world
- managing a shared team economy
- recovering from setbacks rather than merely resetting after them

---

## 2. World structure

### Campaign
A **Campaign** is a collection of connected Scenarios.

A Campaign may:
- tell a coherent story
- branch
- carry selected data from one Scenario to another
- allow different follow-up Scenarios based on victory conditions or player choices

### Scenario
A **Scenario** is the top-level authored play unit.

A Scenario should feel complete on its own, even when it is also one chapter inside a larger Campaign.

### World Map
Each Scenario has a **World Map**.

The World Map is the layer where the player:
- inspects Regions
- sees which Regions are unlocked and enterable
- initiates Region-to-Region travel

World Map travel is strategic and time-consuming. It is not free repositioning.

### Region
A **Region** is the main in-scenario travel space.

A Region is:
- an authored node graph
- explored and traversed by the traveling party
- the place where route quality, Energy, enemy teams, node ownership, and occupation matter

The player is always in exactly one Region at a time.

### Location
A **Location** is an entered place inside a Region.

Locations can represent:
- towns
- settlements
- hideouts
- dungeons
- special sites
- other authored places

A Location may contain:
- NPCs
- multiple screens
- multiple Services
- quests
- story events
- combat encounters
- safe-anchor functionality
- building / restoration / crafting-oriented economic hooks

### Service
A **Service** is a functional interaction available either:
- directly on the Region layer
- or inside a Location

Services should feel like authored world functionality rather than generic menu abstractions.

---

## 3. Region structure vision

Regions use a **node-content model**.

A node is fundamentally a travel point in a Region graph. Its gameplay behavior is determined by its main node content and any attached events.

A node may contain at most one main content item, such as:

- resource pickup
- artifact pickup
- Service
- neutral enemy
- one-time special content

There is no dedicated permanent combat-node type in the intended design.

Blocker behavior is usually created by content, such as a gate service, neutral enemy, hostile team occupation, or authored rule. It is not primarily a separate node type.

Once one-time content is cleared, the node usually becomes an ordinary empty travel node unless an event changes it.

This keeps the Region layer understandable, content-driven, and easier to validate.

### Arrival nodes
Each Region has a protected **arrival node**.

Arrival is a flag rather than a separate node category.

An arrival node may also be:
- a Location node
- or a single Service node

But arrival nodes are protected from enemy spawning and enemy occupation.

### Routes
Routes between nodes are authored and matter.

Route quality should communicate:
- speed
- difficulty
- travel friction

Roads are the baseline. Rough terrain increases travel burden.

### Location nodes vs direct Service nodes
The intended distinction is:

- **direct Service node** = quick functional interaction on the Region layer
- **Location** = entered authored place with richer content, NPCs, multiple Services, or dungeon-like structure

That distinction should remain clear in future design and implementation.

---

## 4. Travel, time, and Energy

The player manages a **traveling party** and a shared **Energy** pool.

Travel should feel constrained in a readable, strategy-friendly way:
- time matters
- route quality matters
- Energy matters
- party composition matters

### Region travel
Inside a Region, travel should be:
- shortest-path based
- node-to-node
- legible
- meaningfully affected by route quality

### World Map travel
Travel between Regions should feel like a deliberate strategic commitment.

It should:
- cost time
- cost Energy
- move the player to a protected arrival node in the destination Region
- force the player to think about what they are leaving behind

### Location entry
Entering and exiting a Location should not itself cost time.

This keeps Locations usable as authored spaces without making basic interaction feel overly cumbersome.

### Rest and recovery
Rest, Services, items, and events should all matter as part of travel endurance.

A safe anchor should feel meaningfully safer than a hostile or unstable Region.

---

## 5. Party, reserve, storage, and loss

### Active party
The **active party** is the battle-legal force used directly in combat.

It should be:
- small
- curated
- strategically meaningful

### Reserve
The **reserve** travels with the player and expands tactical and roster options without directly entering every battle.

### Traveling party
The **traveling party** is the player’s active party plus reserve.

It determines:
- travel presence in the world
- cross-Region hero transfer
- Energy computation
- vulnerability to Region-travel consequences

### Stored units
Stored units are tied to a specific storage Service and do not travel with the player.

Storage should create meaningful logistical decisions:
- what to carry
- what to leave behind
- what to defend
- what to risk

### Cross-Region consequence
Heroes in the traveling party travel between Regions.

Generic units in the traveling party do not survive Region change unless stored first.

This is an intentional strategic pressure point. Region travel should feel consequential.

### Temporarily Unavailable heroes
Heroes removed from the roster should usually enter a **Temporarily Unavailable** state rather than simply disappearing from design logic.

That state should support:
- battle loss consequence
- dismissal consequence
- later re-entry into the hero pool
- competition with other teams for hero availability

---

## 6. Battle vision

Battle is a static formation-based CTB system.

The intended feel is:
- readable
- mostly deterministic
- tactically meaningful
- fast enough to repeat
- deep enough that party composition matters

### Formation
Battle uses:
- Front
- Middle
- Back
- Leader

The active party has up to 5 units, with the leader occupying one of those slots.

### Leadership
Only hero units may be leaders in the intended design.

Leader choice matters both:
- because of aura and bonuses
- and because it shapes the identity of the active party

### Position
Position is tactically meaningful, but battle does not use free movement on a grid.

Position changes are deliberate actions, not free battlefield drift.

### Readability
The player should see enough information to make tactical decisions confidently:
- turn order
- target previews
- min/max damage
- min/max KO / kill result
- clear legality and penalty feedback

Detailed battle rules live in `docs/combat_rules.md`.

---

## 7. Enemy teams and world competition

One of the defining parts of the long-term vision is that the player is not alone on the Region layer.

Enemy teams are AI-controlled traveling parties that:
- move through Regions
- compete for resources, recruits, and position
- attack or avoid the player based on behavior profile
- occupy important nodes
- defend or destroy key Services
- sabotage other teams when it helps them win

This is a major part of the game’s identity.

The world should feel contested, not static.

### Authored setup, systemic behavior
Enemy teams should be:
- **authored in setup**
- **systemic in behavior**

A designer should be able to define:
- team color
- starting node
- template
- alliances
- patrol radius
- personality
- aggression

From there, the game should handle their movement and decision-making under shared world rules.

### Team colors and alliances
Teams are identified by color.

Teams are either:
- allies
- or enemies

There is no permanent middle state.

Alliance changes are authored events, not soft drifting diplomacy.

### Shared world rules
Enemy teams should use the same broad systemic world rules as the player where practical:
- Energy
- movement legality
- service use
- recruitment competition
- hero-pool competition
- storage loss
- leaderless teams where allowed

### Personality and aggression
Enemy behavior should be shaped by:
- broad personality
- aggression threshold
- patrol constraints
- authored objectives

Baseline personalities are:
- Warrior
- Builder
- Explorer

Baseline aggression levels range from:
- Berserk
- to Pacifist

This allows enemy teams to feel distinct without requiring unique hardcoded AI for every Scenario.

### AI behavior philosophy
Enemy-team AI should feel systemic and fair rather than scripted or omniscient.

The long-term vision is:
- medium-depth weighted behavior
- deterministic choices from hidden Scenario-start AI seed data
- no cheating with unrevealed information
- personality and aggression shaping action priorities
- hard patrol constraints unless changed by events
- systemic behavior shaped indirectly by gates, events, team setup, and world rules

AI should be strong enough to compete for resources, recruits, territory, and victory without requiring deep minimax planning.

### Auto-resolve-first design
On the Region layer, battles involving the player should be able to auto-resolve first.

The player should then be able to:
- accept the result
- or replay manually

AI-vs-AI battles should auto-resolve without interrupting play directly.

Auto-resolve should eventually be a full deterministic backend CTB simulation, not a loose strength comparison.

This supports a game with multiple active teams and many possible collisions without making pacing collapse.

---

## 8. Sabotage, ownership, and strategic denial

Enemy-team competition is not limited to direct battle.

The intended vision includes strategic denial and sabotage such as:
- blocking access to important nodes
- occupying Locations
- denying resources
- draining recruit pools
- attacking storage gates
- destroying destroyable Region Services
- contesting owned nodes such as mines
- using sanctuaries or chokepoints to create positional pressure

This makes the Region layer feel alive and contested.

### Ownership
Some nodes and Services may be owned by a team.

Ownership matters because it creates:
- resource flow
- territorial pressure
- reasons to defend infrastructure
- reasons to raid or sabotage

Ownership should transfer immediately when captured.

### Mines and income nodes
Mines and similar owned-resource nodes should act as strategic assets.

They should be:
- capturable
- contestable
- optionally guarded
- worth defending
- meaningful to sabotage indirectly

### Service destruction and restoration
Destroyable Region Services add a useful layer of strategic warfare.

Destruction should feel like:
- targeted denial
- deliberate Energy and time expenditure
- a way to reshape pressure in a Region

Restoration should feel like:
- committing resources to rebuild local support
- setting recovery in motion rather than instantly repairing the world

### Sanctuary
Sanctuary is an important special case.

A sanctuary is a Region Service node where combat cannot be initiated.

This creates:
- protected waiting spaces
- route tension
- strategic occupation pressure
- temporary safety without requiring a full Location

### Storage gates
Direct storage nodes are not just inventory abstractions.

They are:
- logistical anchors
- defensible assets
- tempting sabotage targets
- a place where stored units can be meaningfully lost if not defended

---

## 9. Hero availability beyond ordinary recruitment

The vision includes more than standard hero tavern/guild recruitment.

### Shared hero pool
Heroes should come from a shared scenario-defined pool, possibly with authored overrides.

This allows:
- competition between teams
- hero disappearance and later return
- scenario identity
- meaningful roster churn

### Sealed / Frozen Heroes
Some heroes may appear in special one-time Region Services where they are trapped, sealed, or frozen.

Freeing such a hero should feel like:
- a time-and-Energy investment
- a meaningful prize
- an authored moment in the world
- a direct roster expansion opportunity if the team has room

These nodes should become ordinary travel space after the hero is freed.

---

## 10. Progression vision

The long-term progression model should not be reduced to “quests.”

The intended structure is:

- **Events** are the universal progression engine
- **Quest services** are one specific authored structure layered on top of events
- **Victory conditions** are Scenario-level win rules
- **Defeat conditions** are Scenario-level loss rules
- **Guidance** is an event-driven directional layer, separate from formal victory/defeat logic

### Events
Events should be able to:
- react to time
- react to node entry
- react to team state
- react to story flags
- react to quest completion
- reshape the world immediately through typed event actions

Events are the main tool for:
- unlocking Regions
- changing alliances
- spawning/removing teams
- changing ownership
- destroying/restoring Services
- triggering victories or defeats
- updating guidance
- delivering authored scenario beats

### Quest services
Quest services are optional by default unless a victory condition depends on them.

A quest service should:
- expose at most one currently available quest from its chain
- resolve through return/turn-in
- use starting, progress, and completion messages
- allow authored competition between teams
- optionally block, persist, disappear, or repeat in special guard/toll cases

Quest services are a concrete gameplay structure, not the universal model for all progression.

### Victory and defeat
A Scenario may have:
- one or more victory conditions
- one or more defeat conditions

Only one victory condition needs to be satisfied to win.  
Any defeat condition becoming true causes loss.

Victory and defeat should be formal scenario rules, not merely implicit consequences of quest completion.

### Guidance
Guidance text should:
- point the player toward intended goals
- update through events
- remain visible until replaced
- be hidable
- not be required for the real victory logic to function

An intended guidance chain may be bypassed if the true victory condition is satisfied directly.

### Campaign transitions
Campaign carry-over should be authored per transition from an explicit allowed list.

This allows different Scenario transitions to preserve different subsets of:
- heroes
- troops
- resources
- items
- equipment/artifacts
- story flags
- hero growth state

That flexibility is important to scenario authorship.

---

## 11. Economy, resources, items, and equipment

The economy layer should support both:
- strategic world pressure
- authored Scenario identity

### Resources
Resources are team-global within a Scenario.

The intended baseline resource set is:

- Gold
- Wood
- Stone
- Steel
- Fiber
- Clay
- Gems

Gold is still a normal resource structurally, but it functions as the main universal currency and is typically gathered at a much larger scale than the others.

Resources should matter for:
- recruitment
- service restoration
- building Location services
- quest and event requirements
- victory / defeat conditions
- trade and diplomacy
- campaign carry-over where allowed

### Owned resource services
Mines are the baseline example of an **owned resource service**.

They should:
- pay out automatically each day to the owning team
- be capturable
- be optionally guarded
- be defensible by stationed units
- create real economic pressure on the Region layer

The broader term “owned resource service” should remain available for future expansion beyond mines.

### Trader family
The intended economy model should use several distinct trading/service types rather than one generic merchant.

Examples include:
- **Trading Post** for resource conversion and resource transfer between teams
- **Market** for item buying and selling with gold
- **Freelancer’s Guild** for selling generic units for gold
- **Black Market** for artifact sales

These should feel like authored world institutions, not one flat abstract shop.

### Trade philosophy
Trade should be intentionally lossy by default.

The game should support:
- a gold-based trade model
- a separate default resource-for-resource barter model
- service-specific overrides by authored design

That gives designers control over:
- harsh or generous economies
- regional scarcity
- faction pressure
- scenario-specific market behavior

### Shared team inventory
The team should have shared inventories rather than per-unit general backpacks.

The intended broad split is:
- a shared **item inventory**
- a shared **artifact inventory**
- per-hero **equipped artifact slots**

### Items
General items include:
- consumables
- stackable utility / trigger items
- seeds
- ingredients
- food

Items may support:
- battle use
- field use
- crafting / cooking
- event conditions
- quest requirements
- economy interactions

### Artifacts
Artifacts are the equippable gear layer.

There is no separate general “equipment” category beyond artifacts.

Heroes equip artifacts into:
- 1 Attack slot
- 1 Defense slot
- 3 Misc slots

Artifacts should:
- modify stats or rules while equipped
- support Scenario identity
- participate in carry-over when allowed
- sometimes matter for quests, events, or victory conditions

### Artifact combination
Artifacts may be combined into stronger artifacts through a dedicated service.

This should be:
- irreversible
- content-driven
- economically meaningful
- a way to create long-term artifact progression without needing a separate crafting tree

### Battle spoils and loss
Economy should connect directly to battle consequence.

Winning a team-vs-team battle should feel economically meaningful through loot, resource transfer, and item/artifact capture.

Escape and surrender should exist as alternatives to total defeat, with their own consequences.

### Scenario-defining objects
Some items or artifacts should be central to Scenario identity:
- legendary relics
- key plot objects
- quest-critical tools
- economic prizes
- strategic objectives

The economy layer should therefore support both ordinary logistics and authored story importance.

---

## 12. Location-built services, farming, cooking, and artifact handling

These systems should reinforce a key structural distinction in Ashvale:

- **Region-layer services** are usually explicit placed services on the strategic map
- **Location-layer services** are often **event-driven** and may feel more authored, characterful, or scene-based

That distinction is important to preserve.

### Location-built services
Locations should not require one universal hard-wired construction subsystem.

Instead, Location construction is primarily an **event-driven world-state process**.

A Location interaction may:
- build a new service
- restore a ruined or inactive service
- upgrade an existing service
- reveal or enable a service that was previously unavailable

The trigger may be:
- an NPC
- a magical object
- a workbench
- a counter
- a shrine
- a field plot
- or any other authored interactable

This makes Location development feel more like authored place evolution than like placing fixed strategic-map objects.

### Human-only Location interaction
AI teams do not enter Locations.

That means:
- Location building
- Location restoration
- Location upgrades
- Location-only service interactions

are effectively **human-team systems**, even though their outcomes still become part of shared Scenario world-state.

### Persistence of Location changes
When a Location event builds, restores, or upgrades something, that result should:

- persist in the current Scenario
- be part of shared world-state
- save and load like other meaningful world-state changes

This keeps Locations feeling like real places that evolve.

### Farming
Farming should exist in two structurally different but connected forms:

- **Region farming services**
- **Location farming triggered through events**

#### Region farming
Region farming should feel like a risky strategic process.

A Region farming service:
- holds one active seed type at a time
- may process a large authored quantity of seeds
- may optionally use fertilizer
- may be watered/cared for over time
- may be guarded by stationed units
- may be contested or stolen if left exposed

This makes farming on the Region layer feel like an economic gamble rather than a private menu action.

#### Location farming
In Locations, farming should usually feel safer and more authored.

It may be triggered through:
- a pot
- a greenhouse
- a garden bed
- a field plot
- some other authored world object

Location farming does not need to look like the same hard-wired service placement model used in Regions, even if it calls the same underlying farming flow.

#### Deterministic crop identity
Farming should be deterministic enough to plan around.

The player should understand:
- what seed is being planted
- whether fertilizer is being used
- what the expected outcome is
- how care/watering changes timing

This is a logistics and planning layer, not a hidden-randomness minigame.

### Cooking
Cooking should be a **party-level convenience system**, not a map-service dependency.

The intended feel is:
- open the party menu
- inspect available recipes
- cook directly if ingredients and any required passive skills are present

This keeps cooking integrated with travel logistics rather than tying it to specific world infrastructure.

Recipes should be globally visible, while still allowing:
- filtering by what is currently possible
- authored passive-skill gates for specific foods

### Food
Food should act like a hero-facing field resource.

Food is intended to support:
- recovery
- next-battle preparation
- day/week buff planning
- longer logistical preparation between fights

This helps the game distinguish:
- battle consumables
- field logistics
- hero preparation

Food should remain a field-use layer, not a battle-item layer.

### Artifact handling
Artifact handling should remain its own distinct service-based system.

Artifacts are:
- the hero equipment layer
- a source of long-term progression and identity
- sometimes scenario-defining objects

Artifact combination should therefore feel like:
- a meaningful authored service
- a deliberate upgrade path
- an irreversible commitment

Artifact handling should exist:
- as a direct Region service where appropriate
- or as a Location event/service call where that better fits the place

### Limited crafting scope
The game should **not** explode into a broad crafting sandbox by default.

The intended crafting scope remains deliberately narrow:

- **Cooking**
- **Artifact combination**

That keeps the game focused while still allowing meaningful authored depth in economy and preparation.

## 13. Locations, dungeons, and safe anchors

### Locations
Locations are where authored place identity can flourish.

They should justify entry through:
- NPC interaction
- multiple Services
- story scenes
- more textured presentation
- dungeon or settlement identity
- building, restoration, crafting, or economic specialization where relevant

### Dungeons
Dungeons are a type of Location.

They should feel like entered authored spaces, not simply tougher nodes on the Region layer.

### Safe anchors
Not every Region needs a safe haven.

A safe anchor is simply a Location that provides free guaranteed rest.

That keeps the definition sharp and systemic while allowing wide authored variety.

“Home Base” is better understood as a scenario-authored safe-anchor concept, not as a universal mandatory structure.

---

## 14. Fog of war and information

The world should not be fully transparent at all times.

Fog of war is a major part of Region play, enemy-team fairness, and strategic tension.

### Fair knowledge
Both human and AI teams should follow the same knowledge model.

AI teams should not act on hidden information that they have not explored or revealed.

### Reveal model
Fog should follow a HoMM2/HoMM3-like model:

- unrevealed areas are hidden
- revealed areas stay revealed
- revealed information is live, not stale

This keeps map knowledge simple and readable.

### Vision sources
Vision may come from:
- team position
- allied teams
- scouting passives
- stationed guards
- owned services
- special reveal services

The player should gradually build map knowledge through movement, ownership, and scouting.

### Visible information
When enemy teams are visible, the player should get useful but not always complete information.

Baseline visible information should include:
- team color
- leader
- threat indication
- estimated unit quantities
- estimated hero level ranges

High scouting should eventually allow full inspection, including resources, items, and artifacts.

### Hidden events
Unseen enemy movement and AI-vs-AI outcomes should not generate generic messages by default.

Major events may still be announced through authored events.

If a hidden AI team wins the Scenario, the game should end immediately and explain why.

---


## 15. UI / UX vision and information presentation

Ashvale's UI should be a **hybrid** between clean JRPG readability and HoMM-like strategic information density.

The default presentation should be clean, but the player should be able to inspect, hover, select, or open panels to get the detail needed for tactical and strategic decisions.

### Platform and input philosophy
The whole game should be designed to be:

- controller-friendly
- mouse/keyboard-friendly
- touch-friendly where practical

Mouse/keyboard and controller are the primary development targets, but UI layout and interaction patterns should avoid assumptions that would make future smartphone or tablet support impossible.

That means:
- important actions need controller navigation paths
- hover-only information needs select/tap equivalents
- hit targets should not be tiny
- dense panels should remain readable at smaller sizes
- contextual info should be available without requiring precision mouse hover

### Numbers and certainty
The UI should show exact numbers when the information is known.

Use this general rule:

- exact values for the player's own known state
- exact values for fully scouted or fully known targets
- estimated values when scouting or fog limits information
- no hidden formula dump unless it directly helps readability

Examples:
- battle target preview should show exact min/max damage when known
- threat preview should remain a rough color-coded estimate
- visible enemy stacks should show quantity ranges unless scouting is high enough
- turn order should show the resulting order, not internal CTB math

### Tooltips and panels
Tooltips should be significant but non-intrusive.

Use floating tooltips for quick contextual details:
- route time / Energy cost
- target damage / KO preview
- service availability
- illegal action reason

Use stable panels for selected or inspected information:
- selected node details
- battle help text
- item details
- recipe details
- unit information

The player should never need to guess why a major action is unavailable.

### Region UI
Region mode should use a persistent strategic layout:

- **top info bar**
  - day / week / month
  - time
  - Energy
- **bottom resource bar**
  - team resources
- **top-left minimap**
  - always visible in Region mode
- **Adventure button strip**
  - left-side icon menu
- **bottom-left contextual info panel**
  - selected node info when a node is selected
  - otherwise toggles between active-party summary and occupied-node info
- **temporary news / popup area**
  - upper-left content area, under the top bar and to the right of the Adventure button strip

The Adventure button strip should contain icons for common strategic screens, such as:
- player / party menu
- Scenario Info screen
- World Map / Region selection
- Quest Log
- center on current team
- rest
- settings

### Region node selection
Region movement should use a select-then-confirm model.

For mouse and touch:
- first click/tap selects a node and previews the route
- second click/tap confirms movement where legal

For controller:
- D-pad or mapped navigation selects nodes
- confirm executes the selected move

The selected route should show the shortest legal path where one exists.

If blockers make the destination illegal:
- show the legal part of the route
- outline the first blocker in red
- continue the preview in disabled/gray form past the blocker where useful
- outline additional blockers in red if the preview path crosses them

If time or Energy prevents travel:
- show the relevant cost in red in the route tooltip

### Region tokens, ownership, and service state
Enemy teams should be readable as colored army tokens.

A team token should show:
- a clear color outline
- a small banner for team color
- the leader portrait if there is a leader
- otherwise the strongest unit as the representative portrait

Owned nodes and services should use flag icons.

Region service state should be readable at a glance:
- destroyed services darkened with a destroyed icon
- guarded services showing a small strongest-unit portrait
- farming services using light sparkle while growing
- brighter sparkle when watered
- glow when harvest is ready
- restoration / cooldown / availability shown through tooltip or node info when relevant

### World Map UI
The World Map should reuse the same broad HUD frame as Region mode:
- top info bar
- bottom resource bar
- left contextual strip / panel

Only unlocked Regions should be visible. Locked Regions remain hidden in black fog.

Region selection should feel similar to Region node selection:
- select a Region
- inspect tooltip and left-panel details
- confirm travel

World Map travel confirmation should warn only when confirming would lose units.

The confirmation should show:
- travel time
- arrival time
- Energy status after arrival
- generic-unit loss warning when applicable

It should not reveal detailed destination-node information.

### Location UI
Location mode should feel like JRPG walking exploration with sprites.

Locations should not use a minimap or automatic service list by default.

Interactions are event-driven:
- the designer assigns sprites
- the designer chooses trigger behavior
- triggers may happen by confirm button, collision, or other authored logic
- service calls may be direct or hidden behind dialogue, prompts, or event flow

Location-built, restored, or upgraded services should appear through visible event-state changes:
- new sprites
- changed collision
- changed layers
- new NPCs
- changed objects
- replaced map events

### Battle UI
Battle should use a clear JRPG-style layout:

- player party on the right
- enemies on the left
- CTB bar at the top
- help window / panel below the CTB bar
- battle formation in the center
- party status rows at bottom right
- battle command menu to the left of the party status rows

Units are represented as sprites facing each other.

Position is shown visually by distance from the center rather than as a constant text label.

Generic unit count should be shown directly on the unit sprite, at bottom-left.

Leader status should be shown with a small leader icon on the unit sprite, at bottom-right.

When targeting:
- target preview appears in a tooltip with a panel background
- damage and KO / kill preview are shown
- Agility penalty is shown by color-coded cursor / tooltip feedback
- the help panel shows target name, actual position, and status icons
- CTB order updates live before confirmation

The player should see the resulting turn order, not the internal CTB formula.

### Battle result screen
The same result screen should be used for auto-resolve and manual battle.

It should be a centered panel showing:
- top result banner
- result explanation
- row of player losses
- row of enemy losses
- Accept Result button
- Try Again button

Possible result banners include:
- Win
- Loss
- Enemy Escaped
- You Escaped

### Party menu and roster UX
The party menu should support both mouse-driven and controller-friendly management.

The main layout should include:
- large active-party panel
- reserve slots along the bottom
- right-side menu with:
  - Items
  - Artifacts
  - Cooking
  - Position

Mouse interaction should support drag/drop and stack shortcuts.

Controller interaction should support:
- cursor selection
- confirm to pick up / select
- confirm on destination to move or swap
- context menu for stack actions
- context/info button for unit detail panel

Active-party legality warnings should be explicit and blocking.

Temporarily Unavailable heroes should stay hidden until they return.

### Stack management shortcuts
Mouse/keyboard stack shortcuts should be supported for generic units:

- `Ctrl + Left Click`
  - split one unit from the stack to the next available legal slot
- `Alt + Left Click`
  - merge all units of the same type into the clicked stack where legal
- `Shift + Left Click`
  - split all units of the same type as evenly as possible across available legal slots in the same active container
- `Ctrl + Shift + Left Click`
  - fill available legal empty slots with 1-unit stacks while units remain

These shortcuts apply only within the active container of the selected unit:
- active party
- reserve
- or the currently opened storage

If no legal move exists, the shortcut cancels.

### Items, artifacts, and cooking
Item inventory should be a flat list with:
- icon
- name
- quantity

Item sorting should support:
- manual
- auto by name
- auto by type
- auto by least
- auto by most

Artifact inventory should support similar sorting.

Artifacts are equipped through a hero-specific artifact / equipment interface.

Cooking should show:
- available recipes by default
- a Show All Recipes toggle
- required ingredients with icons and counts
- missing quantities marked red inline
- required passive skills
- time cost
- resulting food effects
- quantity selector

### Artifact combination UI
Artifact combination should resemble cooking:
- show available authored combinations
- show input artifacts
- show output preview
- block combinations that are illegal
- block artifacts required by victory conditions

Artifact combination does not require an additional irreversible-warning prompt.

However, the UI must make the input artifacts and output artifact clear before activation.

### Service UI
Services do not need one universal shared screen pattern.

However, any service must clearly show applicable:
- title
- cost
- time cost
- Energy cost
- item / resource cost
- availability reason
- guards
- threat estimate

Immediate-effect services may show temporary feedback above the team.

Simple confirmation services may show a prompt and then the same temporary effect feedback.

Risky actions should require confirmation when appropriate, including:
- Region travel that loses generic units
- surrender
- escape
- discarding artifacts
- destroying services

### Quest Log, Scenario Info, and guidance
Use distinct terms and surfaces:

- **Adventure button strip**
  - left-side icon menu in Region / World Map UI
- **Scenario Info screen**
  - formal victory and defeat conditions
- **Quest Log**
  - discovered quest-service tasks
- **Guidance**
  - event-driven optional direction text

Quest Log behavior:
- entries are buttons
- primary action centers the camera on the quest-service location
- context / info action shows requirements
- controller and touch must have equivalents to left-click / right-click behavior

Guidance should appear:
- under the top info bar
- to the right of the Adventure button strip
- with optional icon + text
- collapsible
- also recorded in the journal / message history

Guidance may point to hidden or unrevealed content if authored to do so.

### Fog, scouting, and enemy inspection
Fog of war should appear as black fog with a smooth animated edge.

No silhouettes should be shown for unrevealed nodes.

Nodes should appear only once revealed.

Scouting is a passive skill with three levels:
- Basic
- Advanced
- Expert

The game does not need to directly announce scouting effects. Players can infer them from reveal range and inspection detail.

Enemy inspection should scale by scouting:
- default: color, leader, threat color, unit icons, names, quantity ranges, hero level ranges
- Basic: improved reveal / better confidence
- Advanced: enemy resources and better estimates
- Expert: exact stack quantities, exact hero levels, leader stats, items, and artifacts

Threat color should appear on:
- target cursor
- hover / inspection panel outline

### Notifications and logs
Temporary notifications should appear near the upper-left content area and hide after a short time.

An expandable news area should show the latest four news items when expanded.

Clicking a news item or log control should open the full history log.

The full log should record:
- message text
- day / week / month
- guidance changes
- relevant visible events

Popup-worthy events include:
- quest completed
- enemy team received a service buff
- hero becomes Temporarily Unavailable
- service restored or destroyed
- resource received from another team
- mine lost
- team defeated
- Region unlocked

AI and human movement / animation speed should be configurable from very slow to instant.

## 16. Scenario identity and authored control

Even though the game has strong systemic behavior, Regions and Scenarios should still feel authored.

A designer should be able to shape:
- world structure
- team setup
- alliances
- unlock progression
- available heroes
- service distribution
- ownership opportunities
- sabotage pressure
- safe anchors
- story triggers
- branching outcomes
- economic scarcity or abundance
- artifact and item identity
- victory and defeat routes

The systems should create interaction, but the Scenario should still feel intentional.

---


### Player character
The player character is a normal unique hero unit with special human-team rules.

The player creates the player character before a Scenario or Campaign starts, using a stable hero identity such as `hero_player`.

Character creation chooses:
- name
- sex
- simple graphical representation
- starting stats
- starting skills
- starting preset/template

The player character:
- belongs to the human team
- must stay in the traveling party
- may be active or reserve
- cannot be stored, dismissed, recruited, sealed/frozen, Temporarily Unavailable, neutral, or AI-owned
- must be leader-capable
- does not have to be current leader
- always carries over as identity in Campaigns

If the player character is KO at battle end, they recover to 1 HP.

Player-character KO is not automatic Scenario defeat unless recovery is impossible or a Scenario-specific defeat condition says so.


## 17. Scenario authoring, content data, and validation

Ashvale should be heavily data-driven, but not free-form-script driven.

The long-term authoring model should let designers edit:

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
- victory and defeat conditions
- resource amounts, costs, pickups, payouts, and economy settings
- units
- items
- artifacts
- recipes

Code should define the supported system types, formulas, runtime rules, validation rules, and legal behavior boundaries.
The resource enum itself remains code/schema-defined. Designers configure how resources are used, awarded, spent, traded, and produced.

### Designer tool vision
A designer tool should eventually support editing the major authored layers of the game.

That includes:
- Region and Location editing
- service placement and settings
- event pipes
- quest services
- victory / defeat conditions
- team setup
- resource usage and economy settings, items, artifacts, units, and recipes
- validation reports
- test-play entry points

Content should also remain reasonably hand-editable where practical.

### Data-driven but typed
The game should avoid letting content invent arbitrary new mechanics.

Instead, content should choose from explicit typed systems:
- event triggers
- eligibility checks
- conditions
- event actions
- service types
- AI personalities
- aggression levels
- route terrain types
- resource types
- item and artifact definitions

This gives designers expressive power while keeping validation and runtime behavior predictable.

### Validation philosophy
Validation should be strict enough that invalid content cannot be played.

But validation should not prevent designers from saving unfinished work.

A map or Scenario may be saved while invalid, but it should not be marked playable until validation passes.

### Harsh content versus invalid content
The game should allow harsh, unfair, or obscure authored Scenarios if they are structurally valid.

Validation should catch broken content, not forbid difficult design.

For example:
- no safe anchor can be legal
- brutal enemies can be legal
- hidden or obscure objectives can be legal
- missing required references should be invalid
- impossible mandatory victory conditions should be invalid

### Node authoring model
Nodes should be treated primarily as travel points with authored content.

A node's behavior comes from:
- node content
- attached events
- Services
- neutral encounters
- pickups
- blockers created by content

A blocker is usually behavior created by content, not an intrinsic node type.

### Events as typed authoring logic
Events should use typed structures:
- trigger
- eligibility
- conditions
- branches
- action chains
- repeatability
- priority

Events may support nested If / Else branching, but they should not become arbitrary free-form scripts.

### Runtime action failure
Event actions should not fail during normal intended play.

Designers should use eligibility, conditions, and branches to avoid invalid actions.

If an event action still fails at runtime:
- previous successful actions are not rolled back
- the chain follows non-atomic ordered execution
- the player should receive a clear popup/log message when reasonable
- debug logs should make the authoring problem clear

### Best-effort softlock validation
Detailed validation rules live in `docs/validation_system.md`.

Validation should include best-effort graph and softlock analysis.

It should try to catch:
- unreachable required objects
- invalid arrival nodes
- broken Region paths
- impossible victory targets
- missing required heroes or items
- required routes or Services that cannot exist

It does not need to mathematically prove that every Scenario is beatable.

## 18. Implementation posture

The intended long-term direction is:

- authored world structure
- reusable systemic rules
- readable tactical feedback
- low ambiguity in terminology
- content-driven expansion

The source may still contain some legacy names or intermediate runtime structures. Those should not override the current design language.

Use:
- **World Map**
- **Region**
- **Location**
- **Service**
- **Traveling party**
- **Stored units**
- **Temporarily Unavailable**

as the current design terminology.

PvP may exist later as a separate mode, but it should not currently drive the main single-player vision.


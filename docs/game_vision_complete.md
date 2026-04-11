# Complete Game Vision

This document is the current north-star description for the **finished direction** of Project Ashvale.

It is intentionally more concrete than `docs/game_vision.md`, but it is still a design guide rather than a promise that every feature is near-term. Active milestone docs and the current codebase remain the source of truth for immediate implementation work.

## Core player fantasy

The player should feel like they are:
- surviving a broken world one day at a time
- making meaningful route, risk, and time-management choices
- gathering survivors, allies, items, and story fragments
- building a capable traveling party while deciding what to carry forward and what to leave behind
- turning damaged or reclaimed places into real safe havens
- growing from fragile routine into confident regional recovery

The game should combine:
- scenario-level world-map planning
- node-based regional travel
- walkable location exploration
- readable, tactical CTB battles
- service and economy planning across days and weeks
- story and restoration anchored around meaningful places of safety

## Tone

Ashvale should feel:
- melancholic
- mysterious
- intimate rather than epic
- safe and cozy in reclaimed spaces
- uneasy and dystopian in occupied or damaged spaces

It is not grimdark. Hope, routine, and rebuilding are part of the fantasy.

## Terminology and hierarchy

Project Ashvale uses this structure:

- A **campaign** is a collection of connected scenarios. It may be linear or branching, and may or may not tell one continuous story.
- A **scenario** is the top-level authored playable unit.
- Each **scenario** contains exactly one **World Map**.
- A **World Map** contains one or more **Regions**.
- A **Region** is the main travel space inside a scenario.
- A **Region** contains authored **nodes** connected by legal travel routes.
- A **Location** is an enterable place reached from a region node.
- A **Service** is a functional interaction available either directly in a region or inside a location.

For practical use in the project:
- use **Region** as the main term for the regional travel layer
- treat **overworld** as older vocabulary for the same concept
- use **World Map** for the higher-level scenario map above regions
- use **region select** for the interaction mode used on the World Map

The intended hierarchy is:

**Campaign -> Scenario -> World Map -> Region -> Node -> Location**

## Terminology reference

For the canonical meanings of current project terms, see `docs/terminology_map.md`.

That document defines the intended design terminology used in this vision document and also explains how older runtime or content names should be interpreted when they differ from the current design language.


### Campaign

A campaign is a collection of scenarios that may branch or reconnect.

In a campaign:
- the player wins by completing a connected path through 2 or more scenarios to a campaign ending
- scenario transitions may branch based on victory condition, player choice, or authored rules
- some transitions may offer multiple follow-up scenarios

A campaign does not have to be one tightly continuous story. The important part is that its scenarios are connected and that the campaign itself has a winnable structure.

### Scenario

A scenario is the top-level authored play unit.

A single scenario should feel like a complete playable experience even when it is part of a larger campaign.

A scenario may define:
- its regions and world-map structure
- its hero pool and other recruitment rules
- its services, enemies, quests, and victory conditions
- what, if anything, carries over from a previous scenario
- authored story/state choices that affect follow-up scenarios

### Campaign carry-over

Carry-over between scenarios is content-defined per campaign or scenario transition.

Depending on the scenario rules, carry-over may include:
- heroes
- items and equipment
- generic troops
- gold
- selected story flags
- no gameplay carry-over except story continuity

Only explicitly legal entities should carry over. Even when gameplay state resets between scenarios, remembered story choices should still persist when the campaign uses them.

## The big gameplay loops

### Minute-to-minute

The player:
- travels to a destination
- enters a location, service, or battle
- spends time, gold, party durability, or other resources
- gains information, items, recruits, or route access

### Day-to-day

The player:
- plans where to travel before time cutoffs matter
- chooses when to fight, rest, shop, recruit, store units, or return to safety
- balances risk against recovery and progress
- deals with the consequences of attrition, defeat, and hero unavailability

### Week-to-week

The player:
- plans around weekly service refresh and hero-pool return timing
- decides how to spend limited gold and recruit opportunities
- gradually unlocks better routes, safer regions, and stronger locations
- feels that the world and the player's safe anchors are becoming more stable

## World Map vision

Each scenario contains exactly one **World Map**.

The World Map is the top-level strategic layer for the current scenario.
Its purpose is to organize the scenario into multiple authored **Regions** and let the player understand the broader structure of the scenario.

The World Map:
- contains the scenario's starting region
- contains other regions that may become available over time
- acts as the region-selection layer for cross-region travel
- acts as an information screen for scenario-level planning

The player is not physically “inside” the World Map the same way they are inside a region or location.
The World Map is a strategic selection and information layer above region play.

### World Map rules

The World Map can be opened at any time **outside battle**, as long as the player is inside a scenario.

The player can use it:
- for information
- for planning
- for cross-region travel when travel is legal

Travel action is disabled when travel is not legal.

Regions:
- unlock gradually through authored scenario logic
- may also become temporarily enterable or non-enterable through flags and world-state changes
- do not use a built-in one-time-only travel rule, but scenario logic may emulate that behavior by changing region availability flags

### Region travel rules

Cross-region travel follows these rules:

- region travel is initiated from the **region layer**, not from inside a location
- if the player is inside a location, they must first return to the region layer
- if the player is inside a dungeon, region travel is not allowed
- region travel must begin before **11:00**
- if the current time is **11:00 or later**, travel is disabled until the next day
- travel costs **time only**
- region travel does not consume gold or other generic resources
- arrival always happens at **11:00** on the arrival day

Distance uses the **shortest valid path** through enterable adjacent regions.

Practical rule:
- directly adjacent regions cost **1 day**
- if there are 2 regions between the current region and the destination, travel time is **3 days**
- all region steps count equally for travel time

Travel is only possible when a valid contiguous path exists through currently enterable regions.

### Region arrival

Each region must define an **arrival node**.

Arrival-node rules:
- the arrival node is authored by the scenario designer
- it may be any legal node type except an enemy-occupied node
- enemies may not spawn on it
- enemies may not travel onto it

This guarantees that cross-region travel always resolves into a safe, predictable entry point.

### Region presentation

Selecting a region on the World Map should:
- highlight that region clearly by its borders
- show an informative visual panel for the region
- display useful information such as:
  - difficulty
  - terrain
  - expected resources
  - regional identity
  - story or quest relevance
  - size or scale
  - other scenario-defined notes

The World Map should feel like a strategic planning view, not just a menu list.

## Party, roster, and storage vision

### Active party, reserve, and traveling party

Outside battle, **party** should mean the **active party**.

The active party:
- is the current battle-legal fielded group
- has a maximum size of **5**
- must always remain battle-legal
- must always contain exactly one assigned leader

The reserve:
- travels with the player
- has a maximum size of **7**
- can be switched into the active party from the global party-management menu
- does not directly field in battle unless moved into the active party first

The **traveling party** means:

**active party + reserve**

So the maximum traveling-party size is **12**.

### Stored units

Stored units are different from reserve units.

Stored units:
- do not travel with the player
- are assigned to a specific **storage service**
- remain at that storage service until moved again
- can include **heroes or generic stacks**
- use a separate slot cap from the active party and reserve

Each storage service:
- is neutral and can store any unit type
- has **7 slots**
- persists in its parent region
- is independent of other storage services, even within the same region

A region may contain multiple storage services.

### Global party-management rules

There should be a broader out-of-battle party-management menu available outside battle.

That menu should allow the player to:
- inspect units
- choose the active party
- assign the leader
- change active-party positions
- equip items
- consume items
- heal units through item use or other legal actions
- disband generic units
- dismiss heroes when legal

What it should **not** do:
- move units into or out of storage from anywhere

Storage transfer must happen at the storage service itself.

### Position and leadership outside battle

Active-party position is persistent.

Rules:
- battle starts with the active party's current positions
- position changes made in battle persist afterward
- the `Leader` position is persistent like other active-party positions
- reserve units do not keep an active battle position while in reserve
- when a reserve unit is moved into the active party, it normally receives the game's best default position for that unit
- if a reserve or stored unit directly replaces an active unit, it inherits the replaced unit's position
- the leader cannot be removed from the active party unless directly replaced by another legal leader-capable hero

The player character is the default fallback leader, but the player may manually assign another active hero as leader outside battle.

If the current leader becomes temporarily unavailable:
- the game automatically assigns a legal fallback leader
- this fallback is temporary until the player changes it manually

### Generic stacks

Generic units use stack behavior outside battle as well as in battle.

Rules:
- generic stacks of the same type merge automatically on recruitment when possible
- the player may manually split or merge same-type generic stacks outside battle
- partial stacks may be stored
- splitting and merging require legal slot usage

The intended feel is closer to Heroes of Might and Magic stack management than to individual generic-unit micromanagement.

### Heroes

Heroes are scenario-governed units, not generic abstract hires.

Rules:
- heroes come from a scenario-defined pool, with a default pool available when the scenario designer does not fully specify one
- a scenario designer may also author specific custom heroes
- a hero may only appear in one recruit service at a time
- a hero who is already traveling, stored, or temporarily unavailable is not eligible for recruit rerolls
- a hero may be dismissed when legal, but that does not remove them permanently from the scenario

### Temporarily Unavailable heroes

The internal design term for heroes who are not currently usable should be **Temporarily Unavailable**.

A temporarily unavailable hero:
- is removed from the active party
- is removed from reserve
- is removed from storage
- is not recruitable
- does not consume upkeep
- returns to the scenario hero pool at the **start of the next week**

This state can happen because:
- the hero was still KO'd when a battle ended
- the hero was voluntarily dismissed
- another authored rule temporarily removes the hero

The player-facing flavor text can vary by context, but the underlying gameplay state is the same.

### Recruitment rules

Recruitment is legal if at least one of the following is true:
- there is a free active-party slot
- there is a free reserve slot
- there is an existing same-type generic stack in the active party or reserve that can receive the recruited units

Recruit destination rules:
- if there is a free active-party slot, the recruited unit goes to the first free active slot
- otherwise, if there is a free reserve slot, the recruited unit goes to the first free reserve slot
- otherwise, recruitment is illegal and nothing happens beyond player feedback

Hero recruitment:
- happens one hero at a time
- allows the player to recruit one, many, or none from the current service list, as long as each pick is legal and affordable

### Cross-region party state

Cross-region travel does **not** preserve all traveling units equally.

Rules:
- all heroes in the traveling party cross regions with the player
- generic units in the traveling party do **not**
- generic traveling units are lost on region change unless the player stored them beforehand
- the player must be warned clearly before confirming region travel that would discard generic traveling units
- stored units remain in their original region and persist there

When the player leaves a region, that region still remembers its persistent state, including:
- stored units
- recruit offerings
- enemy attrition and enemy-party persistence
- cleared or changed nodes
- service state
- other authored region-local progression

## Home base vision

A “Home Base” is a kind of **Location** that functions as a meaningful safe anchor.

It is not required that every region contain one.
Some scenarios may contain strong safe-haven locations; others may intentionally deny the player a true home base.

A Home Base may provide:
- free rest
- story progression and survivor conversations
- restoration milestones
- baseline services
- hero recovery support
- storage
- a strong emotional sense of safety and return

Home Base is therefore a design role, not merely a fixed building type or guaranteed scenario feature.

## Region vision

Regions should remain node-and-route based rather than free-roaming.

A region is a travel map within a scenario's World Map.

Key principles:
- nodes represent authored destinations with gameplay meaning
- routes define the legal travel graph inside a region
- route access can change because of combat clears, blockers, quests, or future progression conditions
- the player should understand why a path is open, blocked, distant, or unsafe
- travel should feel like planning, not busywork

Nodes can represent:
- enterable locations
- inns
- recruit services
- storage services
- shops and service points
- combat encounters
- dungeons
- single-use bonuses or resources
- blocker nodes guarding deeper routes

## Location vision

Locations should feel like authored spaces with identity, not just generic scene shells.

A Location:
- is entered from a parent region
- may contain zero or more services
- may contain NPCs, story scenes, and quest interaction
- may use one or many walkable screens

Locations may contain:
- service interactions
- NPC dialogue and memory fragments
- restoration context
- scene transitions
- local time costs
- safe interactions, risky interactions, or story interactions

Services inside a location are performed through interaction with NPCs or fitting world objects.

Shared prototype scenes are acceptable during slice development, but the finished game should support strong location identity through content.

## Battle vision

Battles should remain true turn-based CTB, highly readable, and tactically deterministic.

The battle layer should feel closer to a classic Final Fantasy-style formation battle than to a grid tactics game:
- battle is static rather than free-movement
- units act from formation positions instead of moving on a battlefield grid
- target selection is free, but target distance within the formation affects turn-order pressure through agility penalties
- the player should understand the tactical consequences of an action before committing it

The battle module should support:
- clear turn-order communication
- meaningful turn-frequency manipulation
- stack-based generic units plus hero units
- strong role identity for units, heroes, and leaders
- readable consequences for defeat, KO, temporary unavailability, and attrition
- enough tactical depth to make route blockers and high-risk destinations feel significant

Battle should stay deterministic apart from intentional damage variance and should remain testable independently from rendering.

### Formation and targeting

A battle party can field up to 5 units.

Battle formation uses four position labels:
- `Front`
- `Middle`
- `Back`
- `Leader`

The `Leader` position is one of the 5 battle slots, not an extra slot.

Formation rules:
- only hero units can occupy the `Leader` position
- multiple units may share `Front`, `Middle`, or `Back`
- units do not move freely during battle
- changing position is a deliberate battle command that consumes the acting unit's turn
- only hero units may switch into the `Leader` position
- taking the `Leader` position swaps with the current leader rather than creating a second leader

Targeting rules:
- any unit may target any enemy unit
- position does not directly block targeting or change damage by itself
- position matters because the target's **effective row depth** influences agility penalty on attacks and similar actions
- literal position labels remain visible to the player, but battle calculations use effective row depth rather than naïvely collapsing every gap in the formation

### Turn order, commands, and readability

Turn order should behave like a readable CTB timeline:
- the player should always understand who is acting next
- the player should see how a chosen action changes the future turn order before confirming it
- the game should prefer readable outcome previews over exposing raw internal timing math

Battle commands should follow a classic JRPG structure:
- baseline actions include `Attack`, `Defend`, and context-appropriate access to `Skill` and `Item`
- only hero units can use items in battle
- generic units may surface 0-2 direct skills on the main command layer when appropriate
- `Wait` and manual position-change commands belong to a secondary command layer rather than the main baseline menu

Readability expectations:
- show turn-order impact before action confirmation
- show min-max damage on selected targets
- show min-max KO / kill outcome when relevant
- make agility-penalty pressure understandable through clear UI cues rather than opaque math dumps

### Leadership and party legality

The player team's active battle party must always contain a legal leader-capable unit.
In practice, that means a hero unit must be available to hold the `Leader` position.

Leadership rules:
- the player character is the default leader whenever present in the active party
- the player may assign a different hero as leader outside battle
- enemy teams may or may not have a leader
- when a team has a leader, the same leader-position and aura mechanics apply to both player and enemy teams
- the assigned leader is a normal battle participant from turn 1
- if the assigned leader falls in battle, the aura is removed immediately
- a living hero can manually take the `Leader` position later in the same battle to restore the aura immediately
- leader reassignment is never automatic

Leader aura is a baseline rule of the battle system rather than a temporary prototype.
Passive skills and equipment may later extend, modify, or react to that baseline aura behavior.

### Recovery, attrition, and persistence

Battle outcomes should matter beyond the immediate encounter.

The intended persistence model is asymmetric:
- hero HP persists between battles unless restored by other means
- generic units restore HP after battle, but stack-count loss persists
- MP persists for all units
- temporary battle-only buffs and debuffs are cleared after battle
- some overworld-applied buffs may last for 1 battle, 1 day, or 1 week depending on their source

KO and recovery rules:
- a KO'd hero can be revived during battle
- if a non-player hero is still KO'd when a battle ends, that hero becomes temporarily unavailable
- the player character is a special case and returns at 1 HP after a winning battle if still KO'd
- generic unit loss that remains at battle end is permanent unless the units were revived before the battle ended

Enemy parties should follow the same broad persistence logic as the player side.
This allows recurring enemy groups, persistent attrition, and authored enemy recovery behavior between days or after using world services.

### Leadership and party legality

- the active battle party can field up to 5 units
- the player team's active party must always contain a legal leader-capable unit
- the player character is the default leader whenever present in the active party
- enemy teams may or may not have a leader
- when a team has a leader, the same leader-position and aura mechanics apply to both player and enemy teams
- the assigned leader is a normal battle participant from turn 1
- if the assigned leader falls in battle, the aura is removed immediately
- non-player heroes can leave the party after battle if still KO'd
- the player character is a special case and returns at 1 HP after a winning battle

## Service and economy vision

Services should be meaningful because of **cost, stock, quantity, refresh cadence, and world context**, not because they are arbitrarily disabled.

A **Service** is a functional interaction point.
It may exist directly in a region or inside a location.

### Inns and recovery services

- inns are generally usable if present
- resting at inns costs gold
- some safe-anchor locations may provide free or discounted recovery
- a location may offer hero-restoration support, but that is authored service behavior, not a universal Home Base guarantee

### Recruitment services

- generic-unit recruit services offer 1 or more recruitable unit types
- each offer has a quantity limit
- recruit quantities can replenish on a weekly cadence
- hero-recruit services pull from the scenario hero pool
- when the player enters a region, hero offerings for that region reroll from all heroes who are not traveling, stored, or temporarily unavailable
- the same available hero may appear in only one service at a time within the region
- stronger or rarer recruit points can become major regional objectives

### Storage services

- storage services hold a region-persistent 7-slot stored-unit container
- interacting with a storage service should open a broader party/storage management view
- that view should make it easy to move units between active party, reserve, and storage while respecting legality rules

### Shops and service points

- shops should be content-driven and have identity
- stock may be finite, simple, or refreshed on a cadence depending on the service
- the important distinction is what they offer and what they cost, not whether they are globally enabled or disabled

## Quest and story vision

Quests should remain understandable and content-driven.

Longer-term, quests may expand beyond the current minimal slice, but the game should still avoid a bloated scripting-first design.

Desired qualities:
- typed objectives and events where possible
- quests that react to meaningful world changes
- story delivered through places, survivors, and restoration progress
- clear short-term goals tied to the current region and its meaningful safe anchors
- scenario-authored hero, location, and service behavior when that strengthens identity

## Scenario success and failure vision

Scenario victory and failure conditions are content-defined.

By default, scenarios are expected to use a forgiving in-game time limit, but scenarios may also define more specific loss conditions such as:
- failing to complete objectives before a certain day
- losing a required hero
- allowing an enemy hero or force to reach a critical destination
- failing to protect a key town, survivor group, or route

This part of the design is still flexible and may evolve, but the intended direction is:
- victory and failure should be scenario-authored
- time pressure should usually be meaningful without becoming oppressive
- campaign progression should still preserve story continuity and remembered player choices

## Content and editor vision

The game should remain strongly content-driven.

Longer-term, designers should be able to create, update, and delete content through dedicated tooling rather than hand-editing code.

This implies:
- stable ids
- explicit content schemas
- authored content kept separate from runtime mutable state
- gameplay systems that consume typed content definitions cleanly
- minimal hidden logic baked only into rendering or input code

The future editor should be able to reason about:
- campaigns and scenario chains
- World Maps and Regions
- region nodes and links
- locations and scenes
- service definitions
- storage and recruitment services
- hero pools and hero availability rules
- quest content
- battle scenarios and balance values
- scenario victory and failure conditions

## Technical guardrails

Even as scope expands, these constraints remain important:
- separation of concerns stays strong
- input logic does not belong in rendering code
- gameplay rules do not belong in renderer code
- the game should stay responsive and performant
- ownership should remain explicit and leak-resistant
- avoid blocking or heavy repeated work in the main loop
- prefer testable pure logic for rules and simulation
- content loading and runtime state should remain cleanly separated
- future editor support should influence schema design, not force gameplay code into editor-shaped architecture

## What the game is not trying to be

Project Ashvale is not aiming to be:
- an open-world action game
- a giant simulation sandbox
- an ECS-driven tech demo
- a network-first multiplayer game
- a content-unbounded RPG at this stage

The goal is a focused, maintainable strategy/RPG hybrid with strong place identity, service planning, party/logistics consequence, restoration fantasy, and content-driven authored progression.

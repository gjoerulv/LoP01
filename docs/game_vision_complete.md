# Complete Game Vision

This document is the current north-star description for the **finished direction** of Project Ashvale.

It is intentionally more concrete than `docs/game_vision.md`, but it is still a design guide rather than a promise that every feature is near-term. Active milestone docs and the current codebase remain the source of truth for immediate implementation work.

## Core player fantasy

The player should feel like they are:
- surviving a broken world one day at a time
- rebuilding a forgotten town into a real safe haven
- making meaningful route, risk, and time-management choices
- gathering survivors, allies, resources, and story fragments
- growing from fragile routine into confident regional recovery

The game should combine:
- world-level scenario and region planning
- destination-based overworld travel
- walkable location exploration
- readable, tactical CTB battles
- service and economy planning across days and weeks
- story and restoration anchored around a meaningful home base

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

- A **campaign** is a chain or branching graph of scenarios with story progression.
- A **scenario** is one self-contained playable chapter or map.
- Each **scenario** contains exactly one **world map**.
- A **world map** contains one or more **overworlds**.
- An **overworld** is also called a **region** in gameplay language.
- An **overworld/region** contains authored **nodes** connected by legal travel routes.
- A **location** is an enterable place reached from an overworld node.

For practical use in the project:
- **overworld** and **region** refer to the same gameplay layer
- **world map** is the higher-level scenario map above all overworlds
- **region select** is the interaction mode used on the world map to choose which overworld/region to travel to

The intended hierarchy is:

**Campaign → Scenario → World Map → Overworld/Region → Node → Location**

## Initial selection

At the start of play, the player chooses either:
- a **single scenario**
- a **campaign**

### Single scenario

A single scenario is one self-contained playable map, similar in scope to playing a single map in Heroes of Might and Magic II or III.

A single scenario contains:
- exactly one **world map**
- one or more **overworlds/regions**
- nodes, locations, services, battles, and objectives within that scenario

The player wins a single scenario by completing that scenario’s defined objectives.

### Campaign

A campaign is a structured sequence or branching graph of scenarios.

In a campaign:
- each scenario leads to one or more follow-up scenarios
- some transitions may branch based on story progression, choices, or scenario outcome
- the player progresses through multiple scenarios until reaching a full campaign ending

The player wins a campaign by completing a full path through the campaign to its conclusion.

### Campaign carry-over

Carry-over between scenarios is content-defined per campaign or scenario transition.

Depending on the scenario rules, carry-over may include:
- the full party
- only the player character
- gold, units, or items
- selected story flags
- no gameplay progression carry-over except story progression

Even when gameplay state resets between scenarios, the story should continue forward and meaningful player choices should still be remembered.

## The big gameplay loops

### Minute-to-minute

The player:
- travels to a destination
- enters a location, service, or battle
- spends time, gold, party durability, or other resources
- gets information, items, recruits, or route access

### Day-to-day

The player:
- plans where to travel before 02:00 and before region-travel deadlines
- chooses when to fight, rest, shop, recruit, or return home
- balances risk against recovery and progress
- handles the consequences of missing rest or getting defeated

### Week-to-week

The player:
- plans around weekly service refresh and restock cycles
- decides how to spend limited gold and recruit opportunities
- gradually unlocks better routes, safer nodes, and stronger town functions
- feels that the world and home base are slowly becoming more stable

## World vision

Each scenario contains exactly one **world map**.

The world map is the top-level travel structure for the current scenario.
Its purpose is to organize the scenario into multiple authored **overworlds/regions**.

The world map:
- contains the scenario’s starting overworld
- contains other overworlds that may become available over time
- defines the top-level regional structure of the scenario
- allows the player to choose which overworld/region to travel to
- acts as an information screen for regional planning

The player is not physically “inside” the world map the same way they are inside an overworld or location.
The world map is a strategic selection screen and information layer above overworld play.

### Region select rules

The player can consult the world map at any time for information, including during non-travel planning contexts such as cutscenes or scenario overview moments.

However, actual travel between regions/overworlds follows strict rules:
- the player can travel between overworlds only once per day
- region travel must be initiated before **11:00**
- region travel uses that day’s single region-travel allowance
- after region travel is confirmed, the player character arrives in the destination region at **11:00**
- if the current time is **11:00 or later**, region travel is not allowed until the next day

### Cross-region party state

Only the player character travels between regions.

Each region preserves its own party state.
This means that units, recruits, and other region-local party composition remain associated with the region where they were acquired or left.

Example:
- in Region A, the player has 60 peasants and 1 hero unit
- the player travels to Region B alone
- in Region B, the player gathers 30 bandits
- when the player travels back to Region A, the player regains the 60 peasants and 1 hero unit from Region A
- the 30 bandits remain in Region B

This allows each region to retain its own strategic state instead of sharing one global party pool across the entire scenario.

### Region presentation

Selecting a region on the world map should:
- highlight that region clearly by its borders
- show an informative image or visual panel for the region
- display useful information such as:
  - difficulty
  - terrain
  - expected resources
  - regional identity
  - story or quest relevance
  - size or scale
  - other scenario-defined notes

The world map should feel like a strategic planning view, not just a menu list.

## Home base vision

Home base is not just a starting node.
It is the emotional and systemic center of the game.

Home base should eventually provide:
- free rest
- key story progression and survivor conversations
- restoration milestones
- access to some stable baseline services
- a clear sense of returning to safety after danger

Home base is where much of the story unfolds and where the player sees the clearest signs of recovery.

## Overworld vision

The overworld should remain node-and-route based rather than free-roaming.

An overworld is a regional travel map within a scenario’s world map.

Key principles:
- nodes represent authored destinations with gameplay meaning
- routes define the legal travel graph
- route access can change because of combat clears, blockers, quests, or future progression conditions
- the player should understand why a path is open, blocked, distant, or unsafe
- travel should feel like planning, not busywork

Nodes can represent:
- enterable locations
- inns
- recruit posts
- shops and service points
- combat encounters
- dungeons
- single-use bonuses or resources
- blocker nodes guarding deeper routes

## Location vision

Locations should eventually feel like authored spaces with identity, not just generic scene shells.

Locations may contain:
- service interactions
- NPC dialogue and memory fragments
- town-restoration context
- scene transitions
- local time costs
- safe interactions, risky interactions, or story interactions

Shared prototype scenes are acceptable during slice development, but the finished game should support strong location identity through content.

## Battle vision

Battles should remain true turn-based CTB and easy to read.

The battle module should support:
- clear turn order communication
- meaningful turn-frequency manipulation
- stack-based generic units plus hero units
- readable consequences for defeat, KO, and recovery
- enough tactical depth to make route blockers and high-risk destinations feel significant

Battle should stay deterministic apart from intentional RNG and should remain testable independently from rendering.

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

### Inns

- inns are generally usable if present
- resting at inns costs gold
- home-base rest is free
- the choice of where to sleep should matter economically and geographically

### Recruit posts

- recruit locations offer 1–3 recruitable unit types
- each offer has a quantity limit
- recruit quantities replenish on a weekly cadence
- stronger or rarer recruit points can become major regional objectives

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
- clear short-term goals tied to the region and home base

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
- world maps and overworlds/regions
- nodes and links
- locations and scenes
- service definitions
- recruit offers and stock
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

The goal is a focused, maintainable strategy/RPG hybrid with strong place identity, service planning, restoration fantasy, and content-driven authored progression.
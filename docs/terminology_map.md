# Terminology Map

This document defines the current intended project terminology and explains how it maps to any older runtime, content, or serialized names that may still appear in the repository.

Use this file as the terminology source of truth when design docs, AI guidance, source code, content files, and serialized values do not all use the same names.

This is a terminology and interpretation document. It is **not** the full design specification.

Current design truth lives primarily in:

- `docs/game_vision_complete.md`
- `docs/combat_rules.md`
- `docs/core_loop_rules.md`
- `README_DECISIONS.md`

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
- trader
- mine / owned-resource interaction
- Sealed / Frozen Hero interaction
- other authored interactions

---

## 2. Region structure terms

### Node
A **node** is a single-purpose travel point inside a Region graph.

Current intended node categories are:
- **empty / travel node**
- **Location node**
- **single Service node**
- **blocker node**

There is **no dedicated permanent combat-node type** in the current design.

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

### Mine
A **mine** is an owned or capturable Region-layer resource node / Service that generates resources over time.

- A mine may be free or guarded.
- A mine may be captured by teams.
- A mine does not inherently block movement.
- A mine may have stationed units.

### Sealed / Frozen Hero service
A **Sealed Hero** or **Frozen Hero** service is a one-time Region Service that frees a hero from a special **Sealed** state.

- Freeing the hero costs time and Energy.
- The action is illegal if the acting team has no room for the hero.
- The freed hero joins the acting team using normal hero-placement rules.
- After use, the node becomes an empty travel node.

---

## 3. Party and roster terms

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

---

## 5. Energy terms

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

## 6. Legacy runtime and content names

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

## 7. Leader terminology note

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

## 8. Guidance for future work

When writing docs, code comments, prompts, plans, or design notes:

- prefer **World Map** over `overworld_selection`
- prefer **Region** over `overworld` for the in-scenario travel layer
- prefer **Location** for entered places inside a Region
- prefer **Service** for functional interactions
- prefer **traveling party**, **stored units**, **Temporarily Unavailable**, and **Sealed** when those are the correct concepts
- treat enemy-team behavior as **authored setup + systemic behavior**
- treat ownership, sabotage, sanctuary, and service destruction as Region-layer concepts unless explicitly designed otherwise

When source/runtime compatibility requires older names to remain in place:
- preserve compatibility deliberately
- document the mismatch if it could confuse future work
- do not assume legacy names define current design intent

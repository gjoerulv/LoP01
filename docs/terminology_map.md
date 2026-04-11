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
- region-layer Services
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

---

## 4. Enemy-team terms

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

Enemy teams only act while the player is in the same Region.

### Stationary hostile encounter
A **stationary hostile encounter** is not the same thing as an enemy team.

It is temporary hostile content that may exist on an otherwise normal node.
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
- prefer **traveling party**, **stored units**, and **Temporarily Unavailable** over older or narrower wording when those are the correct concepts

When source/runtime compatibility requires older names to remain in place:
- preserve compatibility deliberately
- document the mismatch if it could confuse future work
- do not assume legacy names define current design intent

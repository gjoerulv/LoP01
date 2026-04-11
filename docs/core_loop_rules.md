# Core Loop Rules

This document defines the current intended core loop for the playable game and the major systemic rules that support it.

For terminology, see `docs/terminology_map.md`.

---

## 1. Core loop summary

At a high level, play loops through these layers:

1. **World Map**
   - Select or inspect Regions within the current Scenario.
   - Travel between Regions when travel is legal.

2. **Region**
   - Move the traveling party across a node graph.
   - Spend time and Energy to travel.
   - Interact with Services, Locations, blockers, resources, enemy teams, and hostile encounters.

3. **Location**
   - Walk around an entered place.
   - Interact with NPCs, Services, and scenario content.
   - Some Locations function as safe anchors.

4. **Battle**
   - Resolve combat using the active party against hostile forces.
   - Battle outcome feeds back into the traveling party, roster state, and world state.

---

## 2. Scopes and travel layers

### Scenario
A **Scenario** is the top-level authored play unit.

### World Map
The **World Map** is the scenario-level Region selection layer.

- Every Scenario has a World Map, even if it contains only one Region.
- The World Map may always be opened outside battle while inside a Scenario.
- World Map travel may be inspected at any time, but the travel action is disabled when illegal.

### Region
A **Region** is the main in-scenario travel space.

- The player is always in exactly one Region at a time.
- Regions are node graphs with authored structure and systemic rules layered on top.
- Region travel inside a Region uses the shortest valid path to the selected reachable node.

### Location
A **Location** is an entered place inside a Region.

- A Location may contain zero or more Services.
- A Location may contain NPCs, quests, multiple screens, and dungeon-like content.
- Entering or exiting a Location does **not** cost time.

---

## 3. Time

Time progresses as the player acts in the world.

Important fixed rules:

- Region-to-Region travel must begin **before 11:00**.
- Region-to-Region travel always arrives at **11:00** in the destination Region.
- Entering or exiting a Location does not cost time.
- Resting and travel consume time according to the rules below.

### Region-to-Region travel time
World Map travel time is based on Region distance.

- If two Regions directly touch, travel time between them is **1 day**.
- If there are `N` Regions between the current Region and the destination Region, travel time is `1 + N` days.
- The game uses the shortest valid contiguous path through unlocked and enterable Regions.
- If no valid path exists, travel is illegal.

---

## 4. Traveling party, active party, reserve, and storage

### Traveling party
The **traveling party** is:

- the **active party**
- plus the **reserve**

The traveling party is the unit set that moves with the player inside a Region and between Regions.

### Active party
The **active party** is the current battle-legal party.

- Up to 5 units
- Must always remain battle-legal
- Always has exactly one assigned leader

### Reserve
The **reserve** is the traveling non-active roster.

- Up to 7 units
- Reserve units travel with the player
- Reserve units can be switched into the active party outside battle

### Stored units
Stored units are assigned to a specific storage Service.

- Each storage Service has its own independent storage
- Each storage has up to 7 slots
- Stored units do not travel with the player
- Stored units persist in the Region where they are stored

---

## 5. Cross-Region travel consequences

### Heroes
All heroes in the traveling party travel between Regions with the player.

### Generic units
Generic units in the traveling party do **not** survive Region change.

- If the player begins Region travel while generic units remain in the traveling party, those generic units are lost.
- The game must clearly warn the player before such travel.
- Generic units that the player wants to preserve across Region departure must be stored beforehand.

### Stored units
Stored units remain in their original Region and persist there.

### Region persistence
When the player leaves a Region, the Region persists its relevant state, including:

- stored units
- Services and recruit offerings
- enemy-team state and attrition
- cleared state
- authored world-state progression

---

## 6. Energy

The traveling party has a shared **Energy** pool.

Units do **not** have individual Energy.

### Daily starting Energy
At the start of each day, traveling-party Energy is set to:

`1000 + (X × 100) + Y + Z`

Where:

- `X` = the Agility of the unit with the lowest Agility in the entire traveling party
- `Y` = any passive-skill Energy bonus granted by the current leader
- `Z` = any equipped-item Energy bonus granted by the current leader

If the team has no leader, only the applicable parts of the formula apply.

### Restoring Energy
Energy may be restored by:

- resting
- items
- events
- Services

### Rest action
The traveling party may rest to restore Energy.

- Resting takes **3 hours**
- Resting restores **300 Energy**

### Travel legality
If the traveling party does not have enough Energy to make a move, the move is illegal.

- The destination may still be selected and inspected
- The player may see why the move is illegal
- The move itself may not be performed

---

## 7. Region travel inside a Region

Inside a Region:

- the player may select any reachable node directly
- the game uses the shortest valid path
- same-node movement is effectively not a meaningful move
- blocked nodes remain visible even when unusable

### Routes
Routes have authored quality.

A route is either:

- a **road**
- or some other terrain / route type with penalties

Route quality affects:

- travel time
- Energy cost

Roads represent the baseline with no extra penalty.

---

## 8. World Map travel between Regions

World Map travel has both a time requirement and an Energy requirement.

### Region travel cost
- Region-to-Region travel requires **1000 Energy**
- This Energy is paid **once when travel begins**
- Travel then consumes the required number of days automatically

### Region travel legality
Region travel is illegal if any of the following is true:

- it is 11:00 or later
- the player is currently in battle
- the player is in a Location and has not returned to the Region layer
- the player is in a dungeon and has not returned to the Region layer
- the destination Region is not unlocked
- the destination Region is not enterable
- no valid contiguous path exists
- the traveling party does not have at least 1000 Energy

### Arrival node
Each Region must have a designated **arrival node**.

- Arrival is a flag, not a node type
- An arrival node may also be a Location node or a single Service node
- Arrival nodes are protected:
  - enemies may not spawn there
  - enemies may not occupy there
  - enemies may not block arrival there

---

## 9. Region nodes

Regions use a **single-purpose node model**.

### Node categories
The intended node categories are:

- **empty / travel node**
- **Location node**
- **single Service node**
- **blocker node**

There is **no dedicated permanent combat-node type**.

### Empty / travel node
An empty / travel node:

- may be traveled to
- may contain a single hostile encounter
- may contain a single resource or item
- becomes an ordinary empty travel node after the one-time content is cleared

### Location node
A Location node enters a Location and switches the game to Location Mode.

### Single Service node
A single Service node gives one direct functional interaction on the Region layer.

### Blocker node
A blocker node gates traversal.

A blocker may require:

- a key
- a password
- gold
- resources
- quest progress
- some other authored condition

A blocker may be:

- one-time
- persistent
- payment-per-passage
- hostile by virtue of enemy occupation

When permanently cleared, it becomes an empty travel node.

---

## 10. Services and Locations

### Direct Service nodes
Direct Service nodes are meant for quick functional interactions.

Examples may include:
- storage
- recruitment
- recovery
- supply
- other authored Services

### Locations
Locations are meant for explorable places.

A Location may contain:
- NPCs
- multiple screens
- multiple Services
- quests
- dungeon-like sequences
- bosses or fights

### Safe anchor
A Location functions as a **safe anchor** if it provides:

- free rest
- guaranteed rest

Nothing else is required for a Location to count as a safe anchor.

### Dungeons
Dungeons are a type of Location.

Enemy teams do not enter Locations, including dungeons.

---

## 11. Enemy teams

Enemy teams are AI-controlled traveling parties that operate on the Region layer.

They follow the same broad party rules as the player side where applicable.

### Enemy-team properties
An enemy team:

- moves only inside the current Region
- does not travel between Regions
- does not enter Locations
- may contain heroes and generic units
- may or may not have a leader
- does not suffer sleep / wake-up penalties

Enemy teams only act while the player is in the same Region.

### Enemy-team actions
Enemy teams may:

- move across Region nodes
- occupy nodes
- attack the player
- be attacked by the player
- use direct Region Services
- rest
- recruit heroes and generic units
- pick up resources
- fight stationary hostile encounters
- attack direct-storage gates

Enemy teams do not use Location Services because they do not enter Locations.

### Occupation
If an enemy team occupies a node, the opposing side must defeat that team to use the node.

This includes Location nodes:
- if an enemy team occupies a Location node, the Location is inaccessible until that team is defeated

Enemy teams may remain parked on a node indefinitely if the AI decides to do so.

---

## 12. Storage gates

Storage may exist either:

- inside a Location as a Service
- or directly on the Region layer as a direct storage Service node

A direct storage Service node also functions as a defensible gate for the owning side.

### Storage defense
If an enemy team attacks a direct storage node while the player is absent:

- the defending roster is the stored units at that storage
- stored heroes and stored generic units may defend
- the defending side may begin without a leader
- if a legal hero is available, leadership may still be assigned during battle

If a traveling party is present on the storage node when it is attacked:

- the defending side uses the active party of that traveling team for the defense battle

### Storage loss
If the defender loses the storage battle:

- all units in that storage are dismissed
- stored heroes become Temporarily Unavailable through the normal dismissal pipeline

Only enemy teams may attack storage gates as a systemic rule.

---

## 13. Temporarily Unavailable heroes

A hero becomes **Temporarily Unavailable** when removed from the roster through rules such as:

- being lost at the end of battle
- being dismissed voluntarily
- other authored causes

A Temporarily Unavailable hero is:

- not in the active party
- not in reserve
- not stored
- not recruitable until returned to the hero pool

By default, Temporarily Unavailable heroes return to the relevant hero pool at the start of the next week.

---

## 14. Node clearing outcomes

When temporary node content is cleared, the node becomes an empty travel node.

This applies to:
- one-time hostile encounters
- one-time resource / item pickups
- one-time blockers

Nodes are not destroyed. Their state changes.

---

## 15. Agent / implementation guidance

For future implementation and planning:

- Use current terminology:
  - **World Map**
  - **Region**
  - **Location**
  - **Service**
  - **Traveling party**
  - **Stored units**
  - **Temporarily Unavailable**
- Do not reintroduce older `overworld` terminology into design-facing work unless required for legacy compatibility.
- Treat Regions as authored node graphs with systemic rules layered on top.
- Do not introduce a dedicated permanent combat-node abstraction unless the design changes later.

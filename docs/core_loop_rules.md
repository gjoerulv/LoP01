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
- sanctuary
- trader
- mines and other owned-resource services
- sealed / frozen hero services
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

## 11. Team colors, alliances, and hostility

Each team is assigned a color.

- The player team also has a color.
- The game supports up to **8 teams total** in a Region, including the player.
- Teams are either:
  - **allies**
  - or **enemies**

There is no intermediate diplomacy state.

Alliance and hostility may change only through authored events such as:
- quest outcomes
- story triggers
- special node events

### Allied behavior
Allied teams may:
- share the same node
- avoid fighting each other
- cooperate indirectly through movement and pressure

Allied teams do **not** automatically share:
- node ownership
- service ownership
- service-use rights
- recruit ownership
- storage ownership

An alliance does not eliminate competition.

---

## 12. Enemy teams

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

### Enemy-team setup
Enemy-team setup is authored.

A designer may define:
- team color
- starting node
- template
- alliances
- patrol radius
- personality
- aggression level
- other authored constraints

Behavior after setup is systemic.

### Enemy-team Energy
Enemy teams have their own Energy pools.

They use the same broad Energy rules as the player:
- same daily starting-Energy formula
- same leader-bonus logic where applicable
- same restoration logic where applicable

Enemy legality is always checked against the **real game clock**.

### Enemy-team action cadence
Enemy teams act **after each player action that costs time**, but only:

- in **Region Mode**
- for teams in the **same Region**

Each eligible enemy team gets **one action only** per such enemy phase.

### Enemy-team action order
Enemy action order is **fixed by team color**.

In single-player:
- the human team is always first in line
- enemy teams then follow fixed color order

If multiple human-controlled teams exist, the game still follows strict color order.

### Enemy-team action budget
One enemy-team action may be:

- movement
- attack
- recruit
- rest
- service use
- service destruction
- other legal Region-layer action

One action may not exceed what would consume **1 hour** for that team.

Enemy actions do **not** advance the real game clock.

### Enemy-team rest cooldown
If an enemy team performs an action that requires a cooldown, such as resting, that cooldown is measured against the **real game clock**.

Example:
- resting imposes a **3-hour** wait before that team may act again
- this wait expires based on cumulative player-spent real time

### Enemy-team goals
Enemy-team behavior is driven by systemic priorities shaped by authored personality and aggression.

The three baseline personalities are:

- **Warrior**
- **Builder**
- **Explorer**

General goal space includes:
- victory
- survival
- attacking weak hostile teams
- recruiting
- gathering resources
- blocking access to important nodes
- camping on strategic nodes
- exploring

Typical emphasis by personality:
- **Warrior** prioritizes combat pressure and winning fights
- **Builder** prioritizes recruits and resource growth over aggression
- **Explorer** prioritizes revealing and traversing the Region over camping and blocking

### Enemy-team aggression levels
Aggression level defines how willing an AI team is to take a fight.

Baseline levels are:

- **Berserk** — anything goes
- **Reckless** — fights most battles
- **Opportunistic** — fights when victory appears at least roughly even
- **Careful** — fights only when likely to win
- **Coward** — fights only with overwhelming advantage
- **Pacifist** — never initiates attacks

### Patrol radius
An enemy team may also have an authored patrol rule.

- If patrol is enabled, the team is constrained by its patrol radius
- A patrol radius of **0** effectively makes the team stand still unless other rules override that

### Enemy-team actions
Enemy teams may:

- move across Region nodes
- occupy nodes
- attack hostile teams
- be attacked by hostile teams
- use direct Region Services
- rest
- recruit heroes and generic units
- pick up resources
- fight stationary hostile encounters
- attack direct-storage gates
- destroy destroyable Region services
- capture owned nodes such as mines and storage gates

Enemy teams do not use Location Services because they do not enter Locations.

---

## 13. Combat engagement on the Region layer

### Attack reach rule
A team may attack any hostile team whose occupied node it can legally reach within its current action budget.

### Stationary hostile encounters
Stationary hostile encounters are always **neutral**.

They are not owned by a colored team.

### Shared-node combat
If multiple allied teams share a node and a hostile team attacks that node:

- combat is resolved **one battle at a time**
- the attacker selects which hostile target on that node to fight first

After a battle ends:
- the attacker may stop
- or continue attacking another hostile team on that node if still legal

A team only enters or occupies the target node if all hostile teams remaining on that node have been defeated or are allied.

Otherwise the attacker remains on its previous node.

### Player-involved battle flow
For battles involving the player:

- the game first computes an **auto-resolve** result
- the player may accept that result
- or choose to play the battle manually instead

When a player-involved battle ends, the result screen is shown.

The player may choose to retry the battle from the original pre-battle state.

### AI-versus-AI battle flow
AI-versus-AI battles are always auto-resolved.

Their results are not shown directly to the player.

AI movement and AI-vs-AI outcomes are only visible where the Region has been revealed to the player.

---

## 14. Node occupation and access

If a hostile team occupies a node, the opposing side must defeat that team to use the node.

This includes:

- Service nodes
- blocker nodes
- storage gates
- Location nodes

### Location-node occupation
If a hostile team occupies a Location node:

- the Location is inaccessible until that team is defeated

Enemy teams themselves still do not enter the Location.

### Protected arrival nodes
Even though arrival is only a flag, arrival nodes are protected.

Enemy teams may not:
- occupy them
- spawn there
- block arrival there

---

## 15. Ownership, capture, and competitive control

Some nodes or Services may be owned by a specific team.

Ownership transfers **immediately** when captured.

### Owned examples
Current intended owned-node / owned-service examples include:

- storage gates
- mines and similar daily-resource Services
- other authored owned Services or nodes

### Mines
Mines are resource-generating nodes.

- They may be free to capture
- They may be guarded by neutral hostile encounters
- A team may station units there
- They do not block movement by themselves

To take a guarded mine:
- the attacker must defeat its guardians first

### Ownership and allies
Allies do not automatically gain ownership benefits from allied control.

An allied team may share a node, but that does not grant automatic use of the node's owned functionality.

---

## 16. Sanctuary and protected non-combat spaces

A **sanctuary** is a single Service node.

### Sanctuary rule
No fight may be initiated on a sanctuary node.

Any team occupying a sanctuary node cannot be attacked while there.

This naturally makes sanctuary occupation able to impede hostile passage, even though sanctuary is not formally a blocker-node type.

### Sanctuary ownership and allies
Allied teams may still share a sanctuary node.

Sanctuary protection does not create additional allied ownership rights beyond normal alliance rules.

### Sanctuary destruction
A sanctuary may still be destroyed if it is flagged as destroyable.

General service-destruction rules still apply.

---

## 17. Service destruction and restoration

Only **Region Services** may be destroyed as a systemic rule.

Services inside entered Locations are not part of this default systemic destruction model.

### Destroyable flag
A Service may only be destroyed if it has a **destroyable** flag.

### Who may destroy a service
A service may be destroyed only by the **occupying team**, regardless of ownership.

Destroyable services may be destroyed:
- by their owner
- by an enemy occupier
- by another occupying team when legal

### Destroy-from-distance exception
Special items or skills may destroy a service from a distance if explicitly authored to do so.

In that case:
- normal occupancy may not matter
- but guarded services may not be destroyed until their guardians are defeated

### Destruction cost
Destroying a service costs:

- **1000 Energy**
- **1 hour**

### Restoration
Restoring a service costs resources, but no Energy.

The service is then flagged to be restored and is automatically restored at the start of the next day.

This queued restoration may be cancelled by destroying the service again before the next day begins.

---

## 18. Storage gates

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

Only enemy teams may attack storage gates as a default systemic rule.

---

## 19. Recruitment and competition

Recruitment services are shared competitive resources.

### Generic-unit recruitment
A recruitment Service grows over time according to its authored growth rules.

Teams may recruit:
- all available units
- none
- or any affordable subset

The pool belongs to the Service itself, not to any particular team.

A recruitment Service may hold up to **4 weeks** worth of growth.

### Hero recruitment
Hero-recruit Services draw from the shared Scenario hero pool.

If any team recruits a hero:

- that hero becomes unavailable to all other teams
- until that hero is later lost and returned to the pool through the normal rules

Enemy teams use the same hero-pool logic as the player.

### Sealed / Frozen Hero services
A Sealed Hero or Frozen Hero service is a one-time Region Service that frees a hero who is currently in a special **Sealed** state.

Freeing a sealed hero:

- costs **500 Energy**
- costs **1 hour**
- is illegal if the acting team does not have room for the hero

When freed, the hero immediately joins the freeing team using the same placement rules as normal hero recruitment:

- first free active slot if available
- otherwise first free reserve slot
- otherwise illegal

After the hero is freed, that node becomes an empty travel node.

---

## 20. Temporarily Unavailable heroes

A hero becomes **Temporarily Unavailable** when removed from the roster through rules such as:

- being lost at the end of battle
- being dismissed voluntarily
- being lost through storage destruction
- other authored causes

A Temporarily Unavailable hero is:

- not in the active party
- not in reserve
- not stored
- not recruitable until returned to the hero pool

By default, Temporarily Unavailable heroes return to the relevant hero pool at the start of the next week.

These hero rules apply equally across teams, except that the single-player player character has the special rule that they never permanently disappear from play.

---

## 21. Enemy-team defeat, persistence, and replacement

When an enemy team is defeated:

- it is removed permanently from the Region

Authored events may later spawn a new AI team if desired.

### Enemy-team identity
Enemy teams may be:
- default template-based teams
- scenario-authored specific teams
- or a mixture of both

A template may define things such as:
- typical stack ranges
- possible hero leader
- baseline composition
- baseline behavior identity

Default naming may be tied to team color unless a Scenario overrides it.

---

## 22. Node clearing outcomes

When temporary node content is cleared, the node becomes an empty travel node.

This applies to:
- one-time hostile encounters
- one-time resource / item pickups
- one-time blockers

Nodes are not destroyed. Their state changes.

More drastic node destruction or structural world changes should happen only through authored events.

---

## 23. Agent / implementation guidance

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
- Treat enemy teams as authored setups with systemic behavior.
- Do not introduce a dedicated permanent combat-node abstraction unless the design changes later.
- Do not assume allied control grants shared ownership or shared service rights.
- Keep PvP as a separate future mode unless a task explicitly focuses on it.

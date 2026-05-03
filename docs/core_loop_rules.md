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
   - Interact with Services, Locations, blockers, resources, enemy teams, hostile encounters, quest services, and event nodes.

3. **Location**
   - Walk around an entered place.
   - Interact with NPCs, Services, and scenario content.
   - Some Locations function as safe anchors.

4. **Battle**
   - Resolve combat using the active party against hostile forces.
   - Battle outcome feeds back into the traveling party, roster state, and world state.

5. **Progression**
   - Events, quest services, victory conditions, defeat conditions, and guidance advance or reshape the Scenario.
   - The player is not required to follow the intended guidance chain if a true victory condition is satisfied directly.

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
- quest services
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
- quest-service use when eligible
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
- use eligible quest services

Enemy teams do not use Location Services because they do not enter Locations.

---

## 13. Combat engagement on the Region layer

### Attack reach rule
A team may attack any hostile team whose occupied node it can legally reach within its current action budget.

### Stationary hostile encounters
Stationary hostile encounters are always **neutral**.

They are not owned by a colored team.

Defeating a neutral hostile encounter on the Region layer may also trigger an event-action chain if that encounter is authored to do so.

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
For battles involving the player, battle entry uses the threat-preview and auto-resolve flow defined later in this document.

At a high level:
- the player may see a rough threat preview
- the game may auto-resolve first if auto-resolve is enabled
- the player may accept the result or redo the battle manually
- retries restore the original pre-battle state and deterministic battle seed

### AI-versus-AI battle flow
AI-versus-AI battles always use auto-resolve.

Their results are not shown directly to the player unless revealed through normal fog-of-war or event feedback.

---

## 14. AI behavior, knowledge, and action selection

Enemy-team AI should use medium-depth weighted behavior, not deep minimax planning.

The AI should make plausible strategic decisions under the same world rules as human teams without attempting to solve the entire future state of the Scenario.

### Determinism and seeds
AI world behavior is deterministic from hidden Scenario-start AI seed data.

If a Scenario starts with the same AI seed and the human player makes the same moves, AI world decisions should resolve the same way.

Battle randomness uses a separate deterministic battle / damage seed stream. Different playthroughs may therefore share an AI-behavior seed but still differ in battle outcomes if their battle seed differs.

### AI knowledge
AI teams obey the same knowledge rules as human teams.

An AI team may act only on:
- what that team has explored
- what that team currently has revealed
- information exposed by its scouting / vision rules
- information made available through normal Scenario systems

AI teams should not cheat by using unrevealed map knowledge.

### Action-selection structure
AI action selection should use a priority pipeline rather than a single flat score.

The broad priority order is:

1. pursue a valid victory opportunity
2. avoid defeat or preserve survival where appropriate
3. satisfy urgent tactical or logistical needs
4. pursue personality-shaped goals

Personality and aggression modify how the AI interprets risk, especially when victory and survival conflict.

### Victory, survival, personality, and aggression
Victory opportunity generally overrides personality.

Avoiding defeat interacts with aggression level:

- **Pacifist** and **Coward** strongly avoid losing
- **Careful** usually avoids losing
- **Opportunistic** is more willing to risk a win attempt
- **Reckless** and **Berserk** try to win when the opportunity exists
- **Warrior** accepts higher combat risk when a fight can satisfy a victory condition
- **Builder** and **Explorer** weigh victory fights more evenly against their normal priorities

### Personality effects
Personality affects both:
- goal priority
- movement style

Examples:
- **Warrior** seeks pressure, fights, and denial
- **Builder** values recruits, resources, restoration, and growth
- **Explorer** values reveal, discovery, and traversal

### Pass / wait behavior
AI teams may intentionally pass or wait if that is the best legal action.

Valid reasons include:
- no useful action exists
- movement would expose the team
- the team is guarding a strategic node
- the team is waiting for a cooldown
- the team is constrained by patrol or service timing

### Patrol radius
Patrol radius is hard enforced.

An AI team may not move outside its patrol radius unless events remove, replace, or expand that patrol rule.

Victory opportunity does not override patrol radius by itself.

### Movement and pathing
AI movement uses the same broad model as player movement.

An AI team:
- selects a reachable destination within its one-action budget
- uses the shortest legal path
- pays the normal Energy cost
- may not exceed what would consume 1 hour for that team

AI should also account for relevant service time or Energy costs when deciding whether an action is legal and useful.

### Hostile and allied occupied nodes
AI may path through allied occupied nodes.

Hostile occupied nodes are treated as obstacles unless the AI chooses to fight.

If an AI wants something beyond a hostile occupied node:
- it may attack if its aggression and strength evaluation allow it
- it may choose an alternate route if one exists
- it may recruit or improve first if a nearby service makes the fight more favorable

Pacifist AI never initiates attacks.

### AI service use
AI teams may use any legal **Region** service.

AI teams do not enter Locations and therefore do not use Location-only service interactions.

AI teams may:
- recruit
- rest
- collect resources
- use Region farming services
- use sanctuary
- use Markets
- use Trading Posts for resource exchange
- use Freelancer's Guilds
- use Black Markets
- combine artifacts at artifact-handler services
- cook through the party-menu system
- equip and manage artifacts
- leave guards where legal
- destroy or restore services where legal

AI teams should not send resources to other teams through Trading Posts.

### AI economy and equipment behavior
AI should try to use economic systems well enough to remain competitive.

Examples:
- trade resources to afford useful recruits
- buy artifacts when useful
- combine artifacts when possible
- sell generic units at a Freelancer's Guild only when they are considered low-value and the team needs gold
- cook when available food would be useful
- keep the best available artifacts equipped

Combining artifacts always produces a stronger artifact, so AI should generally prioritize valid combinations when they help the team.

### Destroying and restoring services
AI may destroy a service when:
- the service is useful mainly to enemies
- denial is strategically valuable
- the team can afford the time and Energy cost
- the service is legally destroyable

**Warrior** AI is most likely to actively sabotage.

AI may restore a destroyed service when:
- the service is useful to the team
- the team can afford the restoration cost
- restoration supports its current goals

**Builder** AI is most likely to restore services, but all personalities may restore useful services.

### Authorship boundaries
Designers assign:
- personality
- aggression
- patrol
- team setup
- alliances
- gates
- events
- team spawns

Designers should not assign direct arbitrary AI objectives such as “go attack this exact node now.”

Story-like AI behavior should be achieved through:
- gates
- event conditions
- team spawning
- changes to personality / aggression / patrol
- systemic rules

---

## 15. Fog of war, reveal, and scouting

Fog of war is shared in principle across human and AI teams.

Each team has its own explored / revealed map knowledge.

### Initial reveal
By default, nothing is revealed except the area around the team's current position.

When a team moves to a node, it reveals up to **2 nodes ahead** by default.

Passive scouting can increase this reveal range up to **5 nodes**.

Owned nodes at Scenario start reveal **2 nodes** around those owned nodes immediately.

### Reveal persistence
Fog of war uses a HoMM2/HoMM3-like model.

A revealed area never becomes stale.

Once something is revealed, it remains revealed and gives live information.

### Vision sources
Vision may come from:
- the team's current position
- allied teams
- scouting / passive skills
- stationed guards
- owned services
- authored reveal services

A reveal service may reveal **10 nodes** in a radius around itself.

### AI fog of war
AI teams use the same fog-of-war rules as human teams.

AI teams should not act on unrevealed information.

### Visible and unseen actions
Visible AI movement should be animated when it happens in revealed areas.

The game should support movement-speed settings from very slow to instant.

Unseen AI events do not generate messages by default unless an authored event explicitly does so.

If a visible team uses a service, item, rest, waits, or passes, the player should receive appropriate visible feedback.

### Hidden victory
If a hidden AI team satisfies a victory condition, the game ends immediately and reveals why that team won.

### Enemy inspection and scouting
When an enemy team is visible, the default visible information is:

- team color
- leader
- threat color
- estimated unit quantities by stack
- estimated hero level ranges

Generic unit quantity ranges are:

- `1-4`
- `5-9`
- `10-19`
- `20-49`
- `50-99`
- `100-249`
- `250-499`
- `500-999`
- `1000+`

Hero level ranges are shown in increments of 4:

- `1-4`
- `5-8`
- `9-12`
- and so on

With the highest scouting capability, a team may inspect full visible-team details, including:
- exact composition
- resources
- items
- artifacts

The same scouting rules apply to AI knowledge.

---

## 16. Threat preview, auto-resolve, and auto-combat

Threat preview, auto-resolve, and auto-combat are separate concepts.

### Threat preview
Threat preview is a cheap estimate shown before or around battle selection.

It is **not** a full battle simulation.

Threat preview is based on visible unit counts and stats and ignores usable skills.

Threat color should communicate estimated danger:

- **red** if the enemy is heavily favored
- **yellow** if the enemy is favored
- **green** if the player is heavily favored
- **white** by default

Suggested default thresholds:
- enemy advantage above 75/25 = red
- enemy advantage above 50/50 = yellow
- player advantage above 80/20 = green
- otherwise white

### Auto-resolve result
Auto-resolve is the actual automated battle result.

Auto-resolve uses a full backend CTB battle simulation with AI battle choices.

It should:
- use the same battle rules as manual battle
- be fast
- be deterministic from the battle seed and starting state
- produce the actual result screen
- support AI-vs-AI battles without interrupting play

### Full battle rules in auto-resolve
Auto-resolve uses full battle rules, including:
- leader aura
- passive skills
- food buffs
- artifacts
- position and range rules
- MP constraints
- usable skills and items where enabled

AI teams may use everything available to them.

Human teams may restrict auto-resolve / auto-combat use of:
- usable skills
- items

By default, human-team auto-resolve / auto-combat use of usable skills and items is disabled to avoid wasting MP and items.

This restriction does **not** disable:
- leader aura
- passive skills
- food buffs
- artifacts
- other always-on effects

### Player-involved auto-resolve flow
If auto-resolve is enabled, player-involved Region battles auto-resolve first.

The player sees the result screen and may:
- accept the result
- redo the battle manually

The player cannot meaningfully rerun the same auto-resolve result, because it is deterministic.

### Auto-combat
Manual battle may include a toggleable **auto-combat** mode.

Auto-combat uses the same AI battle controller as auto-resolve.

Given the same:
- pre-battle state
- battle seed
- auto-combat settings
- human-team skill/item restrictions

auto-combat should produce the same result as auto-resolve if allowed to run without player intervention.

### Retry behavior
Retrying a battle restores:
- the exact pre-battle state
- the deterministic battle seed

The same choices should produce the same result.

Different player choices may produce different results.

### AI-vs-AI auto-resolve
AI-vs-AI battles always auto-resolve.

Their results are not shown directly to the player unless visible through normal fog-of-war or event feedback.

---

## 17. Node occupation and access

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

## 18. Ownership, capture, and competitive control

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
- Stationed units affect defense only, not production
- They do not block movement by themselves
- For now, mines are the main baseline example of an **owned resource service**

Default daily payouts at the start of the day are:

- Gold mine = **1000 Gold**
- Wood source = **2 Wood**
- Stone source = **2 Stone**
- Steel / Fiber / Clay / Gems source = **1** of its resource

Scenario content may override these defaults.

Mines may be destroyed and later restored under the normal destroyable-region-service rules when authored to allow it.

To take a guarded mine:
- the attacker must defeat its guardians first

### Ownership and allies
Allies do not automatically gain ownership benefits from allied control.

An allied team may share a node, but that does not grant automatic use of the node's owned functionality.

---

## 19. Sanctuary and protected non-combat spaces

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

## 20. Service destruction and restoration

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

## 21. Storage gates

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


## 22. Resources, currencies, items, and artifacts

### Team-owned resources
Resources belong to the current team globally.

They are:
- shared across all Regions inside the same Scenario
- not tied to individual units
- not tied to specific storage locations
- eligible for scenario-transition carry-over only when the relevant transition allows it

The baseline resource types are:

- **Gold**
- **Wood**
- **Stone**
- **Steel**
- **Fiber**
- **Clay**
- **Gems**

Gold is still a normal resource type, but in practice it is gathered at a much higher rate than the others.

Resources are primarily used for:
- recruitment
- service use where applicable
- restoring destroyed Region Services
- building Services or structures in Locations
- quest conditions and turn-ins
- event conditions and event actions
- victory / defeat conditions where authored

### Team inventories
The team should maintain separate shared inventories for:

- ordinary **items**
- **artifacts**

These are shared team inventories, not individual hero bags.

#### General items
General items are stored in the item inventory.

They include:
- consumables
- ordinary non-consumable items
- quest-like items
- seeds
- ingredients
- food

##### Consumables
Consumables are usable items intended for battle, field use, or both.

Rules:
- only **one** of each consumable type may be held at a time
- trying to add a duplicate consumable is illegal
- only heroes may use items during battle
- consumables are gone immediately when used

##### Other items
Other non-consumable items may exist primarily for:
- quests
- event conditions
- delivery requirements
- authored story logic

These stack up to **999** per item type and occupy one shared inventory entry per type.

##### Seeds and ingredients
Seeds are a special item type used by farming services.

- seeds may be planted at farming services in Regions or Locations where authored
- planted growth may later be affected by fertilization
- the eventual outcome depends on seed type and fertilization use

Ingredients are used to make food.

##### Food
Food is a consumable-like item category used only by heroes.

Food may:
- recover HP / MP
- grant temporary buffs
- grant duration-based world or battle advantages

Food use is a team-inventory action, but its actual consumption applies only to heroes.

#### Artifacts
Artifacts are the game's equippable gear system. There is no separate general “equipment” category beyond artifacts.

Artifacts:
- are stored in a separate shared artifact inventory
- may stack up to **999** copies per artifact type
- may be quest-relevant or scenario-defining
- may be carried over between scenarios only if the relevant transition allows it
- may be discarded unless scenario logic makes doing so illegal or foolish

Artifacts are equipped only by heroes.

Generic units may not equip anything.

#### Hero artifact slots
Each hero has:

- **1 Attack slot**
- **1 Defense slot**
- **3 Misc slots**

An artifact definition states:
- which slot or slots it may occupy
- what effect it has in those slots

Artifact effects remain active as long as the artifact is equipped.

#### Artifact combination
Some artifacts may be combined at an artifact-handling service into stronger artifacts.

Rules:
- combination is irreversible
- the consumed artifacts are permanently gone
- the resulting artifact is added to the team's artifact inventory or equipped inventory as appropriate

### Economic visibility
Resources, items, and artifacts are all valid authored inputs for:
- quests
- events
- trader services
- victory conditions
- defeat conditions
- campaign transition carry-over

---

## 23. Trader services and exchange rules

Trader services are a family of specialized authored services.

Each trader-service instance has **one primary function only**. Do not treat a single service instance as a universal merchant.

The main trader service categories are:

- **Trading Post**
  - exchanges resources for other resources
  - exchanges resources for gold and gold for resources using the defined exchange models
  - sends resources to other teams
  - AI teams may use Trading Posts for resource exchange, but resource sending to other teams is a human-facing activity.
- **Market**
  - buys and sells items for gold only
- **Freelancer's Guild**
  - buys generic units from a team for gold only
- **Black Market**
  - sells artifacts for gold only

A Scenario or specific authored service may override prices, stock, or barter rates where allowed, but the service type itself should remain specialized.

### Shared service-time rule
If a trader-type service performs any actual exchange, then:

- **20 minutes** pass when leaving the service screen

If nothing was exchanged:
- no time passes

Trader services do **not** cost Energy directly.

### Trading Post
Resource sending is part of the Trading Post role, not a separate trader-service type.

#### Sending resources to other teams
Sending resources:
- may target **any** team
- works regardless of alliance state
- happens **instantly**
- is not affected by exchange-rate overrides

If the receiving team is human-controlled, a message should be shown.

#### Gold-based base values
The default base values are:

- **Wood** = 100 gold
- **Stone** = 100 gold
- **Steel** = 200 gold
- **Fiber** = 200 gold
- **Clay** = 200 gold
- **Gems** = 500 gold

Default gold-trade rates at a Trading Post are:

- **buy price** = 5 × base value
- **sell price** = 1/5 of base value, rounded down, minimum 1

So the default gold prices are:

- buy 1 Wood = 500 gold
- buy 1 Stone = 500 gold
- buy 1 Steel = 1000 gold
- buy 1 Fiber = 1000 gold
- buy 1 Clay = 1000 gold
- buy 1 Gem = 2500 gold

And default sell prices are:

- sell 1 Wood = 20 gold
- sell 1 Stone = 20 gold
- sell 1 Steel = 40 gold
- sell 1 Fiber = 40 gold
- sell 1 Clay = 40 gold
- sell 1 Gem = 100 gold

A Scenario may override these defaults, and a specific Trading Post may override the scenario default if explicitly authored to do so.

#### Default resource-for-resource barter
Resource-for-resource exchange uses a separate default barter table rather than deriving all exchanges through gold.

Use these tiers:

- **Tier 1**: Wood, Stone
- **Tier 2**: Steel, Fiber, Clay
- **Tier 3**: Gems

Default buy costs for **1** unit of the target resource are:

| Pay with ↓ / Buy 1 of → | Tier 1 | Tier 2 | Tier 3 |
|---|---:|---:|---:|
| **Tier 1** | 10 | 20 | 50 |
| **Tier 2** | 5 | 10 | 25 |
| **Tier 3** | 2 | 4 | 10 |

A resource may **not** be exchanged for itself.

This produces the default full table:

| Buy 1 of... | Pay Wood | Pay Stone | Pay Steel | Pay Fiber | Pay Clay | Pay Gems |
|---|---:|---:|---:|---:|---:|---:|
| **Wood** | — | 10 | 5 | 5 | 5 | 2 |
| **Stone** | 10 | — | 5 | 5 | 5 | 2 |
| **Steel** | 20 | 20 | — | 10 | 10 | 4 |
| **Fiber** | 20 | 20 | 10 | — | 10 | 4 |
| **Clay** | 20 | 20 | 10 | 10 | — | 4 |
| **Gems** | 50 | 50 | 25 | 25 | 25 | — |

This barter table is the default. A Scenario may override it, and a specific Trading Post may override the scenario default if explicitly authored to do so.

### Market
A Market buys and sells general items for gold only.

Rules:
- available stock is authored by the designer
- default stock is none unless authored
- there is only one of each authored item in stock at a time
- stock restocks at the start of each week
- items sell back for **1/2** of base value, rounded down, minimum 1 gold

### Freelancer's Guild
A Freelancer's Guild buys generic units from the team for gold only.

Rules:
- the team may sell a full stack, a partial stack, or nothing
- default sell value is **1/2** of the unit's original gold price, rounded down, minimum 1 gold

### Black Market
A Black Market sells artifacts for gold.

Rules:
- artifact stock is authored by the designer
- default stock is:
  - 7 random minor artifacts
  - 1 major artifact
- by default, Black Market stock does **not** restock
---

## 24. Location-mode service construction, restoration, and upgrade

Location-mode service flow is fundamentally more event-driven than Region-mode service flow.

### Region-mode versus Location-mode
In **Region mode**:
- services are usually placed directly from a predefined service list
- the designer places the service on a Region node
- the service starts from default settings for that service type
- the designer may then edit the service within its legal limits

In **Location mode**:
- services are typically started by **triggerable events**
- the trigger may be an NPC, counter, stone, object, creature, or any other authored interactable
- the event may call a default service flow and then apply authored settings
- the resulting service still starts from its default behavior, but the designer may edit it within its legal limits

### Building, restoring, and upgrading
In Locations, “building a service” does not imply one universal hard-wired construction subsystem.

Instead, event actions may:
- build a new service
- restore a disabled or ruined service
- upgrade an existing service

This is all part of the broader event-driven Location model.

### Human-only interaction
Only **human teams** may build, restore, or upgrade services in Locations, because AI teams do not enter Locations.

AI may still block human teams from entering a Location by occupying the parent Region node.

### Persistence
When an event in a Location creates, restores, or upgrades a service, that result:

- persists in the current Scenario until later changed by another event
- is part of shared world-state for teams that can later enter the Location
- must be saved and loaded like other world-state changes

---

## 25. Farming services

Farming exists in two forms:

- a default **Region farming service**
- event-driven **Location farming** that calls the same broader farming flow when authored

### Region farming service
A Region farming service:

- is a real Region-layer service
- is not owned by any team by default
- may support stationed guards, similar to mines and gates
- may therefore be contested, defended, sabotaged, or stolen from by other teams

A farming service can run one active growing process at a time.

### Location farming
In Locations, farming is event-driven.

A Location interactable may call the default farming service flow through an event. For example:
- an indoor pot
- a greenhouse patch
- a garden bed
- some other authored farming point

### Seed usage and process size
A farming process uses:

- exactly **one seed type**
- an authored quantity of that seed

By default, a farming service may process up to **999** seeds at once, but the designer may override that limit.

Only one seed type may be active in the service at a time.

### Fertilization
Fertilization is:

- optional by default
- chosen only at planting time
- not applicable later if skipped at planting
- one fertilizer unit per process, regardless of seed quantity

The designer may disallow fertilization for a specific farming service.

Seed type plus fertilizer choice determine the outcome.

The resulting output should be shown to the player **deterministically** when the process starts.

Fertilization may affect:
- growth speed
- output amount
- output type

### Crop care / watering
Crop care is a separate support action.

Rules:
- care / watering costs **1 hour**
- it is allowed at most **once per day per farming service**
- this limit is shared across teams
- the effect lasts until the next day rollover

Daily growth progress is:

- **+1** if the process was not watered for that day
- **+2** if it was watered for that day

Crops always grow, even without care.

### Finished crops
When crops finish growing:

- the result remains in the farming service until collected
- the result does **not** expire by default

If another team gains access to the farming service after completion, that team may collect the result.

This makes unguarded farming intentionally risky on the Region layer.

### Failure and sabotage
Crops do not fail, wither, or spoil by default.

However:
- other teams may sabotage a Region farming service through normal world interaction
- Location-mode events may still author special crop-failure behavior if desired

---

## 26. Cooking and food

Cooking is a party-level system, not a world-service requirement.

### Availability
Cooking is available:

- anywhere **outside battle**
- while inside a **Scenario**
- whenever the **party menu** is available

### Cooking flow
Cooking is a direct recipe-selection flow.

The player:
- opens the cooking section in the party menu
- selects a recipe
- consumes the required ingredients
- receives the resulting food item

The process is irreversible once performed.

There is no cooking minigame.

### Recipe visibility
Recipes are:

- globally known from the start
- always viewable
- optionally filterable to show only currently available recipes

The default menu behavior should show only available recipes, with an option such as a “show all recipes” toggle.

### Skill requirements
Some recipes may require passive or secondary skills.

If at least one unit in the **traveling team** has the required skill, that counts as sufficient for the recipe.

Certain passive skills may also:
- reduce time cost
- increase output
- produce secondary benefits

### Time cost
Cooking consumes time.

The time cost is tied to the recipe, with **1 hour** as the default.

### Food rules
Food:
- is a stackable team-inventory item
- stacks up to **999**
- may only be consumed by heroes
- is **field-use only**
- is **not** a battle-use item

Food effects may include:
- HP recovery
- MP recovery
- battle buffs
- day-based buffs
- week-based buffs
- combinations of multiple effects
- other authored hero-facing outcomes

Duration depends on the specific food item.

---

## 27. Artifact handling and crafting

The only intended crafting systems at this stage are:

- **cooking**
- **artifact combination**

There is no broader general crafting system beyond those two.

### Artifact handler service
An artifact-handling service exists only to combine artifacts.

It does **not**:
- dismantle artifacts
- repair artifacts
- perform broader equipment crafting

### Combination recipes
Artifact combination recipes are:

- globally fixed
- content-authored
- deterministic

By default, an artifact-handling service may combine **all** valid fixed recipes.

However, a specific service may be authored to deny certain otherwise-valid combinations.

### Irreversibility
Artifact combination is irreversible.

Artifacts used in a combination are permanently consumed, and there is no dismantling back into inputs.

### Where artifact handling may exist
Artifact combination may exist as:

- a direct **Region service**
- a Location-mode event that calls the artifact-combination flow

This keeps artifact handling consistent with the broader split between Region hard-wired services and Location event-driven service calls.

---

## 28. Battle spoils, stealing, escape, and surrender

### Battle spoils against teams
If a team defeats another team in battle, the winner gains:

- **all items** from the defeated team
- **all artifacts** from the defeated team
- **1/4** of all non-gold resources from the defeated team

Gold is not included in this automatic quarter-resource transfer rule.

These battle-spoil rules should apply equally to AI and human teams whenever possible.

### Gold stealing during battle
Some battle effects may steal gold directly from the opposing team.

A team cannot lose more gold than it currently has.

If a steal-gold action targets a team with no gold:
- the action is wasted

### Consumable stealing during battle
A separate battle effect may steal:
- one random consumable
from the opposing team's inventory

This is distinct from ordinary battle-victory loot transfer.

### Hero departure and equipped artifacts
If a hero leaves the team for a reason other than being defeated by another team in battle, such as:
- neutral defeat
- dismissal
- other non-team-defeat removal

then that hero leaves with the artifacts currently equipped on them.

### Escape
Escape is a battle-end option performed through the leader.

Rules:
- only the **leader** may escape
- the escaping team survives
- the cost is that the entire **active party** is lost except for the leader
- the reserve is not lost through this rule alone

After escape:
- the team is removed from the Region map
- the team respawns the next day at **11:00**
- the respawn point is:
  - the latest resting place
  - or the original spawn point if there is no resting place

If the team cannot respawn because an enemy team occupies that spawn point:
- the escaped team is defeated

The winning side does not receive direct explicit information about the escaped team's later respawn location beyond logical deduction.

### Surrender
Surrender is a battle-end option where one team offers gold to the opposing team to retreat with the entire team intact.

Rules:
- surrender pays gold to the opposing team
- the surrendering team keeps its entire team intact
- the team is removed from the Region map
- the team respawns the next day at **11:00**
- respawn uses the same resting-place / spawn-point rule as escape

If the surrendering team cannot respawn because the spawn point is occupied by an enemy team:
- that team is defeated

In single-player, surrender should feel broadly similar to the ordinary setback rhythm of being forced out of the Region, except the additional explicit cost is the gold paid to the opposing team.

## 29. Recruitment and competition

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

## 30. Quest, objective, event, and progression structure

### Core terms
Use the following distinction:

- **Quest** = a single authored task that a team may pursue
- **Objective** = a typed requirement used by a quest or a victory condition
- **Victory condition** = what ends the Scenario successfully
- **Progression trigger** = any trigger that changes world state, whether or not it belongs to a quest
- **Event** = the general trigger / condition / effect system that changes world state

Quest logic is only one part of the broader progression system.

Events are the primary systemic progression engine.

### Eligibility vs condition
Keep these concepts separate:

- **Eligibility** = who is allowed to participate or trigger
- **Condition** = whether the actual quest, event, or victory requirement is satisfied

This distinction applies across:
- quest services
- manual events
- automatic events
- victory conditions
- defeat conditions

---

## 31. Quest services

A **quest service** is a specific authored Service structure on the map.

It may contain:
- zero quests
- one quest
- several quests arranged as a chain

A quest service exposes at most **one currently available quest** from that chain at a time.

### Quest chain
A **quest chain** is an ordered list of quests inside one quest service.

When the current quest is completed and turned in:
- it is flagged as completed
- the next quest in the chain may become available immediately through that same service

### Turn-in structure
All quest-service quests are turn-in quests.

A team must return to the quest service to complete the quest formally and receive its event actions.

When a team returns and satisfies the quest's completion condition, the completion message is shown with a **Yes / No** choice:

- **Yes** completes the quest and triggers its event-action chain immediately
- **No** leaves the quest unfinished for now

### Quest-service messages
Each quest-service quest has the following message structure:

- **Starting message**
- **Progress message**
- **Completion message**

Optional portrait-style images may be attached to these messages.

Message behavior:
- if a team visits the quest service for the first time and the quest is not yet completed, that team sees the **Starting message**
- if that team returns later and has not met completion conditions, it sees the **Progress message**
- if the team meets completion conditions on its first visit, the **Completion message** follows immediately after the **Starting message**

### Quest-service variants
Quest services may be designed in different ways:

- disappearing after completion
- persistent but blocking for teams that have not completed them
- persistent and non-blocking after completion
- repeatable guard/toll style

### Repeatable quest guards
Only blocker-style quest-guard services are intended to be repeatable.

For repeatable quest guards:
- the same condition repeats
- the same message repeats
- the same event actions repeat
- repeatability is a deliberate authored choice, not the default

### Participation eligibility
A quest service may be usable by:
- all teams
- or a selected subset of teams

Typical eligibility filters include:
- time window
- team color whitelist
- human-only
- AI-only
- combinations of the above

A team must be eligible both:
- to participate in the quest service
- and to satisfy the quest’s completion condition

### Quest-service competition
Any eligible team may complete a quest service before another.

Unless it is a repeatable blocker-style quest guard:
- once completed, that quest is gone permanently
- other teams cannot complete it later

This means enemy teams may complete or invalidate player-relevant quests.

---

## 32. Quest states and player-facing quest log

### Technical quest state
In the technical sense, a quest-service quest is primarily either:

- **completed**
- or **not completed**

Player-facing visibility is a separate concern.

### Player-facing quest-log states
For the player, a quest-service quest may appear as:

- **Undiscovered**
- **Visible in log**
- **Completed**
- **Failed**

### Discovery
Quest-service quests do not really “start” in the broader systemic sense.

They already exist in the world.

A quest appears in the player's quest log once the player discovers the quest service and sees the quest.

### Failed quests
Failed quests remain visible in a **failed section** of the player's quest log.

A quest may fail because:
- another eligible team completed it first
- its completion conditions became impossible
- other authored failure logic declared it failed

### Quest-log behavior
The quest log should:
- show what needs to be done
- show where the quest service is on the map
- allow jumping to the service if the player is in the same Region

The quest log is distinct from both:
- the formal victory/defeat menu
- the guidance-text system

---

## 33. Objective structure

### Strong typing
Objective types should remain **strongly typed and finite**.

Avoid turning objectives into an open-ended condition language.

### Single-task quests
A quest is a **single task** technically.

A quest does not contain multiple objectives in the long-term system.

If a designer wants a larger arc, that should generally be expressed as:
- a quest chain
- events
- or victory/guidance structure layered on top

### Valid quest-style requirements
Examples of valid quest-service requirements include:
- bring X amount of a resource
- deliver a specific item or artifact
- defeat a specific team
- be a specific team
- have a specific hero
- defeat a specific neutral enemy troop
- satisfy some other explicitly typed objective

---

## 34. Events

Events are the primary systemic progression engine.

An event is defined by:
- **eligibility**
- **conditions**
- an optional **message**
- an optional **image**
- a chain of **event actions**

Event actions apply **immediately** when the event triggers.

### Event actions
Event actions are broader than ordinary rewards.

They may include things like:
- start a fight
- give or consume resources
- give or consume items or artifacts
- recover a team
- grant hero experience
- grant skills
- remove skills
- kill a specific unit
- give troops
- change alliances
- change ownership
- unlock Regions
- spawn or remove teams
- destroy or restore Services
- trigger victory
- trigger defeat
- set story flags
- update guidance

The exact action list should remain explicit and typed.

### Manual events
A **manual event** is triggered when an eligible team manually enters or uses a node or Service tied to that event.

Eligibility may check:
- team color
- human / AI
- time window
- required hero in traveling party
- combinations of the above

Manual events are **one-shot by default**.

A manual event may be marked repeatable if the designer wants it to trigger every time an eligible team enters the node.

### Automatic events
Automatic events have two main subtypes.

#### Day-based automatic events
These run automatically on a specific day number and may optionally repeat every `Y` days after that.

#### Condition-based automatic events
These check automatically at the start of the day whether certain conditions are true.

Typical conditions may include:
- team has item X
- team has artifact Y
- team has hero Z
- team has at least N of resource R
- story flag Q is set
- other typed checks

Condition-based automatic events may be:
- one-shot
- or repeatable while their conditions remain true

### Events and quest services
Quest completion at a quest service should be understood as triggering the quest’s event actions.

Quest services are therefore a specialized structure layered on top of the general event system.

---

## 35. Victory conditions

Victory conditions are **scenario-level rules**, separate from the quest system.

A victory condition may still depend on:
- completing a quest service
- defeating a specific team
- defeating a specific neutral enemy
- owning a resource amount
- holding a specific item or artifact
- delivering a specific item or artifact
- flagging all mines
- or any other explicitly supported typed requirement

### Win structure
A Scenario may have **one or more** victory conditions.

Only **one** victory condition needs to be satisfied for that team to win the Scenario.

This is an **OR** structure.

### Default victory rule
If the designer does not define another victory condition, the default intended victory rule is:

- **defeat all enemy teams**

If there are no enemy teams and no other victory conditions, the Scenario is effectively poorly designed, since it would resolve immediately.

### Shared victory competition
Victory conditions may be shared by multiple teams.

If a team satisfies a valid victory condition first, that team wins.

### Victory event
Winning the Scenario triggers a **victory event** for the winning team.

Victory event actions resolve immediately.

### Guidance can be bypassed
Authored guidance, quest chains, and story-like progression may be bypassed if a true victory condition is satisfied directly.

Example:
- if defeating the dragon is the real victory condition, a team that defeats the dragon immediately wins, even if the intended guidance chain suggested talking to Jon and finding the legendary sword first

This is intentional.

---

## 36. Defeat conditions

Defeat conditions are **scenario-level loss rules**, separate from quests.

### Loss structure
If **any** defeat condition becomes true, that team loses.

This is an **OR** structure.

### Typical defeat conditions
Examples include:
- fail to win within a time frame
- lose a specific hero
- lose allied team(s)
- have a certain Service destroyed
- let an enemy reach a Location
- let another team win first
- other authored loss rules

A Scenario may also have no special defeat condition beyond ordinary inability to win.

---

## 37. Guidance, journal, and player information

Keep these systems distinct.

### Scenario Info screen
The **Scenario Info screen** should clearly state:
- victory conditions
- defeat conditions

This is the formal rule view for the Scenario.

### Quest log / journal
The **quest log / journal** tracks discovered quest-service tasks and their player-facing states.

It should update when:
- a quest is discovered
- a quest is completed
- a quest fails
- other authored quest-service progress changes occur

### Guidance text
**Guidance text** is a separate event-driven hint layer.

It should:
- display directional text and optional icons
- persist until changed by another triggered event
- be hidable by the player
- guide play without replacing the formal victory/defeat menu

Guidance text is not the same thing as the actual victory structure.

---

## 38. Campaign transitions and carry-over

Campaign transitions are authored **per transition**, not controlled by one global campaign-wide carry-over rule.

### Carry-over model
Carry-over is chosen from an explicit allowed list for each scenario transition.

A transition may carry over:
- all
- none
- or a selected subset of legal categories

### Typical carry-over candidates
Candidates include:
- heroes
- generic troops from the traveling party
- items
- resources
- story flags
- hero level and attributes
- hero skills
- hero passive skills
- equipment / artifacts

### Hero carry-over rules
Only heroes that:
- are allowed by that transition
- and are in the traveling party

may carry over.

Hero-related carry-over should be separately controllable for:
- level + attributes
- skills
- passive skills

These are distinct transition flags.

### Story flags
Story flags should always be available as campaign carry-over data.

---

## 39. Temporarily Unavailable heroes

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

Quest-critical hero loss does not necessarily make a quest permanently impossible if that hero can later be recruited again.

---

## 40. Enemy-team defeat, persistence, and replacement

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

## 41. Node clearing outcomes

When temporary node content is cleared, the node becomes an empty travel node.

This applies to:
- one-time hostile encounters
- one-time resource / item pickups
- one-time blockers
- one-time Sealed / Frozen Hero services after the hero is freed

Nodes are not destroyed. Their state changes.

More drastic node destruction or structural world changes should happen only through authored events.

---

## 42. UI / UX rules and information presentation

This section defines cross-system UI expectations for the current vision. Ashvale's UI should be a **hybrid** between clean JRPG readability and HoMM-like strategic information density.

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
---

## 43. Agent / implementation guidance

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
- Treat **events** as the universal progression engine.
- Treat **quest services** as one specific authored structure built on top of events and typed objectives.
- Keep **victory conditions** and **defeat conditions** separate from the quest system.
- Keep **eligibility** and **condition** as separate concepts.
- Keep resources team-global within a Scenario unless a specific system says otherwise.
- Keep items and artifacts as separate shared inventories, with hero-only artifact equip slots.
- Treat trader services as distinct authored service categories with explicit rate tables.
- Do not introduce a dedicated permanent combat-node abstraction unless the design changes later.
- Do not assume allied control grants shared ownership or shared service rights.
- Keep PvP as a separate future mode unless a task explicitly focuses on it.

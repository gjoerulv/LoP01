# Game Vision Complete

This document describes the current intended long-term vision for Project Ashvale.

For terminology, see `docs/terminology_map.md`.

Detailed systemic rules live primarily in:
- `docs/combat_rules.md`
- `docs/core_loop_rules.md`
- `README_DECISIONS.md`

This file stays at the **vision** level. It explains the intended shape of the game and the major layers of play without restating every exact rule and formula.

---

## 1. Core fantasy

Project Ashvale is a turn-based strategy/RPG about surviving, organizing a traveling force, and pushing through hostile territory while building enough strength, allies, and positional advantage to win a Scenario.

The intended experience combines:
- authored world structure
- readable tactical combat
- meaningful roster consequence
- travel and logistics pressure
- enemy-team competition on the Region layer
- strong scenario identity

The player should feel like they are:
- moving through dangerous territory
- making meaningful travel and roster decisions
- interacting with authored places and factions
- competing with other active forces in the world
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

### Service
A **Service** is a functional interaction available either:
- directly on the Region layer
- or inside a Location

Services should feel like authored world functionality rather than generic menu abstractions.

---

## 3. Region structure vision

Regions use a **single-purpose node model**.

The main node categories are:

- **empty / travel node**
- **Location node**
- **single Service node**
- **blocker node**

There is no dedicated permanent combat-node type in the intended design.

Instead:
- a normal node may contain one-time hostile content
- a normal node may contain a one-time item or resource
- once that content is cleared, the node becomes an ordinary empty travel node

This keeps the Region layer understandable and content-driven.

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

### Auto-resolve-first design
On the Region layer, battles involving the player should first produce an auto-resolve outcome that the player may either:
- accept
- or replay manually

AI-vs-AI battles should auto-resolve without interrupting play directly.

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

## 10. Quest services, events, and scenario progression

The long-term progression model should not treat all authored progression as “quests.”

Instead, the design should clearly separate:

- **quest**
- **objective**
- **victory condition**
- **event**
- **guidance**

### Quest services
A quest service is a specific authored map service that offers optional tasks to eligible teams.

The intended feel is closer to a world structure than to a universal RPG journal abstraction:
- a service exists at a place
- a team discovers it by reaching it
- the service exposes one currently available quest at a time
- returning to that service matters
- the service may block, gate, toll, or simply offer optional work

A quest itself is a **single task**. Longer authored arcs should usually be expressed as:
- quest chains inside one quest service
- events
- or scenario-level guidance layered over the world

Quest services should support:
- starting message
- progress message
- completion message
- optional portrait / image support
- optional reward-like outcomes through event actions
- competition between teams when more than one team is eligible to complete them

Turn-in should remain intentional. If a team returns with a completed quest requirement, the completion message may present a **Yes / No** choice rather than forcing immediate completion.

### Events as the universal progression engine
The broader progression model should be **event-driven**.

Events are the general world-state mechanism for:
- changing alliances
- updating guidance
- unlocking Regions
- spawning or removing teams
- changing ownership
- destroying or restoring Services
- giving or removing resources, troops, or skills
- starting fights
- declaring victory or defeat
- updating story flags
- triggering other authored changes

This means quests are not the universal progression system.  
They are one authored structure built on top of the event system.

The intended split is:

- **quest** = a single authored task
- **objective** = a typed requirement used by a quest or victory condition
- **event** = a trigger / condition / effect structure that changes world state

### Eligibility and condition
The design should keep these concepts distinct:

- **eligibility** = who is allowed to participate or trigger
- **condition** = whether the actual requirement has been satisfied

This matters across:
- quest services
- manual events
- automatic events
- victory conditions
- defeat conditions

### Manual and automatic events
Events should support both:
- **manual triggers**, such as entering a node or using a service
- **automatic triggers**, such as day-based checks or condition checks at day start

Manual events should be one-shot by default unless explicitly marked repeatable.

Automatic events should be able to drive the world forward without requiring direct player interaction every time.

### Neutral encounters as progression hooks
Neutral hostile encounters on the Region layer are not only obstacles.

They may also act as progression hooks by triggering event-action chains when defeated, if the designer chooses to author them that way.

---

## 11. Victory, defeat, journal, and guidance

### Victory conditions
Victory conditions are **scenario-level rules**, not quests.

A Scenario may have one or more victory conditions, and only **one** needs to be satisfied for a team to win.

This allows:
- multiple possible win routes
- competitive wins between teams
- authored “intended path” structures that may still be bypassed if a true victory condition is met directly

The default intended fallback victory condition is:
- defeat all enemy teams

But stronger Scenario design should usually define more authored win structures than that.

### Defeat conditions
Defeat conditions are also **scenario-level rules**, separate from quests.

If any defeat condition becomes true, that team loses.

This allows Scenarios to express stakes such as:
- losing a key hero
- missing a time limit
- losing an allied team
- losing a critical Location or Service
- being beaten to victory by another team

### Guidance is not the same as victory
The player should have multiple layers of information:

- **Adventure menu** = formal victory and defeat rules
- **Quest log / journal** = discovered quest-service tasks and their states
- **Guidance text** = event-driven directional hinting

Guidance should:
- be persistent until replaced
- be hidable
- help the player understand the intended next step
- not override the actual rule structure of the Scenario

This is important because the intended path may be different from the true shortest path to victory.

### Failed quests
Failed quest-service tasks should remain visible in a failed section of the player’s log.

That preserves world memory and makes competitive loss or invalidated opportunities feel understandable rather than silently erased.

---

## 12. Campaign transitions and carry-over

Campaign transitions should be authored **per transition**, not governed by one broad global carry-over rule.

The intended model is:
- every scenario transition chooses what may carry over
- carry-over comes from an explicit allowed list
- different branches may preserve different things

Candidate carry-over categories include:
- heroes
- generic troops from the traveling party
- items
- resources
- story flags
- hero level and attributes
- hero skills
- hero passive skills
- equipment / artifacts

This supports both:
- highly continuous campaigns
- and more authored, controlled chapter transitions

Story flags should be a first-class carry-over concept because they are essential to branching narrative and authored continuity.

---

## 13. Locations, dungeons, and safe anchors

### Locations
Locations are where authored place identity can flourish.

They should justify entry through:
- NPC interaction
- multiple Services
- story scenes
- more textured presentation
- dungeon or settlement identity

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

The player should:
- see known and currently revealed parts of a Region
- infer danger from enemy motion and ownership pressure
- discover more through movement and exploration

Enemy movement and AI-vs-AI outcomes should only be visible where the Region is revealed.

This supports both:
- strategic uncertainty
- and a stronger sense that other teams are acting in the world whether or not the player sees every detail

---

## 15. Scenario identity and authored control

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

The systems should create interaction, but the Scenario should still feel intentional.

---

## 16. Implementation posture

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

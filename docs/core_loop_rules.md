# Core Loop Rules

This document describes the current design baseline for the game loop.

Use these terms consistently:
- **World Map** = the scenario-level map used to inspect and select Regions
- **Region** = the main node-and-route travel space inside a scenario
- **Location** = an entered place inside a Region
- **Service** = a functional interaction that can exist directly in a Region or inside a Location
- **Traveling party** = Active Party + Reserve

When implementation is behind the settled design, treat the codebase as the source of truth for current runtime behavior and this document as the source of truth for intended loop rules going forward.

## Day structure

- Day starts at **06:00**
- Day ends at **02:00** the following night
- If the player fails to sleep in a valid bed before **02:00**, apply the wake-up penalty on the next day
- Gold can never go below **0**

## Wake-up penalty

If the player:
- is fully defeated
- or fails to reach valid sleep before **02:00**

then:
- the next day starts at **11:00**
- the player wakes at the appropriate fallback recovery location for the current context
- the player loses **1000 gold**, but not below **0**

## Valid sleeping locations

Current baseline:
- inns
- authored safe-haven rest points
- a scenario's home-base-like safe anchor, if one exists

Design direction:
- inns should generally be usable if present and should cost money
- home-base-like rest should generally remain free when available
- not every scenario or Region is required to provide a true safe haven

## World Map

The **World Map** is the scenario-level Region selection and planning layer.

Rules:
- the World Map can be opened at any time **outside battle**, as long as the player is inside a scenario
- the World Map can still be opened when travel is currently illegal; in those cases, it is informational only
- travel action is disabled whenever cross-region travel is not legal
- Regions are unlocked gradually through scenario progression and authored availability rules

## Cross-region travel

Cross-region travel follows these rules:
- travel is initiated from the **Region** layer, not from inside a Location
- if the player is inside a dungeon, cross-region travel is not allowed
- cross-region travel must begin before **11:00**
- if the current time is **11:00 or later**, cross-region travel is disabled until the next day
- cross-region travel costs **time only**
- cross-region travel does not consume gold or generic resources by default
- pathing uses the **shortest valid path** through currently enterable Regions whose borders connect
- if no contiguous valid path exists, travel is impossible
- travel time is calculated as **1 day + the number of Regions between the current Region and the destination Region**
- all Region steps count equally for cross-region travel time
- arrival always happens at **11:00** on the arrival day

Examples:
- directly adjacent Regions take **1 day** to travel between
- if there are **2 Regions between** the current Region and the destination Region, travel time is **3 days**

## Region arrival

Each Region must define a designated **arrival node**.

Rules:
- the arrival node is chosen by the scenario designer
- the arrival node may be any node type that makes sense for the scenario
- the arrival node may not contain an enemy
- enemies may not travel onto or spawn onto the arrival node

This guarantees that cross-region travel always resolves into a safe, predictable entry point.

## Traveling party and cross-region persistence

The **traveling party** means:
- **Active Party**: up to 5 units
- **Reserve**: up to 7 units

Cross-region travel does **not** preserve all traveling units equally.

Rules:
- all heroes in the traveling party cross Regions with the player
- generic units in the traveling party do **not** cross Regions with the player
- generic traveling units are lost on Region change unless they were stored beforehand
- the player must be warned clearly before confirming Region travel that would discard generic traveling units
- stored units remain at their assigned storage service and persist in their original Region
- a Region may contain multiple independent storage services

## Region mode

The player does not move freely across a Region map.
The player selects destination nodes and travels along authored legal routes.

Current baseline:
- travel follows the authored route graph inside the Region
- travel time is quantized to **15-minute** steps in the current slice
- entering or using the same Location the player is already in costs **0 minutes**
- the player cannot confirm travel that would arrive past **02:00**
- blocker nodes can prevent transit until their gating condition is cleared
- if the player is trapped only because of the late-night cutoff, the wake-up-penalty flow should resolve the dead-end rather than soft-lock the loop

Destination nodes may include:
- enterable Locations
- inns or service nodes
- recruit points
- buff points
- combat nodes
- dungeons
- multi-purpose Locations containing multiple Services
- placeholder nodes that may later contain a resource, event, or fight

## Location mode

Walking inside a Location is free.

Current baseline time costs:
- opening a door / entering a building = **1 minute**
- scene transitions = **1-5 minutes**, depending on the transition
- each chosen dialogue option = **1 minute**
- shopping transaction = **5 minutes**
- recruiting = **10 minutes**
- cutscenes = varies; may take **1 minute to several days** depending on the scene

Location rules:
- Locations are entered from a parent Region
- a Location may contain zero or more Services
- Services are performed through interactions with NPCs or appropriate objects
- cross-region travel cannot be initiated from inside a Location; the player must first return to the Region layer
- the player can open an out-of-battle menu here for party, inventory, skills, and other legal management actions

## Defeat handling

If the player party is fully defeated:
- wake at the appropriate fallback recovery location
- lose **1000 gold**, minimum **0**
- the next day starts at **11:00**

If the player character is KO'd in battle but the party still wins:
- set player HP to **1** after battle

## Temporarily Unavailable heroes

When a hero becomes **Temporarily Unavailable**:
- the hero is removed from the active party
- the hero is removed from reserve
- the hero is removed from storage
- the hero is not recruitable while Temporarily Unavailable
- the hero returns to the scenario hero pool at the start of the next week

This state can be caused by battle outcome or by voluntary dismissal, although player-facing narrative text may differ.

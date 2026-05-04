# Content Scope v0

This document defines the intended bounded content scope for the current playable slice.

It is a **scope cap**, not the full long-term vision.  
For current design truth, see:

- `docs/game_vision.md`
- `docs/core_loop_rules.md`
- `docs/combat_rules.md`
- `README_DECISIONS.md`
- `docs/terminology_map.md`

## Scope goal

The purpose of v0 scope is to keep the game:

- playable
- testable
- content-driven
- small enough to reason about
- rich enough to support meaningful iteration

The current slice should prove the game’s main loop, not deliver the full game.

## World scope

### Scenario scope
The slice should assume:

- **1 Scenario**
- **1 World Map**
- **1 Region**

Even though the long-term design now supports multiple Regions, branching Campaigns, team competition, and more advanced Scenario progression, the bounded v0 slice should still stay small.

### Region scope
The slice should support:

- 1 authored Region graph
- a small set of connected nodes
- a protected arrival node
- a few route-quality variations
- a few meaningful node-content roles

Suggested Region-node distribution:
- several empty / travel nodes
- a few nodes with Location content
- a few nodes with direct Service content
- a few pieces of content that create blocker behavior
- a few temporary hostile or temporary resource interactions on normal nodes

The slice should **not** assume or require a dedicated combat-node type.

## Location scope

The slice should include only a small number of entered Locations.

Suggested target:
- **2 to 4 Locations**

Each Location may contain:
- one or more screens
- a few interactables
- a few Services
- some authored flavor or story content

At least one Location should demonstrate:
- safe-anchor behavior

At least one Location may demonstrate:
- dungeon-like structure

Entering and exiting a Location should not itself cost time.

## Service scope

The slice should use a small but representative set of Services.

Suggested v0 service coverage:
- rest / recovery
- generic-unit recruitment
- hero recruitment
- storage
- quest service
- one owned-resource service such as a mine
- one special authored service such as Sealed / Frozen Hero or sanctuary

The slice does **not** need every long-term service type at once.

## Battle scope

The slice should include enough battle content to prove:

- static formation CTB
- leader legality
- aura baseline
- persistence of hero and generic-unit consequences
- active party integration
- battle write-back into roster state

Suggested battle-content scope:
- a few allied units
- a few enemy templates
- a few hostile encounters
- a few battle scenarios
- at least one hero-led enemy force
- at least one neutral hostile encounter

## Party and roster scope

The slice should include enough roster content to prove:

- active party
- reserve
- storage
- recruit flow
- mustering / party reorganization
- hero persistence and temporary unavailability
- generic stack loss and later replenishment

The slice does **not** need the full long-term range of party-management UI or every possible service interaction yet.

## Enemy-team scope

The current v0 slice may still keep enemy-team simulation modest, but content and architecture should not block the now-settled long-term design.

Suggested v0 enemy-team scope:
- 1 to 3 enemy teams in the Region
- simple authored personalities / aggression setups
- node occupation
- basic movement
- at least one case of team competition for resources, ownership, or access

The slice does not need every sabotage behavior fully surfaced at once, but should not contradict that direction.

## Progression scope

The slice should prove the progression model in a small form.

Suggested v0 coverage:
- a few events
- at least one quest service
- at least one quest chain
- visible guidance text updates
- at least one explicit victory condition
- at least one explicit defeat condition
- at least one event-driven world-state change

The slice should not reduce progression to only “reach node” or “clear combat node” logic anymore.

## Recommended bounded target

A good current v0 target is something roughly like:

- 1 Scenario
- 1 World Map
- 1 Region
- 8 to 15 nodes
- 2 to 4 Locations
- 5 to 8 meaningful Services across Region + Locations
- 1 safe anchor
- 1 storage service
- 1 quest service
- 1 hero-recruit service
- 1 generic-unit recruit service
- 1 owned-resource service
- 1 to 3 enemy teams
- a handful of temporary hostile encounters
- a handful of battle scenarios
- a few quests / events / victory-defeat rules

This is intentionally small enough to iterate on safely.

## Out of scope for v0

The following are intentionally not required for the bounded v0 slice:

- multi-Region scenario content
- full Campaign branching implementation
- full PvP support
- complete endgame economy
- every long-term service type
- every sabotage behavior
- complete AI sophistication
- exhaustive faction / diplomacy variety
- fully realized story presentation
- full UI polish across all systems

The slice should leave room for those later without pretending to implement them now.

## Design principle

v0 should prove that the game can support:

- tactical battle
- travel and logistics pressure
- meaningful roster consequence
- authored places
- contested Region play
- event-driven progression

without letting the current slice bloat faster than it can be tested and maintained.

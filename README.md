# Project Ashvale

Ashvale is a turn-based strategy/RPG built as a content-driven C++/raylib project.

The current repository should be treated as a **post-M8 baseline** with a clarified long-term design direction. The codebase is still a bounded playable slice, but the design language and planning posture are now much more explicit.

For terminology, see `docs/terminology_map.md`.

## Design truth and doc priority

Use these files as the main current design references:

- `docs/game_vision.md`
- `docs/game_shell_flow.md`
- `docs/presentation_game_feel.md`
- `docs/core_loop_rules.md`
- `docs/combat_rules.md`
- `docs/scenario_authoring.md`
- `docs/validation_system.md`
- `docs/content_schema.md`
- `README_DECISIONS.md`
- `docs/terminology_map.md`

When older source names, content keys, prompts, or serialized values still use legacy wording such as `overworld`, prefer the terminology in `docs/terminology_map.md` for design and planning.

## Implementation planning

Current implementation sequencing lives in:

- `docs/implementation_roadmap.md`

Use this roadmap for milestone order and explicit “not yet” boundaries. Use the design docs above as the source of truth for behavior.

## Project identity

Ashvale is intended to combine:

- authored world structure
- readable tactical battle
- meaningful roster consequence
- travel and logistics pressure
- competitive Region-layer world simulation
- strong Scenario identity
- content-driven progression through events, quest services, victory conditions, and defeat conditions

The player should feel like they are moving through contested territory while managing:
- a traveling party
- scarce time and Energy
- risky Region travel
- hero availability
- storage and logistics
- enemy-team pressure
- scenario-specific progression and victory routes

## Current architecture baseline

The current codebase follows these principles:

- explicit app shell and gameplay session flow
- controller / mapper / renderer split
- gameplay logic separated from rendering and input
- typed content loaded from JSON
- content-driven Regions, Locations, Services, units, enemy groups, battles, quests, and scenarios
- pure or mostly-pure gameplay rules where practical
- save/load focused on gameplay state rather than presentation state

Future work should preserve this baseline unless there is a strong reason to refactor it.

## World structure baseline

Use this world hierarchy:

- **Campaign**
- **Scenario**
- **World Map**
- **Region**
- **Node**
- **Location**
- **Service**

### Campaign
A Campaign is a collection of connected Scenarios and may branch.

### Scenario
A Scenario is the top-level authored play unit.

### World Map
The World Map is the scenario-level Region selection layer.

### Region
A Region is the main in-scenario travel space.

### Location
A Location is an entered place inside a Region.

### Service
A Service is a functional interaction available either directly in a Region or inside a Location.

## Current gameplay layers

The current runtime and design posture revolve around these layers:

- **World Map**
- **Region**
- **Location**
- **Battle**
- **Progression**

In older serialized or runtime-compatible contexts, some legacy strings may still use names such as:
- `overworld_selection`
- `overworld_mode`

Those should be interpreted as legacy compatibility terms rather than preferred design language.

## Region and node baseline

The current intended Region model is:

- Regions are authored node graphs
- systemic rules operate on top of that authored structure
- Regions use a **node-content model**

A node is fundamentally a travel point. Its gameplay behavior is determined by its main node content and any attached events.

A node may contain at most one main content item, such as:

- resource pickup
- artifact pickup
- Service
- neutral enemy
- one-time special content

There is **no dedicated permanent combat-node type** in the current intended design.

Blocker behavior is usually created by content, such as a gate service, neutral enemy, hostile team occupation, or authored rule. It is not primarily a separate node type.

Temporary hostile content, temporary resources, one-time blockers, and one-time Sealed / Frozen Hero services may all resolve back into empty travel nodes after use or clearing.

## Travel and logistics baseline

### Traveling party
The traveling party is:
- active party
- plus reserve

### Active party
- up to 5 units
- battle-legal
- exactly one assigned leader

### Reserve
- up to 7 units
- travels with the player

### Stored units
- tied to a specific storage Service
- up to 7 per storage
- do not travel with the player
- persist in the Region where stored

### Region travel
Region-to-Region travel:
- happens from the World Map
- requires **1000 Energy**
- consumes days based on shortest valid Region path
- must begin before 11:00
- sends the player to a protected arrival node

Heroes in the traveling party cross Regions.  
Generic units in the traveling party do not survive Region change unless stored first.

### Energy
Energy is a shared traveling-party resource.

Daily starting Energy is:

`1000 + (lowest traveling-party agility × 100) + leader passive bonus + leader item bonus`

Energy can be restored through:
- rest
- Services
- items
- events

## Battle baseline

Battle is now a settled post-M8 baseline, not a loose prototype.

Key assumptions:

- static formation CTB
- up to 5 units per side
- `Leader` is one of the 5 slots
- only hero units may be leaders in current intended design
- targeting is free
- position affects agility penalty, not target legality
- battle is mostly deterministic, with core randomness limited to damage roll
- hero HP persists between battles
- generic stack HP resets
- generic stack count persists
- MP persists for all units

For exact battle rules, see `docs/combat_rules.md`.

## Enemy-team baseline

Enemy teams are AI-controlled traveling parties on the Region layer.

They are:
- authored in setup
- systemic in behavior

Enemy teams may:
- move inside the current Region
- occupy nodes
- attack hostile teams
- recruit
- rest
- use direct Region Services
- compete for heroes and units
- sabotage by destroying services, attacking storage, and contesting ownership

They do not:
- enter Locations
- travel between Regions
- use the player’s wake/sleep penalty model

The game supports up to 8 total teams per Region, including the player.

## Progression baseline

Progression is no longer treated as “simple quest progression.”

The intended structure is:

- **Events** are the universal progression engine
- **Quest services** are one specific authored service structure layered on top of events
- **Victory conditions** are Scenario-level rules outside the quest system
- **Defeat conditions** are Scenario-level rules outside the quest system
- **Guidance text** is an event-driven hint layer, separate from both the quest log and formal victory/defeat rules

### Quest services
Quest services:
- expose at most one currently available quest from a chain at a time
- always resolve through turn-in
- may be competitive between teams
- may disappear, persist, block, or be repeatable depending on authored type

### Events
Events are the core trigger / condition / effect system for world-state change.

They may:
- start fights
- grant or remove resources, troops, skills, or experience
- change alliances
- unlock Regions
- spawn or remove teams
- destroy or restore Services
- trigger victory or defeat
- update guidance

### Victory and defeat
A Scenario may have:
- one or more victory conditions
- one or more defeat conditions

Only one victory condition needs to be satisfied to win.  
Any defeat condition becoming true causes loss.

## Current milestone status

Milestone 8 is complete and merged to `main`.

M8 established:

- canonical persistent roster state
- active party size = 5
- recruit flow integrated with persistent roster
- reserve and mustering integrated into persistent party logic
- battle initialization from the active party
- persistent battle write-back
- leader / aura baseline integration
- save/load migration support for earlier slice formats

Since then, the docs and terminology have been clarified to cover:

- World Map / Region terminology
- battle system baseline
- Region / node / Location / Service structure
- enemy teams / sabotage / ownership
- quest services / events / victory / defeat / guidance
- campaign transition carry-over
- terminology mapping for legacy names

## Current planning posture

The repo should currently be treated as:

- **post-M8 implementation baseline**
- **more advanced design baseline than the current runtime in some areas**
- **content-driven project with explicit terminology and clarified planning posture**

Future work should:

- preserve architecture clarity
- preserve gameplay / rendering separation
- avoid reintroducing old terminology in design-facing docs and prompts
- tighten vision first where a major system remains ambiguous
- keep changes bounded
- maintain save/load awareness
- treat the current docs as stronger design truth than older historical milestone prompts

## Folder structure

- `src/app`
  - app shell
  - mode transitions
  - controllers
  - render-model mappers

- `src/core`
  - time/day rules
  - save/load serialization

- `src/data`
  - JSON loading
  - content repositories
  - typed content definitions

- `src/gameplay`
  - gameplay state and rules
  - Region/world logic
  - battle runtime
  - location runtime
  - progression systems
  - roster / party state

- `src/rendering`
  - renderers
  - HUD
  - debug overlays
  - presentation helpers

- `content`
  - JSON gameplay content

- `tests`
  - logic-focused tests and slice coverage

- `docs`
  - vision, rules, terminology, and planning docs

## Build

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

## Run all tests
```
ctest --test-dir build --output-on-failure
```

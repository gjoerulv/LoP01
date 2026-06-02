# Project Ashvale

Ashvale is a turn-based strategy/RPG built as a content-driven C++20/raylib/CMake project. The repository is a post-M17 bounded multi-Region, multi-Scenario playable vertical slice with a clarified long-term design direction. Current implementation status, milestone sequencing, and explicit not-yet boundaries live in `docs/implementation_roadmap.md`. For terminology, see `docs/terminology_map.md`.

## Design truth and doc priority

Use these files as the main current design references:

- `docs/implementation_roadmap.md`
- `docs/content_scope_v1.md`
- `docs/technical_direction.md`
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

Archived docs, including `docs/content_scope_v0.md.archived` and `docs/implementation_roadmap.md.00.archived`, are historical context only.

## Implementation planning

Current implementation sequencing lives in:

- `docs/implementation_roadmap.md`

Use this roadmap for milestone order and explicit not-yet boundaries. Use the design docs above as the source of truth for behavior. The current next planned milestone is M18: Passive Effect Spine, unless the user explicitly redirects.

## Project identity

Ashvale is intended to combine:

- authored world structure;
- readable tactical battle;
- meaningful roster consequence;
- travel, Energy, service, and logistics pressure;
- competitive Region-layer world simulation;
- strong Scenario identity;
- content-driven progression through events, quest services, victory conditions, and defeat conditions;
- compact strategic economy through owned services, mines, resources, trader services, stationed guards, and passive effects.

The player should feel like they are moving through contested territory while managing:

- a traveling party;
- scarce time and Energy;
- risky Region travel;
- hero availability;
- storage and logistics;
- resources and owned services;
- enemy-team pressure;
- scenario-specific progression and victory routes.

## Current architecture baseline

The current codebase follows these principles:

- explicit app shell and gameplay session flow;
- controller / mapper / renderer split;
- gameplay logic separated from rendering and input;
- typed content loaded from JSON;
- content-driven Regions, Locations, Services, units, enemy groups, battles, quests, events, outcomes, items, artifacts, World Map, scenarios, campaigns, owned services, resources, mine outputs, unit mine-production passives, and trader ownership curves;
- pure or mostly-pure gameplay rules where practical;
- save/load focused on gameplay state rather than presentation state.

Future work should preserve this baseline unless there is a strong reason to refactor it.

## Current implementation baseline

Completed foundations include:

- battle engine, CTB, static formation, leader aura, deterministic damage, and battle write-back;
- persistent roster, active/reserve party, mustering, and save/load;
- daily clock, Region travel, wake/recovery penalty, and basic services;
- content validation foundation;
- typed event foundation;
- practical enemy-team Region-layer foundation, including patrol movement, hostile occupation, hostile-contact battles, event mutation actions, and save/load;
- deterministic scenario outcome rules with authored victory and defeat conditions;
- inventory and artifact foundation, including per-hero equipment and equipped-artifact battle stat bonuses;
- team Energy pool with daily-starting formula, spend/recover, day-rollover reset, save/load, and HUD exposure;
- minimal World Map region-to-region travel from authored exit nodes;
- minimal Campaign System with thin scenarios, transition graph, explicit allow-list carry-over, campaign state, and campaign selection;
- owned-service and economy foundation with resource pool, owned-service runtime state, mine outputs, stack-backed stationing, narrow mine-production passives, day-boundary mine payout, trader ownership tiers, authored/default trader curves, and validation/test coverage.

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
- **Campaign**

In older serialized or runtime-compatible contexts, some legacy strings may still use names such as:

- `overworld_selection`
- `overworld_mode`

Those should be interpreted as legacy compatibility terms rather than preferred design language.

## Region and node baseline

The current intended Region model is:

- Regions are authored node graphs.
- Systemic rules operate on top of that authored structure.
- Regions use a node-content model.
- A node is fundamentally a travel point.
- Its gameplay behavior is determined by its main node content and any attached events.
- A node may contain at most one main content item, such as:
  - resource pickup;
  - artifact pickup;
  - Service;
  - neutral enemy;
  - one-time special content.

There is no dedicated permanent combat-node type in the current intended design. Blocker behavior is usually created by content, such as a gate service, neutral enemy, hostile team occupation, or authored rule. It is not primarily a separate node type. Temporary hostile content, temporary resources, one-time blockers, and one-time Sealed / Frozen Hero services may all resolve back into empty travel nodes after use or clearing.

## Travel and logistics baseline

### Traveling party

The traveling party is:

- active party;
- plus reserve.

### Active party

- up to 5 units;
- battle-legal;
- exactly one assigned leader.

### Reserve

- up to 7 units;
- travels with the player.

### Stored units

- tied to a specific storage Service;
- up to 7 per storage;
- do not travel with the player;
- persist in the Region where stored.

### Region travel

Region-to-Region travel:

- happens from the World Map;
- requires 1000 Energy;
- consumes days based on shortest valid Region path;
- must begin before 11:00;
- sends the player to a protected arrival node.

Heroes in the traveling party cross Regions. Generic units in the traveling party do not survive Region change unless stored first.

### Energy

Energy is a shared traveling-party resource.

Daily starting Energy is:

`1000 + (lowest traveling-party agility * 100) + leader passive bonus + leader item bonus`

Energy can be restored through:

- rest;
- Services;
- items;
- events.

The leader passive and leader item/artifact terms are currently zero-valued seams until the relevant passive/effect systems exist.

## Battle baseline

Battle is now a settled baseline, not a loose prototype.

Key assumptions:

- static formation CTB;
- up to 5 units per side;
- `Leader` is one of the 5 slots;
- only hero units may be leaders in current intended design;
- targeting is free;
- position affects agility penalty, not target legality;
- battle is mostly deterministic, with core randomness limited to damage roll;
- hero HP persists between battles;
- generic stack HP resets;
- generic stack count persists;
- MP persists for all units.

For exact battle rules, see `docs/combat_rules.md`.

## Enemy-team baseline

Enemy teams are AI-controlled traveling parties on the Region layer. They are:

- authored in setup;
- systemic in behavior.

Enemy teams may:

- move inside the current Region;
- occupy nodes;
- attack hostile teams;
- recruit;
- rest;
- use direct Region Services;
- compete for heroes and units;
- sabotage by destroying services, attacking storage, and contesting ownership.

They do not:

- enter Locations;
- travel between Regions;
- use the player's wake/sleep penalty model.

The game supports up to 8 total teams per Region, including the player.

## Owned service and economy baseline

Owned services are strategic objects controlled by a team. Implemented foundation and settled direction:

- Mines can be owned and generate passive daily resources for the owning team.
- Stationed guards can increase mine output only through explicit passive effects.
- Mine production passives do not stack.
- For each owned service instance and output resource, only the strongest applicable stationed passive counts.
- Ties do not stack.
- Heroes and generic units are both valid if they have the applicable passive.
- Trader services can be owned: Trading Post, Market, Freelancer's Guild, Black Market.
- Ownership benefits are per service type.
- Owning more Markets affects Market pricing only; owning more Trading Posts affects Trading Post rates only.
- Benefits apply only when the owning team uses a service of the same type that it owns.
- Allied-owned services do not count.
- Ownership tiers cap at 8 owned services of the same type.
- Resource exchange uses an authored matrix per tier where supported.
- Other services use service-type-specific authored/default curves.
- Ownership does not bypass lock, destruction, hostile occupation, stock, eligibility, story, or service availability rules.

The current runtime implements the narrow owned-service/economy foundation. Full trader UI, full item-market behavior, AI economy, ownership transfer loops, and broad passive/skill systems remain deferred.

## Progression baseline

Progression is no longer treated as simple quest progression. The intended structure is:

- **Events** are the universal progression engine.
- **Quest services** are one specific authored service structure layered on top of events.
- **Victory conditions** are Scenario-level rules outside the quest system.
- **Defeat conditions** are Scenario-level rules outside the quest system.
- **Guidance text** is an event-driven hint layer, separate from both the quest log and formal victory/defeat rules.

### Quest services

Quest services:

- expose at most one currently available quest from a chain at a time;
- always resolve through turn-in;
- may be competitive between teams;
- may disappear, persist, block, or be repeatable depending on authored type.

### Events

Events are the core trigger / condition / effect system for world-state change.

They may:

- start fights;
- grant or remove resources, troops, skills, or experience;
- change alliances;
- unlock Regions;
- spawn or remove teams;
- destroy or restore Services;
- trigger victory or defeat;
- update guidance.

### Victory and defeat

A Scenario may have:

- one or more victory conditions;
- one or more defeat conditions.

Only one victory condition needs to be satisfied to win. Any defeat condition becoming true causes loss.

## Current planning posture

The repo should currently be treated as:

- a bounded playable implementation slice;
- a broader documented design baseline than the current runtime;
- a content-driven project with explicit terminology and clarified planning posture;
- a post-M17 foundation ready for M18 planning after doc/source consistency checks pass.

Future work should:

- preserve architecture clarity;
- preserve gameplay / rendering separation;
- avoid reintroducing old terminology in design-facing docs and prompts;
- keep implementation slices bounded;
- maintain save/load awareness;
- use `docs/implementation_roadmap.md` for sequencing and not-yet boundaries;
- use `docs/content_scope_v1.md` as the active content-scope cap;
- treat current design docs as stronger design truth than archived milestone prompts.

## Folder structure

- `src/app` — app shell, mode transitions, controllers, render-model mappers.
- `src/core` — time/day rules, save/load serialization.
- `src/data` — JSON loading, content repositories, typed content definitions.
- `src/gameplay` — gameplay state and rules, Region/world logic, battle runtime, location runtime, progression systems, roster / party state, economy foundations.
- `src/rendering` — renderers, HUD, debug overlays, presentation helpers.
- `content` — JSON gameplay content.
- `tests` — logic-focused tests and slice coverage.
- `docs` — vision, rules, terminology, and planning docs.

## Build

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

## Run all tests

```bash
ctest --test-dir build --output-on-failure
```

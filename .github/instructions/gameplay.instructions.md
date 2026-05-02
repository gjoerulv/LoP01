---
applyTo: "src/gameplay/**/*.cpp,src/gameplay/**/*.h,src/data/**/*.cpp,src/data/**/*.h,docs/**/*.md"
---

# Gameplay implementation instructions

- Treat the repository and the current design docs as the source of truth.
- Respect the time and travel rules exactly as described in `docs/core_loop_rules.md`.
- Respect battle logic exactly as described in `docs/combat_rules.md`.
- Respect the current high-level design intent in `docs/game_vision.md`.
- Use `docs/terminology_map.md` when legacy runtime or content names differ from current design language.

## Terminology

- Use current design terminology in discussion and new docs:
  - `World Map` = the scenario-level region selection and information layer
  - `Region` = the main in-scenario travel layer
  - `Location` = an entered place inside a Region
  - `Service` = an interaction available directly in a Region or inside a Location
- Runtime code or serialized values may still contain legacy names. Do not broaden or reinforce outdated terminology in new design work.

## Layer separation

- Keep the four major gameplay layers distinct:
  - `World Map`
  - `Region`
  - `Location`
  - `Battle`
- Do not collapse Region mode and Location mode into one system.
- Battles must remain a separate reusable module shared by Region and Location encounters.
- Do not assume a Location is just a flavored Region node. Entering a Location is a real layer transition.

## Region structure

- Regions are authored node graphs with systemic rules on top.
- Keep Region nodes single-purpose.
- Use the current node model:
  - empty/travel node
  - Location node
  - single Service node
  - blocker node
- Do not introduce a dedicated combat-node abstraction.
- A node may temporarily contain a resource or a single hostile encounter, but once cleared it becomes an empty travel node.
- `Arrival` is a flag, not a node type.
- Arrival nodes may also be Location or Service nodes, but enemies may not spawn on them or occupy them.

## Location and Service rules

- A direct Service node is for quick functional interaction on the Region layer.
- A Location is for entered, explorable spaces with zero or more services, NPCs, screens, and possible dungeon-like content.
- Dungeons are a kind of Location.
- Entering and exiting a Location does not cost time.
- Region travel must be initiated from the Region layer, not from inside a Location.

## Region travel and Energy

- Travel inside a Region uses the shortest valid reachable path.
- The player may select any reachable node directly.
- Same-node travel is not a meaningful move and should not be treated as normal travel.
- Blocked nodes remain visible even when travel to them is illegal.
- Routes have terrain or road quality, and route quality affects both travel time and Energy cost.
- Energy belongs to the traveling party, not to individual units.
- Daily starting Energy is based on the current traveling party rules in `docs/core_loop_rules.md`.
- World Map travel uses the shortest valid contiguous path through enterable Regions and costs the settled one-time Energy amount when travel begins.
- Do not reintroduce the old "one region-travel allowance per day" rule.

## Party and storage assumptions

- Respect the settled party model:
  - `Active party` = the current battle-legal party, up to 5 units
  - `Reserve` = additional traveling units, up to 7
  - `Traveling party` = active party + reserve
  - `Stored units` = units tied to a specific storage service and not traveling with the player
- Hero units and stackable generic units must continue to be modeled differently.
- Do not assume traveling generic units survive Region transfer.
- Do not assume stored units are interchangeable with reserve units.
- Storage may exist in a Location or as a direct Region service.
- A direct storage service may act as a defensible gate for the owning side.

## Enemy teams

- Enemy teams are Region-layer traveling parties that follow the same broad party rules as the player side.
- Enemy teams may contain heroes and generic units.
- Enemy teams do not enter Locations.
- Enemy teams may occupy Region nodes, use direct Region services, recruit, rest, and attack defensible storage gates.
- If an enemy team occupies a Location node, the Location is inaccessible until that team is defeated.
- When design docs and current runtime disagree on future enemy-team behavior, preserve current behavior unless the task explicitly includes a design-alignment refactor.

## General implementation guidance

- Keep game rules data-driven where practical.
- Prefer explicit mode-entry helpers for World Map, Region, Location, and Battle transitions instead of chaining generic mode advancement.
- Prefer a single source of truth for status or event text; remove duplicate shadow strings once event-driven text exists.
- Player defeat, sleep-failure penalties, temporary hero unavailability, and post-battle fallout must stay consistent with the current design docs.
- Do not invent major story elements that contradict `docs/game_vision.md`.
- Prefer a playable incomplete feature over a broad unfinished feature.
- When terminology migration is the task, keep it behavior-preserving and avoid unrelated gameplay refactors.
- When a historical milestone doc conflicts with the current design docs, follow the current design docs.

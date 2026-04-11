---
applyTo: "src/gameplay/**/*.cpp,src/gameplay/**/*.h,src/data/**/*.cpp,src/data/**/*.h,docs/**/*.md"
---

# Gameplay implementation instructions


## Terminology note

Use `docs/terminology_map.md` when interpreting gameplay-layer terms.

In current design language:
- **World Map** is the scenario-level region selection layer
- **Region** is the main in-scenario travel space
- **Location** is an entered place inside a Region
- **Service** is an interaction available in a Region or Location
If older runtime or content names still use `overworld` terminology, interpret them through `docs/terminology_map.md` rather than treating them as current design truth.

- Treat the repository and the current design docs as the source of truth.
- Respect the time system exactly as described in `docs/core_loop_rules.md`.
- Respect battle logic exactly as described in `docs/combat_rules.md`.
- Respect the current high-level design intent in `docs/game_vision_complete.md`.
- Use current design terminology in discussion and new docs:
  - `World Map` = the scenario-level region selection layer
  - `Region` = the main in-scenario travel layer
  - `Location` = an entered place inside a region
  - `Service` = an interaction available in a region or inside a location
- Runtime code may still contain legacy names or serialized strings. Do not broaden or reinforce outdated terminology in new design work.

- Keep the three major layers distinct:
  - `World Map`
  - `Region`
  - `Location`
- Do not collapse Region mode and Location mode into one system.
- Battles must remain a separate reusable module shared by region and location encounters.

- Respect the settled traveling-party model:
  - `Active party` = the current battle-legal party, up to 5 units
  - `Reserve` = additional traveling units, up to 7
  - `Traveling party` = active party + reserve
  - `Stored units` = units tied to a specific storage service and not traveling with the player
- Hero units and stackable generic units must continue to be modeled differently.
- Do not assume traveling generic units survive region transfer.
- Do not assume stored units are interchangeable with reserve units.
- Do not assume cross-region party state works the same for heroes and generic units.

- Respect the World Map / Region travel rules:
  - the World Map can be opened anytime outside battle while inside a scenario
  - region travel must be initiated from the Region layer
  - travel is disabled when illegal, rather than making the World Map unavailable
  - region travel uses the shortest valid contiguous path through enterable regions
  - arrival occurs at the region's designated arrival node
- Do not reintroduce the old "one region-travel allowance per day" rule.
- Do not assume only the player character travels between regions.

- Keep game rules data-driven where practical.
- Prefer explicit mode-entry helpers for battle, location, region, and World Map transitions instead of chaining generic mode advancement.
- Prefer a single source of truth for status/event text; remove duplicate shadow strings once event-driven text exists.
- Player defeat, sleep-failure penalties, temporary hero unavailability, and post-battle fallout must be implemented consistently with the current docs.
- Do not invent major story elements that contradict `docs/game_vision_complete.md`.
- Prefer a playable incomplete feature over a broad unfinished feature.

- When design docs and current runtime disagree, preserve behavior unless the task explicitly includes a design-alignment refactor.
- When terminology migration is the task, keep it behavior-preserving and avoid unrelated gameplay refactors.
- When a historical milestone doc conflicts with the current design docs, follow the current design docs.

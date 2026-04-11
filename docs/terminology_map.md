# Terminology Map

This document maps **current design terminology** to **legacy runtime/content terminology** still present in the codebase.

Use this file when reading older code, tests, content, save strings, or archived design docs.

## Purpose

Project Ashvale has gone through a terminology cleanup.
Some older names still exist in runtime symbols, serialized values, JSON keys, UI theme fields, and archived milestone docs.

This file defines the **current intended vocabulary** and explains how it relates to older names.

## Source-of-truth guidance

When terminology disagrees:

1. Prefer the meanings in this file and the current live design docs.
2. Treat older `Overworld*` naming in source as legacy runtime vocabulary unless the task is explicitly a terminology migration.
3. Preserve legacy serialized strings and schema keys unless the task explicitly includes compatibility-safe migration.

## Current design terminology

### Campaign
A collection of connected scenarios. A campaign may be linear or branching. Winning a campaign requires winning multiple connected scenarios.

### Scenario
The top-level authored playable unit. A scenario may stand alone or be part of a campaign.

### World Map
The scenario-level map used to inspect and select **Regions**.

- Every scenario has exactly one World Map.
- The World Map can be opened any time outside battle while inside a scenario.
- Region travel is initiated from the World Map, but only when region travel is currently legal.

### Region
The main travel layer inside a scenario.

- A scenario contains one or more Regions.
- `Region` is the preferred design term for what older code/docs often called `overworld`.
- The player is always in exactly one active Region while playing a scenario.

### Node
A travel destination inside a Region.

Examples:
- location-entry nodes
- battle nodes
- service nodes
- arrival nodes
- travel/transit nodes

### Location
An enterable place reached from a parent Region.

A Location may:
- contain one or more screens
- include NPCs, interactions, quests, and dialogue
- contain zero or more Services

### Service
A functional interaction available either directly in a Region or inside a Location.

Examples:
- healing/rest
- recruitment
- storage
- item supply
- buffs

### Traveling party
The units currently moving with the player across the scenario.

Traveling party = **Active party + Reserve**.

### Active party
The current battle-legal fielded party.

- up to 5 units
- always exactly 1 assigned leader
- may contain fewer than 5 units if still legal

### Reserve
Traveling non-active units.

- capped at 7
- travels with the player
- can be swapped into the active party outside battle

### Stored units
Units assigned to a specific storage service.

- each storage service has its own independent 7-slot storage
- stored units persist in that Region
- stored units do not travel with the player unless manually withdrawn first

### Temporarily Unavailable
A hero state meaning the hero is currently removed from active, reserve, and storage and is not currently recruitable.

This state covers heroes who are, for design purposes:
- recovering after battle loss
- voluntarily dismissed
- otherwise temporarily removed from the roster

The player-facing narrative text may vary, but the underlying state is the same.

## Legacy runtime and content terminology

The following mappings should be used when reading the current codebase.

### `overworld` (legacy) -> `Region`
Older code and docs often use `overworld` for the in-scenario travel layer.
In current design terminology, this should usually be read as **Region**.

Examples:
- `OverworldController` -> historical runtime name for what is now conceptually the Region controller
- `OverworldRenderer` -> historical runtime name for what is now conceptually the Region renderer
- `gameplay::overworld` -> historical namespace for Region-layer gameplay logic

### `OverworldSelection` / `overworld_selection` (legacy) -> `World Map` / `WorldMapMode`
Older code/save strings may use `overworld selection` for the scenario-level region selection layer.

Interpretation:
- design/UI term: **World Map**
- runtime mode term: **WorldMapMode**
- legacy serialized string: `overworld_selection`

### `OverworldMode` / `overworld_mode` (legacy) -> `Region` / `RegionMode`
Older code/save strings may use `overworld mode` for the active regional travel layer.

Interpretation:
- design/UI term: **Region**
- runtime mode term: **RegionMode**
- legacy serialized string: `overworld_mode`

### `overworld_destination` (legacy content key) -> Region destination/node key
Some content/schema still uses `overworld_destination`.
Treat it as legacy content vocabulary for a Region-layer destination/node reference unless and until the content schema is intentionally migrated.

### `Exit To Overworld` / `Exited to overworld` (legacy strings)
Treat these as older user-facing strings referring to returning to the parent **Region**.

## Leadership terminology

### Current design intent
Only **hero units** can be assigned as leader.

### Current runtime legacy
Some current source/content still uses a separate `Leader` category.

Treat this as a **legacy runtime/content model**, not the preferred long-term design vocabulary.

Important guidance:
- do not expand the old separate `Leader` category in new design work
- do not assume the long-term design wants non-hero leaders
- keep current behavior stable unless the task explicitly includes a bounded leader-category refactor

## World structure rules tied to terminology

### Region travel
- Region travel is chosen on the **World Map**.
- Travel must begin before 11:00.
- Travel is disabled when illegal, but the World Map can still be viewed.
- Region travel requires a valid contiguous shortest path through enterable Regions.
- Arrival always occurs at 11:00 on the destination Region's designated arrival node.

### Cross-region persistence
- Heroes in the traveling party cross Regions with the player.
- Generic units in the traveling party do not; they are lost on Region change unless stored beforehand.
- Stored units remain in their storage service in that Region.

## Naming guidance for future work

When writing new docs, comments, prompts, or user-facing text:

- prefer **World Map** over `overworld selection`
- prefer **Region** over `overworld`
- prefer **Location** for entered places
- prefer **Service** for functional interaction points
- prefer **Traveling party** for `active + reserve`
- prefer **Temporarily Unavailable** for the hero state that returns to the pool later

When changing code:

- avoid broad symbol renames unless the task explicitly includes terminology migration
- preserve save compatibility and schema compatibility unless migration is part of the task
- call out any place where a legacy runtime name still exists but the design term has changed

## Historical docs

Archived milestone docs may use older terminology and older assumptions.
Treat them as project history, not current design truth.

Current design truth should come from the latest live docs such as:
- `docs/game_vision_complete.md`
- `docs/combat_rules.md`
- `docs/core_loop_rules.md`
- `README_DECISIONS.md`
- current instruction files under `.github/instructions/`

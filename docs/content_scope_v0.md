# Vertical Slice Content Scope

This document defines the bounded content scope for the current playable slice.
The `v0` name is historical. Treat this document as a scope cap, not as a milestone checklist or a statement that every item is already fully implemented.

For long-term direction, use `docs/game_vision_complete.md`.
For current implementation work, use the active milestone doc.

## Region count

The current playable slice uses exactly 1 Region.

## Region nodes

Target 10 to 20 Region nodes, such as:
- safe anchor / home-base node
- abandoned town center
- inn
- shop or recruit node
- mine entrance
- dungeon or ruin entrance
- service node
- optional threat / combat node
- combat nodes blocking access to other destinations

## Locations

Implement:
- 1 or 2 town-style Locations per Region
- 1 to 3 dungeon-style Locations per Region
- 1 safe-anchor or home-base-style Location per Region

## Party content

- 1 player character
- 3 hero characters total
- 8 generic unit types
- 8 to 10 enemy groups

## Systems

The bounded slice is expected to include:
- time/day progression
- Region travel time
- Location time costs
- sleeping
- wake-up penalty
- battle module
- recruitment
- shop
- save/load
- opening story sequence
- 2 simple quests

## Terminology note

Current runtime/content may still contain some legacy names such as `overworld_destination`.
In current design terminology:
- `Region` is the main in-scenario travel space
- `World Map` is the higher-level scenario map used to select Regions
- `Location` is an entered place inside a Region

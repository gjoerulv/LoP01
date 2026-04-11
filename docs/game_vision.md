# Game Vision

## Working title

Project Ashvale

## Elevator pitch

A 2D single-player strategy/RPG hybrid that combines:
- World Map-based travel between Regions inside a Scenario
- FF-style Location exploration
- true turn-based CTB battles
- party and roster logistics with lasting consequence
- restoration, survivor support, and safe-anchor progression
- a soft dystopian fantasy tone

For the fuller long-term direction and precise settled terminology, see `docs/game_vision_complete.md`.

## Tone and atmosphere

The world is damaged, but not hopeless.

The mood should feel:
- melancholic
- mysterious
- lived-in
- cozy in safe places
- dystopian in occupied or unstable places

This is not grimdark. Recovery, routine, and rebuilding matter.

## Terminology and hierarchy

For the full terminology model, see `docs/game_vision_complete.md`.

Short version:
- campaign -> scenario -> world map -> region -> node -> location -> service
- use **Region** for the main travel layer inside a Scenario
- use **World Map** for the higher-level scenario map used to select Regions
- use **Location** for entered places inside a Region
- use **Service** for an interaction available inside a Location or directly on a Region node

Legacy runtime/content names may still use older `overworld` wording in some places. Design language should prefer the terms above.

## Story setup

The player wakes in an abandoned town with no memory.

An old man is the only visible survivor. He explains that the country has largely fallen to android occupation. The abandoned town can be restored, but only if the player finds survivors, resources, and forgotten truths.

Ghostly remnants of former townspeople can be discovered and interacted with. These encounters reveal memories, town functions, and clues about what was lost.

The player starts in a small safe anchor. Beneath the house is a mine used for training, danger, and resources.

## Core fantasy

The player should feel like they are:
- surviving day by day
- rebuilding a forgotten refuge
- gathering allies
- slowly making the world livable again
- balancing time, risk, travel, and long-term progress

## Primary inspirations

- Heroes of Might and Magic 2/3: map structure, travel choices, visitable places, stack logistics
- SNES Final Fantasy: towns, dungeons, menus, character flavor
- Final Fantasy X: CTB battle pacing and readable turn-order planning
- restoration/progression games: safe places, recovery, routine, and meaningful rebuilding

## Current playable-slice focus

The current playable slice should prove the core loop of:
- moving through a Region between meaningful destination nodes
- entering Locations and using Services
- managing an active party, reserve, and longer-term roster consequence
- CTB battles with clear persistent outcomes
- day/time pressure, sleep, and wake-up penalty
- early restoration/progression through quests, services, and survivor-facing progress

The slice should stay intentionally bounded and data-driven.

## Non-goals for the current playable slice

- full narrative arc
- huge campaign scope
- final art quality
- advanced enemy AI
- hundreds of items
- full-scale farming systems
- designer-facing editor tooling
- multiplayer/networked play

## Longer-term direction

These are important future directions, but they should be approached only after the single-player slice is stronger:
- richer Region and World Map progression across a full Scenario
- stronger restoration and safe-anchor progression
- deeper party, storage, and hero-availability logistics across Regions
- a designer-facing tool for editing content, events, Locations, Services, and balance data
- multiplayer/networking only if the core single-player loop later proves strong enough to justify it

These should inform structure, but they should not drive premature architecture today. The near-term priority is still a strong, maintainable, data-driven single-player playable slice.

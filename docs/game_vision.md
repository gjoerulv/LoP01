# Game Vision

## Working title

Project Ashvale

## Elevator pitch

A 2D single-player strategy/RPG hybrid that combines:
- HoMM-style World Map and Region travel
- FF-style Location exploration
- true turn-based CTB battles
- cozy restoration, resource gathering, and survivor support
- a soft dystopian fantasy tone

For the fuller north-star direction beyond the current slice, see `docs/game_vision_complete.md`.
For project terminology, see `docs/terminology_map.md`.

## Tone and atmosphere

The world is damaged, but not hopeless.

The mood should feel:
- melancholic
- mysterious
- lived-in
- cozy in safe places
- dystopian in occupied places

This is not grimdark. Recovery, routine, and rebuilding matter.

## Terminology and hierarchy

For precise terminology and long-term structure, see:
- `docs/game_vision_complete.md`
- `docs/terminology_map.md`

Short version:
- campaign -> scenario -> World Map -> Region -> node -> Location
- Region is the main travel space inside a scenario
- World Map is the higher-level scenario planning and Region-selection layer
- Location is an entered place inside a Region
- Service is a functional interaction that may exist directly in a Region or inside a Location

## Story setup

The player wakes in an abandoned town with no memory.

An old man is the only visible survivor. He explains that the country has largely fallen to android occupation. The abandoned town can be restored, but only if the player finds survivors, resources, and forgotten truths.

Ghostly remnants of former townspeople can be discovered and interacted with. These encounters reveal memories, town functions, and clues about what was lost.

The player starts in a home-like safe anchor. Beneath that first refuge is a mine used for training, danger, and resources.

## Core fantasy

The player should feel like they are:
- surviving day by day
- rebuilding a forgotten home
- gathering allies
- slowly making the world livable again
- balancing time, risk, Energy, and long-term progress

## Primary inspirations

- Heroes of Might and Magic 2/3: World Map structure, Region travel, visitable places, and movement-pressure decisions
- SNES Final Fantasy: towns, dungeons, menus, character flavor
- Final Fantasy X: CTB battle pacing and readable turn-order planning
- Stardew-like progression: restoration, routine, resource use, cozy structure

## Current playable-slice focus

The current playable slice should prove the core loop of:
- Region travel between nodes
- Location exploration and interactions
- CTB battles with clear consequences
- day/time pressure, sleep, wake-up penalty, and travel planning
- early restoration/progression through simple quests and Services
- a bounded roster loop with active party, reserve, and battle consequence

The slice should stay intentionally bounded and data-driven.

## Non-goals for the current playable slice

- full narrative arc
- huge world
- final art quality
- advanced enemy-team AI
- hundreds of items
- full-scale farming systems
- designer-facing editor tooling
- multiplayer/networked play

## Longer-term direction

These are important future directions, but they should be approached only after the single-player slice is stronger:
- a designer-facing tool for editing content, events, Locations, Services, and balance data
- richer restoration and safe-anchor progression loops
- weekly Service/economy cadence
- World Map and cross-Region progression beyond the current bounded slice
- multiplayer/networking only if the core single-player loop later proves strong enough to justify it

These should inform structure, but they should not drive premature architecture today. The near-term priority is still a strong, maintainable, data-driven single-player playable slice.

# GitHub Copilot Instructions – LoP01

## Project overview

**LoP01** (Lands of Peril) is a text-based adventure RPG written in **C++17** and built with **CMake**.
Players navigate a world made up of interconnected locations, interact with NPCs, gather items and
engage in turn-based combat.

## Repository layout

| Path | Purpose |
|------|---------|
| `src/` | Game source code |
| `tests/` | Unit and integration tests |
| `data/` | JSON data files (locations, items, enemies) |
| `assets/` | Art, audio and other raw assets |
| `docs/` | Game design and technical documentation |
| `.github/` | CI workflows, Copilot instructions and prompt files |

## Coding conventions

- Follow the guidelines in `.github/instructions/cpp.instructions.md`.
- Gameplay systems follow `.github/instructions/gameplay.instructions.md`.
- Console/UI code follows `.github/instructions/ui.instructions.md`.

## Key design documents

Before implementing any feature read the relevant doc in `docs/`:

- `game_vision.md` – high-level game vision and pillars
- `core_loop_rules.md` – the player-facing game loop
- `combat_rules.md` – turn-based combat specification
- `content_scope_v0.md` – what is in and out of scope for v0
- `technical_direction.md` – architectural decisions and constraints

## Prompt files

Reusable prompt files live in `.github/prompts/`. Use them when bootstrapping new systems:

- `bootstrap-vertical-slice.prompt.md` – spin up a new playable slice
- `implement-battle-module.prompt.md` – add the combat system
- `add-location-scene.prompt.md` – add a new navigable location

## Build & test

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

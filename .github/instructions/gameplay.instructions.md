---
applyTo: "src/**"
---

# Gameplay System Instructions

## Core principles

1. **Data-driven** – game content (locations, items, enemies) lives in `data/` JSON files, not in source code.
2. **Single responsibility** – each system (locations, combat, inventory) is implemented in its own set of files.
3. **Separation of concerns** – game logic must not perform I/O directly; the `Game` class calls logic and passes results to the UI layer.

## Location system

- Every location is identified by a unique string ID (e.g. `"town_square"`).
- A `Location` stores its name, description and a map of exits (`direction → destination ID`).
- The `LocationManager` (or `Game`) owns all `Location` instances.
- The `Player` tracks only the current `Location::Id`.
- Navigation: the player issues a direction command; `Game` resolves it via the current `Location`'s exit map.

## Scene system

- A **scene** describes what the player currently sees (title screen, location view, battle, game-over).
- `Scene::Render()` returns a formatted `std::string`; it never writes to `std::cout` directly.
- The `Game` class decides which scene is active and hands the rendered string to `main`.

## Command processing

- Commands arrive as lowercase, whitespace-trimmed `std::string` values.
- `Game::ProcessCommand()` returns a `std::string` response that is printed by `main`.
- Supported movement aliases: `n/s/e/w`, `north/south/east/west`, `go <direction>`.
- Universal commands: `look` / `l`, `quit` / `q`.

## Adding a new gameplay system

1. Read `docs/game_vision.md` and `docs/core_loop_rules.md` first.
2. Create `src/SystemName.h` and `src/SystemName.cpp`.
3. Register the system in `Game` (constructor / `ProcessCommand`).
4. Add at least one test in `tests/`.
5. Update `CMakeLists.txt` if new source files are added.

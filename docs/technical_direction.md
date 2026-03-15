# Technical Direction

## Language and standard

- **C++17** — chosen for wide compiler support (MSVC 2022, GCC ≥ 11, Clang ≥ 14).
- No external runtime dependencies for the game binary.

## Build system

- **CMake ≥ 3.20** with modern target-based configuration.
- Two build targets:
  - `LoP01` — the game executable.
  - `LoP01Lib` — static library containing game logic (shared with tests).
- Out-of-source builds only (`cmake -B build`).

## Testing

- **Catch2 v3** via `FetchContent` (no manual installation required).
- All tests live in `tests/`.
- Run with `ctest --test-dir build --output-on-failure`.

## Architecture overview

```
main.cpp
  └─ Game
       ├─ Location  (data: id, name, description, exits)
       ├─ Player    (state: current location id)
       └─ Scene     (rendering: title | location | game-over)
```

### Dependency rules

- `main.cpp` may call `Game` only.
- `Game` may call `Location`, `Player` and `Scene`.
- `Scene` may call `Location` (read-only reference for rendering).
- `Location` and `Player` have no dependencies on other game classes.

## Data files

- Location data is hard-coded in `Game::SetupLocations()` for v0.
- From v1 onwards, content is loaded from JSON files in `data/` using a lightweight parser.

## Platform targets

| Platform | Compiler | IDE |
|----------|----------|-----|
| Windows | MSVC 2022 | Visual Studio 2022 |
| Linux | GCC 11+ | VS Code / CLion |
| macOS | Clang 14+ | VS Code / CLion |

Visual Studio 2022 can open the project directly via **File → Open → CMake…** (no `.sln` file needed).

## Future considerations

- A save/load system will use JSON serialisation.
- Combat (v1) will be isolated in a `BattleSystem` class with no I/O coupling.
- All content will eventually be data-driven via `data/*.json` files.

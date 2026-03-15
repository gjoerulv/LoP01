---
mode: agent
description: Bootstrap a new vertical slice (playable end-to-end demo) of the LoP01 game.
---

# Bootstrap Vertical Slice

You are setting up a self-contained playable prototype of **Lands of Peril**.
A vertical slice must demonstrate one complete loop: start → navigate locations → reach an end state.

## Steps

1. **Review** `docs/game_vision.md`, `docs/core_loop_rules.md` and `docs/content_scope_v0.md`.
2. **Scaffold** the directory structure according to the layout in `.github/copilot-instructions.md`.
3. **Implement** the following systems (stub-level is fine for anything outside the slice scope):
   - `Game` – main loop, command processing
   - `Location` + `LocationManager` – at least 4 interconnected locations
   - `Scene` – title screen and location view rendering
   - `Player` – current location tracking
4. **Wire up** `CMakeLists.txt` so `cmake --build build && ./build/LoP01` launches the game.
5. **Add** at least one test per implemented system in `tests/`.
6. **Verify** the slice is playable: start → move between locations → quit.

## Acceptance criteria

- `cmake -B build && cmake --build build` succeeds with no errors.
- `ctest --test-dir build` passes all tests.
- The player can type `start`, navigate between all locations using direction commands, and `quit`.

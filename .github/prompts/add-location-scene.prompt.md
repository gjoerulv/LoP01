---
mode: agent
description: Add a new navigable location scene to LoP01.
---

# Add Location Scene

You are adding a **new location** to **Lands of Peril**.

## Pre-reading

- `docs/game_vision.md` – tone and world-building guidelines
- `docs/content_scope_v0.md` – locations already in scope
- `.github/instructions/gameplay.instructions.md` – location system rules
- `.github/instructions/ui.instructions.md` – scene formatting conventions

## Steps

1. **Design** the location:
   - Choose a unique string ID (lowercase, underscores, e.g. `"old_mill"`).
   - Write a name (title case, ≤ 30 characters).
   - Write a description (3–5 lines, second-person present tense, British English).
   - Decide which existing locations it connects to and in which directions.

2. **Implement** the location in `Game::SetupLocations()`:
   ```cpp
   Location loc("old_mill", "Old Mill",
       "The derelict mill creaks in the wind...");
   loc.AddExit("east", "town_square");
   m_locations["old_mill"] = std::move(loc);
   ```
   Remember to add the reciprocal exit on the connecting location.

3. **Add a test** in `tests/test_location.cpp`:
   - Verify the new location has the expected exits.
   - Verify navigation to/from the new location works via `Game::ProcessCommand`.

4. **Update** `docs/content_scope_v0.md` with the new location entry.

## Acceptance criteria

- `cmake --build build` succeeds.
- `ctest --test-dir build` passes, including the new test.
- The player can reach the new location by navigating from an adjacent location.
- `look` displays the correct name and description.

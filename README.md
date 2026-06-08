# Ashvale

Ashvale is a C++20 / raylib / CMake strategy-RPG prototype. The current codebase is a **post-M22** bounded multi-Region, multi-Scenario vertical slice. The implemented foundation includes battle, roster, save/load, Region/Location flow, typed events, enemy teams, scenario outcomes, a dedicated Scenario Result screen, inventory/artifacts, Energy, World Map, Campaign, owned services/economy, unit passive effects, Trading Post transactions, Trading Post interaction flow, and Scenario-authored player economy/service start state.

## Current baseline

Latest completed milestone: **M22 — Scenario Result Presentation Flow**.

M22 added a dedicated player-facing Scenario Result mode that appears when a deterministic scenario outcome latches. It shows Victory/Defeat, the outcome reason, and the next step before the existing campaign/terminal progression runs on Continue. The mode is transient, suppresses save/load while active, does not change scenario outcome rules, campaign progression, carry-over, content, or save schema, and keeps result rendering separate through a mapper/renderer path.

No next milestone is currently selected. The next planning pass should audit the post-M22 roadmap, active docs, and source before choosing the next narrow milestone.

## Active docs

Read these before planning or implementation:

- `CLAUDE.md`
- `README_DECISIONS.md`
- `docs/implementation_roadmap.md`
- `docs/content_scope_v1.md`
- `docs/technical_direction.md`
- `docs/game_vision.md`
- `docs/game_shell_flow.md`
- `docs/presentation_game_feel.md`
- `docs/core_loop_rules.md`
- `docs/scenario_authoring.md`
- `docs/content_schema.md`
- `docs/validation_system.md`
- `docs/terminology_map.md`

Archived docs are historical context only.

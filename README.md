# Ashvale

Ashvale is a C++20 / raylib / CMake strategy-RPG prototype. The current codebase is a **post-M24** bounded multi-Region, multi-Scenario vertical slice.

The implemented foundation includes battle, roster, save/load, Region/Location flow, typed events, runtime enemy-team spawning, scenario outcomes, a dedicated Scenario Result screen, inventory/artifacts, Energy, World Map, Campaign, owned services/economy, unit passive effects, Trading Post transactions, Trading Post interaction flow, Scenario-authored player economy/service start state, in-play owned-service claiming after defeating hostile guards, and v1 strategic-economy proof content.

## Current baseline

Latest completed milestone: **M24 — v1 Strategic-Economy Proof Content**.

M24 closed the v1 content proof: shipped content now exercises `playerStart`, `leader_energy`, authored Trading Post curve data, guarded service claiming, mine payout, and the test-proven `mine_production` stationed payout path. The compact strategic-economy v1 slice is functionally complete.

Active scope cap: **`docs/content_scope_v2.md`**.

Current next milestone: **M25 — Player-facing Service Stationing Flow**. M25 should make the existing stationed-unit mine-production path reachable through player-facing gameplay without adding a full Storage/Garrison system.

## Active docs

Read these before planning or implementation:

- `CLAUDE.md`
- `README_DECISIONS.md`
- `docs/implementation_roadmap.md`
- `docs/content_scope_v2.md`
- `docs/technical_direction.md`
- `docs/game_vision.md`
- `docs/game_shell_flow.md`
- `docs/presentation_game_feel.md`
- `docs/core_loop_rules.md`
- `docs/scenario_authoring.md`
- `docs/content_schema.md`
- `docs/validation_system.md`
- `docs/terminology_map.md`

Archived scope/roadmap docs are historical context only.

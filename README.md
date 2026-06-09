# Ashvale

Ashvale is a C++20 / raylib / CMake strategy-RPG prototype. The current codebase is a **post-M23** bounded multi-Region, multi-Scenario vertical slice.

The implemented foundation includes battle, roster, save/load, Region/Location flow, typed events, runtime enemy-team spawning, scenario outcomes, a dedicated Scenario Result screen, inventory/artifacts, Energy, World Map, Campaign, owned services/economy, unit passive effects, Trading Post transactions, Trading Post interaction flow, Scenario-authored player economy/service start state, and in-play owned-service claiming after defeating hostile guards.

## Current baseline

Latest completed milestone: **M23 — Owned Service Claiming and Contesting Foundation**.

M23 added the first in-play ownership-transfer path: when the player defeats a hostile team occupying/guarding a node, eligible ownable services at that node can become player-owned. The implementation uses a pure claim rule, one explicit `GameSession` mutation path, the hostile-contact victory seam, runtime `spawnTeam` creation/reactivation, shipped guarded-mine proof content, and tests for claiming, payout/tier/use, save/load, and content integrity.

No next milestone is currently selected. The next planning pass should audit the post-M23 source/docs against `docs/content_scope_v1.md` and `docs/game_vision.md` before deciding whether v1 is complete enough to archive and move to v2, or whether one final v1 cleanup milestone is still needed.

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

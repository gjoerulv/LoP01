# Ashvale

Ashvale is a C++20 / raylib / CMake strategy-RPG prototype. The current codebase is a **post-M21** bounded multi-Region, multi-Scenario vertical slice.

The implemented foundation includes battle, roster, save/load, Region/Location flow, typed events, enemy teams, scenario outcomes, inventory/artifacts, Energy, World Map, Campaign, owned services/economy, unit passive effects, Trading Post transactions, Trading Post interaction flow, and Scenario-authored player economy/service start state.

## Current baseline

Latest completed milestone: **M21 — Scenario Economy Start-State Authoring Foundation**.

M21 added a narrow `playerStart` surface on Scenario content:

- starting Gold through the existing `startGold` runtime path;
- starting non-Gold resources;
- initial player-owned service state using existing owned-service runtime fields;
- strict shape/type/semantic validation;
- application through `GameSession::TransitionToScenario` when a Scenario starts;
- save/load compatibility through existing SaveData fields.

Selected next milestone (planned, not yet implemented): **M22 — Scenario Result Presentation Flow** — a dedicated player-facing scenario-end result step (outcome, reason, next step) over the existing deterministic outcome and campaign-progress paths. See `docs/implementation_roadmap.md` for the full definition.

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

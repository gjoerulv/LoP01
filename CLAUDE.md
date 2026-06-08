# Claude Guidance for Ashvale

## Current baseline

Treat the repository as a **post-M21** C++20 / raylib / CMake game project.

Completed foundations include battle, roster, save/load, Region/Location flow, content validation, typed events, enemy teams, scenario outcomes, inventory/artifacts, Energy, World Map, Campaign, owned-service/economy systems, the narrow unit passive-effect spine, Trading Post transaction rules/APIs, bounded Trading Post interaction flow, and Scenario-authored player economy/service start state.

Latest completed milestone: **M21 — Scenario Economy Start-State Authoring Foundation**.

No next milestone is currently selected. Do not assume M22. Start the next planning pass by auditing the active docs and source.

## Required reading

Before architecture, roadmap, economy, Scenario, or content work, read:

1. `README.md`
2. `README_DECISIONS.md`
3. `docs/implementation_roadmap.md`
4. `docs/content_scope_v1.md`
5. `docs/technical_direction.md`
6. `docs/game_vision.md`
7. `docs/game_shell_flow.md`
8. `docs/presentation_game_feel.md`
9. `docs/core_loop_rules.md`
10. `docs/scenario_authoring.md`
11. `docs/content_schema.md`
12. `docs/validation_system.md`
13. `docs/terminology_map.md`

Archived files are historical context only. Do not use archived roadmap/scope files as current requirements.

## Working rules

- Keep implementation slices narrow, test-backed, and aligned with the active roadmap.
- If docs and source disagree, stop and report the mismatch.
- Keep gameplay rules out of rendering/input layers.
- Keep authored static content separate from runtime mutable state.
- Preserve save/load compatibility unless the task explicitly includes migration work.
- Avoid per-frame scans, repeated content parsing, graph rebuilds, large needless copies, and hidden nested scans.
- Avoid demo-specific source branches; prove systems through generic data and tests.
- Do not introduce broad frameworks before a scoped consumer needs them.

## Source comments

Production source comments should document durable contracts, not milestone bookkeeping. Avoid comments such as `M21 Phase 1:` in production source. Use comments only for non-obvious invariants, validation traps, save/load contracts, compatibility behavior, performance-sensitive choices, or deliberate limitations.

Test comments are acceptable when they explain non-obvious regression intent.

## Current settled system boundaries

- `playerStart` is the Scenario-authored surface for starting Gold, non-Gold resources, and initial player-owned service state.
- `playerStart.gold` is an alias for legacy top-level `startGold`; authoring both is invalid.
- Scenario start-state applies to runtime `GameSession` state when a Scenario starts; it is not persisted back into content.
- Gold remains a single source of truth through the existing `gold_` / ResourceType delegation path.
- Owned-service state uses existing runtime fields and player ownership for M21 start-state; no general team authoring exists yet.
- World Map initial unlocks remain authored through World Map content; no Scenario `unlockedRegions` override exists.
- Trading Post interaction is implemented as a bounded Location-mode service flow; broader shop/inventory UI is deferred.
- Unit `passive_effects` currently support only `mine_production` and `leader_energy`.
- Artifact `statBonus` remains on the artifact battle-stat path; artifact Energy, item effects, statuses, active abilities, and broad skill systems are deferred.

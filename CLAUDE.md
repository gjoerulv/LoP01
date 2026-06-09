# Claude Guidance for Ashvale

## Current baseline

Treat the repository as a **post-M25** C++20 / raylib / CMake game project.

Completed foundations include battle, roster, save/load, Region/Location flow, content validation, typed events, runtime enemy-team spawning, scenario outcomes, a dedicated Scenario Result screen, inventory/artifacts, Energy, World Map, Campaign, owned-service/economy systems, the narrow unit passive-effect spine, Trading Post transaction rules/APIs, bounded Trading Post interaction flow, Scenario-authored player economy/service start state, in-play owned-service claiming/contesting after defeating hostile guards, v1 strategic-economy proof content, and player-facing mine stationing/unstationing.

Latest completed milestone: **M25 — Player-facing Service Stationing Flow**.

Active scope cap: **`docs/content_scope_v2.md`**.

Current next milestone: **not yet selected**. Candidate v2 directions are listed in `docs/implementation_roadmap.md` §5 and `docs/content_scope_v2.md` §5. Do not treat Storage/Garrison, an owned-service management view, enemy-side capture, or other v2 expansion items as already implemented.

## Required reading

Before architecture, roadmap, economy, Scenario, content, UI, or rendering work, read:

1. `README.md`
2. `README_DECISIONS.md`
3. `docs/implementation_roadmap.md`
4. `docs/content_scope_v2.md`
5. `docs/technical_direction.md`
6. `docs/game_vision.md`
7. `docs/game_shell_flow.md`
8. `docs/presentation_game_feel.md`
9. `docs/core_loop_rules.md`
10. `docs/scenario_authoring.md`
11. `docs/content_schema.md`
12. `docs/validation_system.md`
13. `docs/terminology_map.md`

Archived files, including `docs/content_scope_v1.md` once archived by the user, are historical context only. Do not use archived roadmap/scope files as current requirements.

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

Production source comments should document durable contracts, not milestone bookkeeping. Avoid comments such as `M25 Phase 1:` in production source. Use comments only for non-obvious invariants, validation traps, save/load contracts, compatibility behavior, performance-sensitive choices, or deliberate limitations.

Test comments are acceptable when they explain non-obvious regression intent.

## Current settled system boundaries

- Scenario outcome rules are deterministic and latched through `GameSession`; presentation uses the dedicated transient Scenario Result mode.
- Scenario Result mode is not normal save/load progression. Save/load is suppressed while it is active, and accidental serialized `scenario_result` mode self-heals to Region mode.
- `playerStart` is the Scenario-authored surface for starting Gold, non-Gold resources, and initial player-owned service state.
- `playerStart.gold` is an alias for legacy top-level `startGold`; authoring both is invalid.
- Scenario start-state applies to runtime `GameSession` state when a Scenario starts; it is not persisted back into content.
- Gold remains a single source of truth through the existing `gold_` / `ResourceType` delegation path.
- Runtime owned-service state is mutable: player-side guarded-service claiming is implemented after defeating a hostile guard at the node.
- Claiming mutates runtime owned-service state only; content definitions are never mutated.
- Owned services do not have to be guarded. The current capture path proves guarded claiming, not a universal guard requirement.
- Allied ownership does not grant player benefits and is not claimable in the current player-side claim path.
- Runtime `spawnTeam` can create a missing enemy team or reactivate/move an existing one; it is not a general team-definition authoring system.
- World Map initial unlocks remain authored through World Map content; no Scenario `unlockedRegions` override exists.
- Trading Post interaction is implemented as a bounded Location-mode service flow; broader shop/inventory UI is deferred.
- Unit `passive_effects` currently support only `mine_production` and `leader_energy`.
- `mine_production` is reachable through gameplay: M25 added a bounded, text-prompt stationing flow at player-owned mines. Stationing is physical placement — a roster stack is in exactly one place at a time (an active slot, a reserve slot, or stationed at one owned service), so stationed units stay owned, leave the travelling/battle party, and never duplicate. Generic stacks may be split; capacity is up to 5 stationed stacks per mine; the Player Character can never be stationed. Mutations live only behind `GameSession` (`TryStationStackAtService`, `TryStationSplitAtService`, `TryUnstationStackFromService`); `App`/`StationingInteraction` never edits stationing or roster slots directly. No schema bump was required.
- M25 stationing is guard/worker capacity only. It is not Storage/Garrison: there are no stationed-defender combat, capacity-loss/siege, enemy-side stationing, or service-defense rules. Stationing targets are mines only; other ownable (trader) kinds are not stationing targets yet.
- Artifact `statBonus` remains on the artifact battle-stat path; artifact Energy, item effects, statuses, active abilities, and broad skill systems are deferred.

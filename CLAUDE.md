# Claude Guidance for Ashvale

## Current baseline

Treat the repository as a **post-M30** C++20 / raylib / CMake game project.

Completed foundations include battle, roster, save/load, Region/Location flow, content validation, typed events, runtime enemy-team spawning, scenario outcomes, a dedicated Scenario Result screen, inventory/artifacts, Energy, World Map, Campaign, owned-service/economy systems, the narrow unit passive-effect spine, Trading Post transaction rules/APIs, bounded Trading Post interaction flow, Scenario-authored player economy/service start state, guarded and unguarded player-side owned-service claiming, v1 strategic-economy proof content, player-facing mine stationing/unstationing, a bounded read-only owned-service overview / strategic service readout panel, a bounded unit-storage foundation, cross-Region generic-unit travel loss with an explicit warning/confirmation, and the M30 contested-infrastructure loop: deterministic service defense with stationed/stored defenders, storage loss with Temporarily Unavailable heroes, enemy-side service capture pressure, opt-in service destruction/restoration, and a persisted service event log with overview presentation.

Latest completed milestone: **M30 — v2 Completion: Contested Infrastructure, Service State, and Closure Audit**.

Active scope: `docs/content_scope_v3.md`.

Selected next milestone: **M31 — Shell Entry + Scenario/Campaign Selection**.

`docs/content_scope_v2.md` is complete and should be archived by the user. Do not keep extending v2.

Do not treat the documented v3 candidates — Scenario Context, fog/reveal, threat preview/auto-resolve, quest/journal, full item economy, full AI economy, full settings/mods shell, or final service-management UI — as already implemented.

## Required reading

Before architecture, roadmap, economy, Scenario, content, UI, shell, or rendering work, read:

1. `README.md`
2. `README_DECISIONS.md`
3. `docs/implementation_roadmap.md`
4. `docs/content_scope_v3.md`
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

- Keep implementation slices bounded, test-backed, and aligned with the active roadmap.
- If docs and source disagree, stop and report the mismatch.
- Keep gameplay rules out of rendering/input layers.
- Keep authored static content separate from runtime mutable state.
- Preserve save/load compatibility unless the task explicitly includes migration work.
- Avoid per-frame scans, repeated content parsing, graph rebuilds, large needless copies, and hidden nested scans.
- Avoid demo-specific source branches; prove systems through generic data and tests.
- Do not introduce broad frameworks before a scoped consumer needs them.

## Source comments

Production source comments should document durable contracts, not milestone bookkeeping. Avoid comments such as `M25 Phase 1:` in production source.

Use comments only for non-obvious invariants, validation traps, save/load contracts, compatibility behavior, performance-sensitive choices, or deliberate limitations.

Test comments are acceptable when they explain non-obvious regression intent.

## Current settled system boundaries

- Scenario outcome rules are deterministic and latched through `GameSession`; presentation uses the dedicated transient Scenario Result mode.
- Scenario Result mode is not normal save/load progression. Save/load is suppressed while it is active, and accidental serialized `scenario_result` mode self-heals to Region mode.
- `playerStart` is the Scenario-authored surface for starting Gold, non-Gold resources, and initial player-owned service state. It is runtime start-state, not content mutation.
- Owned services can be claimed by both guarded capture after hostile victory and peaceful legal node entry via `GameSession::ResolveNodeEntryClaims`.
- Hostile-occupied travel may start battle before the moving player team is placed on the destination node. This is intended final-direction behavior.
- M27's owned-service overview is read-only presentation. It must not become remote stationing, storage, repair, capture, or service-management UI without a scoped milestone.
- M28 storage is distinct from M25 stationing: storage cap 7, stationing cap 5; storage uses `StoredUnitSaveState`; stationing uses stationed guard/worker refs.
- M29 travel loss affects only slotted active/reserve generic stacks. Stored and stationed stacks survive Region travel. Heroes and Player Character travel.
- M30 service attacks are node-level against player-owned attackable services (Mine, trader kinds, Storage; never Rest/Shop/Recruit/Muster; never the arrival node). Player absent uses deterministic `ServiceDefenseRules`; player present uses the existing interactive battle surface.
- M30 capture resolves placed stacks atomically: generics dismissed, heroes Temporarily Unavailable, refs cleared, Player Character never placed/lost/TU, ownership transfers immediately.
- M30 destruction/restoration is opt-in (`destroyable` + validated `restore_cost`) and uses the bounded Region-mode maintenance action.
- M31 should implement shell entry and Scenario/Campaign selection. It should not implement full character creation, full settings/mods, full save-slot metadata, Scenario Region Context, fog/scouting, or new gameplay systems unless required for safe content selection.

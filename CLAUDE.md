# Claude Guidance for Ashvale

## Current baseline

Use `docs/implementation_roadmap.md` as the authoritative source for the current implementation baseline, latest completed milestone, selected next milestone, and milestone candidates. Do not duplicate or infer milestone state from this file.

Active scope: `docs/content_scope_v3.md`.

Completed foundations include battle, roster, save/load, Region/Location flow, content validation, typed events, runtime enemy-team spawning, scenario outcomes, a dedicated Scenario Result screen, inventory/artifacts, Energy, World Map, Campaign, owned-service/economy systems, the narrow unit passive-effect spine, Trading Post transaction rules/APIs, bounded Trading Post interaction flow, Scenario-authored player economy/service start state, guarded and unguarded player-side owned-service claiming, v1 strategic-economy proof content, player-facing mine stationing/unstationing, a bounded read-only owned-service overview / strategic service readout panel, bounded unit storage, cross-Region generic-unit travel loss with explicit warning/confirmation, the v2 contested-infrastructure loop, the v3 shell entry flow, the M32 Scenario Context (authored `regions` boundary) + authored starting roster (`playerStart.roster`) with a clean scenario-start reset, and the M32 fog/reveal foundation with reveal-gated enemy visibility.

Do not treat documented v3 candidates — full Scenario Region Context (per-Region variable/flag/override passing; M32 only added the smaller authored `regions` boundary), fog/reveal polish beyond the M32 radius-2 foundation, threat preview/auto-resolve, quest/journal, full item economy, full AI economy, full settings/mods shell, authored hero pool, or final service-management UI — as implemented unless `docs/implementation_roadmap.md` says the relevant milestone is complete.

## Required reading

Before architecture, roadmap, economy, Scenario, content, UI, shell, or rendering work, read:

1. `docs/implementation_roadmap.md`
2. `docs/content_scope_v3.md`
3. `docs/technical_direction.md`
4. `docs/game_vision.md`
5. `docs/game_shell_flow.md`
6. `docs/presentation_game_feel.md`
7. `docs/core_loop_rules.md`
8. `docs/scenario_authoring.md`
9. `docs/content_schema.md`
10. `docs/validation_system.md`
11. `docs/terminology_map.md`
12. `README.md`
13. `README_DECISIONS.md`

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

Production source comments should document durable contracts, not milestone bookkeeping. Avoid comments such as `M25 Phase 1:` in production source. Use comments only for non-obvious invariants, validation traps, save/load contracts, compatibility behavior, performance-sensitive choices, or deliberate limitations. Test comments are acceptable when they explain non-obvious regression intent.

## Current settled system boundaries

- Scenario outcome rules are deterministic and latched through `GameSession`; presentation uses the dedicated transient Scenario Result mode.
- Scenario Result mode is not normal save/load progression. Save/load is suppressed while it is active, and accidental serialized `scenario_result` mode self-heals to Region mode.
- `playerStart` is the Scenario-authored surface for starting Gold, non-Gold resources, initial player-owned service state, and (M32) the starting roster (`playerStart.roster.{active,reserve}`). It is runtime start-state, not content mutation.
- Owned services can be claimed by both guarded capture after hostile victory and peaceful legal node entry via `GameSession::ResolveNodeEntryClaims`.
- Hostile-occupied travel may start battle before the moving player team is placed on the destination node. This is intended final-direction behavior.
- The owned-service overview is read-only presentation. It must not become remote stationing, storage, repair, capture, or service-management UI without a scoped milestone.
- Storage is distinct from stationing: storage cap 7, stationing cap 5; storage uses `StoredUnitSaveState`; stationing uses stationed guard/worker refs.
- Cross-Region travel loss affects only slotted active/reserve generic stacks. Stored and stationed stacks survive Region travel. Heroes and Player Character travel.
- Service attacks are node-level against player-owned attackable services. Player absent uses deterministic `ServiceDefenseRules`; player present uses the existing interactive battle surface.
- Capture resolves placed stacks atomically: generics dismissed, heroes Temporarily Unavailable, refs cleared, Player Character never placed/lost/TU, ownership transfers immediately.
- Destruction/restoration is opt-in (`destroyable` + validated `restore_cost`) and uses the bounded Region-mode maintenance action.
- Shell entry uses `GameMode::Title` for the main menu and an App-local screen state machine that is never persisted. Content starts go through `GameSession::StartCampaign` / `StartStandaloneScenario` behind the validation/playability gate.
- Standalone Scenario selection enforces `standaloneSelectable`. Settings/Mods/Credits/Tutorial/PvP remain hidden until real systems/content exist.
- Continue is a bounded single-save path (`saves/slot_1.json`); there are no save slots or save metadata. Quicksave is suppressed at the shell so a fresh session cannot overwrite a real save; quickload remains.
- Scenario start (M32): the clock always resets to day 1 and reveal is reseeded. A scenario authoring `playerStart.roster` rebuilds active/reserve and clears inventory/equipment/TU/log before campaign carry-over; a scenario authoring no roster keeps the prebuilt default Player Character roster/inventory (the documented default-roster path, relied on by v1/v2 economy-proof tests). Shipped scenarios author no roster.
- Scenario Context (M32): an authored `regions` id list bounds which Regions/World Map entries the active Scenario exposes. Empty/absent => all Regions (backward compatible). The World Map read model and `GameSession::TravelToRegion` gate on `IsRegionInScenarioContext`. The context is derived from the active scenario (not persisted) and re-derived on load. Full per-Region Scenario Region Context remains future scope.
- Fog/reveal (M32): per-Region reveal is HoMM-persistent runtime state (`revealedNodesByRegion_`), seeded radius-2 around the start node + start-owned-service nodes and extended on movement/World Map arrival. It is saved/loaded. The Region read model adds a runtime `revealed` flag distinct from authored `discovered` (structural availability); enemy hostile markers + a bounded `enemyEstimate` are gated by reveal, but travel-legality mechanics still use the true hostile set. Reveal does not block travel.

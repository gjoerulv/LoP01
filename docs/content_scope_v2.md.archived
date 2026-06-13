# Ashvale Content Scope v2

## 1. Purpose

`content_scope_v2.md` is the active content and systems scope cap after M24. v1 proved the compact strategic-economy loop: Scenario-authored player economy start state, owned services, mine payout, unit passive hooks, Trading Post trade data, guarded service claiming, Scenario Result presentation, and campaign progression all work together in the shipped slice and tests.

v2 should build on that proof by making infrastructure management more player-facing and strategically legible. The next work should not inflate into a full AI economy, full item economy, editor tooling, or a broad campaign shell. The goal is to turn the existing runtime systems into deliberate player decisions.

## 2. v2 target

v2 targets a stronger infrastructure-control loop:

- player-facing stationing/unstationing of eligible units at owned services;
- visible benefit from stationed `mine_production` units in normal play;
- consistent player-side ownership claiming for both unguarded legal node-entry and guarded post-battle capture;
- a bounded Storage/Garrison foundation only after the stationing and ownership-claiming seams are proven;
- clearer presentation of owned services, claimed services, and stationed units;
- continued use of authored Scenario start-state, runtime ownership, save/load, and content validation discipline;
- minimal content additions that exercise the loop without creating a large campaign or full economy simulation.

## 3. v2 design constraints

- Use existing runtime state where practical: `OwnedServiceSaveState`, stationed units, owned roster, resources, Gold, service IDs, and existing Region travel / enemy-team occupancy state.
- Keep authored static content separate from runtime mutable state.
- Preserve the stack-backed stationed-unit invariant: stationed units must correspond to owned units where that invariant currently applies.
- Keep ownership claiming as runtime state mutation. Do not mutate content definitions during gameplay.
- Do not add broad team/owner authoring just to support stationing or player-side claiming.
- Preserve existing save/load compatibility unless a scoped migration is explicitly selected.
- Avoid per-frame catalog scans, repeated content parsing, graph rebuilds, or hidden nested scans.
- Keep UI/service interactions bounded and text-prompt based until the current rendering model is intentionally expanded.

## 4. Selected v2 milestones

The first selected v2 milestone was: **M25 — Player-facing Service Stationing Flow** *(complete)*.

M25 made stationing reachable in gameplay through a narrow interaction path without becoming a full Storage/Garrison system: a player can assign an eligible owned unit to an eligible owned mine and see the existing `mine_production` payout benefit through normal play.

M25 answered:

- where the player initiates stationing;
- which owned services can receive stationed units;
- which owned units can be stationed;
- how stationed units are removed or returned;
- how the stack-backed invariant is preserved;
- how the result is shown clearly enough for the player;
- how save/load preserves stationed assignments.

The second selected v2 milestone was: **M26 — General Owned-Service Claiming Semantics** *(complete)*.

M26 closed the gap between the final ownership model and the prior implementation: guarded claiming works after defeating a hostile guard, and peaceful/unguarded player-side claiming now happens on legal node entry via `GameSession::ResolveNodeEntryClaims`. Player-side claiming is systemic without expanding into enemy-side capture, service destruction/restoration, or Storage/Garrison. The questions below record the settled M26 answers; see `docs/implementation_roadmap.md` §4.

M26 answered:

- when legal node entry claims unguarded ownable services;
- how guarded/hostile-occupied service nodes resolve battle-before-placement, victory, loss, and capture;
- how claim logic stays centralized behind `GameSession` rather than scattered through `App`;
- how runtime `OwnedServiceSaveState` changes without mutating authored content;
- how claiming interacts safely with existing stationed units and save/load;
- how tests prove no double-claim, double-arrival, or no-claim regressions.

The third selected v2 milestone was: **M27 — Owned Service Overview / Strategic Service Readout** *(complete)*.

M27 added a bounded, read-only owned-service overview opened with `O` from Region mode (transient `OwnedServiceOverviewMode`), assembled by a pure mapper/render-model/renderer from existing `GameSession` accessors.

It lists player-owned services with location/region, kind, owner/status, stationed `count/5` + unit names, a daily-output preview (`GameSession::PreviewMineDailyOutput`, matching payout), and Trading Post tier. It mutates nothing and is never persisted; no schema bump. M25 stationing remains reachable through the mine Location-zone interaction.

M27 is a strategic visibility foundation, not the final service-management UI. It serves the milestone-agnostic vision by making owned-service state legible and by establishing a reusable read model for future Region map overlays, selected-node panels, Adventure-strip screens, Storage/Garrison, service defense, and service-state presentation.

The fourth selected v2 milestone was: **M28 — Storage Foundation** *(complete)*.

M28 built **Storage** — the final-vision 7-slot per-service unit store (units persist in-Region, don't travel, are retrievable) — as a placement bucket distinct from M25 stationed guards. The "Storage/Garrison" milestone label was pressure-tested: the docs make Storage a concrete concept while "garrison" is the stationed-guards concept M25 already covers, so M28 implemented Storage and deferred all defense/capture/loss. Owned non-Player-Character stacks store/retrieve at a player-owned storage service behind explicit `GameSession` methods, preserving the one-place-at-a-time stack invariant; additive `stored_units` save (no schema bump); a `home_base_storage` service is authored and player-owned via `playerStart`; the M27 overview shows a read-only `Stored n/7` row.

The fifth selected v2 milestone was: **M29 — Cross-Region Generic Unit Preservation / Travel Warning** *(complete)*.

M29 made Storage strategically necessary by implementing the final travel consequence (`core_loop_rules` §5): only traveling (slotted active/reserve) generic stacks are lost on confirmed Region-to-Region travel, while stored and stationed stacks survive with refs intact and heroes/Player Character travel. The at-risk set is exposed through the pure `GameSession::PreviewRegionTravelGenericLosses()` read, and the confirmed-travel removal inside `TravelToRegion` deletes exactly that set — also fixing the M15-era whole-roster removal that would otherwise have deleted placed stacks. A bounded two-stage confirmation on the existing World Map screen lists per-stack `Nx Name` losses; cancel keeps the screen open so units can be stored first, and a no-loss party travels on the first confirm. App-local pending state only — no new GameMode, no schema bump, no content changes. It did not add remote storage management, service defense, enemy capture, or a shell/menu rewrite.

The sixth and final selected v2 milestone was: **M30 — v2 Completion: Contested Infrastructure, Service State, and Closure Audit** *(complete)*.

M30 turned the service/storage/stationing loop into a contested infrastructure loop: a deterministic node-level service-defense resolver (stationed + stored stacks defend; the active party defends via the existing battle surface when the player stands on the attacked node), storage loss with generic dismissal and a minimal Temporarily Unavailable hero pipeline (weekly return-to-reserve as the hero-pool stand-in; the Player Character can never be lost or TU), enemy-side capture pressure through legal Region-layer `ProcessEnemyPhase` action with authored enemy-group strength, opt-in service destruction/restoration (`destroyable` + validated `restore_cost`, 1000 Energy + 1 hour, day-start completion, §20 cancel-by-destruction), a persisted bounded service event log plus overview/status presentation, and a shipped content proof (player-owned `river_depot` storage gate, destroyable copper mine, raider pressure events). All M30 save fields are additive — no schema bump. Deliberate simplifications and deferrals are recorded in `docs/implementation_roadmap.md` §4/§5.

## 5. v2 closure

**v2 is complete.** All six selected milestones (M25–M30) shipped with tests and shipped-content proof, and the v2 target in §2 is met: infrastructure control is player-facing, strategically legible, and now contested.

This file is **ready to archive** (user action, mirroring the v1 note in §8). Larger systems intentionally NOT built in v2 — full-simulation service-defense battles, the shared hero pool, enemy-side destruction/sabotage, AI economy, item economy, fog-of-war, shell flow, and the final service-management UI — are recorded as v3 candidates in `docs/implementation_roadmap.md` §5, not as open v2 promises. Create `docs/content_scope_v3.md` before selecting the next milestone.

## 6. Not in early v2 scope unless explicitly selected

- Full AI economy.
- Enemy-side capture of player-owned services.
- Service destruction/restoration, sabotage, siege, and stationed-defender combat.
- Full item economy, crafting, cooking, seeds, ingredients, or broad item-use systems.
- Full skill tree, broad passive-effect system, statuses, active abilities, or spell system.
- Full shell/menu/character-creation/settings flow.
- General team-definition authoring.
- Full per-Scenario content directories / Scenario Region Contexts.
- Full fog-of-war / visibility simulation.
- Multiplayer or internet play.
- Editor tooling.

## 7. Relationship to milestone-agnostic docs

Milestone-agnostic docs remain source-of-truth for final vision and rules:

- `docs/game_vision.md`
- `docs/technical_direction.md`
- `docs/core_loop_rules.md`
- `docs/combat_rules.md`
- `docs/game_shell_flow.md`
- `docs/presentation_game_feel.md`
- `docs/scenario_authoring.md`
- `docs/content_schema.md`
- `docs/validation_system.md`
- `docs/terminology_map.md`

Do not rewrite these documents merely to record milestone status. Update them only when the actual long-term rule, schema, terminology, or architectural direction changes.

## 8. v1 archival note

`docs/content_scope_v1.md` should be archived by the user after this v2 scope is accepted. Once archived, it is historical context only. Active planning should use this v2 scope and `docs/implementation_roadmap.md`.

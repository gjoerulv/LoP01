# Ashvale Content Scope v2

## 1. Purpose

`content_scope_v2.md` is the active content and systems scope cap after M24.

v1 proved the compact strategic-economy loop: Scenario-authored player economy start state, owned services, mine payout, unit passive hooks, Trading Post trade data, guarded service claiming, Scenario Result presentation, and campaign progression all work together in the shipped slice and tests.

v2 should build on that proof by making infrastructure management more player-facing and strategically legible. The next work should not inflate into a full AI economy, full item economy, editor tooling, or a broad campaign shell. The goal is to turn the existing runtime systems into deliberate player decisions.

## 2. v2 target

v2 targets a stronger infrastructure-control loop:

- player-facing stationing/unstationing of eligible units at owned services;
- visible benefit from stationed `mine_production` units in normal play;
- a bounded Storage/Garrison foundation only after the stationing seam is proven;
- clearer presentation of owned services, claimed services, and stationed units;
- continued use of authored Scenario start-state, runtime ownership, save/load, and content validation discipline;
- minimal content additions that exercise the loop without creating a large campaign or full economy simulation.

## 3. v2 design constraints

- Use existing runtime state where practical: `OwnedServiceSaveState`, stationed units, owned roster, resources, Gold, and service IDs.
- Keep authored static content separate from runtime mutable state.
- Preserve the stack-backed stationed-unit invariant: stationed units must correspond to owned units where that invariant currently applies.
- Do not add broad team/owner authoring just to support stationing.
- Do not mutate content definitions during gameplay.
- Preserve existing save/load compatibility unless a scoped migration is explicitly selected.
- Avoid per-frame catalog scans, repeated content parsing, graph rebuilds, or hidden nested scans.
- Keep UI/service interactions bounded and text-prompt based until the current rendering model is intentionally expanded.

## 4. v2 first priority

The first selected v2 milestone was:

**M25 — Player-facing Service Stationing Flow**  *(complete)*

M25 made stationing reachable in gameplay through a narrow interaction path without becoming a full Storage/Garrison system: a player can assign an eligible owned unit to an eligible owned mine and see the existing `mine_production` payout benefit through normal play. The questions below record the settled M25 answers; see `docs/implementation_roadmap.md` §4.

M25 answered:

- where the player initiates stationing;
- which owned services can receive stationed units;
- which owned units can be stationed;
- how stationed units are removed or returned;
- how the stack-backed invariant is preserved;
- how the result is shown clearly enough for the player;
- how save/load preserves stationed assignments.

## 5. Candidate v2 milestones after M25

These are candidates, not commitments:

1. **Storage/Garrison Foundation.** Add a bounded service kind or interaction that stores units/guards services, with clear capacity and persistence rules. This should follow M25, not precede it.
2. **Owned Service Presentation / Management View.** Show owned services, claim state, stationed units, and expected payout/tier effects through a readable model/renderer without broad UI sprawl.
3. **Service Destruction / Restoration Slice.** Add a narrow destruction/restoration loop only after ownership and stationing are visible and testable.
4. **Enemy-side Capture Pressure.** Let non-player teams contest or capture player-owned services only after service defense and stationing rules are established.
5. **Inventory / Artifact HUD Presentation.** Present inventory/artifacts in a bounded render model without implementing full item use or crafting.
6. **Campaign Branch-choice Presentation.** Surface multiple `nextScenarioIds` as a choice if authored content needs it.
7. **Market / Black Market / Freelancer Behavior.** Build only one trader-service behavior at a time and avoid broad item-economy sprawl.

## 6. Not in early v2 scope unless explicitly selected

- Full AI economy.
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

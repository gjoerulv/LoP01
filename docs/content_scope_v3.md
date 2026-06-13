# Ashvale Content Scope v3

## 1. Purpose

`content_scope_v3.md` is the active content and systems scope cap after M30.

v1 proved the compact strategic-economy loop. v2 turned infrastructure into a contested, player-facing loop: owned services, stationing, storage, cross-Region generic loss warnings, service defense, storage loss, Temporarily Unavailable heroes, enemy capture pressure, destruction/restoration, and service-state presentation all work together in shipped content and tests.

v3 should not keep adding more isolated infrastructure systems. The next bottleneck is **scenario readiness**: the game needs a player-facing shell, safer authored-content boundaries, stronger validation, better knowledge/visibility rules, clearer threat information, and objective/guidance presentation so a Scenario can be selected, understood, played, won/lost, saved, and resumed without relying on debug-like flows or developer memory.

## 2. v3 target

v3 targets a playable authored Scenario/Campaign experience built on the v1/v2 systems.

The target loop is:

1. the player starts from a shell/menu flow;
2. chooses Campaign, Standalone Scenario, or Tutorial content;
3. invalid or incompatible content is blocked or clearly explained;
4. the Scenario starts with authored start-state and player-facing context;
5. the player can understand visible objectives, known map state, enemy/service threats, and owned assets;
6. Region-layer battles and service threats have readable previews/results;
7. save/load and result flow preserve identity and content compatibility;
8. the Scenario can reach a clear victory/defeat/result state;
9. the resulting v3 proof content can be played as a coherent authored slice.

v3 is about **scenario readiness, player information, and authored progression**. It is not the full final game.

## 3. v3 design constraints

- Use the milestone-agnostic docs as source-of-truth: `game_vision.md`, `core_loop_rules.md`, `combat_rules.md`, `game_shell_flow.md`, `scenario_authoring.md`, `content_schema.md`, `technical_direction.md`, `validation_system.md`, `presentation_game_feel.md`, and `terminology_map.md`.
- Preserve the completed v1/v2 systems unless a scoped milestone explicitly revises them.
- Do not mutate authored static content at runtime.
- Keep runtime mutable state in `GameSession`/save structures or scoped gameplay services.
- Keep gameplay rules out of rendering/input layers.
- Preserve save/load compatibility unless a scoped migration is explicitly selected.
- Prefer bounded shell/menu/panel flows over broad UI rewrites.
- Prefer validation gates and explicit diagnostics over silent fallback behavior.
- Avoid per-frame catalog scans, graph rebuilds, repeated parsing, and hidden nested scans.
- Keep implementation milestones larger than early v2 milestones where practical, but not as broad as the M30 umbrella milestone.
- Do not keep expanding v2. v2 is complete and should be archived by the user.

## 4. Selected v3 milestones

The first selected v3 milestone is:

**M31 — Shell Entry + Scenario/Campaign Selection** *(planned)*.

M31 should implement the first real shell entry path and content-selection gate:

- title/main-menu entry flow;
- Continue / New Game / Load Game at a bounded level;
- Campaign / Standalone Scenario / Tutorial selection surface;
- validation/playability status before start;
- safe blocking or clear messaging for invalid/incompatible content;
- a narrow start handoff into the existing Scenario runtime;
- no character-creation system unless a prebuilt/default path requires a small placeholder;
- no Mods/settings/accessibility shell except stub/disabled entries if needed for layout.

M31 is selected first because every later v3 milestone benefits from a real way to choose, validate, start, load, and resume authored content safely.

Candidate follow-on v3 milestones, to be selected after M31 based on code/doc audit:

1. **Scenario Context + Start-State Authoring.** Add the smallest Scenario Region Context / content-partitioning and authored start-state expansion needed to prevent global-content leakage and support scenario-specific rosters, services, resources, regions, and hero pools.
2. **Fog, Reveal, and Enemy Visibility Foundation.** Add persistent reveal/explored state, basic scouting, visible enemy estimates, and UI/read-model support without implementing full hidden-information AI.
3. **Threat Preview + Auto-Resolve Foundation.** Add cheap threat preview for Region/service battles and a bounded auto-resolve path that uses or aligns with the existing battle rules. This is the direct successor to M30's deterministic strength-comparison stand-in.
4. **Quest / Guidance / Journal Foundation.** Make objectives, guidance, event-updated goals, and victory/defeat explanation player-facing and persistent enough for authored Scenarios.
5. **v3 Content Proof + Release Validation Pass.** Build a small authored Scenario/Campaign proof using the v3 systems and add release-readiness validation checks so v3 can be closed honestly.

## 5. Explicitly out of v3 unless selected later

The following are final-vision systems or future candidates, but should not be smuggled into v3 milestones unless the active roadmap explicitly selects them:

- full item economy, cooking, recipes, seeds, ingredients, consumables, and broad item-use systems;
- full AI economy and equipment behavior;
- full service-management UI with remote stationing/storage/repair/capture actions;
- full editor tooling;
- mod management/load-order implementation beyond shell visibility stubs;
- PvP or networking;
- full battle command depth, spells, timed status effects, broad skill trees, and complete MP/item command systems;
- full final fog/scouting polish beyond the v3 foundation;
- full character-creation depth unless a milestone explicitly selects it;
- complete campaign production content;
- complete Scenario Region Context if a smaller scenario-boundary slice is enough;
- broad trader-service families beyond the current Trading Post foundation.

## 6. Relationship to M30 simplifications

M30 intentionally shipped final-compatible stand-ins. v3 should replace the most consequential stand-ins only when a milestone selects them:

- deterministic absent-player service defense may later become full-simulation auto-resolve with battle AI;
- Temporarily Unavailable heroes currently return to reserve after a weekly delay, standing in for shared hero-pool re-entry;
- enemy pressure currently targets player-owned services only and captures rather than sabotages/destroys;
- event/service log lines still lean on ids in places where final presentation wants display names;
- owned-service overview remains read-only and is not the final service-management UI;
- Scenario/Campaign authoring remains thinner than the final docs describe;
- shell flow is still incomplete relative to `game_shell_flow.md`.

## 7. v3 closure definition

v3 is complete when the project has a coherent authored Scenario/Campaign readiness slice:

- shell/menu selection can start valid playable content and reject invalid content safely;
- Scenario/Campaign authoring has enough scoping/start-state support to avoid obvious global-content leakage;
- player-visible map/enemy/service knowledge is sufficient for informed Region decisions;
- threat preview/auto-resolve foundations reduce reliance on hidden deterministic stand-ins;
- objectives/guidance are visible and persistent;
- save/load and result flow preserve content identity and compatibility;
- a shipped proof Scenario/Campaign demonstrates the loop; and
- the active roadmap can honestly archive `content_scope_v3.md`.

## 8. Roadmap discipline

`docs/implementation_roadmap.md` is the active milestone plan for this scope.

Archived docs are historical context only. Do not use archived roadmap/scope files as current requirements unless comparing history.

Milestone-agnostic docs remain source-of-truth for final rules. Update them only when the actual long-term rule, schema, terminology, or architectural direction changes.

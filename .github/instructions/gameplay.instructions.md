---
applyTo: "src/gameplay/**/*.cpp,src/gameplay/**/*.h,src/data/**/*.cpp,src/data/**/*.h,docs/**/*.md"
---

# Gameplay implementation instructions

- Treat the repository and the current active design docs as the source of truth.
- Respect `docs/implementation_roadmap.md` for current sequencing and not-yet boundaries.
- Respect `docs/content_scope_v1.md` for post-M17 content-scope limits.
- Respect `docs/technical_direction.md` for architecture, state ownership, and performance principles.
- Respect the time, travel, Energy, service, ownership, and economy rules in `docs/core_loop_rules.md`.
- Respect battle logic exactly as described in `docs/combat_rules.md`.
- Respect the current high-level design intent in `docs/game_vision.md`.
- Respect content authoring rules in `docs/scenario_authoring.md` when changing content schemas, events, quests, or Services.
- Respect validation levels, severities, and gates in `docs/validation_system.md` when changing validators or validation reports.
- Respect content schema conventions in `docs/content_schema.md` when changing content JSON, loaders, schemas, or validation.
- Use `docs/terminology_map.md` when legacy runtime or content names differ from current design language.
- Treat archived docs as historical context only.

## Current baseline

The project is post-M17. Battle, roster, save/load, typed events, scenario outcomes, inventory/artifacts, team Energy, minimal World Map travel, minimal Campaign, and owned-service/economy foundations exist.

Do not write guidance or implementation assumptions as if the project is still post-M8, post-M11, post-M16, or single-Region only.

## Terminology

Use current design terminology in discussion and new docs:

- `World Map` = the scenario-level Region selection and information layer.
- `Region` = the main in-scenario travel layer.
- `Location` = an entered place inside a Region.
- `Service` = an interaction available directly in a Region or inside a Location.

Runtime code or serialized values may still contain legacy names. Do not broaden or reinforce outdated terminology in new design work.

## Layer separation

- Keep the major gameplay layers distinct:
  - World Map;
  - Region;
  - Location;
  - Battle;
  - Progression;
  - Campaign.
- Do not collapse Region mode and Location mode into one system.
- Battles must remain a separate reusable module shared by Region and Location encounters.
- Do not assume a Location is just a flavored Region node.
- Entering a Location is a real layer transition.

## Region structure

- Regions are authored node graphs with systemic rules on top.
- Use the node-content model: nodes are travel points whose gameplay behavior comes from main node content plus event attachments.
- Keep main node content single-purpose.
- A node may contain at most one main content item, such as a resource pickup, artifact pickup, Service, neutral enemy, or one-time special content.
- Do not introduce a dedicated combat-node abstraction.
- Blocker behavior is usually created by content, such as a gate service, neutral enemy, hostile team occupation, or authored rule.
- One-time content usually resolves back into an empty travel node when cleared.
- `Arrival` is a flag, not a node type.
- Arrival nodes may also be Location or Service nodes, but enemies may not spawn on them or occupy them.

## Location and Service rules

- A direct Service node is for quick functional interaction on the Region layer.
- A Location is for entered, explorable spaces with zero or more services, NPCs, screens, and possible dungeon-like content.
- Dungeons are a kind of Location.
- Entering and exiting a Location does not cost time.
- Region travel must be initiated from the Region layer, not from inside a Location.

## Region travel and Energy

- Travel inside a Region uses the shortest valid reachable path.
- The player may select any reachable node directly.
- Same-node travel is not a meaningful move and should not be treated as normal travel.
- Blocked nodes remain visible even when travel to them is illegal.
- Routes have terrain or road quality, and route quality affects both travel time and Energy cost when implemented.
- Energy belongs to the traveling party, not to individual units.
- Daily starting Energy is based on the current traveling-party rules in `docs/core_loop_rules.md`.
- World Map travel uses the shortest valid contiguous path through enterable Regions and costs the settled one-time Energy amount when travel begins.
- Do not reintroduce the old "one region-travel allowance per day" rule.

## Party and storage assumptions

Respect the settled party model:

- `Active party` = the current battle-legal party, up to 5 units.
- `Reserve` = additional traveling units, up to 7.
- `Traveling party` = active party + reserve.
- `Stored units` = units tied to a specific storage service and not traveling with the player.
- Hero units and stackable generic units must continue to be modeled differently.
- Do not assume traveling generic units survive Region transfer.
- Do not assume stored units are interchangeable with reserve units.
- Storage may exist in a Location or as a direct Region service.
- A direct storage service may act as a defensible gate for the owning side.

## Enemy teams

- Enemy teams are Region-layer traveling parties that follow the same broad party rules as the player side.
- Enemy teams may contain heroes and generic units.
- Enemy teams do not enter Locations.
- Enemy teams may occupy Region nodes, use direct Region services, recruit, rest, and attack defensible storage gates.
- If an enemy team occupies a Location node, the Location is inaccessible until that team is defeated.
- When design docs and current runtime disagree on future enemy-team behavior, preserve current behavior unless the task explicitly includes a design-alignment refactor.

## Owned services and economy

When touching owned services or economy:

- Mines can be owned and generate passive daily resources for the owning team.
- Stationed guards can increase mine output only through explicit passive effects.
- Mine production passives do not stack.
- For each owned service instance and output resource, only the strongest applicable stationed passive counts.
- Ties do not stack.
- Heroes and generic units are both valid if they have the applicable passive.
- Trader ownership benefits are per service type.
- Owning more Markets affects Market pricing only; owning more Trading Posts affects Trading Post rates only.
- Ownership benefits apply only when the owning team uses a service of the same type that it owns.
- Allied-owned services do not count.
- Ownership tiers cap at 8 owned services of the same type.
- Ownership does not bypass lock, destruction, hostile occupation, stock, eligibility, story, or service availability rules.

The current runtime implements the narrow foundation for these rules. Do not implement a full AI economy, broad skill tree, full trader item economy, or broad ownership-transfer loop unless explicitly selected.

## Source comments

Production gameplay/data source comments should be rare and durable.

Use them for invariants, non-obvious contracts, save/load or validation traps, compatibility constraints, and performance-sensitive choices. Do not add milestone/phase labels such as `M17 Phase 3a` to production source as permanent comments. Milestone context belongs in roadmap docs, decision logs, commit messages, prompts, and tests. Prefer names and tests over explanatory comments when the code is straightforward. Remove or update stale comments in the same patch that changes the behavior they describe.

Test comments may explain non-obvious regression intent, especially around save compatibility, validation, stale references, and edge-case rules.

## General implementation guidance

- Keep game rules data-driven where practical.
- Prefer explicit mode-entry helpers for World Map, Region, Location, Battle, and Campaign transitions instead of chaining generic mode advancement.
- Prefer a single source of truth for status or event text; remove duplicate shadow strings once event-driven text exists.
- Player defeat, sleep-failure penalties, temporary hero unavailability, and post-battle fallout must stay consistent with the current design docs.
- Do not invent major story elements that contradict `docs/game_vision.md`.
- Prefer a playable incomplete feature over a broad unfinished feature.
- When terminology migration is the task, keep it behavior-preserving and avoid unrelated gameplay refactors.
- When a historical milestone doc conflicts with current design docs, follow the current design docs.
- Respect player-character rules: the player character is a unique hero with special human-team rules and must remain in the traveling party.
- Respect `docs/game_shell_flow.md` for game start, character creation entry flow, save/load shell rules, and settings categories.
- Respect `docs/presentation_game_feel.md` for feedback, pacing, mode transitions, battle intro/result presentation, and accessibility-sensitive presentation actions.

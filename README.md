# Project Ashvale

Ashvale is a turn-based strategy/RPG built as a content-driven C++20/raylib/CMake project. The repository is a post-M18 bounded multi-Region, multi-Scenario playable vertical slice.

Current implementation status, milestone sequencing, and explicit not-yet boundaries live in `docs/implementation_roadmap.md`. For terminology, see `docs/terminology_map.md`.

## Design truth and doc priority

Use these files as the main current design references:

- `docs/implementation_roadmap.md`
- `docs/content_scope_v1.md`
- `docs/technical_direction.md`
- `docs/game_vision.md`
- `docs/game_shell_flow.md`
- `docs/presentation_game_feel.md`
- `docs/core_loop_rules.md`
- `docs/combat_rules.md`
- `docs/scenario_authoring.md`
- `docs/validation_system.md`
- `docs/content_schema.md`
- `README_DECISIONS.md`
- `docs/terminology_map.md`

Archived docs, including `docs/content_scope_v0.md.archived` and `docs/implementation_roadmap.md.00.archived`, are historical context only.

## Implementation planning

Current implementation sequencing lives in `docs/implementation_roadmap.md`.

The current next planned milestone is **M19: Service Economy Expansion**, unless the user explicitly redirects.

## Current implementation baseline

Completed foundations include:

- battle engine, CTB, static formation, leader aura, deterministic damage, and battle write-back;
- persistent roster, active/reserve party, mustering, and save/load;
- daily clock, Region travel, wake/recovery penalty, and basic services;
- content validation foundation;
- typed event foundation;
- practical enemy-team Region-layer foundation, including patrol movement, hostile occupation, hostile-contact battles, event mutation actions, and save/load;
- deterministic scenario outcome rules with authored victory and defeat conditions;
- inventory and artifact foundation, including per-hero equipment and equipped-artifact battle stat bonuses;
- team Energy pool with daily-starting formula, spend/recover, day-rollover reset, save/load, HUD exposure, and current-leader `leader_energy` passive contribution;
- minimal World Map region-to-region travel from authored exit nodes;
- minimal Campaign System with thin scenarios, transition graph, explicit allow-list carry-over, campaign state, and campaign selection;
- owned-service and economy foundation with resource pool, owned-service runtime state, mine outputs, stack-backed stationing, day-boundary mine payout, trader ownership tiers, authored/default trader curves, and validation/test coverage;
- passive/effect spine foundation with canonical unit `passive_effects`, legacy mine-passive authoring compatibility, typed validation, M17 mine-production behavior preservation, and leader `leader_energy` feeding the daily Energy passive term.

## Current architecture baseline

The current codebase follows these principles:

- explicit app shell and gameplay session flow;
- controller / mapper / renderer split;
- gameplay logic separated from rendering and input;
- typed content loaded from JSON;
- content-driven Regions, Locations, Services, units, battles, quests, events, outcomes, items, artifacts, World Map, scenarios, campaigns, owned services, resources, mine outputs, unit passive effects, and trader ownership curves;
- pure or mostly-pure gameplay rules where practical;
- save/load focused on gameplay state rather than presentation state.

Future work should preserve this baseline unless there is a strong reason to refactor it.

## World structure baseline

Use this hierarchy:

- **Campaign**
- **Scenario**
- **World Map**
- **Region**
- **Node**
- **Location**
- **Service**

Older runtime/serialized terms such as `overworld`, `overworld_mode`, or `overworld_selection` are compatibility names, not preferred design language.

## Energy baseline

Daily starting Energy is:

`1000 + (lowest traveling-party agility * 100) + leader passive bonus + leader item/artifact bonus`

The leader passive bonus is implemented through the current leader unit's `leader_energy` passive effects. The leader item/artifact Energy term remains deferred.

## Owned service and economy baseline

Owned services are strategic objects controlled by a team. Implemented foundation and settled direction:

- Mines can be owned and generate passive daily resources for the owning team.
- Stationed guards can increase mine output only through explicit `mine_production` passive effects.
- Mine production passives do not stack.
- For each owned service instance and output resource, only the strongest applicable stationed passive counts.
- Ties do not stack.
- Heroes and generic units are both valid if they have the applicable passive effect.
- Trader services can be owned: Trading Post, Market, Freelancer's Guild, Black Market.
- Ownership benefits are per service type and apply only when the owning team uses a service of that same type that it owns.
- Allied-owned, enemy-owned, and neutral services do not grant ownership benefits to the player.
- Ownership tiers cap at 8 owned services of the same type.
- Ownership does not bypass lock, destruction, hostile occupation, stock, eligibility, story, or service availability rules.

Full trader UI, item-market transactions, AI economy, ownership-transfer loops, and broad passive/skill systems remain future work.

## Passive effect baseline

Unit definitions may author canonical `passive_effects`:

- `mine_production` applies only to stationed units on owned mine/resource services and feeds mine-output calculation.
- `leader_energy` applies only from the current leader and feeds the daily Energy passive bonus.

The legacy `mine_production_passive` authoring key is still accepted as compatibility input and is converted at load to canonical `passive_effects`. Authoring both keys on the same unit is invalid. Runtime consumers read the canonical passive-effect representation, not a legacy runtime field.

Artifact `statBonus` battle effects remain on the existing artifact path. Artifact Energy, item effects, status effects, skill trees, and active abilities are not part of the current passive-effect spine.

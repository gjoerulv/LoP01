# Content Scope v1

## Purpose

This is the active content-scope document for the post-M18 Ashvale project. It replaces the archived pre-M15/M16 bounded-slice scope in `docs/content_scope_v0.md.archived`.

Use this document to decide **how much authored content** a milestone should add and which systems that content is allowed to exercise. It is not the full game vision, not a schema reference, and not the implementation roadmap.

Related active docs:

- `docs/implementation_roadmap.md` — active technical roadmap after M18.
- `docs/content_schema.md` — intended and current authored data shapes.
- `docs/scenario_authoring.md` — authoring rules and validation expectations.
- `docs/core_loop_rules.md` — systemic gameplay rules.
- `docs/technical_direction.md` — architecture and performance principles.
- `docs/terminology_map.md` — current terminology.

Archived docs should be read only for historical context.

## 1. Current playable baseline

The current baseline is post-M18:

- C++20 / raylib / CMake project.
- Content-driven JSON loading through `ContentRepository`.
- Battle, roster, reserve, save/load, quests, typed events, scenario outcomes, inventory/artifacts, Energy, World Map, Campaign, owned-service/economy, and passive-effect foundations exist.
- The playable content is a bounded multi-Region, multi-Scenario vertical slice.
- M17 introduced the owned-service/economy foundation: resources, owned services, mine outputs, stack-backed stationing, daily mine payout, trader ownership tiers, and authored/default trader curves.
- M18 introduced a narrow unit passive-effect spine: canonical `passive_effects`, legacy mine-passive authoring compatibility, `mine_production` effects for owned mines, and `leader_energy` effects for daily starting Energy.
- Full per-Scenario Region Contexts, per-scenario content directories, authored starting rosters, and full campaign branching UI are not implemented.

The current content should prove systems generically. Do not add demo-specific source branches to make authored content work.

## 2. v1 content goal

The v1 content goal is a compact strategic-economy proof:

> A compact campaign slice where owned services matter: mines produce resources, stationed guards can influence mine output through explicit passive effects, owned trader services create service-type-specific economy advantages, and leader passives can affect daily strategic Energy.

Recommended scale for v1:

- 1 authored campaign.
- 2 to 3 scenarios.
- 2 to 4 Regions total.
- 5 to 10 Region nodes per Region unless a milestone explicitly needs more.
- A small number of owned services: at least one mine/resource service, at least one storage/garrison-compatible service, and at least one trader service type.
- A small number of units with passive service/economy hooks: `mine_production` and `leader_energy`.
- Enough enemy teams to test ownership pressure and guarded services, but not a full AI economy.

This target is intentionally modest. The project should gain systemic depth before content volume.

## 3. Owned service content rules

Ownership benefits apply to the owning team only unless a later system explicitly changes alliance benefits. Allies do not automatically receive ownership benefits.

Service ownership must not bypass:

- locked service state;
- destroyed or temporarily unavailable service state;
- hostile occupation;
- stock limits;
- eligibility checks;
- story/event requirements;
- service-specific availability rules.

## 4. Mine and resource-service scope

Mines/resource services may generate passive daily resources for the owning team.

Owned resource output rules:

- Base output comes from authored service data or service defaults.
- The owning team receives the output at the appropriate daily payout step.
- Units may be stationed at owned mines/resource services as guards.
- Stationed units can affect production only through explicit `mine_production` passive effects.
- Production passives do not stack.
- For each owned service instance and each resource output, only the single strongest applicable stationed passive applies.
- Ties do not stack.
- If a service outputs multiple resources, compute the strongest modifier separately per resource.
- Stationed-unit defensive behavior remains separate from production behavior.

Do not implement or author broad passive-skill trees just to support this rule. Use the narrow passive-effect surfaces implemented by M18.

## 5. Trader-service ownership scope

Trader service types:

- Trading Post;
- Market;
- Freelancer's Guild;
- Black Market.

Ownership tier rules:

- Each owned service of the same trader service type contributes one ownership tier.
- Ownership tiers are capped at 8 owned services.
- Owning more services of one type must not affect another type.
- Benefits apply when the owning team uses a service of that same type that it owns.
- Allied-owned, enemy-owned, and neutral services do not grant benefits to the player.

M17 established the ownership tier model, validation, default curves, and minimal proof. Future milestones may build player-facing trader transactions, item economy, or service UI on this foundation.

## 6. Passive-effect content scope

The current passive-effect spine supports only unit passive effects with immediate consumers:

- `mine_production`: applies to stationed stack-backed units on owned mine/resource services; feeds daily mine payout.
- `leader_energy`: applies only to the current leader; feeds daily starting Energy.

Rules:

- Use canonical `passive_effects` for new content.
- `mine_production_passive` is a legacy authoring alias for M17 compatibility only.
- Do not author both `mine_production_passive` and `passive_effects` on the same unit.
- Do not invent new passive-effect kinds in content before code/schema/validation support them.
- Do not use unit passive effects as a substitute for artifact effects, item effects, skill trees, or battle statuses.

Artifact `statBonus` battle effects remain on the artifact path. Artifact Energy, item effects, status effects, active abilities, and skill-tree UI are out of v1 content scope unless a later milestone explicitly promotes them.

## 7. Content constraints for AI agents

When modifying content:

- Keep ids stable and opaque.
- Prefer adding small, reusable content objects over hardcoded demo branches.
- Use current schema fields; do not invent unsupported mechanics in JSON.
- If a desired content behavior needs a new system, update the roadmap rather than faking it in content.
- Put systemic rules in `core_loop_rules.md`, schema shape in `content_schema.md`, validation details in `validation_system.md`, and milestone scope in `implementation_roadmap.md`.
- Keep authored proof content minimal and representative.
- Every new content system must have loader tests, validation tests, and at least one end-to-end or integration proof where practical.

## 8. Not in v1 content scope

The following remain out of scope unless explicitly promoted by the active roadmap:

- full procedural generation;
- full editor tooling;
- mod loading;
- large campaign branching UI;
- per-scenario content directories;
- full Scenario Region Context implementation;
- full hero-instance identity model;
- full skill tree UI;
- full AI economy;
- generic origin-storage across Regions;
- campaign save-slot UI;
- broad recipe/cooking/artifact-combination suite unless selected as a milestone;
- broad item-market economy beyond the current owned-service foundation;
- artifact Energy and item-effect execution until selected by roadmap.

## 9. Update rule

Update this document when the playable content target changes. Do not use it as a changelog. The implementation roadmap owns milestone status.

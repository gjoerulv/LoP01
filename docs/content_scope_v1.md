# Content Scope v1

## Purpose

This is the active content-scope document for the post-M16 Ashvale project.

It replaces the archived pre-M15/M16 bounded-slice scope in `docs/content_scope_v0.md.archived`.

Use this document to decide **how much authored content** a milestone should add and which systems that content is allowed to exercise. It is not the full game vision, not a schema reference, and not the implementation roadmap.

Related active docs:

- `docs/implementation_roadmap.md` — active technical roadmap after M16.
- `docs/content_schema.md` — intended and current authored data shapes.
- `docs/scenario_authoring.md` — authoring rules and validation expectations.
- `docs/core_loop_rules.md` — systemic gameplay rules.
- `docs/technical_direction.md` — architecture and performance principles.
- `docs/terminology_map.md` — current terminology.

Archived docs should be read only for historical context.

---

## 1. Current playable baseline

The current baseline is post-M16:

- C++20 / raylib / CMake project.
- Content-driven JSON loading through `ContentRepository`.
- Battle, roster, reserve, save/load, quests, typed events, scenario outcomes, inventory/artifacts, Energy, World Map, and Campaign foundations exist.
- The playable content is a bounded multi-Region, multi-Scenario vertical slice.
- M15 introduced minimal World Map region-to-region travel.
- M16 introduced thin Scenario and Campaign definitions, explicit carry-over, and campaign selection.
- Regions, units, items, artifacts, events, quests, services, and the World Map remain globally loaded.
- Full per-Scenario Region Contexts, per-scenario content directories, authored starting rosters, and full campaign branching UI are not implemented.

The current content should prove systems generically. Do not add demo-specific source branches to make authored content work.

---

## 2. v1 content goal

The next playable content target should move Ashvale from a traversal/campaign proof into a small strategic-economy proof.

The v1 content goal is:

> A compact campaign slice where owned services matter: mines produce resources, stationed guards can influence mine output through explicit passive effects, and owned trader services create service-type-specific economy advantages.

The content should remain small enough that tests and manual verification stay practical.

Recommended scale for v1:

- 1 authored campaign.
- 2 to 3 scenarios.
- 2 to 4 Regions total.
- 5 to 10 Region nodes per Region unless a milestone explicitly needs more.
- A small number of owned services:
  - at least one mine/resource service,
  - at least one storage/garrison-compatible service,
  - at least one trader service type,
  - optionally one service from each trader type once the rules exist.
- A small number of units with passive service/economy hooks.
- Enough enemy teams to test ownership transfer and guarded services, but not a full AI economy.

This target is intentionally modest. The project should gain systemic depth before content volume.

---

## 3. Owned service content rules

Owned services are strategic objects controlled by a team.

Examples:

- mines / resource services,
- Markets,
- Trading Posts,
- Black Markets,
- Freelancer's Guilds,
- gates and storage-like service points where units can be stationed.

Ownership benefits apply to the owning team only unless a later system explicitly changes alliance benefits.

Allies do not automatically receive ownership benefits.

Service ownership must not bypass:

- locked service state,
- destroyed or temporarily unavailable service state,
- hostile occupation,
- stock limits,
- eligibility checks,
- story/event requirements,
- service-specific availability rules.

---

## 4. Mine and resource-service scope

Mines/resource services may generate passive daily resources for the owning team.

Owned resource output rules:

- Base output comes from authored service data or service defaults.
- The owning team receives the output at the appropriate daily payout step.
- Units may be stationed at owned mines/resource services as guards.
- A team must own a mine/resource service before stationing guards there.
- If enemy units guard or occupy the service, they must be defeated before ownership/stationing changes.
- Stationed units can affect production only through explicit passive effects.
- Production passives do not stack.
- For each owned service instance and each resource output, only the single strongest applicable stationed passive applies.
- Ties do not stack. Two `+2 Stone` passives still produce `+2 Stone`, not `+4 Stone`.
- If a service outputs multiple resources, compute the strongest modifier separately per resource.
- Stationed-unit defensive behavior remains separate from production behavior.

Example:

- A Stone Mine produces `2 Stone`.
- A stationed Kobold has `+1 Stone output at Stone Mine`.
- A stationed Stonewright has `+2 Stone output at Stone Mine`.
- Output is `4 Stone`, not `5 Stone`, because only the strongest applicable Stone modifier applies.

Do not implement or author broad passive-skill trees just to support this rule. The content should use the narrow passive-effect surface implemented by the active milestone.

---

## 5. Trader-service ownership scope

Trader-service ownership benefits are grouped by service type.

Trader service types:

- Trading Post,
- Market,
- Freelancer's Guild,
- Black Market.

Ownership tier rules:

- Each owned service of the same trader service type contributes one ownership tier.
- Ownership tiers are capped at 8 owned services.
- Owned Markets affect Market prices only.
- Owned Trading Posts affect Trading Post exchange rates only.
- Owned Black Markets affect Black Market prices only.
- Owned Freelancer's Guilds affect Freelancer's Guild rates only.
- Owning more services of one type must not affect another type.
- Benefits apply when the owning team uses a service of that same type that it owns.
- Allied-owned services do not count.
- Enemy-owned services do not count.
- Neutral services do not grant owned-service benefits unless later rules explicitly support neutral reputation or access.

Curves:

- Each service type has a default ownership curve.
- Designers may author service-type-specific curves when supported.
- Trading Post exchange should use an authored tier matrix when supported.
- Curves are capped by authored settings and the global 8-tier cap.

M17 should avoid deep economy simulation. It should establish the ownership tier model, validation, default curves, and a minimal proof.

---

## 6. Content constraints for AI agents

When modifying content:

- Keep ids stable and opaque.
- Prefer adding small, reusable content objects over hardcoded demo branches.
- Use current schema fields; do not invent unsupported mechanics in JSON.
- If a desired content behavior needs a new system, update the roadmap rather than faking it in content.
- Do not duplicate rules across many docs. Put systemic rules in `core_loop_rules.md`, schema shape in `content_schema.md`, validation details in `validation_system.md`, and milestone scope in `implementation_roadmap.md`.
- Keep authored proof content minimal and representative.
- Every new content system must have:
  - loader tests,
  - validation tests,
  - at least one end-to-end or integration proof where practical.

---

## 7. Not in v1 content scope

The following remain out of scope unless explicitly promoted by the active roadmap:

- full procedural generation,
- full editor tooling,
- mod loading,
- large campaign branching UI,
- per-scenario content directories,
- full Scenario Region Context implementation,
- full hero-instance identity model,
- full skill tree UI,
- full AI economy,
- generic origin-storage across Regions,
- campaign save-slot UI,
- large world-map art/polish pass,
- broad recipe/cooking/artifact-combination suite unless selected as a milestone.

---

## 8. Update rule

Update this document when the playable content target changes.

Do not use it as a changelog. The implementation roadmap owns milestone status.

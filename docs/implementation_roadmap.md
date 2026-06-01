# Ashvale Implementation Roadmap

## Purpose

This is the active post-M16 implementation roadmap.

The previous foundational roadmap is archived as `docs/implementation_roadmap.md.00.archived`. That file records the Phase 1–7 build-up to the current architecture. Do not continue the archived roadmap in place.

Use this document to plan the next technical milestones from the current baseline.

Related docs:

- `docs/technical_direction.md` — architecture and performance principles.
- `docs/content_scope_v1.md` — active content-scope target.
- `docs/content_schema.md` — authored data shapes.
- `docs/scenario_authoring.md` — authoring rules and validation expectations.
- `docs/validation_system.md` — validation model and validation-code expectations.
- `docs/core_loop_rules.md` — systemic gameplay rules.
- `docs/game_shell_flow.md` — player-facing shell flow outside active gameplay.
- `docs/terminology_map.md` — terminology.

---

## 1. Current baseline

The codebase is a post-M16 bounded multi-Region, multi-Scenario vertical slice.

Stable foundation already implemented:

- explicit `App` / `GameSession` flow,
- controller / mapper / renderer split,
- C++20 + raylib + CMake + nlohmann/json + Catch2,
- battle engine and battle write-back,
- roster, active party, reserve, save/load,
- Region graph travel and Location entry,
- time, day rollover, wake/recovery penalty,
- team Energy pool with daily-starting formula, spend/recover primitives, and day-rollover reset,
- typed events and typed conditions,
- story flags and fired event ids,
- start-of-day and region-node-entry event firing,
- enemy teams on the Region layer,
- hostile occupation and contact battles,
- enemy-team runtime persistence including alliances,
- scenario outcome rules with defeat priority, default victory fallback, authored victory/defeat, and latched outcome persistence,
- inventory and artifact foundation:
  - item definitions,
  - artifact definitions,
  - team item inventory,
  - unequipped artifact inventory,
  - per-hero artifact equipment,
  - `giveItem`, `takeItem`, `giveArtifact`, `takeArtifact`,
  - equipped-artifact stat bonuses in battle setup,
- minimal World Map:
  - optional `content/world_map.json`,
  - exit-node-gated World Map access,
  - 1000 Energy region-to-region travel gate,
  - before-11:00 departure rule,
  - next-day 11:00 arrival,
  - generic traveling-party unit loss with warning,
  - persisted unlocked Region set,
- minimal Campaign System:
  - optional `content/scenarios.json`,
  - optional `content/campaigns.json`,
  - thin Scenario definitions,
  - Campaign definitions,
  - campaign selection shell,
  - victory-only linear progression,
  - defeat ends campaign run,
  - explicit carry-over allow-list,
  - campaign save/load state,
  - scenario transition reset contract,
  - global scenario-outcome fallback preserved.

The original foundation phases are complete in the minimal sense. The project is not feature-complete.

---

## 2. Active direction

The next phase should deepen strategic gameplay instead of adding more traversal/shell infrastructure.

The highest-value next direction is:

> Owned Services and Economy Foundation.

Reason:

- Mines, trader services, ownership, stationed units, resources, and passive service bonuses are core to the strategy layer.
- Recent design decisions changed mine-production and trader-service ownership rules.
- These systems connect future AI, economy, passive skills, storage, ownership transfer, and campaign progression.
- They are more valuable now than presentation polish or a broader campaign UI.

Move faster by planning larger cohesive milestones, but avoid combining unrelated systems into one sprint.

---

## 3. M17 — Owned Services and Economy Foundation

### Goal

Make owned services real enough for strategic gameplay.

M17 should establish:

- service ownership runtime state,
- stationing guards at owned services where legal,
- mine/resource-service daily output,
- strongest-only stationed passive production modifiers,
- owned trader-service tier calculation,
- save/load and validation,
- small content proof.

### M17-a — Owned service runtime model and validation

Scope:

- Define a minimal runtime representation for service ownership.
- Distinguish resource services from trader services where needed.
- Track owner team id / color for owned services.
- Track stationed guards for services that allow storage/garrison.
- Validate service ownership/stationing authoring.
- Persist service ownership and stationed guards.
- Keep current Region/Location/Service architecture intact.

Acceptance:

- Owned service state round-trips through save/load.
- Services that allow stationing can hold stationed units.
- Stationing is legal only for the owning team.
- Hostile guarded/occupied services cannot be taken/stationed without battle/clearance.
- Validation catches unknown service ids, invalid owner ids, invalid stationing config, and impossible references.

Out of scope:

- full AI service use,
- full passive skill engine,
- full economy curves,
- service UI polish,
- generic origin-storage implementation beyond what is needed for service stationing.

### M17-b — Mine/resource-service daily payout

Scope:

- Implement daily payout for owned mine/resource services.
- Use authored/default base output.
- Add a narrow passive-effect seam for stationed production modifiers.
- Apply strongest-only rule:
  - per owned service instance,
  - per output resource,
  - strongest applicable stationed passive only,
  - ties do not stack.
- Support multi-resource outputs by computing strongest modifier per resource.
- Keep defensive stationing separate from production.

Acceptance:

- Owned mine generates resources for owning team at daily payout.
- Unowned/enemy-owned mines do not pay the player.
- Stationed units without matching passive do not change output.
- Matching stationed passives apply only when the owning team owns and stations the unit.
- Strongest modifier wins.
- Multi-resource output applies strongest modifier separately per resource.
- Tests cover ties, wrong owner, no stationed units, and multiple modifiers.

Out of scope:

- full passive-skill tree,
- UI for stationed-unit passives,
- AI optimization of stationed units,
- alliance economy benefits.

### M17-c — Trader-service ownership tiers

Scope:

- Implement owned trader-service tier calculation.
- Service types:
  - Market,
  - Trading Post,
  - Black Market,
  - Freelancer's Guild.
- Each owned service of the same type contributes one ownership tier.
- Tier cap: 8.
- Owned Markets affect Market prices only.
- Owned Trading Posts affect Trading Post exchange rates only.
- Owned Black Markets affect Black Market prices only.
- Owned Freelancer's Guilds affect Freelancer's Guild rates only.
- Benefits apply only when the owning team uses an owned service of that same type.
- Add default curves and authored curve hooks.

Acceptance:

- Ownership tiers count only same-type owned services.
- Allied/enemy/neutral services do not contribute.
- Tier cap is enforced at 8.
- Each service type can calculate its default discount/exchange effect.
- Authored override curves validate structurally.
- Tests prove service-type isolation.

Out of scope:

- full shop UI,
- full item buying/selling economy if the service flow is not ready,
- full resource exchange UI if not already available,
- AI trading,
- market stock simulation beyond current service capabilities.

---

## 4. M18 — Passive Effect Spine

### Goal

Avoid one-off passive hooks by creating a small, typed passive-effect spine.

M18 should absorb the narrow passive seams created by M17 and prepare future systems.

Likely scope:

- Unit-authored passive effect definitions.
- Passive effect categories with strict enum/type validation.
- Query helpers for:
  - stationed mine-output bonus,
  - leader Energy bonus,
  - future service/economy modifiers.
- Pure evaluation functions.
- No skill-tree UI.
- No broad status-effect framework unless proven necessary.
- No combat passive explosion.

Acceptance:

- Mine-output passive from M17 can be represented through the new passive spine.
- M14 leader Energy bonus seam can query the passive spine while remaining zero/default if no passive exists.
- Unsupported passive effect types fail validation.
- Tests prove strongest-only evaluation and category isolation.

Out of scope:

- skill trees,
- leveling,
- passive UI,
- broad effect dispatch engine,
- combat status system.

---

## 5. M19 — Service Economy Expansion

### Goal

Make trader services more useful after ownership tiers exist.

Candidate scope:

- Market buy/sell interactions.
- Trading Post exchange-rate matrix.
- Black Market artifact purchase flow.
- Freelancer's Guild generic-unit transaction flow.
- Stock/cost previews.
- Ownership-tier impact visible in service calculations.
- Save/load of service stock where needed.

This milestone should be planned after M17 shows which service flows already exist and which are still placeholders.

Out of scope unless explicitly selected:

- full economy AI,
- dynamic pricing simulation,
- global supply/demand,
- large shop UI polish.

---

## 6. M20 — Items, Recipes, Cooking, and Artifact Combination

### Goal

Turn M13 item/artifact definitions into more playable interactions.

Candidate scope:

- field-use item consumption,
- battle `Item` command,
- cooking recipes,
- food effects,
- artifact combination recipes,
- artifact-handler service,
- validation for recipe cycles and invalid outputs.

This is a large system cluster. Split it if needed.

Do not start this before M17/M18 if owned-service/passive foundations are more urgent.

---

## 7. M21+ candidates

Potential later milestones:

- campaign follow-up:
  - campaign branching choice UI,
  - rewards,
  - better result screen,
  - campaign save-slot UX,
- per-Scenario Region Contexts,
- per-region enemy/world-state partitioning,
- event-driven Region unlock,
- generic origin-storage across Regions,
- AI service use and sabotage,
- presentation and shell polish,
- mod loading,
- editor/tooling,
- larger content expansion.

These are not current priorities unless the active milestone explicitly promotes them.

---

## 8. Non-regression rules

Future work must not regress:

- M12 scenario outcome semantics:
  - defeat priority,
  - authored victory override,
  - default victory only when no authored victory conditions exist,
  - latched outcome persistence.
- M13 artifact ownership invariant:
  - equipped artifacts live only in hero equipment,
  - unequipped artifacts live only in team artifact inventory.
- M14 Energy semantics:
  - negative spend fails,
  - recover clamps to daily max,
  - day rollover resets Energy.
- M15 World Map semantics:
  - exit-node-gated access,
  - 1000 Energy departure gate,
  - next-day 11:00 arrival,
  - generic traveling-party units are lost with warning.
- M16 Campaign semantics:
  - campaign optionality,
  - standalone play remains legal,
  - thin scenarios,
  - global fallback scenario outcome preserved,
  - explicit carry-over allow-list,
  - no authored starting roster in M16 model.

---

## 9. Architecture constraints

Follow `docs/technical_direction.md`.

When adding systems:

- Keep pure rules testable without raylib.
- Keep App as orchestration, not rule storage.
- Keep rendering models separate from gameplay state.
- Do not parse content in frame loops.
- Avoid repeated full-content scans in update/render paths.
- Prefer id-to-index maps or prevalidated references for repeated lookups.
- Avoid unnecessary per-frame allocation/string churn.
- Keep save/load additive and migration-safe.
- Prefer explicit content fields over inferred magic.
- Do not hardcode demo ids in source.
- Every authored demo must exercise generic systems.

---

## 10. Validation gate for every milestone

A milestone is not complete until:

- source builds,
- full Catch2 suite passes,
- new pure rules have unit tests,
- new content loaders have validation tests,
- save/load migration is tested when save state changes,
- real content proof exists when the feature is player-visible,
- docs are updated and no active docs contradict source,
- deferred items are listed explicitly.

---

## 11. Archived roadmap policy

When this roadmap becomes too large or a new major stage begins:

1. Rename it to `docs/implementation_roadmap.md.01.archived`.
2. Create a new `docs/implementation_roadmap.md`.
3. Preserve this filename for active roadmap references.

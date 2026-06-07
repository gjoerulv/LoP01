# Gameplay implementation instructions

These instructions apply to gameplay, data, validation, save/load, and test work.

## Current baseline

Treat the codebase as post-M20:

- Region/Location/Battle/Campaign foundations exist.
- Owned-service/economy foundations exist.
- Unit passive-effect spine exists for `mine_production` and `leader_energy` only.
- Trading Post transaction APIs exist for resource barter and resource/Gold buy/sell.
- Trading Post interaction flow exists as a bounded Location-mode service interaction with per-visit time cost.
- Full shop/inventory UI, item economy, AI economy, broad effect systems, artifact Energy, item effects, battle statuses, and skill trees are not implemented.

## Design-source discipline

Before gameplay changes, check the relevant active docs:

- `docs/implementation_roadmap.md`
- `docs/content_scope_v1.md`
- `docs/core_loop_rules.md`
- `docs/content_schema.md`
- `docs/validation_system.md`
- `docs/technical_direction.md`
- `docs/terminology_map.md`

If docs and code disagree, stop and report the conflict before guessing.

## Gameplay rules

- Prefer pure rule helpers for calculations.
- Keep authored static content separate from runtime save state.
- Keep runtime state explicit and serializable where persistence matters.
- Do not add demo-specific branches in source.
- Avoid broad systems without a current consumer.
- Do not change battle assumptions unless the user explicitly reopens combat design.
- Preserve save compatibility unless the task explicitly includes migration work.

## Economy, Trading Post, and passive effects

Owned services:

- Owned mines pay resources through the daily payout path.
- Mine production modifiers come only from explicit `mine_production` passive effects on valid stationed stack-backed units.
- Strongest-only and non-stacking semantics are required per owned-service instance and output resource.
- Trader ownership benefits are service-type-specific and must be gated by the exact service being used.

Trading Post:

- Non-Gold barter uses resolved Trading Post exchange matrices.
- Gold may not appear in Trading Post barter matrices.
- Gold buy/sell uses base resource values plus tier `priceFactor`.
- Usable unowned/allied/enemy/neutral Trading Posts resolve at effective tier 0.
- Locked, destroyed, and hostile-occupied Trading Posts are refused outright.
- Resource/Gold mutation must route through existing GameSession resource APIs.
- The current Trading Post UI is intentionally bounded: buy, sell, barter, live prompt feedback, and a 20-minute visit cost charged once on exit after at least one successful trade.

Passive effects:

- New unit content should use canonical `passive_effects`.
- `mine_production_passive` is a legacy authoring alias only.
- Runtime consumers must read canonical passive effects, not a legacy runtime field.
- `leader_energy` affects only the current leader's daily Energy passive term.
- Artifact `statBonus` remains separate.

Artifact Energy, item effects, status effects, and active abilities remain deferred until explicitly selected.

## Performance

Avoid:

- per-frame content scans;
- repeated JSON/content parsing;
- repeated graph rebuilds;
- avoidable large copies;
- hidden nested scans in day-boundary, service-transaction, or interaction paths.

Build lookup maps once per pass when a rule must inspect many services, units, stacks, or nodes.

## Comments

Production comments should explain durable invariants, validation traps, save/load contracts, compatibility constraints, or non-obvious performance/correctness choices.

Do not add milestone/phase comments in production source. Test comments are acceptable when they explain non-obvious regression intent.

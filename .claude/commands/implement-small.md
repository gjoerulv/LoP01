# Implement small command

Use this for an approved narrow implementation slice.

## Before editing

- Read `CLAUDE.md`.
- Read the active docs relevant to the touched system.
- Summarize the intended change and likely files.
- Stop if docs/source conflict in a way that affects behavior.

## Implementation rules

- Keep the patch scoped to the approved slice.
- Prefer pure rules and direct tests before UI.
- Add validation for new authored data.
- Preserve save/load compatibility unless migration is explicitly in scope.
- Avoid demo-specific source branches.
- Avoid per-frame scans, repeated parsing, repeated graph rebuilds, and needless large copies.
- Do not add broad framework abstractions without an immediate consumer.

## Post-M19 reminders

- M17 economy foundation is complete.
- M18 passive-effect spine is complete for `mine_production` and `leader_energy`.
- M19 headless Trading Post transactions are complete.
- New unit passive content should use canonical `passive_effects`.
- Do not reintroduce runtime `mineProductionPassive` or old `UNIT_PASSIVE_*` semantics.
- Do not fold artifact/item/status systems into the passive-effect spine unless the approved slice explicitly does that.
- Do not duplicate Trading Post transaction math in UI; call the existing rules/GameSession APIs.

## Comments

Production comments should explain durable invariants, validation traps, compatibility behavior, save/load contracts, or performance-sensitive choices. Avoid milestone/phase labels.

## Finish

Run targeted tests and the full suite where practical.

Report files changed, tests added/updated, commands run, and any residual risk.

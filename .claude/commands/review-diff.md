# Review current diff

Review the current git diff for correctness, scope, and consistency with project docs. Do not edit files unless explicitly asked.

## Check

1. Does the diff stay within the requested scope?
2. Does it contradict any authoritative docs?
3. Does it introduce stale terminology?
4. Does it add unnecessary redundancy or bloat?
5. Are tests added or updated where appropriate?
6. Are code changes separated from rendering/input when required?
7. Are content/schema/validation changes consistent with:
   - `docs/content_schema.md`
   - `docs/validation_system.md`
   - `docs/scenario_authoring.md`
   - `docs/content_scope_v1.md`
8. Are architecture/performance choices consistent with:
   - `docs/technical_direction.md`
9. Are UI/presentation changes consistent with:
   - `docs/game_shell_flow.md`
   - `docs/presentation_game_feel.md`
   - `.github/instructions/ui.instructions.md`
10. Does the diff preserve the post-M16 baseline and avoid treating completed M12-M16 systems as future work?
11. Does the diff avoid demo-specific source hardcoding where generic data/rules should be used?
12. Does the diff preserve save/load compatibility or explicitly test migration behavior?
13. Do production source comments explain durable invariants, contracts, compatibility constraints, or performance-sensitive choices instead of milestone bookkeeping?
14. Are stale, redundant, misleading, or milestone-specific comments removed or rewritten?

## Report

- OK to commit / not OK to commit
- blockers
- recommended fixes
- optional polish
- tests that should be run

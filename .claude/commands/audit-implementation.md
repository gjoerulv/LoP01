# Implementation audit command

Use this when asked to audit source against docs or review a completed branch/commit.

## Read first

- `CLAUDE.md`
- `docs/implementation_roadmap.md`
- `docs/content_scope_v1.md`
- `docs/technical_direction.md`
- relevant rule/schema/validation docs for the changed area
- the changed source and tests

## Current baseline

The active baseline is post-M18 unless the audited commit proves otherwise.

M17 owned-service/economy and M18 passive-effect spine are complete. Do not report them as missing future work.

## Audit priorities

Check:

- source/doc alignment;
- active docs alignment;
- stale references to old baselines or archived docs;
- contradictions, ambiguity, redundancy, or uncertainty;
- correctness and save/load compatibility;
- performance traps: per-frame scans, repeated parsing, graph rebuilds, needless large copies, hidden nested loops;
- source comments that are stale, milestone-specific, redundant, or misleading;
- tests for boundary behavior and regression traps.

## Output

Return a binary result:

- Accept; or
- Reject with a focused revision prompt.

If fixes are needed, list only blocking fixes. Do not bury blockers under general suggestions.

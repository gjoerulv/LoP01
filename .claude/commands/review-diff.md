# Diff review command

Use this before committing or when asked to review a patch.

## Read first

- `CLAUDE.md`
- the current diff
- relevant docs for touched systems

## Review standard

Be strict and binary:

- Accept if the diff is coherent, scoped, tested, and aligned.
- Reject with a focused revision prompt if there is a blocking issue.

## Check for

- behavior/doc mismatches;
- stale or misleading comments;
- milestone/phase comments in production source;
- missing validation for new authored data;
- missing regression tests for important boundaries;
- save/load compatibility issues;
- transaction/resource atomicity issues;
- performance-hostile patterns;
- broad framework expansion beyond the requested slice;
- demo-specific source branches.

For post-M19 work, remember:

- unit passive effects are canonicalized through `passive_effects`;
- `mine_production_passive` is legacy authoring compatibility only;
- `mine_production` and `leader_energy` are the only implemented unit passive-effect consumers;
- artifact `statBonus` remains separate;
- artifact Energy, item effects, status effects, and skill trees remain deferred;
- Trading Post transactions exist as headless pure/GameSession APIs;
- Trading Post UI, per-visit time-cost flow, and other trader-service transaction behaviors remain deferred unless the current slice explicitly selects them.

## Output

Give:

1. Accept or Reject;
2. blocking issues only if rejected;
3. concise non-blocking notes only if useful;
4. a revision prompt when rejected.

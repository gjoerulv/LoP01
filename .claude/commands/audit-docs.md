# Documentation audit command

Use this when asked to audit documentation consistency.

## Read first

- `README.md`
- `CLAUDE.md`
- `docs/implementation_roadmap.md`
- `docs/content_scope_v1.md`
- `docs/technical_direction.md`
- `docs/core_loop_rules.md`
- `docs/content_schema.md`
- `docs/validation_system.md`
- `docs/terminology_map.md`
- `.github/copilot-instructions.md`
- relevant `.github/instructions/*.md`
- relevant `.claude/commands/*.md`

## Current baseline

The active baseline is post-M18. The next planned milestone is M19 unless redirected.

Archived docs are historical context only. Do not treat archived roadmaps or old content-scope files as current truth.

## Audit checks

Find:

- stale baseline statements;
- wrong next-milestone references;
- references to M17/M18 as future work;
- stale references to `mineProductionPassive`, old `UNIT_PASSIVE_*` behavior, or mine-only passive semantics where the canonical `passive_effects` spine is meant;
- contradictions between roadmap, schema, validation, core rules, README, and agent guidance;
- redundant or bloated documentation that should be shortened.

## Output

Return a concise report and a binary recommendation:

- docs are ready; or
- fix docs first.

If asked to patch docs, update only files that matter. Do not rewrite stable docs unnecessarily.

# Audit implementation against docs

Audit the current implementation against the authoritative docs. Do not edit files unless explicitly asked.

## Read first

- `CLAUDE.md`
- `README.md`
- `README_DECISIONS.md`
- `docs/implementation_roadmap.md`
- `docs/content_scope_v1.md`
- `docs/technical_direction.md`
- `docs/game_vision.md`
- `docs/core_loop_rules.md`
- `docs/combat_rules.md`
- `docs/scenario_authoring.md`
- `docs/validation_system.md`
- `docs/content_schema.md`
- `docs/terminology_map.md`
- relevant `.github/instructions/*.md`
- relevant `.github/agents/*.md`

Then inspect source, tests, content, and CMake/build files.

## Current baseline to verify

The implementation should be treated as post-M16. Battle, roster, save/load, typed events, scenario outcomes, inventory/artifacts, team Energy, minimal World Map, and minimal Campaign foundations should already exist.

Do not plan as if M8-M16 are still future work.

## Report

1. what currently matches the docs;
2. what conflicts with the docs;
3. what is obsolete but still present;
4. what is missing for the next safe milestone;
5. what should not be implemented yet;
6. suggested first implementation patch, with files to touch.

## Rules

- Do not invent new design.
- If docs and code disagree, report the disagreement before proposing code changes.
- Prefer small, reversible changes.
- Identify tests that should be added or updated.
- Do not perform broad rewrites in the audit phase.
- Use `docs/technical_direction.md` to flag performance/architecture problems such as repeated parsing, per-frame scans, unnecessary graph rebuilds, large needless copies, or renderer-owned gameplay state.

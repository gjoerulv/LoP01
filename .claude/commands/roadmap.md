# Roadmap planning command

Use this when asked to plan the next implementation milestone.

## Read first

- `CLAUDE.md`
- `README.md`
- `README_DECISIONS.md`
- `docs/implementation_roadmap.md`
- `docs/content_scope_v1.md`
- `docs/technical_direction.md`
- `docs/core_loop_rules.md`
- `docs/content_schema.md`
- `docs/validation_system.md`
- `docs/terminology_map.md`
- relevant `.github/instructions/*.md`

## Current baseline

Treat the repository as post-M19 unless the current branch/commit proves otherwise.

Do not plan M17, M18, or M19 as future work. M17 owned-service/economy foundation, M18 passive-effect spine, and M19 headless Trading Post transaction layer are complete baselines.

The likely next milestone is M20: Trading Post Interaction Flow, unless the user redirects.

## Output

Provide:

1. current-state audit focused on the requested area;
2. binary recommendation: ready to implement, or fix docs/source first;
3. blocking fixes only, if any;
4. phased implementation plan;
5. best first slice;
6. risks/failure modes before optimizations.

Do not implement code in a roadmap response.

## Constraints

- Keep milestones narrow and test-backed.
- Do not add broad frameworks before there is a real consumer.
- Preserve performance posture from `docs/technical_direction.md`.
- Avoid bloated source comments and milestone labels in production code.

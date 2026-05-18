# Plan first implementation milestone

Plan the smallest safe implementation milestone.

Do not edit files.

Read:
- `CLAUDE.md`
- `docs/implementation_roadmap.md`
- `docs/content_schema.md`
- `docs/validation_system.md`
- `docs/scenario_authoring.md`
- `docs/core_loop_rules.md`
- `README_DECISIONS.md`
- relevant source/test files

Produce:
1. milestone title
2. objective
3. non-goals
4. files likely to change
5. data structures to introduce or modify
6. validation/tests to add
7. exact acceptance criteria
8. risks and rollback plan

Keep the milestone narrow enough to finish in one or two reviewable patches.

Milestone planning bias:
- prefer pure/headless logic first
- prefer small, independently testable slices
- avoid renderer/input changes unless the milestone requires them
- identify docs that become stale after completion
- keep source-of-truth behavior docs separate from implementation-status roadmap updates

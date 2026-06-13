# Game Architect Agent

Use this agent for architecture, roadmap, and cross-system gameplay decisions.

Do not maintain milestone status in this file. Always read `docs/implementation_roadmap.md` for the current baseline, latest completed milestone, selected next milestone, and candidate directions.

Read first:

- `CLAUDE.md`
- `docs/implementation_roadmap.md`
- `docs/content_scope_v3.md`
- `docs/technical_direction.md`
- relevant milestone-agnostic final-vision docs for the affected system

Architectural rules:

- Keep gameplay rules in `GameSession` or dedicated gameplay/rules modules, not renderers/input.
- Keep authored static content separate from runtime mutable state.
- Preserve save/load compatibility unless migration is explicitly scoped.
- Prefer bounded, test-backed slices over broad framework construction.
- Do not treat future v3 candidates as implemented unless the roadmap says they are complete.

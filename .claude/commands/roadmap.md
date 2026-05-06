# Create implementation roadmap

Create a concise implementation roadmap from the current docs and code.

Do not edit files.

Read:
- `CLAUDE.md`
- `README.md`
- `README_DECISIONS.md`
- `docs/game_vision.md`
- `docs/game_shell_flow.md`
- `docs/presentation_game_feel.md`
- `docs/core_loop_rules.md`
- `docs/combat_rules.md`
- `docs/scenario_authoring.md`
- `docs/validation_system.md`
- `docs/content_schema.md`
- `docs/terminology_map.md`
- `docs/technical_direction.md`
- `.github/copilot-instructions.md`
- relevant `.github/instructions/*.md`

Then inspect the current code structure enough to avoid planning against nonexistent architecture.

Produce:
1. current implementation baseline
2. doc/code conflicts that must be resolved first
3. implementation phases
4. first small milestone
5. acceptance checks per phase
6. likely tests needed
7. explicit "not yet" boundaries

Roadmap constraints:
- Keep milestones small.
- Do not implement full vision at once.
- Prefer foundations first: schema, validation, content loading, deterministic state, tests.
- Keep gameplay logic separated from rendering/input.
- Keep docs as source of truth.

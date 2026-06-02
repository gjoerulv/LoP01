# Create implementation roadmap

Create a concise implementation roadmap from the current docs and code. Do not edit files.

## Read

- `CLAUDE.md`
- `README.md`
- `README_DECISIONS.md`
- `docs/implementation_roadmap.md`
- `docs/content_scope_v1.md`
- `docs/technical_direction.md`
- `docs/game_vision.md`
- `docs/game_shell_flow.md`
- `docs/presentation_game_feel.md`
- `docs/core_loop_rules.md`
- `docs/combat_rules.md`
- `docs/scenario_authoring.md`
- `docs/validation_system.md`
- `docs/content_schema.md`
- `docs/terminology_map.md`
- `.github/copilot-instructions.md`
- relevant `.github/instructions/*.md`
- relevant `.github/agents/*.md`

Then inspect the current code structure enough to avoid planning against nonexistent architecture.

## Current baseline assumption to verify

The repo should be treated as post-M17 unless the code contradicts the docs:

- M12 scenario outcomes complete;
- M13 inventory/artifacts complete;
- M14 team Energy complete;
- M15 minimal World Map complete;
- M16 minimal Campaign System complete;
- M17 owned-service/economy foundation complete.

The next planned milestone is M18: Passive Effect Spine, unless the user explicitly redirects or current `main` contradicts the active docs.

## Produce

1. current implementation baseline;
2. doc/code conflicts that must be resolved first;
3. implementation phases;
4. first small milestone;
5. acceptance checks per phase;
6. likely tests needed;
7. explicit not-yet boundaries.

## Roadmap constraints

- Keep milestones small enough to review.
- Move faster than the early M12-M17 micro-slices only when the phase remains coherent and testable.
- Do not implement the full vision at once.
- Prefer foundations first: schema, validation, content loading, deterministic state, save/load, and tests.
- Keep gameplay logic separated from rendering/input.
- Keep docs as source of truth.
- Do not expand authored content volume before the system foundation needs it.
- Respect `docs/content_scope_v1.md` for content scale.
- Respect `docs/technical_direction.md` for architecture/performance.

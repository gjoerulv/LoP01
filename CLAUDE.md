# Project Ashvale - Claude Code Instructions

This project is documentation-driven. Before making design or architecture changes, read the authoritative docs listed below.

## Authoritative docs

Read these first when relevant:

1. `README.md`
2. `docs/game_vision.md`
3. `docs/core_loop_rules.md`
4. `docs/combat_rules.md`
5. `docs/scenario_authoring.md`
6. `docs/validation_system.md`
7. `docs/content_schema.md`
8. `docs/terminology_map.md`
9. `README_DECISIONS.md`
10. `.github/copilot-instructions.md`
11. `.github/instructions/gameplay.instructions.md`

For UI work, also read:

- `.github/instructions/ui.instructions.md`

For architecture work, also read:

- `.github/agents/game-architect.agent.md`
- `docs/technical_direction.md`

## Core rules

- Do not invent new game-design rules that contradict the docs.
- Do not reintroduce old terminology such as `overworld`, `combat node`, or `game_vision_complete.md` as current design truth.
- Use the current terms: World Map, Region, Location, Service, node content, Scenario Info screen, Adventure button strip.
- Treat nodes as travel points with one main node content item plus optional event attachments.
- Treat content schema rules in `docs/content_schema.md` as authoritative for authored JSON direction.
- Treat validation rules in `docs/validation_system.md` as authoritative for validation planning.
- Keep authored initial state separate from runtime save state.
- Use `kind + id` as mod override identity.
- Respect player-character rules in `docs/core_loop_rules.md`, `docs/combat_rules.md`, and `docs/content_schema.md`.

## Workflow

- Work on a git branch.
- Before editing, summarize the plan and list the files you intend to change.
- Prefer small, reviewable patches.
- Run available build/tests after meaningful code changes.
- Do not delete or archive docs unless explicitly asked.
- Do not make broad rewrites when a targeted change is enough.
- If the docs conflict, stop and report the conflict instead of guessing.
- If implementation conflicts with the docs, report it and propose options before changing the design.

## First task pattern

For large tasks, start by producing:

1. current-state audit
2. proposed implementation roadmap
3. milestone breakdown
4. first small patch recommendation

Do not start implementing the full vision in one pass.
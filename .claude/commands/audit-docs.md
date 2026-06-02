# Audit live Markdown docs

Audit all live Markdown files for contradictions, ambiguity, stale terminology, duplicate definitions, malformed references, and unclear agent guidance. Do not edit files unless explicitly asked.

## Read first

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
- `.github/instructions/*.md`
- `.github/agents/*.md`
- `.claude/commands/*.md`

Treat archived docs/prompts as historical context only. In particular, do not treat `docs/content_scope_v0.md.archived` or `docs/implementation_roadmap.md.00.archived` as active scope or roadmap truth.

## Current baseline to verify

The active docs should describe the project as post-M17, not post-M8/post-M11/post-M16/single-Region-only. The active next milestone should be M18 — Passive Effect Spine — unless the docs explicitly record a newer user decision.

## Check for

1. blocker contradictions;
2. ambiguous or underspecified rules;
3. stale/outdated wording;
4. duplicate definitions or needless redundancy;
5. malformed numbering or dangling bullets;
6. active docs pointing to removed, renamed, or archived docs as current truth;
7. old terms used as current design truth;
8. agent instructions that contradict the active roadmap or current baseline.

## Known current terms

- World Map
- Region
- Location
- Service
- node content
- Scenario Info screen
- Adventure button strip
- traveling party
- player character
- Scenario Region Context

## Old terms that should not be reintroduced as current design truth

- overworld
- combat node
- game_vision_complete.md
- blocker node as a formal node type
- Adventure menu as the formal victory/defeat screen

## Report format

- Blockers
- Recommended cleanup
- Optional polish
- Files checked
- Safe-to-commit recommendation

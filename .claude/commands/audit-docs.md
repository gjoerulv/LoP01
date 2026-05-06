# Audit live Markdown docs

Audit all live Markdown files for contradictions, ambiguity, stale terminology, duplicate definitions, malformed references, and unclear agent guidance.

Do not edit files unless explicitly asked.

Read first:
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
- `.github/copilot-instructions.md`
- `.github/instructions/*.md`
- `.github/agents/*.md`

Treat archived docs/prompts as historical context only.

Check for:
1. blocker contradictions
2. ambiguous or underspecified rules
3. stale/outdated wording
4. duplicate definitions or needless redundancy
5. malformed numbering or dangling bullets
6. active docs pointing to removed/renamed docs
7. old terms used as current design truth

Known current terms:
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

Old terms that should not be reintroduced as current design truth:
- overworld
- combat node
- game_vision_complete.md
- blocker node as a formal node type
- Adventure menu as the formal victory/defeat screen

Report format:
- Blockers
- Recommended cleanup
- Optional polish
- Files checked
- Safe-to-commit recommendation

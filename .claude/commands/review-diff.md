# Review current diff

Review the current git diff for correctness, scope, and consistency with project docs.

Do not edit files unless explicitly asked.

Check:
1. Does the diff stay within the requested scope?
2. Does it contradict any authoritative docs?
3. Does it introduce stale terminology?
4. Does it add unnecessary redundancy or bloat?
5. Are tests added or updated where appropriate?
6. Are code changes separated from rendering/input when required?
7. Are content/schema/validation changes consistent with:
   - `docs/content_schema.md`
   - `docs/validation_system.md`
   - `docs/scenario_authoring.md`
8. Are UI/presentation changes consistent with:
   - `docs/game_shell_flow.md`
   - `docs/presentation_game_feel.md`
   - `.github/instructions/ui.instructions.md`

Report:
- OK to commit / not OK to commit
- blockers
- recommended fixes
- optional polish
- tests that should be run

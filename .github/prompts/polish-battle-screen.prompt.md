---
agent: 'agent'
description: 'Improve battle readability without changing battle rules'
---

Improve battle presentation for the current playable slice.

Read first:
- README.md
- README_DECISIONS.md
- docs/combat_rules.md
- docs/technical_direction.md

Task:
- Keep the existing battle rules intact
- Render allies and enemies in clearly separated screen areas
- Highlight the active unit clearly
- Render HP, MP, and Life clearly
- Show turn order as a visible UI strip
- Show action selection clearly
- Show target selection clearly
- Keep battle result/status messaging readable
- Prefer improvements that make the current prototype easier to understand moment-to-moment

Constraints:
- Do not add major new battle mechanics
- Do not move gameplay ownership into rendering code
- Keep changes incremental and test-safe where possible

Focus on readability, state clarity, and current battle feel.
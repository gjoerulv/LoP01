---
agent: 'agent'
description: 'Implement Milestone 7 home base, service economy, and weekly cadence in small compilable steps'
---

Implement Milestone 7: home base, services, economy, and weekly cadence.

Read first:
- README.md
- README_DECISIONS.md
- docs/game_vision.md
- docs/game_vision_complete.md
- docs/core_loop_rules.md
- docs/content_scope_v0.md
- docs/technical_direction.md
- docs/milestone_7_services_economy_weekly_cadence.md

Primary goal:
Make the slice feel more like a living strategy/RPG loop by giving home base and regional services meaningful economic and weekly identity, while preserving the Milestone 6 architecture.

Required work priorities:

1. Preserve baseline architecture
- Preserve explicit `App` / `GameSession` flow
- Preserve controller / mapper / renderer split
- Keep gameplay logic separate from rendering
- Prefer explicit and readable changes over generalized systems

2. Make home base feel special
- Keep home base as the free-rest safe hub of the slice
- Make that identity clear in rules and presentation

3. Make inns economically meaningful
- Add paid rest at inns
- Keep cost handling explicit and readable
- Keep home-base rest free as the contrast case

4. Make recruit posts content-driven
- Add content-driven recruit offers, quantities, and remaining counts
- Keep schemas typed and editor-friendly
- Keep runtime quantities separate from static authored content

5. Introduce weekly cadence
- Add weekly refresh behavior for recruit quantities and related service state where appropriate
- Keep refresh rules explicit and testable

6. Tests and docs
- Add tests for new pure logic where feasible
- Update docs when behavior changes in a non-obvious way
- Record meaningful tradeoffs in `README_DECISIONS.md`

Constraints:
- Do not add major new combat mechanics
- Do not expand to new regions
- Do not add broad inventory/equipment systems unless explicitly required
- Do not introduce generic scripting/event/service frameworks
- Keep the codebase compilable in small steps
- Prefer a coherent playable slice over broad incomplete systems
- Do not restructure stable architecture unless required to complete milestone goals cleanly
- Maintain responsiveness, clear ownership, and leak-resistant code

Expected output:
1. short implementation plan
2. affected files/modules
3. implementation in small compilable steps
4. tests/docs updates
5. `README_DECISIONS.md` note for non-obvious tradeoffs

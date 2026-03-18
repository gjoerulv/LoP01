---
agent: 'agent'
description: 'Implement Milestone 5 world-loop consequences and progression'
---

Implement Milestone 5: World loop consequences.

Read first:
- docs/game_vision.md
- docs/core_loop_rules.md
- docs/content_scope_v0.md
- docs/technical_direction.md
- docs/milestone_5_world_loop.md

Primary goal:
Extend the existing playable slice into a more complete day-to-day gameplay loop.

Required work:

1. Transition consistency
- Preserve the current controller/mapper/renderer split
- Prefer explicit helpers for moving between overworld, location, and battle
- Remove leftover duplicate status-text paths when event-driven text already exists

2. Sleep and rest
- Implement sleeping at valid rest locations
- Use the simplest readable UI prompt for resting
- Advance to the next valid day start after resting

3. Wake-up penalty and defeat flow
- Apply the documented penalty after full defeat or missing sleep
- Return the player to an appropriate safe fallback for the slice
- Keep the flow understandable in HUD/status text

4. Quest progression
- Use existing quest JSON as the source of truth
- Add a minimal quest-state layer
- Support visible progress/completion for at least the existing simple quests

5. Save/load and tests
- Extend save/load only where needed for milestone state
- Add tests for new pure logic where feasible

Constraints:
- Do not add new major combat mechanics
- Do not expand to new regions
- Keep content data-driven
- Keep the codebase compilable in small steps
- Prefer a playable end-to-end loop over broad incomplete systems
- Do not restructure stable architecture unless required to complete milestone goals cleanly

Expected output:
1. short implementation plan
2. affected files/modules
3. implementation in small compilable steps
4. update README_DECISIONS.md with tradeoffs
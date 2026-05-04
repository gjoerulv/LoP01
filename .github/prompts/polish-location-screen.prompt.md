---
agent: 'agent'
description: 'Improve location-mode readability without changing core architecture'
---

Improve location mode presentation for the current playable slice.

Read first:
- README.md
- README_DECISIONS.md
- docs/game_vision.md
- docs/core_loop_rules.md
- docs/technical_direction.md

Task:
- Keep the current location-mode architecture and flow intact
- Improve prototype location readability
- Make building/interactable visuals clearer
- Make current interaction prompts clearer
- Add or refine a dialogue/result panel where useful
- Add or refine a HUD showing day, time, relevant resources, and location name
- Help the player understand available services, their role, and their current cost/stock state where relevant
- Help the player understand interaction outcomes without adding new systems

Constraints:
- Walking remains free
- Interaction time costs must remain consistent with docs/core_loop_rules.md
- Do not add broad new gameplay systems
- Do not move gameplay logic into rendering code

Focus on clarity, service identity, and readable prototype presentation.

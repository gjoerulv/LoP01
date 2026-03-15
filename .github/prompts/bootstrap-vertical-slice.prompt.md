---
agent: 'agent'
description: 'Bootstrap the first playable vertical slice for the game'
---

Bootstrap the first playable vertical slice for this project.

Read and follow:
- docs/game_vision.md
- docs/core_loop_rules.md
- docs/combat_rules.md
- docs/content_scope_v0.md
- docs/technical_direction.md

Requirements:
- C++20
- raylib
- CMake
- placeholder assets only
- data-driven JSON content
- save/load support
- tests for pure logic

Deliverables for this task:
1. Propose the folder structure
2. Propose the gameplay/data architecture
3. Create the initial CMake project
4. Create the core application loop
5. Create game state scaffolding for:
   - title
   - opening sequence
   - overworld selection
   - overworld mode
   - location mode
   - battle mode
6. Create starter JSON data files
7. Create README and README_DECISIONS.md

Constraints:
- Do not attempt to build the full game
- Keep scope to the vertical slice
- Avoid overengineering
- Prefer small compilable increments
- Explain any simplifying assumptions

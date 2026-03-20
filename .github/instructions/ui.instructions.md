---
applyTo: "src/rendering/**/*.cpp,src/rendering/**/*.h,src/app/mappers/**/*.cpp,src/app/mappers/**/*.h"
---

# UI instructions

- Favor clarity and readability over flashy effects.
- Use simple panels, lists, prompts, labels, and status strips.
- The UI should evoke retro strategy/RPG readability.
- Show time, day, gold, and current place prominently.
- Overworld UI should clearly show:
  - selected destination
  - travel time preview
  - whether travel is allowed
  - whether a destination enters location mode or battle
  - whether a node appears cleared, blocked, or unavailable
- Location UI should clearly show:
  - location name
  - current interaction prompt
  - available services or actions
  - lightweight result/status text for interactions
  - service cost or remaining quantity when relevant
- Battle UI must clearly show:
  - turn order
  - active unit
  - HP / MP / Life
  - action menu
  - target selection
- Keep placeholder UI art minimal and functional.
- Prefer communicating current slice rules clearly over adding decorative complexity.
- Do not assume that service identity is best communicated through binary enabled/disabled text; cost, stock, and refresh state are usually more valuable.

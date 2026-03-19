---
applyTo: "src/rendering/**/*.cpp,src/rendering/**/*.h,src/app/mappers/**/*.cpp,src/app/mappers/**/*.h"
---

# Rendering and presentation instructions

- The game should not rely on a single always-visible debug text screen.
- Each major mode should have its own readable presentation:
  - title
  - overworld
  - location
  - battle
- Shared HUD elements should be factored into reusable rendering helpers.
- Debug information should be available through a toggleable overlay, not as the default main layout.
- Placeholder visuals are acceptable, but they must remain spatially readable and game-like.
- Prioritize clear layout, interaction prompts, and state readability over decorative effects.
- Prefer consistent screen composition:
  - top or corner HUD
  - main play area
  - bottom or side action/dialogue panel
- Use strong visual distinction between:
  - current selection
  - active unit
  - interactable object
  - disabled or unavailable action
- Rendering and mapper code should not own gameplay rules.
- Prefer renderer/view classes that consume session state or mapped render models.
- UI text should help explain state changes such as:
  - where the player is
  - what they can do here
  - whether travel is available or blocked
  - whether a node/service has already been used or cleared
- Keep battle UI deterministic and readable.
- Keep overworld navigation understandable at a glance.
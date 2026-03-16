---
applyTo: "src/rendering/**/*.cpp,src/rendering/**/*.h,src/ui/**/*.cpp,src/ui/**/*.h"
---

# Rendering and presentation instructions

- The game should no longer rely on a single always-visible debug text screen.
- Each major mode should have its own visual presentation:
  - title
  - overworld
  - location
  - battle
- Shared HUD elements should be factored into reusable rendering helpers.
- Debug information should be available through a toggleable overlay, not as the default main layout.
- Placeholder visuals are acceptable, but they must be spatially readable and game-like.
- Prioritize clear layout, interaction prompts, and state readability over decorative effects.
- Prefer consistent screen composition:
  - top or corner HUD
  - main play area
  - bottom or side action/dialogue panel
- Use strong visual distinction between:
  - current selection
  - active unit
  - interactable object
  - disabled/unavailable action
- Rendering code should not own gameplay state.
- Prefer renderer/view classes that consume existing gameplay/session state.
- Keep battle UI deterministic and readable.
- Keep overworld navigation visible and understandable at a glance.
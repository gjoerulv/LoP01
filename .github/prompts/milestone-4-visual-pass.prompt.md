---
agent: 'agent'
description: 'Turn the scaffold into the first graphical gameplay slice'
---

Implement Milestone 4: First visual gameplay pass.

Read first:
- docs/game_vision.md
- docs/core_loop_rules.md
- docs/combat_rules.md
- docs/content_scope_v0.md
- docs/technical_direction.md
- docs/milestone_4_visual_pass.md

Primary goal:
Make the game feel graphical rather than text-based, while preserving existing core gameplay logic.

Required work:

1. Content loading
- Keep content loading stable from the output directory
- Preserve the current CMake copy step or improve it only if needed
- Remove failure-oriented placeholder behavior from the main UI path

2. Rendering architecture
- Introduce mode-specific rendering instead of a single always-on debug screen
- Add separate renderer classes or functions for:
  - title
  - overworld
  - location
  - battle
  - shared HUD
  - debug overlay

3. Overworld presentation
- Draw a simple but readable map screen
- Render destinations as nodes from content data
- Render connections or travel paths if practical
- Show player position clearly
- Show selected/hovered node
- Show travel time preview
- Use contextual controls for selecting and confirming travel

4. Location presentation
- Keep the existing location logic
- Improve the layout so it resembles a prototype town scene
- Add clear interactable markers or prompts
- Add a bottom dialogue panel for NPC interaction text
- Add a visible local HUD for time, day, gold, and location name

5. Battle presentation
- Keep the current battle logic unless small changes are required for presentation
- Render allies and enemies in separate areas
- Show active unit highlight
- Show HP, MP, and Life in readable panels or bars
- Render turn order as a real UI element
- Render action menu and target selection visually

6. Debug overlay
- Move development/debug information behind an F1 toggle
- Normal play should not be dominated by diagnostics text

Constraints:
- Use placeholder art only
- Do not add major new gameplay systems
- Do not expand content scope significantly
- Prefer clean, modular rendering code
- Preserve and reuse existing gameplay logic
- Keep the project compilable at each step

Expected output:
1. short implementation plan
2. affected files/modules
3. implementation in small compilable steps
4. update README_DECISIONS.md with tradeoffs
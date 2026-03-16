---
applyTo: "src/gameplay/**/*.cpp,src/gameplay/**/*.h,src/data/**/*.cpp,src/data/**/*.h,docs/**/*.md"
---

# Gameplay implementation instructions

- Respect the time system exactly as described in docs/core_loop_rules.md.
- Respect battle logic exactly as described in docs/combat_rules.md.
- Do not collapse overworld mode and location mode into one system.
- Keep the three major layers distinct:
  - overworld selection
  - overworld travel
  - location exploration
- Battles must be a separate reusable module shared by overworld and location encounters.
- Hero units and stackable units must be modeled differently.
- Player defeat and sleep-failure penalties must be implemented consistently.
- Keep game rules data-driven where possible.
- Do not invent major story elements that contradict docs/game_vision.md.
- Prefer a playable incomplete feature over a broad unfinished feature.
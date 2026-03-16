---
agent: 'agent'
description: 'Implement the CTB battle module'
---

Implement the battle module for this project.

Read first:
- docs/combat_rules.md
- docs/core_loop_rules.md
- docs/content_scope_v0.md

Task:
- Build a reusable battle module used by both overworld and location encounters
- Implement CTB turn order
- Support 5 units per side
- Support 1 leader unit, hero units and stackable generic units
- Implement:
  - basic attack
  - defend
  - 2 sample skills
  - MP consumption
  - KO/defeat rules
  - player survives at 1 HP if battle is won after player KO
- Add unit tests for turn order and damage application
- Add a simple debug/test battle screen with placeholder UI

Keep formulas simple and faithful to the docs.

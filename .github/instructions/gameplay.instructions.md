# Gameplay instructions

- Treat the project as post-M25.
- Current gameplay foundations include deterministic scenario outcomes, Scenario Result presentation, Scenario `playerStart`, owned-service economy, Trading Post interaction, player-side guarded-service claiming, v1 strategic-economy proof content, and player-facing mine stationing/unstationing.
- Ownership and guarding are related but separate: an owned service does not have to be guarded, and a guarded service is only one capture path.
- Claimed services mutate runtime owned-service state only; content definitions must not be mutated.
- `mine_production` is player-facing: M25 added a bounded stationing flow at player-owned mines (physical one-place-at-a-time placement behind `GameSession`, mines only). Stationing is guard/worker capacity only — not Storage/Garrison.
- Do not add enemy-side capture, AI economy, service destruction/restoration, unguarded claiming, Market/Black Market/Freelancer behavior, Storage/Garrison, or broader v2 systems without an explicit roadmap milestone.
- Gameplay-rule changes must be tested in pure/unit tests where practical and integration/end-to-end tests where the rule crosses systems.

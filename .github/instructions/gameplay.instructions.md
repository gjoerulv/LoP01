# Gameplay instructions

- Treat the project as post-M24.
- Current gameplay foundations include deterministic scenario outcomes, Scenario Result presentation, Scenario `playerStart`, owned-service economy, Trading Post interaction, player-side guarded-service claiming, and v1 strategic-economy proof content.
- Ownership and guarding are related but separate: an owned service does not have to be guarded, and a guarded service is only one capture path.
- Claimed services mutate runtime owned-service state only; content definitions must not be mutated.
- `mine_production` exists and is proven through runtime/save-data stationing state; player-facing service stationing is the selected next milestone and is not yet implemented.
- Do not add enemy-side capture, AI economy, service destruction/restoration, unguarded claiming, Market/Black Market/Freelancer behavior, Storage/Garrison, or broader v2 systems without an explicit roadmap milestone.
- Gameplay-rule changes must be tested in pure/unit tests where practical and integration/end-to-end tests where the rule crosses systems.

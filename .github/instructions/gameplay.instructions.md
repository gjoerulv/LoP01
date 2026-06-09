# Gameplay instructions

- Treat the project as post-M23.
- Current gameplay foundations include deterministic scenario outcomes, Scenario Result presentation, Scenario `playerStart`, owned-service economy, Trading Post interaction, and player-side guarded-service claiming.
- Ownership and guarding are related but separate: an owned service does not have to be guarded, and a guarded service is only one capture path.
- Claimed services mutate runtime owned-service state only; content definitions must not be mutated.
- Do not add enemy-side capture, AI economy, service destruction/restoration, unguarded claiming, Market/Black Market/Freelancer behavior, or v2 content without an explicit roadmap milestone.
- Gameplay-rule changes must be tested in pure/unit tests where practical and integration/end-to-end tests where the rule crosses systems.

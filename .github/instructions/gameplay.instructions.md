# Gameplay implementation instructions

Use these instructions for gameplay, economy, Scenario, Campaign, service, battle, and content-driven systems.

## Current baseline

The repository is post-M21. Do not treat M17, M18, M19, M20, or M21 as future work.

Implemented foundations include owned services, resources, mine payout, unit passive effects, Trading Post transaction rules/APIs, Trading Post interaction flow, and Scenario-authored player economy/service start state.

## Core rules

- Keep gameplay rules deterministic and testable.
- Keep rendering/input separate from gameplay state mutation.
- Keep authored initial/static content separate from runtime save state.
- Use existing `GameSession` APIs for Gold/resources, owned services, Trading Post transactions, time, Energy, and save/load.
- Do not bypass service gates: lock, destruction, hostile occupation, eligibility, story, stock, and availability remain authoritative.
- Preserve Gold single-source-of-truth through the existing `gold_` / `ResourceType::Gold` delegation path.
- Preserve save/load compatibility unless migration is explicitly part of the task.

## Scenario start-state

Scenario `playerStart` can author player starting Gold, non-Gold resources, and initial player-owned service state. It is applied to runtime `GameSession` state at Scenario start.

Do not expand this into general team authoring, authored starting rosters, item/artifact start-state, per-scenario content directories, or World Map unlock overrides unless the active roadmap explicitly selects that scope.

## Economy and service systems

Trading Post interaction is currently bounded to buy/sell/barter, prompt feedback, and a 20-minute per-visit time cost charged once on exit after at least one successful trade.

Do not assume broader Market, Black Market, Freelancer's Guild, item-market, AI economy, or ownership-contesting behavior exists.

## Comments and tests

Production comments should explain durable invariants, compatibility, validation traps, save/load contracts, or performance-sensitive choices. Avoid milestone/phase labels in source comments.

Tests should cover the rule boundary being changed. Test comments may explain non-obvious regression intent.

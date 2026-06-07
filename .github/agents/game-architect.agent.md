# Game Architect Agent

Use this guidance for architecture, system-boundary, and milestone-planning work.

## Current baseline

The project is post-M18. Foundations exist for battle, roster, save/load, validation, typed events, enemy teams, scenario outcomes, inventory/artifacts, Energy, World Map, Campaign, owned-service/economy, and a narrow unit passive-effect spine.

The next planned milestone is M19: Service Economy Expansion, unless the user redirects.

## Architectural priorities

- Preserve explicit state and explicit transitions.
- Keep gameplay logic out of rendering/input.
- Prefer pure rules and typed data over generic framework dispatch.
- Add schema and validation together.
- Preserve save/load compatibility unless migration is explicitly in scope.
- Avoid demo-specific source branches.
- Avoid broad abstraction before there is a real consumer.

## System boundaries to protect

- Unit passive effects currently support only `mine_production` and `leader_energy`.
- Artifact `statBonus` remains on the artifact/battle-stat path.
- Artifact Energy, item effects, statuses, active abilities, and skill trees are deferred.
- Owned-service economy is founded but not a full item market, trader UI, or AI economy.
- M19 should likely connect existing trader tiers/curves to narrow transactions, not expand every economy system at once.

## Review posture

When auditing plans or diffs:

- identify stale docs, stale comments, contradictions, and ambiguous source contracts;
- reject plans that broaden into generic engines without immediate consumers;
- reject code that introduces per-frame scans, repeated parsing, avoidable large copies, or repeated graph rebuilds;
- require tests for behavior boundaries and regression traps;
- prefer deleting weak comments over growing comment clutter.

# Game Architect agent guidance

Use this guidance when reviewing architecture, milestone plans, or cross-system changes.

## Current baseline

Ashvale is post-M20. The current stable slice includes battle, roster, save/load, Region/Location flow, validation, typed events, enemy teams, scenario outcomes, inventory/artifacts, Energy, World Map, Campaign, owned-service economy, unit passive effects, Trading Post transaction rules/APIs, and a bounded Trading Post interaction flow.

Do not plan M17, M18, M19, or M20 as future work. They are completed foundations. Consult `docs/implementation_roadmap.md` for the latest milestone status.

## Architectural priorities

- Preserve gameplay/presentation separation.
- Keep input and transient interaction state in `src/app`.
- Keep economy, service, and validation rules in gameplay/data layers.
- Prefer pure rules and explicit state transitions.
- Avoid broad generic frameworks until a scoped milestone has a real consumer.
- Avoid per-frame content scans, repeated parsing, graph rebuilds, and hidden nested scans.
- Keep save/load focused on gameplay state, not transient presentation state.

## Review posture

Be strict about:

- doc/source contradictions;
- demo-specific source branches;
- ownership or availability rules bypassed by UI code;
- duplicate Gold/resource sources of truth;
- passive/effect scope creep;
- trader-service expansion that quietly becomes a full marketplace;
- source comments that are stale, milestone-specific, or redundant.

Prefer small, test-backed slices. If a proposed plan is not coherent, reject it and ask for a narrower revision.

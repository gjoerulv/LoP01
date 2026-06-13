# Project-wide Copilot instructions

Primary project guidance lives in `CLAUDE.md`.

Current milestone state, latest completed milestone, selected next milestone, and milestone candidates live in `docs/implementation_roadmap.md`. Do not duplicate or infer milestone state from this file.

Active scope: `docs/content_scope_v3.md`.

When editing this repository:

- preserve deterministic, testable gameplay rules;
- keep gameplay rules out of rendering/input layers;
- keep authored static content separate from runtime mutable state;
- preserve save/load compatibility unless the active milestone explicitly selects migration work;
- avoid broad frameworks before a scoped consumer needs them;
- avoid per-frame scans, repeated parsing, repeated graph rebuilds, and hidden nested scans;
- keep production comments focused on durable contracts, not milestone bookkeeping.

For architecture or roadmap work, read `CLAUDE.md`, `docs/implementation_roadmap.md`, `docs/content_scope_v3.md`, and the relevant milestone-agnostic docs before changing source.

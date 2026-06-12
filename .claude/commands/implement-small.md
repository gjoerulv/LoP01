# Implement-small command guidance

Use this for narrow implementation slices only.

Current baseline: **post-M30**. Do not treat M17-M30 work as future work.

Before editing, read `CLAUDE.md`, `docs/implementation_roadmap.md`, and the active content scope. If no milestone is selected in the active roadmap, stop and request/perform a planning audit instead of inventing implementation scope. As of the post-M30 baseline, no next milestone is selected and v2 is ready to archive.

Implementation rules:

- Change only the files required by the selected slice.
- Add tests with every gameplay/data/schema change.
- Keep gameplay logic out of rendering/input layers.
- Keep content definitions separate from runtime mutable state.
- Avoid per-frame scans, repeated content parsing, graph rebuilds, large needless copies, and hidden nested scans.
- Avoid milestone/phase labels in production comments.
- Preserve battle-before-placement for hostile-occupied travel, centralize ownership mutation behind `GameSession`, and avoid double-running arrival/capture side effects.
- Preserve M25 stationing, M28 storage, M29 travel-loss, and M30 service-defense/destruction/TU invariants; avoid unit duplication/loss and dangling stationed/stored refs. Do not add full-simulation defense battles, shared hero pool, or other v3 systems unless the active milestone explicitly selects them.

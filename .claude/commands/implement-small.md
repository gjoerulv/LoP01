# Implement-small command guidance

Use this for narrow implementation slices only.

Current baseline: **post-M25**. Do not treat M17-M25 work as future work.

Before editing, read `CLAUDE.md`, `docs/implementation_roadmap.md`, and `docs/content_scope_v2.md`. If no milestone is selected in the active roadmap, stop and request/perform a planning audit instead of inventing implementation scope. At this baseline, M26 is selected unless the active roadmap has changed.

Implementation rules:

- Change only the files required by the selected slice.
- Add tests with every gameplay/data/schema change.
- Keep gameplay logic out of rendering/input layers.
- Keep content definitions separate from runtime mutable state.
- Avoid per-frame scans, repeated content parsing, graph rebuilds, large needless copies, and hidden nested scans.
- Avoid milestone/phase labels in production comments.
- For M26 claiming work, preserve battle-before-placement for hostile-occupied travel, centralize ownership mutation behind `GameSession`, and avoid double-running arrival/capture side effects.
- Preserve M25 stationing invariants and avoid unit duplication/loss. Do not implement full Storage/Garrison unless the active milestone explicitly selects it.

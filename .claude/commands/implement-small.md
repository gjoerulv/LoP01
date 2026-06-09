# Implement-small command guidance

Use this for narrow implementation slices only.

Current baseline: **post-M24**. Do not treat M17-M24 work as future work.

Before editing, read `CLAUDE.md`, `docs/implementation_roadmap.md`, and `docs/content_scope_v2.md`.

If no milestone is selected in the active roadmap, stop and request/perform a planning audit instead of inventing implementation scope. At this baseline, M25 is selected unless the active roadmap has changed.

Implementation rules:

- Change only the files required by the selected slice.
- Add tests with every gameplay/data/schema change.
- Keep gameplay logic out of rendering/input layers.
- Keep content definitions separate from runtime mutable state.
- Avoid per-frame scans, repeated content parsing, graph rebuilds, large needless copies, and hidden nested scans.
- Avoid milestone/phase labels in production comments.
- For stationing work, preserve the stack-backed stationed-unit invariant and avoid unit duplication/loss. Do not implement full Storage/Garrison unless the active milestone explicitly selects it.

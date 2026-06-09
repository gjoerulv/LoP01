# Implement-small command guidance

Use this for narrow implementation slices only.

Current baseline: **post-M23**. Do not treat M17-M23 work as future work.

Before editing, read `CLAUDE.md` and the active roadmap. If no milestone is selected, stop and request/perform a planning audit instead of inventing implementation scope.

Implementation rules:

- Change only the files required by the selected slice.
- Add tests with every gameplay/data/schema change.
- Keep gameplay logic out of rendering/input layers.
- Keep content definitions separate from runtime mutable state.
- Avoid per-frame scans, repeated content parsing, graph rebuilds, large needless copies, and hidden nested scans.
- Avoid milestone/phase labels in production comments.
- For ownership work, remember that M23 implements player-side guarded-service claiming only; enemy-side capture, unguarded claiming, service destruction/restoration, and AI economy require explicit future milestones.

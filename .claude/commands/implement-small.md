# Small implementation command

When implementing a scoped Ashvale change:

1. Re-read `CLAUDE.md` and the relevant active docs.
2. Confirm the baseline is post-M20 unless the branch/commit proves otherwise.
3. State the intended files to change before editing.
4. Keep the slice narrow and test-backed.
5. Preserve separation of concerns:
   - App/input owns interaction state and command translation;
   - gameplay/session owns rules and mutable game state;
   - data/validation owns authored JSON contracts;
   - renderers draw from models and should not own gameplay logic.
6. Avoid broad framework work, demo-specific branches, and speculative systems.
7. Avoid per-frame content scans, repeated parsing, graph rebuilds, large needless copies, and hidden nested scans.
8. Run targeted tests and full tests when practical.
9. Report files changed, tests run, and any unresolved risks.

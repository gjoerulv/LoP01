# Audit implementation command guidance

Use this to review source changes against the active docs and current architecture.

Current baseline: read from `CLAUDE.md` and `docs/implementation_roadmap.md`; as of the post-M27 baseline, **M27 — Owned Service Overview / Strategic Service Readout** is complete and no next milestone is selected unless the roadmap says otherwise.

Audit priorities:

- Verify the branch/commit is current and not stale.
- Check source/docs alignment.
- Check separation of concerns: gameplay rules in gameplay/data layers, App handles input/flow, renderers draw models.
- Check save/load compatibility and runtime/content-state separation.
- Check performance traps: per-frame scans, repeated parsing, graph rebuilds, large copies, hidden O(n²) paths.
- Check that ownership claiming remains the narrow player-side path unless a later roadmap explicitly broadens it.
- Reject implementations that silently change ownership, economy, scenario, roster, stationing, or save semantics without tests and docs.
- Preserve M25 stationing invariants, M26 player-side claiming semantics, and M27 read-only overview semantics unless a later milestone explicitly changes them.
- Treat the M27 overview as a strategic readout foundation, not final service-management UI. Reject attempts to add remote stationing, Storage/Garrison, service repair/destruction, ownership transfer, or other management actions through it unless selected by roadmap.

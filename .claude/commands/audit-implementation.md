# Audit implementation command guidance

Use this to review source changes against the active docs and current architecture.

Current baseline: **post-M25**.

Latest completed milestone: **M25 — Player-facing Service Stationing Flow**.

Current selected milestone: **M26 — General Owned-Service Claiming Semantics** unless the active roadmap has changed.

Audit priorities:

- Verify the branch/commit is current and not stale.
- Check source/docs alignment.
- Check separation of concerns: gameplay rules in gameplay/data layers, App handles input/flow, renderers draw models.
- Check save/load compatibility and runtime/content-state separation.
- Check performance traps: per-frame scans, repeated parsing, graph rebuilds, large copies, hidden O(n²) paths.
- Check that ownership claiming remains the narrow M26 path unless a later roadmap explicitly broadens it.
- Reject implementations that silently change ownership, economy, scenario, roster, stationing, or save semantics without tests and docs.
- Preserve M25 stationing invariants while changing service claiming.

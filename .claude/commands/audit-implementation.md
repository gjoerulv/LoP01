# Audit implementation command guidance

Use this to review source changes against the active docs and current architecture.

Current baseline: **post-M23**. Latest completed milestone: **M23 — Owned Service Claiming and Contesting Foundation**.

Audit priorities:

- Verify the branch/commit is current and not stale.
- Check source/docs alignment.
- Check separation of concerns: gameplay rules in gameplay/data layers, App handles input/flow, renderers draw models.
- Check save/load compatibility and runtime/content-state separation.
- Check performance traps: per-frame scans, repeated parsing, graph rebuilds, large copies, hidden O(n²) paths.
- Check that owned-service claiming remains the narrow M23 path unless a later roadmap explicitly broadens it.
- Reject implementations that silently change ownership, economy, scenario, or save semantics without tests and docs.

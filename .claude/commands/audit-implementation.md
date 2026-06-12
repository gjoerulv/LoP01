# Audit implementation command guidance

Use this to review source changes against the active docs and current architecture.

Current baseline: read from `CLAUDE.md` and `docs/implementation_roadmap.md`; as of the post-M29 baseline, **M29 — Cross-Region Generic Unit Preservation / Travel Warning** is complete and the next milestone is not selected unless the roadmap says otherwise.

Audit priorities:

- Verify the branch/commit is current and not stale.
- Check source/docs alignment.
- Check separation of concerns: gameplay rules in gameplay/data layers, App handles input/flow, renderers draw models.
- Check save/load compatibility and runtime/content-state separation.
- Check performance traps: per-frame scans, repeated parsing, graph rebuilds, large copies, hidden O(n²) paths.
- Check that ownership claiming remains the narrow player-side path unless a later roadmap explicitly broadens it.
- Reject implementations that silently change ownership, economy, scenario, roster, stationing, storage, travel-loss, or save semantics without tests and docs.
- Preserve M25 stationing invariants, M26 player-side claiming semantics, M27 read-only overview semantics, M28 storage invariants, and M29 travel-loss semantics unless a later milestone explicitly changes them.
- Treat the M27 overview as a strategic readout foundation, not final service-management UI. Reject attempts to add remote stationing, storage/garrison management, service repair/destruction, ownership transfer, or other management actions through it unless selected by roadmap.
- For travel-loss changes, require explicit warning/confirmation before traveling generic stacks are lost, confirmed removal through `GameSession`, stored/stationed units unaffected, and no broad service-defense/capture/loss scope creep.

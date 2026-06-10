# Review-diff command guidance

When reviewing a diff:

- Compare the diff to active docs and roadmap, not archived files.
- Treat the project baseline as whatever `CLAUDE.md` and `docs/implementation_roadmap.md` currently state.
- Be binary where possible: accept 100% or reject with a specific revision prompt.
- Reject stale roadmap/baseline wording, duplicated source-of-truth rules, unsafe save/load behavior, and untested gameplay/schema changes.
- Reject changes that regress M25 stationing invariants, M26 player-side claiming semantics, or M27 read-only overview semantics unless a later roadmap explicitly replaces them.
- Reject ownership changes that scatter mutation through `App`, double-run arrival/capture side effects, break guarded battle-before-placement behavior, or silently broaden into enemy-side capture/destruction/Storage/Garrison.
- Reject attempts to turn the M27 overview into remote stationing, repair, ownership transfer, Storage/Garrison, or other service-management UI unless the active roadmap explicitly selects that work.
- For comment-only cleanup, prefer a direct patch over sending work back to an agent.
- Production comments should document durable contracts, not milestone bookkeeping.

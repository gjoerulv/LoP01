# Review-diff command guidance

When reviewing a diff:

- Compare the diff to active docs and roadmap, not archived files.
- Treat the project as post-M23 unless the branch says otherwise.
- Be binary where possible: accept or reject with a specific revision prompt.
- Reject stale roadmap/baseline wording, duplicated source-of-truth rules, unsafe save/load behavior, and untested gameplay/schema changes.
- For comment-only cleanup, prefer a direct patch over sending work back to an agent.
- Production comments should document durable contracts, not milestone bookkeeping.

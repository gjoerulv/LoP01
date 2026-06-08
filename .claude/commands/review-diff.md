# Diff review command

When reviewing a diff:

1. Verify the exact commit or branch state.
2. Review against active docs and current baseline: post-M22.
3. Return a binary result:
   - accept 100%, or
   - reject with only blocking fixes.
4. Be strict about correctness, validation, save/load compatibility, architecture, and performance.
5. For docs/comment-only fixes, provide a direct patch instead of asking for unnecessary agent implementation work.
6. Flag stale milestone labels in production source comments unless they are temporary and removed before merge.
7. Test comments are acceptable when they explain non-obvious regression intent.

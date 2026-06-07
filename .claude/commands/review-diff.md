# Diff review command

When reviewing a diff:

1. Verify the diff target branch/commit.
2. Review against the active post-M20 docs and current source.
3. Prioritize correctness, architecture, performance, validation, save/load, and test coverage over style.
4. Flag only issues that can realistically break behavior, confuse future work, or violate the active docs.
5. Production comments should be sparse and durable. Flag milestone/phase comments, stale comments, and comments that merely narrate obvious code.
6. Tests may include comments when they explain non-obvious regression intent.
7. Return binary feedback when asked: accept 100%, or reject with a revision prompt.

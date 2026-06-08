# Implement small command

When implementing a scoped Ashvale change:

1. Read `CLAUDE.md` and the relevant active docs first.
2. Verify the branch baseline. Expected current baseline is post-M22 unless the branch proves otherwise.
3. Keep the implementation slice narrow and test-backed.
4. Do not expand into unselected systems.
5. Preserve separation of concerns and save/load compatibility.
6. Preserve authored-content vs runtime-state boundaries.
7. Avoid milestone/phase labels in production source comments.
8. Add or update tests for the changed behavior.
9. Run targeted tests and the full test suite when practical.
10. Report files changed, tests run, and any remaining risks.

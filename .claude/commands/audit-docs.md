# Documentation audit command

When auditing docs:

1. Treat active docs as current source-of-truth candidates, but verify against source.
2. Archived docs are historical context only.
3. Current active docs should describe a post-M20 baseline.
4. Look for:
   - stale milestone status;
   - contradictions between roadmap, content scope, schema, validation, and code;
   - references to M17/M18/M19/M20 as future work;
   - claims that Trading Post transaction or interaction flow is unimplemented;
   - excessive or redundant documentation.
5. Keep fixes concise. Do not turn docs into a changelog.
6. If a doc update is needed, update only the files that prevent ambiguity or agent misdirection.

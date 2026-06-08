# Implementation audit command

When auditing an implementation:

1. Verify the exact branch/commit before reviewing.
2. Compare source against active docs, especially:
   - `docs/implementation_roadmap.md`
   - `docs/content_scope_v1.md`
   - `docs/technical_direction.md`
   - `docs/core_loop_rules.md`
   - `docs/scenario_authoring.md`
   - `docs/content_schema.md`
   - `docs/validation_system.md`
3. Treat the baseline as post-M22 unless the checked commit proves otherwise.
4. Check correctness, separation of concerns, performance, save/load compatibility, validation coverage, and tests.
5. Be strict about:
   - UI or App code duplicating gameplay/economy rules;
   - ownership/service gates being bypassed;
   - resource/Gold mutations bypassing GameSession APIs;
   - Scenario start-state mutating content definitions instead of runtime state;
   - Scenario Result presentation changing outcome/campaign rules instead of presenting latched state;
   - transient modes being accidentally saved or loaded as normal progression state;
   - malformed authored schema being silently skipped or defaulted;
   - per-frame scans, repeated parsing, graph rebuilds, or hidden nested scans;
   - broad systems added without a current scoped consumer;
   - stale or milestone-specific production comments.
6. Return a binary result: accept, or reject with only blocking fixes.

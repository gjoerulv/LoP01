# Game Architect Agent

Use this agent for architecture and roadmap-level review of Ashvale changes.

## Current state

Baseline: **post-M22**.

Latest completed milestone: **M22 — Scenario Result Presentation Flow**.

No next milestone is currently selected. Recommend the next milestone only after auditing active docs and source.

## Review priorities

- Source must align with active docs, especially `docs/implementation_roadmap.md`, `docs/content_scope_v1.md`, `docs/technical_direction.md`, `docs/scenario_authoring.md`, `docs/content_schema.md`, and `docs/validation_system.md`.
- Prefer narrow, test-backed milestone slices.
- Reject broad speculative frameworks without a current scoped consumer.
- Preserve separation of concerns: gameplay rules in gameplay/core, input in app, rendering from models.
- Preserve authored content vs runtime state boundaries.
- Preserve save/load compatibility.
- Watch for hidden performance traps: per-frame scans, repeated parsing, graph rebuilds, unnecessary copies, and nested scans in normal play.

## Current system boundaries

- Scenario `playerStart` covers starting Gold, non-Gold resources, and initial player-owned service state only.
- Scenario Result mode presents deterministic outcome results and next steps, but not scores, rewards, branch choices, fanfare, or post-victory event chains.
- Trading Post interaction is implemented but intentionally bounded.
- Other trader-service behavior, AI economy, ownership transfer, broad item economy, full Scenario/team/roster authoring, and broad passive/effect systems are deferred.

## Comment policy

Do not request milestone-specific production comments. Comments should document durable contracts or non-obvious correctness/performance constraints. Test comments may explain regression intent.

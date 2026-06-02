# Project Ashvale - Claude Code Instructions

This project is documentation-driven. Before making design, architecture, roadmap, or implementation changes, read the authoritative docs listed below.

## Authoritative docs

Read these first when relevant:

1. `README.md`
2. `README_DECISIONS.md`
3. `docs/implementation_roadmap.md`
4. `docs/content_scope_v1.md`
5. `docs/technical_direction.md`
6. `docs/game_vision.md`
7. `docs/game_shell_flow.md`
8. `docs/presentation_game_feel.md`
9. `docs/core_loop_rules.md`
10. `docs/combat_rules.md`
11. `docs/scenario_authoring.md`
12. `docs/validation_system.md`
13. `docs/content_schema.md`
14. `docs/terminology_map.md`
15. `.github/copilot-instructions.md`
16. `.github/instructions/gameplay.instructions.md`

For UI work, also read:

- `.github/instructions/ui.instructions.md`

For architecture work, also read:

- `.github/agents/game-architect.agent.md`

Archived docs and historical milestone prompts are historical context only. Do not use `docs/content_scope_v0.md.archived` or `docs/implementation_roadmap.md.00.archived` as current scope, roadmap, or behavior truth.

## Current baseline

The current codebase is post-M17.

Completed foundations include:

- battle, roster, save/load, Region/Location flow, and content validation foundation;
- typed events and scenario outcome rules;
- practical enemy-team Region-layer foundation;
- inventory and artifacts foundation;
- team Energy pool foundation;
- minimal World Map region-to-region travel;
- minimal Campaign System foundation;
- owned-service/economy foundation: resources, owned services, mine outputs, stack-backed stationing, narrow mine-production passives, day-boundary mine payout, trader ownership tiers, authored/default trader curves, validation, and proof tests.

Do not treat M8, M11, M12, M13, M14, M15, M16, or M17 as future work.

## Current planning posture

The next planned milestone is M18: Passive Effect Spine, unless the user explicitly redirects.

M18 planning should stay narrow, coherent, and test-backed. It should generalize only the passive/effect seams that have an immediate consumer, not create a broad skill tree, status system, or economy framework.

## Claude Code project commands

Reusable Claude Code workflows live in `.claude/commands/`.

Use these command prompts when relevant:

- `audit-docs.md` for Markdown consistency audits;
- `audit-implementation.md` for code-vs-doc audits;
- `roadmap.md` for implementation roadmap planning;
- `first-milestone.md` for planning the first small implementation milestone;
- `implement-small.md` for approved narrow implementation patches;
- `review-diff.md` for pre-commit diff review;
- `doc-patch.md` for focused documentation updates.

Do not treat command files as design truth. They are workflow prompts. The docs listed above remain authoritative.

## Core rules

- Respect `docs/technical_direction.md` for architectural principles, performance posture, and source-layout constraints.
- Respect `docs/content_scope_v1.md` for current post-M17 content scope.
- Respect `docs/presentation_game_feel.md` for moment-to-moment presentation, audio/visual tone, transitions, and feedback.
- Do not invent new game-design rules that contradict the docs.
- Do not reintroduce old terminology such as `overworld`, `combat node`, or `game_vision_complete.md` as current design truth.
- Use the current terms: World Map, Region, Location, Service, node content, Scenario Info screen, Adventure button strip.
- Treat nodes as travel points with one main node content item plus optional event attachments.
- Treat content schema rules in `docs/content_schema.md` as authoritative for authored JSON direction.
- Treat validation rules in `docs/validation_system.md` as authoritative for validation planning.
- Keep authored initial state separate from runtime save state.
- Use `kind + id` as mod override identity when mod loading is implemented.
- Avoid demo-specific source branches; prove systems through authored content and generic rules.

## Source comments and code documentation

Keep production source comments durable and sparse.

Do not add comments merely to describe milestone progress, patch phases, or agent workflow. Good production comments explain one of these:

- a non-obvious invariant or contract;
- a correctness, security, data-integrity, or save/load trap;
- a performance-sensitive choice;
- a deliberate limitation that prevents accidental broadening;
- compatibility behavior that looks wrong but is intentional.

Avoid milestone labels such as `M17 Phase 3a` in production source unless the comment is temporary and removed before merge. Milestone context belongs in roadmap docs, decision logs, prompts, commits, and tests — not durable source contracts.

Test comments are acceptable when they clarify non-obvious regression intent or why a scenario matters. Do not clutter tests with restatements of obvious assertions.

When reviewing diffs, flag comments that are stale, milestone-specific, redundant, or inconsistent with the code. Prefer deleting weak comments over rewriting them into longer comments.

## Workflow

- Work on a git branch.
- Before editing, summarize the plan and list the files you intend to change.
- Prefer small, reviewable patches.
- Run available build/tests after meaningful code changes.
- Do not delete or archive docs unless explicitly asked.
- Do not make broad rewrites when a targeted change is enough.
- If the docs conflict, stop and report the conflict instead of guessing.
- If implementation conflicts with the docs, report it and propose options before changing the design.
- For Claude-plan review, expect binary feedback: accepted 100%, or rejected with a revision prompt.
- Do not ask for an "Exact Next Prompt" unless explicitly requested.

## First task pattern

For large tasks, start by producing:

1. current-state audit;
2. proposed implementation roadmap;
3. milestone breakdown;
4. first small patch recommendation.

Do not start implementing the full vision in one pass.

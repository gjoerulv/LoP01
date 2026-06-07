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

For UI work, also read `.github/instructions/ui.instructions.md`. For architecture work, also read `.github/agents/game-architect.agent.md`.

Archived docs and historical milestone prompts are historical context only. Do not use `docs/content_scope_v0.md.archived` or `docs/implementation_roadmap.md.00.archived` as current scope, roadmap, or behavior truth.

## Current baseline

The current codebase is post-M19.

Completed foundations include:

- battle, roster, save/load, Region/Location flow, and content validation foundation;
- typed events and scenario outcome rules;
- practical enemy-team Region-layer foundation;
- inventory and artifacts foundation;
- team Energy pool foundation;
- minimal World Map region-to-region travel;
- minimal Campaign System foundation;
- owned-service/economy foundation: resources, owned services, mine outputs, stack-backed stationing, day-boundary mine payout, trader ownership tiers, authored/default trader curves, validation, and proof tests;
- passive-effect spine foundation: canonical unit `passive_effects`, legacy mine-passive authoring compatibility, `mine_production` effects for owned mines, and current-leader `leader_energy` effects for daily Energy;
- headless Trading Post transaction foundation: pure barter/Gold quote rules, service-specific ownership/use gates, GameSession transaction APIs, tier-0 fallback/default behavior, Gold delegation, and validation/test coverage.

Do not treat M8, M11, M12, M13, M14, M15, M16, M17, M18, or M19 as future work.

## Current planning posture

The likely next milestone is **M20: Trading Post Interaction Flow**, unless the user explicitly redirects. M20 should expose the already-implemented Trading Post transaction API through the smallest coherent service interaction flow without becoming a full shop UI, broad item economy, AI economy, or all-trader-service expansion.

## Core rules

- Respect `docs/technical_direction.md` for architectural principles, performance posture, and source-layout constraints.
- Respect `docs/content_scope_v1.md` for current post-M19 content scope.
- Respect `docs/presentation_game_feel.md` for presentation, audio/visual tone, transitions, and feedback.
- Do not invent new game-design rules that contradict the docs.
- Use current terms: World Map, Region, Location, Service, node content, Scenario Info screen, Adventure button strip.
- Treat content schema rules in `docs/content_schema.md` as authoritative for authored JSON direction.
- Treat validation rules in `docs/validation_system.md` as authoritative for validation planning.
- Keep authored initial state separate from runtime save state.
- Avoid demo-specific source branches; prove systems through authored content and generic rules.

## Source comments and code documentation

Keep production source comments durable and sparse.

Do not add comments merely to describe milestone progress, patch phases, or agent workflow. Good production comments explain one of these:

- a non-obvious invariant or contract;
- a correctness, security, data-integrity, or save/load trap;
- a performance-sensitive choice;
- a deliberate limitation that prevents accidental broadening;
- compatibility behavior that looks wrong but is intentional.

Avoid milestone labels such as `M19 Phase 2` in production source unless the comment is temporary and removed before merge. Milestone context belongs in roadmap docs, decision logs, prompts, commits, and tests — not durable source contracts.

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

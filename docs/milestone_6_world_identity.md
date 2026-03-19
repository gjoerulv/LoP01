# Milestone 6 - World Identity and Route Rules

## Purpose

Milestone 5 completed the first full world-loop baseline for the playable slice.

Milestone 6 should make the world behave more like the typed content model already implies, without changing the overall architecture.

The goal is not broad feature growth.
The goal is to make locations, travel, and light world state feel more coherent and less placeholder.

## Main outcome

After this milestone, the player should be able to:

- experience clearer differences between locations that currently share prototype scene layout
- use services only where they are actually valid for the current location
- travel through the overworld using route-aware rules instead of placeholder travel logic
- see lightweight persistent world state where the slice already implies it
- continue playing through the existing overworld / location / battle loop with the same explicit architecture

## Scope

This milestone should focus on:

- location identity
- location-valid services
- route-aware overworld travel
- minimal persistent node/world state
- light quest trigger expansion only where needed to support the above
- lightweight UI clarity improvements
- save/load updates only where needed to support the above

It should avoid:

- broad content expansion
- extra regions
- major combat redesign
- inventory/equipment systems
- large event scripting systems
- full service-framework generalization
- large renderer rewrites

## Architectural expectations

- Preserve the explicit `App` / `GameSession` flow.
- Preserve the controller / mapper / renderer split.
- Keep gameplay logic separate from rendering.
- Keep additions data-driven where practical.
- Prefer typed, explicit runtime state over generic scripting systems.
- Add tests for new pure logic where feasible.
- Keep milestone work incremental and compilable.

## Gameplay priorities

### 1. Make location identity real in the slice

Several locations currently share prototype scene layout.
That is acceptable as a content shortcut, but gameplay behavior should not leak across locations just because they reuse the same scene definition.

Milestone 6 should ensure that a location can expose only the interactions/services intended for that location.

### 2. Make valid services location-aware

Rest/services should be valid because the current location allows them, not only because a shared prototype scene contains a matching interaction zone.

For the current slice, the most important case is rest/sleep validity.

The implementation should choose the smallest clean approach that preserves current architecture.

### 3. Make overworld travel route-aware

The overworld should behave more like the rules in `docs/core_loop_rules.md`.

Milestone 6 should move travel toward:

- same node costs 0 minutes
- travel legality follows route links
- travel time preview reflects route/path cost
- unavailable or blocked paths are communicated clearly

A small explicit path/travel rule is preferred over a large generalized navigation system.

### 4. Add minimal persistent node/world state

The slice already suggests that some nodes should change after use or after victory.

Milestone 6 may add a very small amount of persistent world state for things like:

- a battle node being cleared
- a single-use node being consumed
- similar small state needed to make the current world loop feel coherent

Only persist what the current slice actually needs.

### 5. Keep quest growth minimal

Quests should remain typed and small.

Milestone 6 may extend quest trigger support only as needed to connect quests to clearer world actions such as:

- destination reached
- node cleared
- location interaction completed

Avoid broad acceptance trees, branching quest graphs, or scripting-heavy quest systems.

## Recommended implementation order

1. Fix repo docs/prompts/instructions so they match the Milestone 5 baseline and Milestone 6 intent
2. Make location-valid service rules explicit
3. Harden rest/sleep validity
4. Make overworld travel route-aware
5. Add minimal persistent node/world state
6. Extend quest triggers only where needed
7. Add light presentation improvements to communicate the above

## Acceptance checklist

- [ ] Milestone 5 architecture remains intact
- [ ] Shared prototype scenes do not accidentally expose invalid services at unrelated locations
- [ ] Rest is valid only at intended rest locations for the current slice
- [ ] Same-node travel can be treated as 0 minutes
- [ ] Route-aware travel rules exist for the current slice
- [ ] Blocked/unavailable travel can be communicated clearly
- [ ] At least one small piece of node/world state persists meaningfully
- [ ] Save/load is extended only where needed
- [ ] New pure logic has tests where feasible

## Out of scope notes

Milestone 6 is not the milestone for:

- adding lots of new content
- redesigning battle systems
- adding broad UI frameworks
- adding free-walk overworld movement
- building a generalized live-service/action/event engine

The milestone should stay small, explicit, and testable.
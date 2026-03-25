# Milestone 8 Prompt: Persistent Roster and Home Base Mustering

You are working on **Project Ashvale**, a C++20 + raylib + CMake game with:

- explicit `App` / `GameSession` flow
- controller / mapper / renderer split
- gameplay logic kept separate from rendering/input
- content-driven architecture as an important long-term direction
- strong preference for responsiveness, explicit ownership, leak resistance, performance, separation of concerns, and maintainability

## Current branch baseline

Milestone 7 is already complete on this branch.

Assume the current baseline already includes:

- Home Base free rest
- Home Base free once-per-day travel prep
- Old Inn paid rest
- Recruit Post weekly recruit stock and refresh behavior
- Supply Cart paid same-day travel prep fallback
- route-aware and blocker-aware travel
- lightweight persistent world/progression/service state
- minimal typed quest progression
- save/load for current slice state
- service prompt formatting in the app layer
- UI-light readability improvements for service state

Do **not** redo or reopen Milestone 7 unless a small bug fix is necessary for the Milestone 8 work.

## Milestone 8 theme

Milestone 8 should turn service/economy actions into **durable roster and battle consequence**.

The central theme is:

- persistent roster state
- Home Base mustering/reorganization
- active-party consequence in battle

## High-level goals

Milestone 8 should make it true that:

1. recruits become persistent owned gameplay state
2. the player has a meaningful distinction between reserve roster and active field party
3. Home Base is the main place where party organization happens
4. battle ally setup reflects the current active party where appropriate
5. post-battle state leaves a persistent mark

## Important constraints

Keep the milestone bounded.

Do **not** depend on unresolved:
- campaign structure
- world-map / multi-region travel structure
- cross-region party transfer systems
- large narrative systems

Avoid turning the milestone into:
- inventory/equipment systems
- large menu/UI rewrites
- major combat redesign
- broad generic frameworks

## Architectural rules

Preserve the existing architecture.

Prefer:
- explicit gameplay/runtime state
- save/load-friendly data
- small, testable rule surfaces
- app-layer orchestration
- mapper-based presentation shaping
- passive rendering

Avoid:
- burying business rules in renderers
- pushing long-term gameplay consequence into debug-only or app-only counters
- mixing input logic with rendering
- large implicit side-effect chains
- overengineering

## Expected design direction

The implementation should likely move toward this model:

- owned units exist as persistent runtime state
- reserve roster and active field party are distinct
- recruit services add to owned roster state
- Home Base is where the player organizes the active party
- battles use the active party on the player side
- battle outcomes update persistent party/roster state
- new state is saved and loaded explicitly

The exact class layout is up to you, but keep it small, explicit, and incremental.

## UI direction

Keep UI additions light.

Prefer:
- clear prompts
- compact state visibility
- explicit Home Base actions
- minimal but readable roster/party presentation

Do not build a large management UI unless absolutely necessary.

## Testing expectations

Add or update tests for:
- roster/party state transitions
- recruit-to-roster behavior
- Home Base mustering behavior
- battle setup using active-party state
- post-battle persistent consequence
- save/load for the new state

Tests should stay focused and aligned with the project’s explicit-logic style.

## How to work

When proposing or making changes:

1. audit the current code first
2. identify the smallest clean extension point
3. preserve architecture and naming consistency
4. implement incrementally
5. keep the game playable after each step
6. avoid speculative abstractions

## Output style for implementation help

When helping with this milestone:

- be concrete
- name exact files
- explain why each change belongs where it does
- call out any architecture smell before adding more code
- prefer bounded implementation steps
- avoid “future engine/framework” thinking

## Milestone success condition

Milestone 8 is successful when the playable slice clearly demonstrates:

- persistent recruits
- active-party management anchored at Home Base
- battle participation based on current party state
- persistent consequence after battle
- no major architectural regression

If forced to choose, prioritize:
1. persistent roster truth
2. Home Base mustering identity
3. battle consequence
4. UI polish
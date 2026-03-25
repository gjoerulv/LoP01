# Milestone 8: Persistent Roster and Home Base Mustering

## Purpose

Milestone 8 should turn the post-M7 service/economy groundwork into durable gameplay consequence.

Milestone 7 made several services meaningful in terms of:
- gold
- time
- weekly cadence
- player-facing clarity

But the largest remaining gap is still this:

- recruit actions affect economy and messaging
- recruit actions do not yet become persistent battle-relevant gameplay state

Milestone 8 should close that gap in a bounded way that will likely survive future vision changes.

## Core milestone theme

Milestone 8 focuses on:

- persistent roster state
- Home Base as the main mustering/reorganization anchor
- active-party consequence in battle

This should strengthen the strategy/RPG identity of the slice without requiring:
- multi-region systems
- campaign structure decisions
- inventory/equipment systems
- major combat redesign
- large UI rewrites

## Why this milestone now

This is the most valuable next step because it:

1. turns recruit services into durable consequence
2. gives Home Base a stronger systemic identity
3. deepens progression without depending on unresolved world-map decisions
4. fits the current architecture and content-driven direction
5. remains bounded to the current single-region slice

## Current post-M7 baseline

The repo already includes:

- explicit `App` / `GameSession` flow
- controller / mapper / renderer split
- route-aware and blocker-aware overworld travel
- lightweight persistent world/progression state
- unified wake-penalty recovery flow
- minimal typed quest progression
- save/load for current slice state
- Home Base free rest
- Home Base free once-per-day travel prep
- Old Inn paid rest
- Recruit Post weekly recruit stock
- Supply Cart paid travel prep fallback
- app-layer service prompt formatting
- UI-light service readability improvements

Milestone 8 should build on this baseline rather than reopening it.

## Milestone 8 goal

The player should be able to:

- acquire recruits through services
- keep those recruits as persistent owned state
- organize a current active field party
- use that active party in battle
- feel battle outcomes leave a persistent mark
- rely on Home Base as the main place to reorganize and recover

## Intended player-facing outcome

After Milestone 8, the game loop should feel closer to:

1. earn or spend resources
2. recruit units
3. those units become owned roster state
4. organize who is currently in the field party
5. take that party into battle
6. return with meaningful persistent outcome
7. use Home Base to stabilize and reorganize

That is the milestone’s real payoff.

## Scope

### In scope

- persistent roster state
- distinction between reserve roster and active field party
- recruit actions adding to persistent roster state
- Home Base as the primary mustering/reorganization point
- battle ally composition using the active party where appropriate
- simple persistent post-battle party consequence
- save/load support for new roster/party state
- lightweight UI/state visibility needed to support the flow
- tests for the new gameplay rules and persistence behavior

### Out of scope

- multi-region roster transfer
- inventory/equipment
- broad injury/medical systems
- broad resurrection systems
- major combat redesign
- generic party-management framework
- large menu/UI overhaul
- campaign/scenario/world-map expansion
- large content expansion unrelated to roster/home-base consequence

## Design direction

### 1) Introduce explicit persistent roster state

Milestone 8 should add a typed gameplay-facing runtime model for owned units.

At minimum, the game should distinguish between:

- reserve roster
- active field party

This state should live in gameplay/runtime state and be save/load friendly.

Avoid solving this as:
- UI-only state
- app-only bookkeeping
- debug-only counters
- ad hoc battle overrides

### 2) Recruiting should add to persistent owned state

Recruit services should stop being only:

- gold spent
- stock reduced
- message shown

They should instead produce durable roster consequence.

A recruit action should mean:
- gold changed
- weekly stock changed
- time changed
- owned roster changed

### 3) Home Base should become the mustering anchor

Home Base should gain one strong strategic identity in addition to free rest and travel prep:

- organize active field party from owned roster state

This should be the main place where the player stabilizes and prepares for further action.

Keep the implementation small and explicit.

### 4) Battle allies should reflect the current active party

Milestone 8 should stop relying only on static allied setup for the player’s side.

Enemy setup can stay content-authored.

The player-side allied composition should come from current active-party state where appropriate.

This is the key bridge from service/economy consequence to battle consequence.

### 5) Battle should leave a persistent mark

Post-battle state should matter.

Keep this bounded and slice-appropriate.

Examples of acceptable M8-level consequence:
- unit loss persists
- reduced current readiness/health persists
- active-party state changes after battle
- Home Base or resting helps recover

Avoid turning this into a sprawling injury/revival subsystem.

### 6) Keep UI additions lightweight

The goal is stronger state clarity, not a UI-heavy milestone.

Prefer:
- clear prompts
- compact status visibility
- simple Home Base management flow
- explicit state transitions

Avoid:
- large modal systems
- full management screens unless truly necessary
- renderer-driven gameplay decisions

## Architectural guidance

Milestone 8 should preserve and respect:

- explicit ownership
- explicit mode transitions
- gameplay logic separate from rendering/input
- controller / mapper / renderer split
- content-driven data where it helps
- responsiveness and performance
- leak resistance and maintainability

Important guidance:

- do not let `App.cpp` become the long-term home of roster/business rules
- keep gameplay consequence logic in gameplay-facing state/rules
- keep rendering passive and derived from gameplay state
- keep save/load updates explicit and testable

## Recommended implementation shape

The exact class/file breakdown can be decided during implementation, but the structure should likely follow this pattern:

- gameplay/runtime state owns persistent roster + active party
- app layer orchestrates interactions and display
- mappers convert roster/party state into lightweight UI models
- battle setup reads active-party state
- save/load persists the new state explicitly

## Acceptance criteria

Milestone 8 is successful when all of the following are true:

1. Recruiting through the Recruit Post creates persistent owned unit state.
2. The player has a clear active-party versus reserve distinction.
3. Home Base is the primary place where party organization happens.
4. Battles use the current active party for the player side where appropriate.
5. Post-battle party state changes persist meaningfully.
6. Save/load preserves the new roster/party state correctly.
7. The implementation remains explicit, testable, and aligned with the current architecture.
8. The slice remains coherent without depending on unresolved campaign/world-map decisions.

## Non-goals and guardrails

Do not let Milestone 8 expand into:
- inventory systems
- equipment screens
- extra regions
- broad new service classes
- large combat rebalance work
- a generic simulation layer
- premature abstractions that reduce clarity

When in doubt, prefer:
- the smallest clean design
- explicit state
- predictable flow
- persistent consequence over new feature count

## Summary

Milestone 8 should make recruits real.

That means:
- owned roster state
- active field-party state
- Home Base mustering
- persistent battle consequence

This is a meaningful step toward a fuller final product while staying bounded, durable, and compatible with the current architecture.
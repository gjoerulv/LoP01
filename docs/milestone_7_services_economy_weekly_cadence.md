# Milestone 7 - Home Base, Services, and Weekly Cadence

## Purpose

Milestone 6 hardened world identity, routing, lightweight world state, and quest/world coherence.

Milestone 7 should be a **larger, more visible update** that makes the game feel more like a living strategy/RPG loop rather than only a route-and-consequence prototype.

The primary goal is to make home base and region services matter economically and rhythmically across days and weeks.

## Main outcome

After this milestone, the player should be able to:

- feel that home base is the central safe hub of the slice
- rest for free at home base and pay for rest at inns
- visit recruit locations that offer content-defined unit types with limited quantities
- see recruit quantities replenish on a weekly cadence
- interact with at least one more service/economy system that is content-driven rather than hardcoded
- keep playing through the same explicit overworld / location / battle loop without architectural drift

## Scope

Milestone 7 should focus on:

- home base identity as the main safe hub
- inn pricing and paid rest flow
- content-driven recruit offers and recruit quantities
- weekly refresh cadence for recruit quantities and related service state
- content-driven service/economy schemas that stay friendly to future editor tooling
- clear UI communication for cost, quantity, refresh state, and home-base vs inn distinction

It should avoid:

- broad inventory/equipment systems
- a giant economy simulation
- broad narrative scripting systems
- free-roam overworld redesign
- large rendering rewrites
- building the designer-facing editor itself

## Architectural expectations

- Preserve the explicit `App` / `GameSession` flow.
- Preserve the controller / mapper / renderer split.
- Keep gameplay rules separate from rendering.
- Keep authored content separate from mutable runtime state.
- Favor typed, content-driven schemas over hardcoded service logic.
- Keep pure rules testable.
- Maintain responsiveness and explicit ownership.

## Gameplay priorities

### 1. Make home base feel special

Home base should clearly differ from ordinary destinations.

For this milestone, that should at minimum mean:
- resting there is free
- it is treated as the safest service anchor in the slice
- its presentation and service messaging reflect that role

### 2. Make inns part of the economy

Inns should not merely be “valid beds.”

Milestone 7 should move toward:
- paid rest at inns
- clear communication of rest cost before confirming
- keeping home-base rest free as the contrast case

### 3. Make recruit locations content-driven and limited

Recruit locations should evolve from generic placeholders into authored service content.

Milestone 7 should support:
- 1-3 recruitable unit offers per recruit location
- quantity per offer
- recruit action consuming quantity
- clear UI for current offers and remaining quantity

### 4. Introduce weekly cadence meaningfully

Weekly cadence should begin to matter in planning.

Milestone 7 should include:
- explicit weekly refresh of recruit quantities
- clear communication of current day/week state where needed
- save/load support for the new runtime quantities/refresh state

### 5. Keep schemas editor-friendly

Service/economy data added in this milestone should be structured so future designer tools can reason about it.

That means:
- stable ids
- explicit fields
- content describing offers/prices/types
- runtime state tracking remaining counts and refresh state separately

## Recommended implementation order

1. Add or refine the long-term vision docs and active milestone docs so M7 has a stable target
2. Introduce home-base free-rest vs inn paid-rest rules
3. Add content schema for recruit offers and quantities
4. Add runtime state and save/load for remaining recruit quantities
5. Add weekly refresh rules and tests
6. Improve service/HUD/location presentation so costs, offers, and refresh state are clear
7. Optionally add one more simple content-driven service/economy hook if the above is stable

## Acceptance checklist

- [ ] Milestone 6 architecture remains intact
- [ ] Home base is clearly identified as the free-rest safe hub
- [ ] Inns support paid rest with clear cost communication
- [ ] At least one recruit location exposes content-driven unit offers
- [ ] Recruit quantities decrease when used
- [ ] Recruit quantities refresh on a weekly cadence
- [ ] New service/economy runtime state persists through save/load
- [ ] New service/economy rules are content-driven and typed
- [ ] New logic has tests where feasible
- [ ] Presentation clearly explains cost, quantity, and refresh state

## Out of scope notes

Milestone 7 is not the milestone for:

- full inventory/equipment/equipment-slot systems
- a broad item economy
- a giant restoration meta-game
- branching story scripting frameworks
- full editor tooling implementation
- performance work disconnected from milestone needs

The milestone should feel like meaningful progress in game identity while still remaining explicit, data-driven, and maintainable.

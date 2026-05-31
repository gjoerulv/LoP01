# Validation System

This document defines the intended validation model for Ashvale content.

Validation should validate the schema conventions defined in `docs/content_schema.md`.

Use this document when implementing or modifying:
- content validation
- editor validation
- Scenario playability checks
- release-readiness checks
- CI validation
- authored-content schemas
- reference cleanup tooling

Related docs:
- `docs/scenario_authoring.md`
- `docs/presentation_game_feel.md`
- `docs/content_schema.md`
- `docs/game_vision.md`
- `docs/core_loop_rules.md`
- `docs/terminology_map.md`
- `README_DECISIONS.md`

---

## 1. Validation goals

Validation exists to keep authored content playable, debuggable, and safe to iterate on.

Validation should:
- catch broken references
- catch invalid schemas
- catch invalid system-specific settings
- catch obvious graph and progression problems
- catch likely softlocks where practical
- provide useful editor feedback
- run headlessly in tests and CI
- allow designers to save unfinished work

Validation does **not** exist to forbid harsh design.

A Scenario may be brutal, obscure, or unfair if it is structurally valid.

---

## 2. Validation levels

Use three validation levels.

### Save validation
Save validation:
- runs when content is saved in the editor
- never blocks saving
- reports errors, warnings, and info messages

Designers may save invalid work-in-progress content.

### Playable validation
Playable validation:
- runs before content is played
- runs on game startup for playable content
- blocks play if any errors exist
- allows warnings

Invalid content should not be playable.

### Release validation
Release validation:
- runs before content is packaged, published, or marked release-ready
- blocks on errors
- allows warnings only if they are acknowledged or suppressed with a reason

Release validation is stricter than playable validation but should still allow intentional design choices.

---

## 3. Severity levels

Validation messages use three severities.

### Error
An **Error** means content is structurally invalid and cannot be played.

Examples:
- missing required reference
- invalid enum value
- no valid arrival node in a playable Region
- invalid service configuration
- instant unavoidable win/loss at Scenario start
- player team has no legal leader

### Warning
A **Warning** means content is playable but suspicious, risky, harsh, or possibly unintended.

Examples:
- no safe anchor
- unused definition
- quest-critical object is very hidden
- victory-critical service is destroyable
- action chain spends resources without an obvious condition
- artifact-combination cycle exists

### Info
An **Info** message is a helpful authoring note.

Examples:
- default victory condition was expanded
- unused optional library content exists
- Region size label was derived from node count
- editor applied a safe default

---

## 4. Validation message format

Every validation message should include:

- severity
- stable validation code
- exact authored-object path
- human-readable message
- optional quick-fix suggestion
- optional related object references

Example:

```text
ERROR VAL-REF-001
Scenario: ashvale_intro
Region: west_fields
Node: old_gate
Service: quest_gate_01
Problem: referenced item `key_rusty` does not exist.
Suggestion: select a valid item id or remove the item requirement.
```

Editor UI should make the object path clickable where possible.

---

## 5. Reference cleanup on deletion

The editor may offer safe reference cleanup when deleting content.

Example:
- designer deletes a unit
- editor warns that the unit is used by events, quest services, teams, or conditions
- designer may cancel or confirm deletion
- editor may reset related references to safe defaults where possible

Automatic cleanup is an editor convenience, not a replacement for validation.

Rules:
- validation remains the authority
- cleanup must not silently make ambiguous content playable
- if a safe default is not possible, leave a validation error
- if a condition target is deleted, a branch may default to `true` only if that fallback is explicit and visible to the designer
- if an action target is deleted and no safe default exists, the action remains invalid until fixed

---

## 6. Id scope rules

Use scoped ids where appropriate.

Recommended rules:
- item ids are globally unique
- artifact ids are globally unique
- unit ids are globally unique
- recipe ids are globally unique
- Scenario ids are globally unique
- Region ids are globally unique or library-unique
- node ids are unique within a Region
- route ids are unique within a Region
- Location ids are unique within a Scenario or content library
- event ids are unique within their owning scope
- quest ids are unique within their quest service or Scenario scope
- service ids are unique within their owning Region or Location scope

Duplicate ids inside their required scope are errors.

---

## 7. Reference and schema validation

Validation should treat referenced presentation ids such as music, ambience, stinger, portrait, and visual effect ids as normal content references.

Missing references are errors.

Validate references to:
- Campaigns
- Scenarios
- Regions
- Locations
- nodes
- routes
- Services
- teams
- units
- heroes
- items
- artifacts
- recipes
- events
- quests
- story flags
- resources
- passive skills
- active skills
- AI personalities
- aggression levels
- terrain types

Unused definitions are warnings or info messages, not errors.

Unused content may be legitimate library content.

---

## 8. World Map validation

World Map adjacency is manually authored and bidirectional.

Errors:
- adjacency references missing Region
- adjacency is not bidirectional
- required Region has no valid path from the starting Region
- travel path to required Region depends on a locked or unenterable Region with no unlock path

Warnings:
- non-enterable Region exists forever
- optional Region is disconnected
- Region is visible in authoring data but never unlockable

### Implemented subset (M15)

`content/world_map.json` loading enforces the **structural** subset:

- duplicate World Map region entry (`WORLDMAP_ENTRY_DUPLICATE`)
- entry references an unknown / not-loaded Region (`WORLDMAP_REGION_UNKNOWN`)
- the entry's Region has a missing/empty `arrivalNodeId` (`WORLDMAP_ARRIVAL_NODE_MISSING`)
- the Region's `arrivalNodeId` does not exist among its nodes (`WORLDMAP_ARRIVAL_NODE_UNKNOWN`)
- an `exitNodeId` does not exist in the source Region (`WORLDMAP_EXIT_NODE_UNKNOWN`)
- an `adjacency` pair references a region not present in `entries[]` (`WORLDMAP_ADJACENCY_UNKNOWN_ENTRY`)

Adjacency is loaded as auto-symmetric, so "not bidirectional" cannot occur. The path-dependent checks (required Region unreachable, path through a permanently-locked Region) and the warnings above are **deferred**; M15 covers reachability only through the pure `WorldMapTravelRules` BFS used at travel time (and its tests).

---

## 9. Region validation

A playable Region must define:
- id
- display name
- description
- node list
- route list
- valid arrival node
- reveal defaults
- service and content references where used

Errors:
- no nodes
- no valid arrival node
- arrival node references missing node
- arrival node is invalid for arrival
- route references missing node
- route has invalid terrain enum
- route has invalid road flag
- enterable Region has no valid arrival
- required node is unreachable from the arrival node

Warnings:
- non-critical node is disconnected
- optional node is unreachable
- Region has no meaningful content
- Region has unusual size/content density

Region size label is derived from node count.

Suggested labels:
- Tiny
- Small
- Medium
- Large
- Huge
- Gigantic

The exact thresholds may be tuned later.

---

## 10. Node-content validation

Nodes are travel points with authored content.

Validation should enforce one main content item per node.

Main node content examples:
- resource pickup
- artifact pickup
- Service
- neutral enemy
- one-time special content

Errors:
- node has more than one main content item
- node content references missing object
- node content uses invalid resource/artifact/service/enemy data

Events may be attached to any node.

Valid node forms include:
- empty node
- node with content only
- node with event only
- node with content and attached event

### Event priority on nodes
If a node has both attached event and main content, event priority must be explicit.

Allowed values:
- `beforeContent`
- `afterContent`
- `replacesContent`

Default:
- `beforeContent`

Validation should warn if legacy data omits event priority.

---

## 11. Route validation

Routes are authored and stateful.

A route defines:
- source node
- destination node
- road flag
- terrain enum

Travel time and Energy are computed from route quality and distance.

Errors:
- route references missing node
- route duplicates another route illegally
- route has invalid terrain
- route has invalid road flag
- route is required for progression but cannot become active

Routes may be:
- active
- hidden
- destroyed
- restored

Events may destroy, restore, reveal, or activate authored routes.

Events do not create entirely new route definitions from nothing at runtime.

---

## 12. Service validation

Each Service type should have a type-specific validator.

This can be refactored later if multiple validators become identical.

Defaults:
- every service type has global default data
- omitted fields use legal defaults
- invalid defaults are errors
- overrides outside legal limits are errors

Errors include:
- storage capacity is invalid
- mine resource is not in the resource set
- Market stock references missing item
- Black Market stock references missing artifact
- Sealed / Frozen Hero service references invalid hero
- farming service has invalid seed/fertilizer/output configuration
- quest service has invalid quest chain
- recruitment service references missing unit
- artifact handler references invalid combination recipe
- Location call references service type not callable from Location

Warnings or errors depending on fallback:
- destroyable service is required for victory
- service needed for progression can be permanently destroyed
- service is destroyable but not restorable
- service is restorable but no restoration path exists

If a fallback exists, prefer warning.
If no fallback exists and progression can break, prefer error.

---

## 13. Event validation

Event trigger types are explicit enums.

Known trigger types:
- Region node-entry
- Location collision
- Location confirm
- start-of-day automatic
- neutral encounter defeated
- service used
- service restored
- service destroyed
- quest completion

Event conditions and actions are typed objects, not free-form script strings.

Example:

```json
{ "type": "teamHasResource", "resource": "Wood", "amount": 10 }
{ "type": "takeResource", "resource": "Wood", "amount": 10 }
```

Errors:
- unknown trigger type
- unknown condition type
- unknown action type
- invalid referenced id
- invalid enum value
- invalid time range, such as start day after end day
- impossible branch caused by contradictory eligibility and condition
- event cycle or same-day repeat loop

Warnings:
- take/spend action without obvious guarding condition
- give-hero or give-unit action without apparent capacity branch
- branch condition checks content that may never exist
- event has no effect
- event is repeatable and can fire very frequently

---

## 14. Event priority validation

Automatic events use authored priority.

Priority must be unique inside the same trigger group.

The designer tool may present automatic events as an ordered list.

Moving events up or down should assign or reassign unique priority values.

Saved data should not contain duplicate priority numbers in the same automatic trigger group.

Duplicate priority in the same trigger group is an error.

List order may still be used by the editor to manage priority assignment.

---

## 15. Event cycle validation

Validation should detect event cycles.

Errors:
- event A directly triggers event A
- event A triggers event B, and event B triggers event A
- repeatable same-day loop can run indefinitely
- automatic event chain can retrigger itself without a day or state boundary

Cycles that are intentionally bounded by state changes may be allowed if validation can prove the boundary.

If validation cannot prove the boundary, report an error or high warning depending on severity.

---

## 16. Runtime event-action validation

Event action chains are non-atomic ordered flows.

Validation should encourage safe authoring, but runtime must still guard actions.

Rules:
- take actions re-check availability at execution time
- take actions fail hard if unavailable
- take actions never clamp and never make resources negative
- give actions obey target capacity and overflow rules
- failed actions should show/log a clear reason when reasonable
- previous successful actions are not rolled back

Validation should warn when an action chain appears to rely on failure behavior.

Designers should use conditions and If / Else branches instead.

---

## 17. Quest validation

Quest services with zero quests are valid.

A zero-quest service may show the default empty / abandoned message.

Validation may warn that a quest service has no quests.

Quest objectives use the shared typed condition structure.

Quest completion actions validate like event actions.

Warnings:
- quest visible to player can be completed by other teams
- enemy can permanently fail a player-visible quest
- quest has no meaningful completion action
- quest service has no quests

High warning:
- victory-critical quest can be completed by another team and no alternate victory path exists

Error:
- quest objective references missing content
- quest chain references invalid quest
- quest required for victory can never be completed

---

## 18. Victory and defeat validation

If no victory condition is authored, validation/runtime expands the default victory condition:

- defeat all enemy teams

This expansion should produce an info message.

Errors:
- no authored victory condition and no enemy teams exist
- victory condition is true at Scenario start
- defeat condition is true at Scenario start
- victory condition references missing team/content
- defeat condition affects no valid teams
- no legal winning team exists
- all possible victory paths are structurally impossible

Victory and defeat conditions use the shared typed condition model.

Eligible and affected teams must be valid.

---

## 19. Team and AI validation

Player team errors:
- no player team
- more than one player team in single-player Scenario
- invalid start Region
- invalid start node
- active party is not battle-legal
- no legal leader
- player character missing where required

AI team errors:
- invalid personality
- invalid aggression
- invalid patrol radius
- missing patrol center where required
- start node outside patrol
- duplicate team color where illegal
- invalid leader setup
- invalid alliance reference

Team colors are limited to 8 teams per Region, including the player.

### Team templates
Team templates are designer helpers, not direct Scenario runtime requirements.

Enemy team templates always contain:
- exactly 1 hero
- 1-2 generic unit types
- valid stack ranges
- personality
- aggression

Templates do not include artifacts or resources by default.

Invalid templates are errors in the tool/template library, but not necessarily Scenario errors unless the Scenario uses them.

---

## 20. Item, artifact, and recipe validation

Item errors:
- invalid subtype
- invalid stack cap
- invalid base value
- invalid usable context
- effect action references invalid content

Food errors:
- food is not field-use only
- food does not target heroes correctly
- food has invalid effect or duration

Recipe errors:
- input ingredient missing
- output item missing
- output item is not food subtype
- required passive skill missing
- invalid time cost
- output quantity <= 0

Artifact errors:
- invalid allowed slot
- invalid effect
- invalid base value
- invalid combinable flag
- ultimate artifact marked combinable

Artifact combination recipe errors:
- not exactly 2 input artifacts
- not exactly 1 output artifact
- input artifact missing
- output artifact missing
- non-combinable artifact used as input

Artifact combination cycles are warnings, not errors.

Cycles may indicate poor design, but they are legal if intentionally authored and acknowledged.

---

## 21. Playability validation gate

Content is playable if:
- it has no validation errors
- player team is valid
- starting Scenario/Region/node is valid
- arrival nodes are valid
- required references are valid
- default or authored victory can resolve structurally
- required progression graph passes best-effort checks

Warnings do not block play.

Invalid content may be saved but must not be playable.

---

## 22. Release validation gate

Release-ready content must:
- have no errors
- have all warnings resolved, acknowledged, or suppressed with a reason

Suppressed warnings should store:
- validation code
- authored object path
- designer reason
- timestamp or revision metadata where practical

Suppressing a warning does not suppress future different warnings unless explicitly matched by code and path.

---

## 23. Softlock validation

Softlock validation is best-effort.

It should check:
- required quest item reachability
- victory target reachability
- required hero availability
- arrival node validity
- Region path validity
- route destruction/restoration dependencies
- required service availability
- required Region unlock path
- generic-loss risk if required generic units must survive Region travel

Softlock validation should not promise perfect proof of beatability.

Validation catches likely structural impossibilities.
Designers remain responsible for authored gameplay quality.

---

## 24. Validation output formats

Validation should eventually support:

- editor panel output
- clickable editor references
- JSON report
- console output
- CI-friendly exit codes
- test assertions

Validation should be pure/headless.

It should not require rendering.

---

## 25. Incremental validation

The editor should support incremental validation.

Recommended model:
- validate edited object immediately
- validate owning scope after edit
- validate full Scenario on save
- validate playable content before test play
- validate all packaged content in CI/release workflows

Incremental validation should not replace full validation.

---

## 26. Quick fixes

Validation messages may include quick-fix suggestions where practical.

Examples:
- add missing arrival node
- select a valid referenced item
- remove invalid service stock entry
- assign unique event priority
- convert missing reference to default condition
- add capacity branch around give-hero action

Quick fixes should be explicit and reviewable.

They should not silently change designer intent.


---

## 27. Player character validation

Player-character validation applies to human teams.

### Errors
Validation errors:
- standalone Scenario has no player character for the human team
- single-player human team has no player character
- `playerCharacterHeroId` references missing hero identity
- player character is not a hero unit
- player character is not leader-capable
- player character is not in the human team's traveling party
- player character is stored
- player character is dismissible
- player character can become Temporarily Unavailable
- player character appears in an AI team
- player character appears in an AI team template
- player character appears in a recruit service
- player character appears in a Sealed / Frozen Hero service
- player character appears in a neutral enemy encounter
- player character can be permanently removed by authored content
- player character is unavailable at Scenario start

### Warnings
Validation warnings:
- an event can move the player character out of the traveling party
- an event can affect the player character with a kill/remove action
- an escape/recovery path can fail because the respawn point may be blocked
- carry-over rules exclude player-character progression data in a way that may surprise the designer
- a defeat condition depends on player-character loss but recovery rules may prevent that loss in normal play

### Character creation validation
Character creation validation should ensure:
- selected name is valid
- selected sex value is valid
- selected appearance values are valid
- selected starting preset exists
- starting stats are legal
- starting skills are legal and not banned by the Scenario
- player character satisfies leader-capable hero requirements

### Campaign validation
Campaign validation should ensure:
- player-character identity carries over between Campaign Scenarios
- Scenario transitions do not remove the player-character identity
- progression carry-over rules are explicit when they reset level, skills, passives, equipment, or artifacts

---

## 28. Agent / implementation guidance

When implementing validation:

- keep validators pure where practical
- make validators deterministic
- return structured validation messages
- include authored-object paths
- prefer typed validators over string parsing
- keep reference validation separate from softlock analysis
- distinguish errors from warnings clearly
- avoid overpromising perfect softlock detection
- make validation useful in editor, startup, and CI
- do not block saving work-in-progress content
- do block play when errors exist

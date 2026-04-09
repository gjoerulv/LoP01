# README_DECISIONS

## Current design decisions

### 1) Keep architecture explicit and small

Decision:
- Use a small, explicit gameplay state machine (`GameSession`) with a thin app shell (`App`).
- Preserve the current controller/mapper/renderer split with shared HUD and debug overlay.

Why:
- Avoids overengineering while keeping the slice extendable.
- Keeps gameplay flow readable and testable.

### 2) Keep pure gameplay logic separate from raylib glue

Decision:
- Core loop logic (`GameClock`, `GameSession`, quest state, save data shaping) remains testable without raylib.
- Rendering/input integration stays in `src/app` and `src/rendering`.

Why:
- Supports deterministic tests and faster iteration.

### 3) Keep content data-driven with typed gameplay-facing models

Decision:
- JSON remains the source of truth.
- Typed definitions are used where needed (`locations`, `battle scenarios`, `quests`) for safer gameplay integration.

Why:
- Preserves content iteration speed while reducing runtime shape ambiguity.

### 4) Slice day model is explicit and rule-aligned

Decision:
- Slice day is modeled as a 20-hour cycle (`06:00 -> 02:00`) with rollover.

Why:
- Encodes core-loop time pressure directly in a simple, testable form.

### 5) Rest flow is interaction-driven in Milestone 5

Decision:
- Rest is triggered by location-scene interaction outcomes, not by location type alone.
- Current slice trigger is `inn_door`.

Why:
- Matches scene-level service intent (a town can contain multiple interaction types).

Tradeoff:
- This is an intentional slice simplification, not a final generalized service framework.

### 6) Wake-penalty recovery is unified

Decision:
- Missed sleep and full defeat share one wake-penalty recovery flow.
- Penalty application remains centralized in `GameSession::ApplyWakePenalty()`.
- Recovery placement uses an explicit slice fallback policy, not nearest-location search.

Why:
- Keeps consequences consistent and transition logic readable.

### 7) Battle return routing is explicit

Decision:
- Pre-battle outer mode is captured on battle entry.
- Allied victory returns to the surrounding loop (`LocationMode` or `OverworldMode`).
- Enemy victory routes through wake-penalty recovery.

Why:
- Prevents brittle mode chaining and keeps return behavior deterministic.

### 8) Milestone 5 quest progression model is minimal by design

Decision:
- Quests use a minimal typed model loaded from `quests.json`.
- Slice quests currently start immediately `InProgress`.
- Progression is completion-oriented for this slice.

Why:
- Delivers visible progression with low complexity in the bounded playable loop.

Tradeoff:
- No acceptance flow, branching, rewards, or scripting yet.

### 9) Save/load scope is minimal for Milestone 5

Decision:
- Save/load persists completed quest ids for milestone quest state.
- Existing session fields continue to restore current time/day/gold/mode/region/destination state.
- No speculative future-facing persistence was added.

Why:
- Persists only what is required for current playable-slice resume behavior.

### 10) Battle status text uses event stream as single source

Decision:
- Deprecated `LastActionText` / `lastActionText_` path was removed.
- Battle status text is derived from `BattleEvent` summaries.

Why:
- Avoids duplicate outcome state and keeps combat UI text consistent.

### 11) Milestone 6 prioritizes world identity over feature breadth

Decision:
- Milestone 6 focuses on making the existing world model behave more coherently before adding broader feature scope.
- Location-valid rest should be determined by the current location, not only by shared prototype scene layout.
- Overworld travel should move toward route-aware rules using the existing content structure.
- Persistent world state should remain minimal and slice-driven.

Why:
- The current architecture is already strong enough to extend without major restructuring.
- The biggest gap in the current slice is world-behavior coherence, not missing subsystems.
- This improves game feel and content credibility without destabilizing the codebase.

Tradeoff:
- Some content and presentation remain placeholder while rules are tightened.
- Milestone 6 intentionally favors correctness/coherence over broad new content.

### 12) Static content and runtime state should stay separate

Decision:
- Static authored data in `content/*.json` should describe the designed world, services, and balance inputs.
- Runtime state such as cleared nodes, quantities, stock, or weekly refresh state should live in gameplay/session/save layers, not by mutating authored content definitions.

Why:
- Keeps content schemas editor-friendly.
- Prevents accidental blending of design data with mutable runtime state.
- Makes future tooling and save/load behavior much easier to reason about.

### 13) Milestone 7 favored visible service/economy progress

Decision:
- Milestone 7 focused on home base identity, service economy, recruit availability, and weekly cadence.
- Service work in that milestone modeled cost, stock, quantity, and refresh rather than expanding “valid/invalid service gating” as a general pattern.

Why:
- The slice needed a more visible sense of game identity, not only coherence hardening.
- Inns, recruit posts, and travel-prep services mattered more as economic/service systems than as binary usable/unusable interactions.

Tradeoff:
- Milestone 7 broadened runtime state and service communication before recruits became full persistent party state.
- That was acceptable for M7 because the milestone’s goal was service/economy grounding first, not yet durable roster consequence.

### 14) Content schemas should remain friendly to future editor tooling

Decision:
- Prefer stable ids, explicit typed fields, and content schemas that can be created/updated/deleted by future designer-facing tools.
- Avoid burying content semantics in code-only conditionals when they can live in data.

Why:
- The long-term plan includes an editor for designers.
- Stable, typed content schemas reduce migration pain when tooling is introduced.

### 15) Responsiveness, performance, and ownership clarity are non-negotiable

Decision:
- Keep input handling out of rendering code and keep rendering out of gameplay rules/controllers.
- Favor RAII and clear ownership to avoid leaks.
- Avoid unnecessary per-frame allocations, repeated content parsing, and blocking work in the main loop.

Why:
- The game should feel responsive even while systems deepen.
- Clear ownership and separation reduce leak risk and make performance issues easier to diagnose.

### 16) Region travel rules remain the long-term scenario/world-map direction

Decision:
- Use the following as long-term design direction for future scenario/world-map work:
  - region travel happens through the scenario world map / region select layer
  - the player may travel between regions only once per day, before 11:00
  - region travel places the player character in the destination region at 11:00
  - only the player character travels between regions
  - each region preserves its own party state

Why:
- Keeps region switching strategically meaningful.
- Supports authored regional identity and region-specific progression.
- Allows scenarios to use multiple overworlds without flattening party state into one global pool.

Current-slice note:
- The current playable slice is still bounded to one region.
- These rules are vocabulary and design direction, not fully implemented baseline behavior yet.

Tradeoff:
- Cross-region party rules are more explicit and less intuitive than a single global party model.
- This should remain content-driven and clearly communicated in UI when implemented.

### 17) Milestone 7 travel prep and service readability remain explicit and UI-light

Decision:
- Travel prep remains one shared same-day travel effect granted by authored services (Home Base prep and Supply Cart prep).
- Active prep does not stack with another unused prep charge.
- Unused prep expires at day rollover.
- Service tooltips use a consistent multiline structure (explanation, cost/effect, reset timing).
- HUD remains compact and persistent-state focused, with week visibility and active prep icon visibility.

Why:
- Delivers visible service/economy clarity without introducing an inventory system, generic buff framework, or large UI rewrite.
- Keeps the implementation aligned with the explicit `App` / `GameSession` / controller / mapper / renderer architecture.

Tradeoff:
- The service layer communicates state clearly, but recruit outcomes still stop short of becoming durable roster/party gameplay state.
- That gap was intentionally deferred to Milestone 8.

### 18) Milestone 8 established persistent roster and active-party consequence

Decision:
- Recruitment now produces durable roster state rather than only economic/message outcomes.
- The game uses a canonical roster/slot model with owned counts, an active battle party, and reserve state.
- Home Base is currently the primary mustering anchor for moving units between active and reserve state.
- Save/load persists the canonical roster model and migrates older save shapes into the current structure.

Why:
- This turns recruitment into real strategy/RPG consequence.
- It gives Home Base a stronger long-term role as more than a rest/service node.
- It grounds future progression in persistent party state rather than transient recruit events.

Tradeoff:
- The current mustering flow is a bounded slice implementation, not the final out-of-battle party-management UX.
- Broader inventory/equipment/campaign roster design remains intentionally out of scope.

### 19) Active battle party target size is 5

Decision:
- The intended active battle party capacity is 5 slots.
- Both player and enemy teams can field up to 5 battle participants.
- The leader occupies one of those 5 slots rather than existing as an additional slot.
- Earlier 3-slot assumptions should be treated as implementation history, not as design truth.

Why:
- Aligns the roster model, combat rules, and battle format.
- Supports a clearer separation between active fielded units and carried substitutes.
- Matches the intended long-term combat and party-management direction.

Tradeoff:
- Any remaining 3-slot assumptions in runtime logic, tests, or save migration should be treated as migration work rather than re-opened design decisions.

### 20) Battle remains static formation CTB rather than grid combat

Decision:
- Battle is true turn-based CTB with no free battlefield movement.
- Units occupy abstract formation positions (`Front`, `Middle`, `Back`, and optional `Leader`) rather than tiles on a movement grid.
- Position changes are explicit battle commands that consume the acting unit’s turn.
- The detailed combat rules live in `docs/combat_rules.md`; this file only records the top-level design direction.

Why:
- Keeps combat readable, tactical, and content-friendly without introducing map-scale movement complexity.
- Fits the intended FF-style battle feel more closely than grid tactics.
- Keeps combat state explicit and easier to test.

Tradeoff:
- Positional tactics come from row choice, target selection, CTB timing, and skills rather than pathfinding or terrain.

### 21) Effective row depth is the combat-facing positional rule

Decision:
- Stored position labels and combat-facing row depth are not always the same thing.
- Agility penalty and range interaction use the target’s effective row depth, not only its literal stored label.
- Gaps are allowed in the formation: a team may have living units in `Front` and `Back` with no `Middle`, and the `Back` units remain effectively `Back` while `Front` still exists.
- When all living units in the nearest row are gone, the next surviving row becomes the new effective front for battle calculations.
- The attacker’s own row does not directly affect agility penalty.

Why:
- Preserves player-readable formation labels while making targeting penalties depend on which enemy line is actually closest.
- Matches the intended “how far does this unit need to reach?” battle feel.
- Avoids fake row normalization that would erase intentional gaps.

Tradeoff:
- The rule is more precise than a naive label-only formula, so UI and docs must communicate it clearly.

### 22) Leaders are hero-only and are manually managed

Decision:
- Only hero units can be assigned to the `Leader` position.
- The player team must always have a legal leader somewhere in party state.
- Enemy teams may have a leader but are not required to.
- If a leader is KO’d, aura effects end immediately and no replacement is assigned automatically.
- A living hero may manually take the `Leader` position later, and the aura reactivates immediately when that reassignment occurs.
- Outside battle, leader choice belongs to broader party management rather than to the current mustering table flow.

Why:
- Reinforces the game’s hero-led, story-driven party identity.
- Keeps aura behavior explicit and tactically legible.
- Separates durable party leadership from the narrower Home Base mustering slice UI.

Tradeoff:
- The final out-of-battle party-management UX is not yet fully designed or named.
- AI-controlled enemy teams must also follow the same manual leader-assignment rule when relevant.

### 23) Combat is mostly deterministic and readability-first

Decision:
- Damage roll is the main source of randomness in battle.
- Hit/miss, dodge, proc, and similar special outcomes should be deterministic when they exist.
- Battle UI should expose the information needed for tactical planning: visible turn-order preview, min/max damage, and min/max KO preview.
- The game should show resulting CTB order changes directly rather than surfacing hidden delay math.

Why:
- Supports tactical decision-making and player trust.
- Keeps combat closer to readable CTB planning than to swing-heavy RPG randomness.
- Lets position/range/skill choices feel deliberate rather than opaque.

Tradeoff:
- Some internal formulas may remain partially hidden as long as their effects are communicated clearly through previews.

### 24) Attrition is asymmetric between heroes and generic stacks

Decision:
- Hero units and generic stacks do not recover the same way after battle.
- Hero HP persists between battles unless restored by other means.
- Generic stacks restore current HP to max after battle, but lost stack count persists.
- MP persists for all units.
- Temporary in-battle buffs/debuffs are removed after battle; overworld-applied effects use explicit durations such as one battle, one day, or one week.
- Persistent enemy groups follow the same broad rules as the player’s party.

Why:
- Creates durable consequence without making every surviving generic stack track partial post-battle HP.
- Gives heroes stronger individual identity and ongoing risk.
- Supports persistent encounter state in the overworld.

Tradeoff:
- This asymmetry is less uniform than “everything fully heals” or “nothing heals,” but it better matches the intended strategy/RPG loop.

## Assumptions

- Region count remains one in the current slice.
- Placeholder content names/values are non-final.
- Current milestone prioritizes complete loop behavior over broader content expansion.

## Historical notes (short)

- Milestone 4 established mode-specific renderers (`Title`, `Overworld`, `Location`, `Battle`) plus shared HUD/debug overlay.
- Milestone 5 established the first complete world-loop baseline.
- Milestone 6 hardened route/world/node/quest coherence while preserving the explicit architecture.
- Milestone 7 grounded Home Base/service/economy identity and weekly cadence.
- Milestone 8 established persistent roster state, 5-slot active parties, leader legality, and durable battle-party consequence.

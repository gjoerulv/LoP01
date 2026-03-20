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

### 13) Milestone 7 should favor visible service/economy progress

Decision:
- Milestone 7 should be a larger-feeling update centered on home base identity, service economy, recruit availability, and weekly cadence.
- Future service work should model cost, stock, quantity, and refresh rather than expanding “valid/invalid service gating” as a general pattern.

Why:
- The next milestone should feel like meaningful game progression, not only coherence hardening.
- Inns, recruit posts, and shops matter more as economic/service systems than as binary usable/unusable interactions.

Tradeoff:
- Milestone 7 is broader than Milestone 6 and should therefore be planned clearly before implementation starts.

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

### 16) Region travel is world-map-driven and region-local party state is preserved

Decision:
- Region travel happens through the scenario world map / region select layer.
- The player may travel between regions only once per day, before 11:00.
- Region travel resets the player character into the destination region at 11:00.
- Only the player character travels between regions.
- Each region preserves its own party state.

Why:
- Keeps region switching strategically meaningful.
- Supports authored regional identity and region-specific progression.
- Allows scenarios to use multiple overworlds without flattening party state into one global pool.

Tradeoff:
- Cross-region party rules are more explicit and less intuitive than a single global party model.
- This should remain content-driven and clearly communicated in UI.


## Assumptions

- Region count remains one in the current slice.
- Placeholder content names/values are non-final.
- Current milestone prioritizes complete loop behavior over broader content expansion.

## Historical notes (short)

- Milestone 4 established mode-specific renderers (`Title`, `Overworld`, `Location`, `Battle`) plus shared HUD/debug overlay.
- Milestone 5 established the first complete world-loop baseline.
- Milestone 6 hardened route/world/node/quest coherence while preserving the explicit architecture.

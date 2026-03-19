# README_DECISIONS

## Current design decisions

### 1) Keep architecture explicit and small

Decision:
- Use a small, explicit gameplay state machine (GameSession) with a thin app shell (App).
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
- Progression is destination-triggered and completion-oriented for this slice.

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

## Assumptions

- Region count remains one in the current slice.
- Placeholder content names/values are non-final.
- Current milestone prioritizes complete loop behavior over broader content expansion.

## Historical notes (short)

- Milestone 4 established mode-specific renderers (`Title`, `Overworld`, `Location`, `Battle`) plus shared HUD/debug overlay.
- Early bootstrap phases used scaffold-first battle/location shells before world-loop consequences were fully wired.

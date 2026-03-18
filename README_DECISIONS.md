# README_DECISIONS

## Current design decisions

### 1) Simplest expandable architecture

Decision:
- Use a small, explicit state-machine scaffold (`GameSession`) with a thin app shell (`App`).

Why:
- Matches technical direction to avoid overengineering.
- Keeps extension straightforward when each mode gets real gameplay.

### 2) Keep pure logic separate from raylib

Decision:
- `GameClock` and `SaveGameRepository` are in `src/core` and testable without raylib.

Why:
- Supports deterministic tests and faster iteration.
- Keeps rendering/input glue confined to `src/app`.

### 3) Content loading remains data-driven while gameplay-facing models become typed

Decision:
- Keep content JSON-driven, but use typed gameplay-facing models where they improve clarity, validation, and controller/mapper integration.

Why:
- Preserves fast iteration from content files.
- Avoids leaving core gameplay systems coupled to raw JSON document shapes.

### 4) Simplified slice day model

Decision:
- Slice-day modeled as a 20-hour cycle (06:00 -> 02:00), with rollover to next day.

Why:
- Encodes the rule directly and keeps loop logic simple and testable for the playable slice.


### 6) Combat formula interpretation for CTB prototype

Decision:
- Implemented agility penalty steps as `(target position value - attacker range value)`.

Why:
- This is the simplest interpretation that keeps penalties aligned with the intent (units attacking beyond effective range are slowed).
- It preserves the documented 0 / 50% / 75% penalty tiers and remains deterministic and testable.


## Assumptions documented

- Region count currently uses one region in starter JSON (aligned with v0 scope doc).
- Placeholder values/content names are intentionally non-final.
- Save data stores minimal session data needed to restore core loop state.


### 10) Replace deprecated battle last-action string with event-only status text

Decision:
- Removed the old commented `LastActionText` / `lastActionText_` path and committed to `BattleEvent` summaries as the single battle-status source.

Why:
- Avoids duplicate state for the same outcome.
- Keeps combat UI text aligned with the event stream already used by the renderer and app shell.

Tradeoff:
- Event payloads need to stay expressive enough for UI text, so an explicit `infoText` field was added for non-targeted informational messages.

### 11) App transition cleanup uses explicit gameplay entry helpers

Decision:
- Extracted app-level helpers for mode initialization and battle/location transitions, and added explicit `GameSession` helpers for entering overworld and battle modes.

Why:
- Removes brittle double-`AdvanceMode()` jumps when entering battle from the overworld.
- Keeps transition intent readable and makes future milestone work easier to extend.

Tradeoff:
- `AdvanceMode()` still exists for the linear title/opening/front-end flow, so the codebase now intentionally uses both linear advancement and explicit mode entry where each is a better fit.


## Historical milestone notes

### 5) Battle and location are scaffolds only in this milestone

Decision:
- Provide mode shells and transitions now; full battle/location systems deferred.

Why:
- Requirement is to bootstrap vertical slice foundation, not complete full systems.
- Preserves small compilable increments.

### 7) Location scene collision/rendering prototype

Decision:
- Use placeholder tile rendering and simple rectangular collision/interaction zones for the first playable town scene.

Why:
- Keeps the location module playable with minimal complexity.
- Matches vertical-slice scope and allows easy expansion to richer map data later.

### 8) Milestone 4 visual-pass renderer extraction

Decision:
- Switched from a single app-level drawing layout to mode-specific renderers using render models (`Title`, `Overworld`, `Location`, `Battle`) plus shared `HUD` and toggleable `DebugOverlay`.

Why:
- Keeps gameplay state as source-of-truth while making rendering modular.
- Avoids a giant `App::Draw` implementation and aligns with the visual-pass architecture goal.

Tradeoff:
- Rendering remains deliberately straightforward, but gameplay state, mappers, and render models are now separated explicitly. Further decomposition should only happen when it reduces real complexity.

### 9) Battle-screen readability pass

Decision:
- Kept existing battle logic and inputs, and improved only presentation with placeholder rectangles, clear HP/MP bars, active/target highlights, turn-order badges, and a simple action panel.

Why:
- Meets milestone visual goals without introducing new mechanics.
- Keeps CTB rules stable while making combat state readable at a glance.

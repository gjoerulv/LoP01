# README_DECISIONS

## Vertical slice bootstrap decisions

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

### 3) JSON content repository starts document-based

Decision:
- `ContentRepository` currently loads JSON documents as-is rather than full typed model classes.

Why:
- Fastest path to data-driven bootstrap.
- Leaves room to incrementally add strongly typed definitions per system.

### 4) Time model simplification for v0 scaffold

Decision:
- Slice-day modeled as a 20-hour cycle (06:00 -> 02:00), with rollover to next day.

Why:
- Encodes the rule directly and keeps logic simple for early tests.

### 5) Battle and location are scaffolds only in this milestone

Decision:
- Provide mode shells and transitions now; full battle/location systems deferred.

Why:
- Requirement is to bootstrap vertical slice foundation, not complete full systems.
- Preserves small compilable increments.

### 6) Combat formula interpretation for CTB prototype

Decision:
- Implemented agility penalty steps as `(target position value - attacker range value)`.

Why:
- This is the simplest interpretation that keeps penalties aligned with the intent (units attacking beyond effective range are slowed).
- It preserves the documented 0 / 50% / 75% penalty tiers and remains deterministic and testable.

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
- The app currently performs explicit mapping from gameplay/session state to render models in one place; this is intentionally simple for the first visual pass and can be split into dedicated mapper files later if needed.

### 9) Battle-screen readability pass

Decision:
- Kept existing battle logic and inputs, and improved only presentation with placeholder rectangles, clear HP/MP bars, active/target highlights, turn-order badges, and a simple action panel.

Why:
- Meets milestone visual goals without introducing new mechanics.
- Keeps CTB rules stable while making combat state readable at a glance.

## Assumptions documented

- Region count currently uses one region in starter JSON (aligned with v0 scope doc).
- Placeholder values/content names are intentionally non-final.
- Save data stores minimal session data needed to restore core loop state.

## Milestone 4 acceptance checklist

- [ ] Content folder loads successfully from build output
- [ ] Debug text is hidden by default
- [ ] F1 toggles debug overlay
- [ ] Title screen has dedicated presentation
- [ ] Overworld has visible map nodes
- [ ] Player can select travel visually
- [ ] Travel preview is shown before confirm
- [ ] Location mode shows interact prompts
- [ ] Location mode has a dialogue panel
- [ ] Battle mode shows allies and enemies visually
- [ ] Battle mode shows active unit clearly
- [ ] Battle mode shows turn order as UI
- [ ] Battle mode shows action menu visually
- [ ] Main gameplay no longer feels primarily text-based

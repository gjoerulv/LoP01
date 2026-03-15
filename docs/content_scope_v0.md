# Content Scope – v0

This document defines exactly what content is **in scope** for the v0 milestone.
Anything not listed here is out of scope and should be deferred.

## In scope

### Locations (6)

| ID | Name | Connected to |
|----|------|--------------|
| `town_square` | Town Square | market (N), blacksmith (E), south_gate (S) |
| `market` | Market District | town_square (S), tavern (E) |
| `blacksmith` | Blacksmith's Forge | town_square (W) |
| `tavern` | The Rusty Flagon | market (W), darkwood_forest (N) |
| `south_gate` | South Gate | town_square (N) |
| `darkwood_forest` | Darkwood Forest | tavern (S) |

### Scenes

- Title screen
- Location scene (name + description + exits)
- Game-over screen

### Commands

- `start`
- `look` / `l`
- `north` / `n`, `south` / `s`, `east` / `e`, `west` / `w`
- `go <direction>`
- `quit` / `q`

### Systems

- Location system (`Location`, `LocationManager` via `Game`)
- Scene rendering (`Scene`)
- Player tracking (`Player`)
- Main game loop (`Game`)

## Out of scope for v0

- Combat / battle system (v1)
- Items and inventory (v2)
- NPCs and dialogue (v3)
- Saving / loading game state
- Colour / ANSI escape codes
- Audio
- Any graphical UI

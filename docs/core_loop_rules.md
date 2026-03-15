# Core Loop Rules

## Overview

The core gameplay loop describes everything the player can do on their turn and how the game responds.

## Turn structure

Each player input constitutes one **turn**. A turn consists of:

1. **Player issues a command** (typed at the `>` prompt).
2. **Game processes the command** and updates state.
3. **Game renders the new scene** and displays the result.

## Command categories

### Navigation

The player moves between locations using directional commands.

| Input | Canonical direction |
|-------|---------------------|
| `north` / `n` | north |
| `south` / `s` | south |
| `east` / `e` | east |
| `west` / `w` | west |
| `go <direction>` | as above |

If the current location has no exit in the requested direction the game responds:
> *You can't go that way.*

### Inspection

| Command | Effect |
|---------|--------|
| `look` / `l` | Redisplay the current scene in full |

### Meta

| Command | Effect |
|---------|--------|
| `quit` / `q` | End the session and show the game-over screen |

## Scene transitions

| Trigger | New scene |
|---------|-----------|
| Game launch | Title screen |
| `start` command | Location scene (starting location) |
| Successful move | Location scene (new location) |
| Player HP reaches 0 | Game-over screen |
| `quit` command | Game-over screen |

## Unknown commands

If the game does not recognise a command it responds:
> *You can't go that way.*

(For v0 this keeps the response set minimal.)

## Entry point

The starting location is always `town_square`.

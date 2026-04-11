---
applyTo: "src/rendering/**/*.cpp,src/rendering/**/*.h"
---

# Rendering instructions

- Keep rendering code presentation-focused.
- Rendering should not own gameplay rules, legality checks, or progression decisions.
- Favor readable, layered visuals over decorative complexity.

## Terminology

Use current design terminology in rendering code comments and UI-facing presentation:

- **World Map** = scenario-level region selection layer
- **Region** = main in-scenario travel space
- **Location** = entered place inside a Region
- **Service** = interaction available in a Region or Location

If runtime types, content keys, or serialized values still use legacy `overworld` terminology, do not treat that wording as current design truth. Preserve compatibility where needed, but prefer current terminology in new rendering work.

## Layer responsibilities

### World Map rendering
World Map rendering should focus on:
- region outlines and labels
- current Region highlight
- unlocked / locked / non-enterable Region states
- selected destination Region
- shortest-path day preview
- 1000 Energy travel requirement
- travel legality / disabled travel messaging
- warnings when region travel would abandon traveling generic units

### Region rendering
Region rendering should focus on:
- node graph readability
- route readability
- current node and selected node
- shortest-path preview to the selected node
- route terrain / road readability when relevant
- Energy cost preview
- blocked / occupied / cleared state readability
- arrival-node readability
- direct Service node readability
- Location node readability
- storage-gate readability

Do not render the Region layer as if there is a dedicated permanent combat-node type. A node may contain a hostile encounter and later become an empty travel node.

### Location rendering
Location rendering should focus on:
- entered-place identity
- walkable scene readability
- NPC interaction prompts
- service interaction prompts
- location result / status messaging
- safe-anchor readability when relevant

Remember that entering and exiting a Location does not cost time.

### Battle rendering
Battle rendering should focus on:
- active unit clarity
- target clarity
- turn-order readability
- row / leader readability
- min/max damage preview
- min/max KO / kill preview
- status / buff / debuff readability
- legality and selection feedback without exposing unnecessary internal math

## State distinctions that must render clearly when shown

When the relevant screen exposes roster or world-state information, clearly distinguish:
- active party
- reserve
- stored units
- temporarily unavailable heroes
- enemy-team occupation
- protected arrival node
- storage gate ownership / threat state when relevant

## Visual guidance

- Prefer strong silhouettes and readable contrast over detailed clutter.
- Keep node and route presentation easy to parse at a glance.
- Use color, icons, labels, and small state markers to communicate legality and access conditions.
- Avoid baking rule assumptions into visuals that are not guaranteed by design.
- When a node changes state after clearing, render the new state explicitly rather than implying the old state persists.

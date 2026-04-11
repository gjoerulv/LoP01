---
applyTo: "src/rendering/**/*.cpp,src/rendering/**/*.h,src/app/mappers/**/*.cpp,src/app/mappers/**/*.h"
---

# UI instructions

- Favor clarity and readability over flashy effects.
- Use simple panels, lists, prompts, labels, and status strips.
- The UI should evoke retro strategy/RPG readability.
- Show time, day, gold, current layer, and current place prominently when relevant.
- Use current design terminology in UI and UI-facing comments:
  - **World Map** = scenario-level region selection layer
  - **Region** = main in-scenario travel space
  - **Location** = entered place inside a Region
  - **Service** = interaction available in a Region or Location
- If older runtime or serialized names still use legacy `overworld` terminology, do not reintroduce that wording into new UI copy.

- World Map UI should clearly show:
  - current Region
  - selectable destination Regions
  - whether a Region is unlocked and enterable
  - shortest-path travel day preview
  - 1000 Energy travel requirement
  - whether travel is currently legal
  - warning text when travel would abandon traveling generic units

- Region UI should clearly show:
  - selected node
  - shortest-path travel preview
  - Energy cost preview
  - route quality / terrain impact when relevant
  - whether travel is allowed
  - whether a node is empty, blocked, occupied, contains a Location, or contains a direct Service
  - whether a node is a protected arrival node
  - whether a node appears cleared, blocked, occupied, or unavailable

- Region UI should also clearly distinguish:
  - direct Service node
  - Location node
  - blocker node
  - enemy-team occupation
  - storage gate
- Do not imply a dedicated permanent combat-node type in the UI. A node may contain a hostile encounter and later become an empty travel node.

- Location UI should clearly show:
  - location name
  - current interaction prompt
  - available services or actions
  - lightweight result/status text for interactions
  - service cost or remaining quantity when relevant
  - whether the Location functions as a safe anchor
- Remember that entering and exiting a Location does not cost time.

- Party-management UI should clearly distinguish:
  - active party
  - reserve
  - stored units
  - temporarily unavailable heroes
- When relevant, clearly communicate legality rules:
  - active party must remain battle-legal
  - leader replacement constraints
  - storage-only actions must be performed at storage services

- Battle UI must clearly show:
  - turn order
  - active unit
  - HP / MP / Life
  - action menu
  - target selection
  - min/max damage preview
  - min/max KO / kill preview
- Battle targeting UI should communicate agility penalty clearly, preferably without exposing unnecessary math.

- Keep placeholder UI art minimal and functional.
- Prefer communicating current slice rules clearly over adding decorative complexity.
- Do not assume that service identity is best communicated through binary enabled/disabled text; cost, stock, refresh state, legality, and access conditions are usually more valuable.

# UI Instructions

Use these instructions when changing UI, HUD, menus, map presentation, service screens, battle presentation, or player-facing information.

Always follow the current terminology in `docs/terminology_map.md`.

Primary design references:
- `docs/presentation_game_feel.md`
- `docs/game_vision.md`
- `docs/game_shell_flow.md`
- `docs/core_loop_rules.md`
- `docs/combat_rules.md`
- `README_DECISIONS.md`

## Core UI posture

Ashvale UI should be:

- clean by default
- detailed on hover, select, inspect, or context action
- controller-friendly across the whole game
- mouse/keyboard-friendly
- touch-aware where practical

Mouse/keyboard and controller are the primary targets, but do not design UI that depends on mouse-only hover.

Any hover-only information must have a controller/touch equivalent through selection, tooltip lock, context action, or info panel.

## Information-density rule

Use a hybrid style:

- JRPG-clean for primary presentation
- HoMM-like density for strategic inspection and planning

Show exact values for known information.

Show estimates when fog, scouting, or hidden information applies.

Do not expose internal formulas unless the user or design doc explicitly asks for that.

## Common UI terms

Use these terms consistently:

- **Adventure button strip**
  - left-side icon menu in Region / World Map UI
- **Scenario Info screen**
  - formal victory / defeat condition screen
- **Quest Log**
  - discovered quest-service tasks
- **Guidance**
  - event-driven directional text
- **Contextual info panel**
  - bottom-left Region / World Map info panel
- **Context action**
  - controller/touch/keyboard equivalent of right-click or secondary action

Do not call the Adventure button strip the Scenario Info screen.

## Region UI

Region mode should keep a stable strategic frame:

- top info bar:
  - day / week / month
  - time
  - Energy
- bottom resource bar:
  - team resources
- top-left minimap
- left-side Adventure button strip
- bottom-left contextual info panel
- upper-left news popup area offset to the right of the Adventure button strip

Node movement uses select-then-confirm:

- first click/tap/select previews
- second click/tap/confirm moves if legal

Route preview should show the shortest legal path when available.

Use:
- red values for Energy/time blockers
- red outlines for blockers
- gray/disabled path display beyond blockers where useful

## World Map UI

World Map should reuse the broad Region HUD frame.

Only unlocked Regions are visible.

Locked Regions remain hidden in black fog.

Region travel confirmation should warn only when the travel will lose units.

Do not show detailed destination-node information from the World Map.

## Location UI

Location mode should feel like JRPG walking with sprites.

Do not add a default minimap or automatic service list unless explicitly requested.

Location interactions are event-driven.

An interactable may be:
- NPC
- object
- counter
- stone
- shrine
- field plot
- any other authored sprite/event

The designer controls whether interaction triggers through confirm, collision, or another event rule.

## Battle UI

Battle layout:

- player party on the right
- enemies on the left
- CTB bar at the top
- help window below CTB
- formation in the center
- party status rows at bottom right
- command menu left of party status rows

Do not show row labels constantly as text.

Use sprite placement and distance from center to communicate position.

When targeting:
- show target preview in a tooltip with panel background
- show damage and KO / kill preview
- show Agility penalty by color-coded cursor/tooltip feedback
- update CTB bar live before confirmation
- do not explain internal CTB math unless explicitly requested

Manual battle and auto-resolve use the same result screen.

## Party menu

Party menu layout should include:

- large active-party panel
- reserve slots along bottom
- right-side menu:
  - Items
  - Artifacts
  - Cooking
  - Position

Support both drag/drop mouse interaction and controller cursor interaction.

Generic stack shortcuts should be preserved where implemented:

- `Ctrl + Left Click`: split one unit to next available legal slot
- `Alt + Left Click`: merge all same-type units into clicked stack where legal
- `Shift + Left Click`: split same-type units evenly across available legal slots in the same active container
- `Ctrl + Shift + Left Click`: fill legal empty slots with 1-unit stacks while units remain

Controller equivalents should use selection, destination confirmation, and context menus.

## Items, artifacts, cooking

Item inventory is a flat list:
- icon
- name
- quantity

Do not force item categories into the main inventory UI unless explicitly requested.

Artifacts are managed through a hero-specific artifact/equip interface.

Cooking UI should show:
- available recipes by default
- Show All Recipes toggle
- ingredient icons and held/required counts
- missing counts marked red inline
- required passive skills
- time cost
- resulting food effects
- quantity selector

Artifact combination UI:
- similar to cooking
- shows available authored combinations
- shows input artifacts and output preview
- performs legal combinations immediately
- does not require an extra irreversible-warning prompt
- blocks use of artifacts required by victory conditions

## Services

Do not force every Service into one universal screen.

Service UI may be custom.

However, show relevant information before confirmation:
- title
- cost
- time cost
- Energy cost
- resource/item cost
- unavailable reason
- guard portrait
- threat level

Confirm high-risk actions:
- Region travel that loses generic units
- surrender
- escape
- discarding artifacts
- destroying services

Do not add an extra irreversible warning for artifact combination.

## Quest, guidance, logs

Quest Log:
- shows discovered quest-service tasks
- primary action centers on quest-service location
- context/info action shows requirements

Guidance:
- appears under top info bar and to the right of the Adventure button strip
- may include icon + text
- is collapsible
- also appears in journal / history
- may point to hidden or unrevealed content if authored

News popups:
- temporary
- upper-left content area
- latest four visible when expanded
- full log/history available on click or equivalent action

## Fog and scouting UI

Fog is black with a smooth animated edge.

No silhouettes for unrevealed nodes.

Nodes appear only when revealed.

Scouting controls inspection depth:

- default: color, leader, threat color, unit icon/name, quantity ranges, hero level ranges
- Basic: improved reveal / better confidence
- Advanced: enemy resources and better estimates
- Expert: exact stacks, hero levels, leader stats, items, artifacts

Threat color should appear on:
- target cursor
- hover / inspection panel outline

## Do not regress terminology

Avoid older wording like:
- `overworld` as current design truth
- `combat node` as a current node type
- references to deleted or archived vision docs

Use:
- World Map
- Region
- Location
- Service
- Adventure button strip
- Scenario Info screen
- Quest Log
- Guidance

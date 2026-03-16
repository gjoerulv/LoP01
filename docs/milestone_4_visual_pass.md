# Milestone 4 - First Visual Gameplay Pass

## Purpose

The project already has the early vertical-slice scaffold:
- state flow
- time/day tracking
- content loading
- save/load
- location mode
- battle mode

This milestone is about turning that scaffold into a visibly playable prototype.

The goal is not to add major new mechanics.
The goal is to make the current mechanics look and feel like a game.

## Main outcome

After this milestone, the game should no longer feel like a debug/text prototype.

The player should see:
- a dedicated title screen
- a dedicated overworld screen
- a dedicated location screen
- a dedicated battle screen
- a compact shared HUD
- a toggleable debug overlay

The normal gameplay view should not be dominated by diagnostics text.

## Scope

This milestone should preserve existing gameplay rules where possible.

It should focus on:
- presentation
- screen layout
- readability
- contextual controls
- renderer structure

It should avoid:
- major new battle mechanics
- large content expansion
- multiple new regions
- deep quest expansion
- adding engine-like abstractions

## Rendering architecture

The rendering layer should consume gameplay state, not own it.

Use the following approach:

- gameplay/session state stays in existing gameplay/app systems
- the app layer maps gameplay state into lightweight render models
- renderer classes consume those render models
- renderers do not perform gameplay mutations

Preferred rendering modules:
- RenderContext
- UiTheme
- HudRenderer
- DebugOverlay
- TitleRenderer
- OverworldRenderer
- LocationRenderer
- BattleRenderer

## Shared presentation rules

- placeholder visuals are acceptable
- layout clarity matters more than decorative detail
- interaction and state readability matter more than effects
- use consistent HUD placement
- show selection, active state, and interactables clearly
- hide debug information behind F1 by default

## Overworld expectations

The overworld screen should:
- show a visible map area
- show destination nodes from content data
- show the player current location
- show the selected destination
- show travel preview before confirmation
- distinguish node types visually
- use contextual controls instead of debug text as the main interaction model

This is the first screen that should evoke the HoMM-inspired layer.

## Location expectations

The location screen should:
- resemble a prototype town/interior scene
- show the player avatar clearly
- show buildings, service zones, doors, and NPCs clearly
- show interact prompts when near something usable
- show a dialogue panel when talking
- show local HUD info such as day, time, gold, and location name

This is the first screen that should evoke the FF-inspired layer.

## Battle expectations

The battle screen should:
- show allies and enemies in distinct areas
- show the active unit clearly
- show HP, MP, and Life clearly
- show turn order as a proper UI element
- show an action menu visually
- show target selection visually

This is the first screen that should evoke the CTB battle identity.

## Debug expectations

Debug information is still useful during development, but:
- it must be hidden by default
- it must be toggled with F1
- it should be compact and readable
- it should not replace the real UI

## Hook-up plan

The current application/game session should eventually map live state into these render models:

- HudModel
- DebugOverlayModel
- TitleScreenModel
- OverworldRenderModel
- LocationRenderModel
- BattleRenderModel

The existing gameplay code should remain the source of truth.

## Recommended implementation order

1. Add rendering folder and stub files
2. Add debug overlay toggle
3. Add shared HUD renderer
4. Extract title rendering
5. Extract overworld rendering
6. Extract location rendering
7. Extract battle rendering
8. Remove the always-on debug-text main layout

## Acceptance checklist

- [ ] Content loads correctly from build output
- [ ] Normal gameplay is not dominated by debug text
- [ ] F1 toggles debug overlay
- [ ] Title screen has its own presentation
- [ ] Overworld has visible destination nodes
- [ ] Overworld shows player position and travel preview
- [ ] Location mode shows interact prompts
- [ ] Location mode has a dialogue panel
- [ ] Battle mode shows allies and enemies visually
- [ ] Battle mode shows turn order as a UI strip
- [ ] Battle mode shows action menu visually
- [ ] The game feels more graphical than text-based
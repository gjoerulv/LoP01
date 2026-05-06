# Game Shell Flow

This document defines the player-facing shell flow outside active gameplay.

Use this document when implementing or modifying:
- boot flow
- title screen
- main menu
- game mode selection
- Campaign selection
- Standalone Scenario selection
- Tutorial entry
- character creation entry flow
- save/load flow
- settings
- mods menu
- validation visibility in shell UI
- dev/debug shell options

Related docs:
- `docs/game_vision.md`
- `docs/core_loop_rules.md`
- `docs/content_schema.md`
- `docs/validation_system.md`
- `docs/terminology_map.md`
- `README_DECISIONS.md`

This document should not duplicate in-game Region, Location, battle, or party-menu UI rules. Those live in the core UI/gameplay docs.

---

## 1. Shell UI goals

The shell UI should be:

- clear
- controller-friendly
- mouse/keyboard-friendly
- touch-friendly where practical
- readable on PC and future smart-device targets
- safe for save/load and mod/content selection
- simple enough for AI agents to implement without inventing extra game modes

The shell UI is not a separate game system. It is the entry and configuration layer for the authored content and runtime systems.

---

## 2. Boot flow

The game should boot into a title flow, then main menu.

Recommended sequence:

1. optional splash/logo sequence
2. title screen
3. main menu

Logos and splash screens are allowed, but must be skippable after a short minimum display time.

Dev builds may skip splash screens entirely.

---

## 3. First boot defaults

There is no first-boot setup wizard.

The game should choose sensible defaults automatically.

### Language
The game should try to detect language from OS/local settings.

Fallback order:
1. detected supported language
2. English
3. configured default language
4. first available language if neither English nor configured default exists

Language can be changed later in Settings.

### Input method
The game should infer the initial input method:

- controller if a controller is connected and active
- keyboard + mouse otherwise
- touch on smart-device platforms

The player can still use any supported input method.

### Display
Default display mode:

- borderless fullscreen window
- best local resolution based on system display settings

Display options can be changed later in Settings.

### Accessibility
Accessibility settings are available in Settings.

The game should not force an accessibility setup flow on first boot.

---

## 4. Title screen

The title screen should wait for input.

Examples:
- PC/controller: “Press any button”
- touch: “Tap to start”

After input, transition to the main menu.

---

## 5. Main menu

Main menu options:

```text
Continue
New Game
Load Game
Settings
Mods
Credits
Quit
```

`Quit` may be hidden or replaced by platform conventions where appropriate.

`Mods` may be hidden or disabled on platforms where file-based mods are not supported.

---

## 6. Continue

Continue loads the most recent valid save.

If the most recent save depends on missing or incompatible content:
- show a clear warning
- route the player to Load Game
- do not attempt unsafe loading silently

---

## 7. New Game

New Game opens Game Mode Selection.

Game Mode Selection options:

```text
Campaign
Standalone Scenario
Tutorial
PvP
```

PvP should be hidden or disabled in normal player builds until it is implemented enough to use.

Dev builds may expose PvP earlier for testing.

---

## 8. Tutorial

Tutorial is authored content, not a separate mechanical mode.

The shell may expose Tutorial as a convenience shortcut.

There should be:
- one designated tutorial Scenario
- one designated tutorial Campaign

The tutorial entries should use the same validation, character creation, save/load, and content rules as other Scenarios/Campaigns unless explicitly documented otherwise.

---

## 9. Game mode selection

### Campaign
Campaign mode opens Campaign Selection.

### Standalone Scenario
Standalone Scenario mode opens Standalone Scenario Selection.

Only Scenarios with `standaloneSelectable: true` should appear in normal standalone Scenario selection.

### Tutorial
Tutorial opens the designated tutorial Scenario/Campaign selection or starts the configured tutorial flow directly if only one tutorial entry is available.

### PvP
PvP remains hidden/disabled until implemented.

When implemented, PvP should follow the same shell principles:
- explicit mode selection
- validation before play
- clear player/team setup
- save/load rules if PvP saves are supported

---

## 10. Campaign selection

Campaign selection should show:

- Campaign name
- description
- supported languages
- difficulty estimate
- length estimate
- author/source/mod
- completion status
- validation/playability status in dev/editor mode

Campaigns are available if installed and valid.

Campaign locking may be supported later through content flags, but is not a default shell requirement.

Starting a Campaign normally leads to character creation before Scenario 1.

A Campaign may define character-creation defaults or restrictions.

Allowed Campaign character-creation restrictions:
- starting skills

A Campaign may define flavor/defaults for:
- default player-character hero id
- allowed text/language content
- starting presentation

Campaigns should not introduce permanent class restrictions unless the design changes.

---

## 11. Standalone Scenario selection

Standalone Scenario selection should show:

- Scenario name
- description
- author/source/mod
- difficulty estimate
- size
- supported mode/player count
- team count
- estimated length
- supported languages
- validation/playability status in dev/editor mode

Standalone Scenarios normally require character creation.

A Scenario may provide a prebuilt player-character setup and skip full character creation if explicitly authored.

If character creation is skipped, show a confirmation screen before starting.

Allowed Scenario character-creation restrictions:
- starting skills

Do not show Scenarios with `standaloneSelectable: false` in normal Standalone Scenario selection.

---

## 12. Invalid content visibility

Normal player builds:
- hide invalid Campaigns/Scenarios, or show them disabled with a simple reason
- do not expose raw validation reports by default

Dev/editor builds:
- show invalid Campaigns/Scenarios disabled
- expose validation report access
- may include a “Show invalid content” option

Invalid content must not be playable.

---

## 13. Character creation

Character creation happens before starting a Campaign or Standalone Scenario unless the content explicitly provides a prebuilt player-character setup.

Character creation fields:

- name
- male/female icon choice
- appearance
- starting preset/template
- starting stats
- starting skills

The UI should use male/female icons without a visible “sex” text label.

### Name
Name input rules:

- free text
- required
- maximum 16 characters
- alphanumeric characters only
- trim leading/trailing whitespace

Provide a random name button.

### Randomizers
Character creation should include:

- randomize name
- randomize stats/skills
- randomize graphical representation
- optional randomize all

### Starting presets
Initial starting presets:

- Warrior
- Builder
- Explorer

These are starting presets only.

They are not permanent classes.

### Scenario/Campaign restrictions
Campaigns and Scenarios may restrict starting skills.

Other creation categories should remain broadly available unless a future design explicitly narrows them.

### Preview
Character creation should show:

- derived stats
- starting role explanation
- selected skills
- appearance preview

### Navigation
The player may go back from character creation to Campaign/Scenario selection before starting.

### Saved character templates
Players may save character-creation templates within this scope.

A saved character template should store reusable creation choices such as:
- name if desired
- appearance
- starting preset
- selected starting stats
- selected starting skills

Saved templates are convenience data, not Campaign carry-over data.

---

## 14. Load Game

Load Game should group saves by mode.

Recommended flow:

```text
Load Game
  Campaign
    Campaign name
      Save file
  Standalone Scenario
    Scenario name
      Save file
  PvP
    PvP save file, if supported
```

Within each group, show most recent saves first.

Save entries should show:
- save name / slot label
- Campaign or Scenario name
- current day/time where applicable
- playtime
- timestamp
- mod/content warning status if applicable

---

## 15. Save slots

Support:

- manual saves
- autosaves
- quicksave where platform supports it

Saves are grouped by Campaign / Scenario / mode.

Autosaves use designated slots.

Required autosave slots:
- after battle
- start of day
- entering a Region

Keep a small rotating autosave history where practical.

---

## 16. Saving rules

There should be no authored event action or Scenario rule that globally prevents saving.

Saving is allowed from the Settings menu whenever that menu can be opened.

The game should not restrict saving by:
- Location
- Region
- danger level
- time of day
- Scenario state
- safe zone

If the game is in a transient non-menu state, saving becomes available again as soon as the menu is available.

Examples of transient non-menu states:
- battle animation
- event action execution
- loading transition
- result calculation
- modal confirmation that must finish first

Do not support saving halfway through an event action chain or unresolved calculation frame.

---

## 17. Save metadata

Save files should record:

- game version
- schema version
- active content packages
- mod list
- mod load order
- mod versions
- content ids used by the save
- Campaign/Scenario id
- current Region/Location where applicable
- current day/time where applicable

If required mods/content are missing:
- warn clearly
- block loading if required content cannot be resolved
- allow “try anyway” only in dev/editor mode

If content versions differ:
- warn clearly
- allow load if validation/migration passes
- block load if incompatible

---

## 18. Settings categories

Settings should be grouped into:

- Gameplay
- Video
- Audio
- Controls
- Language
- Accessibility
- Mods
- Debug, dev-only

Settings should be reachable from the main menu and from in-game Settings where appropriate.

---

## 19. Gameplay settings

Gameplay settings should include:

- auto-resolve enabled by default
- human usable-skill use in auto-resolve on/off
- human item use in auto-resolve on/off
- AI animation speed
- human movement speed
- tutorial hints
- confirmation prompts
- text speed
- battle result pause/confirmation behavior

Defaults:
- auto-resolve enabled
- human usable skills in auto-resolve disabled
- human item use in auto-resolve disabled

Leader, passive, aura, and artifact effects are not disabled by those usable-skill/item toggles unless explicitly stated elsewhere.

---

## 20. Video settings

Video settings should include:

- resolution
- fullscreen/windowed/borderless
- UI scale
- vsync
- pixel-perfect / integer scaling if using pixel art
- brightness/gamma if needed

Default:
- borderless fullscreen window
- best local resolution

---

## 21. Audio settings

Audio settings should include:

- master volume
- music volume
- SFX volume
- UI volume
- mute when unfocused

---

## 22. Controls settings

Controls settings should include:

- keyboard remapping
- controller remapping
- touch layout/options where supported
- reset to defaults
- input test screen

The whole game should remain controller-friendly.

Touch support should be considered where practical, especially for future smart-device targets.

---

## 23. Language settings

Language settings should include:

- UI language
- content language if separate
- fallback-language information if content is missing translation

Apply language changes immediately where practical.

If reload or restart is required, tell the player clearly.

---

## 24. Accessibility settings

Accessibility settings should include:

- text speed
- font size / UI scale
- reduce motion
- disable screen shake
- colorblind/threat-color alternatives
- tooltip delay
- hold-to-confirm / confirm delay
- high contrast UI option
- combat animation speed

Accessibility options should be available from Settings, not forced through a first-boot wizard.

---

## 25. Mods menu

The Mods menu should detect installed mods and allow basic management where the platform supports it.

Mods menu should show:

- installed mods
- enabled/disabled state
- load order
- metadata
- author
- mod version
- target game version
- supported languages
- validation status
- conflict summary if known

Changing enabled mods or load order requires restart/reload.

Disabled mods remain installed but ignored.

Normal player builds:
- show simple conflict/error summaries

Dev/editor builds:
- show detailed conflict and override reports
- expose validation reports

---

## 26. Validation visibility from shell

Normal players should not see raw validation reports by default.

They should see simple content availability messages.

Dev/editor builds may show:

- full validation reports
- invalid Campaigns/Scenarios
- mod conflict details
- override reports
- content health details

Validation report access may be exposed through:
- Mods menu
- dev/debug menu
- editor UI

---

## 27. Dev/debug shell

Dev/debug menu options may include:

- open Scenario directly
- skip character creation
- validation report
- content browser
- test battle
- test Location
- test Region
- spawn team
- clear saves
- reload content
- run validator

Dev/debug options should be excluded from release builds or hidden behind a dev config flag.

---

## 28. Agent / implementation guidance

For AI agents and implementation work:

- do not add a first-boot wizard unless explicitly requested later
- use auto-detected defaults for language, input, and display
- keep shell flow separate from in-game Region/Location/battle UI
- do not expose PvP in normal player builds until implemented
- treat Tutorial as authored Scenario/Campaign content
- enforce `standaloneSelectable` for Standalone Scenario selection
- do not add save-prevention event actions
- save only when menus are available and state is not mid-resolution
- record mod/content metadata in saves
- keep normal-player validation messages simple
- expose detailed validation reports only in dev/editor contexts
- keep Settings categories stable and explicit

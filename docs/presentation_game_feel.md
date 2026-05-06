# Presentation and Game Feel

This document defines Ashvale's moment-to-moment presentation, game feel, audiovisual tone, and feedback style.

Use this document when implementing or modifying:
- menu transitions
- UI feedback
- mode transitions
- battle intro/outro presentation
- music and ambience behavior
- sound effects
- animation pacing
- fog reveal presentation
- service feedback
- notification presentation
- presentation-related event actions

Related docs:
- `docs/game_vision.md`
- `docs/game_shell_flow.md`
- `docs/core_loop_rules.md`
- `docs/combat_rules.md`
- `docs/content_schema.md`
- `docs/validation_system.md`
- `docs/terminology_map.md`
- `README_DECISIONS.md`

This document should guide how the game feels without duplicating mechanical rules.

---

## 1. Presentation goals

Ashvale should feel like a **warm but dangerous fantasy adventure**.

Core tone keywords:
- storybook adventure
- SNES-era JRPG warmth
- HoMM-style strategic wonder
- dangerous wilderness
- melancholic mystery
- clear tactical feedback

The game should feel readable, tactile, lightly atmospheric, and never sterile.

The default presentation should be snappy, expressive, and respectful of player time.

---

## 2. Overall tone

Default tone:
- mostly sincere
- lightly charming
- occasional dry humor
- not parody
- not grimdark
- not childish
- heroic when earned
- cozy in safe places
- tense in hostile Regions

World Map and Region play should emphasize strategic wonder and danger.

Location mode should emphasize warmth, character, and JRPG exploration.

Battle should feel tactical, energetic, and readable.

Menus should feel clean, tactile, and slightly ornate fantasy, not debug-like.

---

## 3. Visual style

Default visual assumption:
- stylized 2D fantasy pixel-art direction
- readable over hyper-detailed
- colorful but not garish
- atmospheric
- tile/sprite friendly
- scalable to PC and future smart-device targets

The style should be retro-inspired, not retro-limited.

Use old-school charm with modern readability, accessibility, and responsiveness.

---

## 4. Animation style

Animations should be expressive but economical.

Use:
- short movement bobs
- icon pulses
- small sprite gestures
- simple battle attack motions
- clean transition animations
- subtle state changes

Avoid:
- long unskippable animations
- excessive bounce
- constant particles
- noisy reward spam
- slow repeated transitions

Common actions should feel fast.

Important moments may linger briefly.

---

## 5. Ambient motion

Environments should support subtle ambient motion where authored.

Examples:
- grass/wind shimmer
- water movement
- torch flicker
- drifting fog
- dust, snow, or rain
- light sparkle on ready/active services

Ambient motion must respect reduce-motion settings.

---

## 6. Menu feel

Menus should open quickly with a soft slide/fade and subtle sound.

Default timing:
- open: 100-180 ms
- close: 80-140 ms
- nested panel transition: quick slide/fade

Responsiveness is more important than flourish.

Menus should not use long blocking animations.

### Menu pause behavior
In single-player Region and Location modes, menus pause active input flow.

Battle command menus wait for command selection.

Settings should pause where practical.

Since single-player time advances through player actions, pause primarily means no background action is processed while a menu is open.

---

## 7. Selection and invalid-action feedback

Selections should feel tactile.

Use:
- clear highlight
- small cursor motion
- soft UI click
- selected panel brightening or lifting
- disabled options visibly dimmed

Invalid actions should use:
- soft error sound
- small shake or pulse
- reason text where useful

Avoid harsh error spam.

---

## 8. Tooltips and explanation panels

Tooltips must use a panel background.

Do not present tooltip text as naked floating text.

Tooltip behavior:
- mouse hover: short delay
- controller focus: immediate
- touch focus: tap/hold or explicit info button
- avoid covering selected target where possible
- all critical tooltip information must be accessible without hover

Complex rules may use pinned or expanded explanation panels.

---

## 9. Region mode presentation

Region travel should feel like strategic movement over an adventure map.

On confirmed travel:
- route glows briefly
- team marker moves smoothly along route
- time and Energy changes update visibly
- arrival has a small settle animation
- arrival events transition cleanly

Movement should be fast by default and adjustable in settings.

### Illegal movement
Illegal movement should be informative, not punishing.

Use:
- red blocked route segment
- tooltip reason
- soft error sound
- cursor or route pulse

Avoid modal popups unless the reason is complex or critical.

### Fog reveal
Fog reveal should feel rewarding.

When moving:
- fog softly peels back or fades
- newly revealed nodes briefly glint
- major discoveries may show a popup/log entry

Reduce-motion settings should simplify or shorten reveal effects.

### Visible enemy movement
Visible enemy movement should be animated according to AI speed setting.

Default:
- quick but readable
- visible/relevant enemy actions may show popups
- unseen AI actions should not interrupt the player

Instant AI mode resolves without animation but still updates logs.

---

## 10. Region service presentation

Services should have authored identity and state feedback.

Examples:
- sanctuary: calm glow / bell-like ambience
- destroyed service: darkened with broken icon
- restoring service: construction/repair marker
- farming growing: light sparkle
- farming watered: brighter sparkle
- harvest ready: glow
- guarded service: guard portrait / threat marker

Services should feel like places or interactions, not just data forms.

Even simple services should have:
- title
- icon or sprite identity
- short feedback
- cost/effect clarity

---

## 11. World Map presentation

World Map travel should feel more significant than Region-node travel.

On confirmed World Map travel:
- show clear confirmation panel
- highlight route briefly
- show travel-day transition
- update day/Energy clearly
- play arrival stinger or ambience change

World Map should have broader, calmer, journey-like music or ambience distinct from Region music.

World Map travel should not feel like a long cutscene every time.

---

## 12. Location mode presentation

Location mode should feel like SNES-era JRPG exploration.

Use:
- direct character control
- sprite-based interactions
- warm interior/exterior ambience
- dialogue boxes with portraits when authored
- small character gestures where useful

Interactables are designer-authored.

Important interactables should be readable through sprite design.

Optional hidden content may be subtle.

Do not add a universal sparkle to every interactable unless authored.

### Location interactions
Interaction can trigger by:
- confirm button
- collision/touch
- authored trigger

Presentation:
1. short facing/attention pause
2. dialogue box or service screen opens
3. soft interaction sound
4. result feedback
5. return to Location

### Location service flow
Location services should feel event-driven.

Example:
1. NPC dialogue or object prompt
2. player confirms
3. service screen opens
4. result feedback appears
5. return to Location

Location services use the same mechanics as Region services where applicable, but may have more flavor around them.

---

## 13. Battle start presentation

Battle start should have a clear, short intro sequence.

Default sequence:
1. encounter trigger / enemy contact
2. quick screen flash or wipe
3. battle intro sting
4. battle music starts or crossfades
5. teams appear in formation
6. CTB/UI appears
7. first actor prompt begins

Default duration:
- normal battle: 1-2 seconds
- boss/major battle: may be longer if authored

Battle intros should be speed-scaled or skippable where practical.

---

## 14. Battle music categories

Battle music should support categories:
- normal battle
- dangerous battle
- boss/major encounter
- victory
- defeat
- escape/surrender/result

Threat level may influence intro sting or music choice.

---

## 15. Battle action presentation

Basic attacks should be readable and snappy.

Use:
- attacker emphasis
- target hit flash
- sound effect
- damage numbers
- status update
- clean CTB update

Skills should have stronger presentation than basic attacks:
- unique icon or sound
- small effect animation
- clear targeting feedback
- visible resource/MP change

Major skills may be longer, but should remain skippable or speed-scaled.

Avoid long animations for common actions.

---

## 16. KO and result presentation

KO should be noticeable but not melodramatic.

Use:
- changed pose/drop/fade
- KO status icon
- short sound
- CTB update

Player-character KO should not imply permanent loss.

Player-character recovery is shown through result/recovery flow.

### Result screen
Battle result screen should be decisive and readable.

Show:
- banner: Victory / Defeat / Enemy Escaped / You Escaped / Surrendered
- short music sting
- unit losses
- spoils/resources/items
- Accept / Try Again where applicable

Avoid excessive reward animations.

---

## 17. Music direction

Music should be melodic fantasy with SNES/JRPG and strategy-adventure influence.

Style:
- memorable themes
- warm instrumentation
- regional identity
- light melancholy
- tension when threatened
- not generic orchestral wallpaper

Possible palette:
- strings
- woodwinds
- harp
- bells
- light percussion
- chiptune/synth accents where desired
- folk instruments per Region

Use light adaptive music, not overly complex layering.

Examples:
- World Map journey theme
- Region calm theme
- Region danger layer when enemy nearby
- battle intro sting
- Location ambience/music per area

---

## 18. Ambience direction

Major modes should support ambience.

Examples:
- World Map: wind, distant birds, broad air
- Region: biome-specific ambience
- Location: interior/exterior loops
- Battle: subtle environment ambience under music where appropriate
- Service: short local ambience or stinger where authored

Ambience should support place identity without becoming noisy.

---

## 19. UI sound direction

UI sounds are important.

They should be:
- soft
- distinct
- consistent by action type
- not annoying
- not overly loud

Common UI sounds:
- confirm
- cancel
- invalid
- open menu
- close menu
- page/tab
- item acquired
- resource changed
- quest update
- warning

Avoid repetitive or harsh sounds.

---

## 20. Feedback layers

Important events should use layered feedback.

Use multiple channels where appropriate:
- local animation/effect
- short popup/news item
- log entry
- icon/state change
- sound cue

Do not rely on only one channel.

### Resource changes
Resource changes should show:
- icon
- delta amount
- subtle sound
- resource bar update
- optional log entry for major changes

Small repeated changes should aggregate where practical.

### Quest and guidance updates
Quest/guidance updates should use:
- short notification
- journal/log update
- gentle stinger
- icon pulse on Quest Log / Guidance where useful

Major story updates may show dialogue/message panels.

---

## 21. Pacing

Default pacing should be snappy but not frantic.

Principles:
- common actions are fast
- important decisions are readable
- repeated animations are short
- movement/AI/battle speed is adjustable
- confirmation prompts are reserved for risky actions

Confirm risky or irreversible actions:
- Region travel that loses generics
- surrender
- escape
- discard artifact
- destroy service
- overwrite save
- delete save
- changing mod load order
- starting invalid/dev content

Do not confirm common actions such as artifact combination unless the design changes.

---

## 22. Speed and comfort settings

Players should be able to adjust or reduce repeated presentation.

Relevant settings:
- AI movement speed
- human movement speed
- battle animation speed
- text speed
- reduce motion
- instant AI movement
- skip repeated battle intro option, if later added

Reduce motion should:
- reduce menu animation
- reduce screen shake
- simplify fog reveal animation
- reduce particle intensity
- shorten or fade battle transition effects

---

## 23. Mode tone summary

### World Map
Wide, adventurous, strategic, slightly lonely.

### Region
Exploratory, dangerous, resource-conscious, place-specific.

### Location
Warm, intimate, characterful, JRPG-like.

### Battle
Tense, readable, tactical, energetic.

### Menus
Clean, tactile, slightly ornate fantasy, not sterile debug UI.

### Services
Place-like or interaction-like, not just forms.

---

## 24. Platform feel

Use the same core presentation across PC, controller, and touch.

Input-specific behavior:
- mouse hover uses delayed tooltip
- controller focus shows immediate tooltip
- touch uses tap/hold or explicit info button
- all critical information must be accessible without hover

Future smart-device support should consider:
- scalable UI
- readable font sizes
- touch-friendly hit targets
- no hover-only critical interactions
- UI scale settings
- animation speed settings

Do not compromise PC/controller quality.

---

## 25. Accessibility rules

Never rely only on color.

Threat colors, route legality, status, and resource warnings should also use:
- icons
- labels
- patterns
- text

Never rely only on audio.

Audio cues should have visual equivalents.

Presentation actions must respect accessibility settings.

---

## 26. Content authoring hooks

Content authors should be able to assign presentation ids where relevant.

Authorable presentation fields may include:
- World Map music
- Region music
- Region ambience
- Location music
- Location ambience
- battle music override
- boss music override
- service stingers
- event stingers
- portrait ids
- visual effect ids

This document does not define final asset schemas. `docs/content_schema.md` should own those when implemented.

---

## 27. Presentation event actions

Events may trigger limited typed presentation actions.

Examples:
- play stinger
- change music
- fade screen
- show message
- shake screen
- flash screen
- show portrait
- play ambient cue

Presentation actions should not hide gameplay mechanics.

They are feedback/presentation side effects unless explicitly paired with gameplay actions.

Presentation actions must respect accessibility settings.

---

## 28. Agent / implementation guidance

For AI agents and implementation work:
- avoid sterile/debug-like player-facing UI
- keep common interactions fast
- keep important decisions readable
- use moderate presentation juice, not excessive effects
- keep music/ambience hooks typed and validateable
- do not make presentation actions into free-form scripts
- respect reduce-motion and accessibility rules
- do not rely only on color or audio
- keep shell flow in `docs/game_shell_flow.md`
- keep mechanical rules in core/combat/schema/validation docs
- use this document for moment-to-moment feel, tone, and feedback

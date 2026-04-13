# Combat Rules

This document is the battle-system source of truth for Ashvale's current vision.

It defines the intended battle model, baseline formulas, persistence rules, and battle-facing UI expectations. Balance values may still be tuned later, but the core structure in this file should be treated as the design baseline.

For broader systemic context, see:
- `docs/core_loop_rules.md`
- `docs/game_vision_complete.md`
- `docs/terminology_map.md`

## Battle format

- Battles are true turn-based CTB battles.
- The same battle module is used by both **Region-layer** and **Location** encounters.
- Each side can field up to 5 units.
- There are no rounds.
- Battle should remain readable, tactical, and mostly deterministic.
- The only baseline RNG is damage roll variance. Other exceptions, if added later, should be explicit and deterministic where possible.

## Core battlefield model

- Combat is static. There is no free battlefield movement and no movement grid.
- Units are placed in abstract formation positions:
  - `Front`
  - `Middle`
  - `Back`
  - `Leader`
- `Leader` is one of the 5 battle slots, not an additional slot.
- Any number of non-leader units may share the same labeled row.
- The player may place any unit in `Front`, `Middle`, or `Back`.
- Only hero units may occupy the `Leader` position.
- Position changes are explicit battle commands and consume the acting unit's turn.
- A unit's stored position label remains the same unless changed by a position command.
- Battle logic may still treat the unit as being at a different **effective row depth** for range and agility-penalty purposes.

## Unit categories

### Hero units

- Hero units are unique named characters.
- Hero units do not stack.
- Hero units have personality and story relevance.
- Only hero units are leader-capable in current intended design.
- If reduced to 0 HP, a hero is KO'd.
- A KO'd hero can be revived during battle.
- If a non-player hero is still KO'd when battle ends, that hero leaves the party and becomes **Temporarily Unavailable** through the broader world rules.
- If the player character is still KO'd when battle ends but the allied team wins, the player character returns to the party at 1 HP.
- Heroes that leave the party are not considered permanently dead. They may later return through the game's shared hero-availability systems.

### Generic units

- Generic units are stackable.
- Generic stacks use `Life` as stack count.
- When a generic stack reaches 0 Life, it is defeated and removed unless revived before battle ends.
- Lost stack count persists after battle.
- Generic stacks do not automatically refill to their pre-battle size.
- Recruiting can increase stack size later through normal progression and services.

## Leadership and party legality

- The player team's active battle party must always contain a legal leader-capable unit before battle starts.
- Enemy teams may have zero or one assigned leader.
- If a team has an assigned leader, that unit is a normal battle participant from turn 1.
- The assigned leader occupies the `Leader` position.
- The player character is the default assigned leader whenever present in the active battle party, unless the player has explicitly chosen another leader through out-of-battle party management.
- If the player changes which hero is leader outside battle, that assignment persists into battle.
- During battle, a different living hero may manually take the `Leader` position with a position command.
- Changing to `Leader` swaps positions with the current leader.
- If the current leader is KO'd, there is no automatic replacement.
- A different living hero may manually take `Leader` later in the same battle.
- When a new leader is assigned in battle, the aura reactivates immediately on that swap action.
- Enemy AI follows the same leadership rules. If an enemy team wants a new leader after the current leader is KO'd, the AI must spend an action to do so.

## Leader aura

The assigned leader grants a baseline aura while alive.

- The leader's `Attack`, `Defense`, `Magic`, and `Resistance` are directly added to the stats of all allies on that team, including the leader.
- The aura is active while the assigned leader is alive and in the `Leader` position.
- If the assigned leader reaches 0 HP, the aura is removed immediately.
- If another hero manually becomes the leader during battle, the aura reactivates immediately using the new leader's stats.
- This aura rule is a real baseline rule, not a placeholder.
- Passive skills and equipment may later modify or extend how aura works, but they build on this baseline rather than replacing it.

## Stats

Each unit has:

- `Attack`
- `Defense`
- `Magic`
- `Resistance`
- `MinDamage`
- `MaxDamage`
- `HP`
- `MP`
- `Agility`
- `Life`
- `Position`
- `Range`

### Interpretation

- `HP` = current health of the current active body.
- `MP` = magic resource.
- `Agility` = how quickly the unit acts in CTB.
- `Life`:
  - for generic units, this is stack count
  - for heroes, this is effectively always 1
- `Magic` affects magic damage and the strength or duration of certain effects.
- `Resistance` reduces magic damage and is also the default counter-stat against magic-driven effects.
- `MinDamage` and `MaxDamage` define physical damage roll range.
- `Attack` and `Defense` affect physical damage.
- `Range` can be:
  - `Melee`
  - `Ranged`
  - `LongRanged`

## Positions, effective row depth, and targeting

### Stored position labels

Each living non-leader unit has a stored labeled row:

- `Front`
- `Middle`
- `Back`

The assigned leader, if any, occupies:

- `Leader`

Stored labels matter for party setup and manual position changes.

### Effective row depth

Range penalties do **not** use the unit's literal stored label directly. They use the unit's **effective row depth**.

The battle system derives effective row depth like this:

1. Ignore KO'd or removed units.
2. Look at the living non-leader units on a team.
3. Sort occupied labeled rows in natural order: `Front`, then `Middle`, then `Back`.
4. Reinterpret the nearest occupied labeled row as effective `Front`.
5. Reinterpret the next occupied labeled row behind it as effective `Middle`.
6. Reinterpret the next occupied labeled row behind it as effective `Back`.
7. If a team has an assigned leader and any living non-leader ally remains, the leader is treated as effective `Leader`.
8. If the leader is the only living unit left on that team, the leader is treated as effective `Front`.

Examples:

- If all living non-leader units on a team are labeled `Back`, they are still treated as effective `Front` because they are the nearest occupied row.
- If a team has living units in labeled `Front` and labeled `Back`, but none in labeled `Middle`, that gap remains real for battle calculations. The labeled `Front` units are treated as effective `Front`, and the labeled `Back` units are treated as effective `Back`. If the labeled `Front` row is later emptied, the labeled `Back` row becomes the new effective `Front`.
- If the nearest occupied row is removed or revived, effective row depth is recalculated immediately.
- If a unit changes position, effective row depth is recalculated immediately.

### Targeting

- Any unit may target any enemy unit.
- Position does not gate target legality.
- There is no front-row body-blocking rule.
- Position mainly matters through agility penalties based on target effective row depth and attacker range.
- The attacker's own row does not directly affect this penalty.

## Range and agility penalty

### Range values

Use these range values:

- `Melee` = 0
- `Ranged` = 1
- `LongRanged` = 3

### Effective depth values

Use these effective depth values for the target:

- effective `Front` = 0
- effective `Middle` = 1
- effective `Back` = 2
- effective `Leader` = 5

### Penalty rule

Let:

- `rangeValue` = the attacker's range value
- `targetDepth` = the defender's effective row depth
- `gap` = `targetDepth - rangeValue`

Then:

- if `gap <= 0`, there is no agility penalty
- if `gap == 1`, the acting unit suffers a 50% agility penalty for that action
- if `gap >= 2`, the acting unit suffers a 75% agility penalty for that action
- final agility can never go below 1

Equivalent shorthand:

- one step beyond comfortable range = 50% penalty
- two or more steps beyond comfortable range = 75% penalty

### Agility-penalty behavior

- This penalty applies when the unit performs an action against a target beyond its comfortable range.
- The penalty affects the acting unit's CTB timing for that action.
- The penalty does not permanently change the unit's base Agility stat.
- Position by itself does not alter turn order. The penalty only occurs when an action targets a sufficiently deep effective row.

## Baseline damage formulas

These formulas are the current baseline and should be treated as real design rules unless later balance testing proves that they need to change.

### Physical damage

1. Roll base damage:

`baseDamage = random(MinDamage, MaxDamage) * Life`

2. Compute stat delta:

`statDelta = attacker.Attack - defender.Defense`

3. Convert delta into multiplier:

- if `statDelta > 0`, increase damage by 10% per point
- if `statDelta < 0`, decrease damage by 5% per point
- cap the total adjustment at `+200%` maximum and `-90%` minimum

4. Apply multiplier:

`damageMultiplier = clamp(1.0 + positiveBonus - negativePenalty, 0.10, 3.00)`

5. Final damage:

`finalDamage = max(1, floor(baseDamage * damageMultiplier))`

### Magic damage

1. Compute base magic:

`baseMagic = attacker.Magic * skillPower`

2. Compute stat delta:

`statDelta = attacker.Magic - defender.Resistance`

3. Convert delta into multiplier using the same baseline curve and caps as physical damage:
- +10% per positive point
- -5% per negative point
- clamp to `0.10` minimum and `3.00` maximum

4. Final magic damage:

`finalMagicDamage = max(1, floor(baseMagic * damageMultiplier))`

### Elements and specific resistances

- Baseline magic is reduced by `Resistance`.
- Specific enemies may also have elemental strengths, weaknesses, or immunities.
- These are explicit content-driven modifiers layered on top of the baseline formula.

## CTB and action economy

### CTB baseline

- CTB is turn-based with no rounds.
- `Agility` determines how quickly a unit receives turns.
- The player should always see one clear turn-order bar.
- The game does not need to expose hidden internal delay math, but it should show the resulting turn-order changes immediately when selecting actions.

### Basic action structure

The intended command structure is FF-style, not the old `Skill 1 / Skill 2` placeholder model.

The long-term baseline action model is:

- `Attack`
- `Defend`
- `Skill`
- `Item`

With additional contextual direct-skill shortcuts for units that have very few skills.

#### Hero command baseline
Heroes use the full command structure:
- `Attack`
- `Defend`
- `Skill`
- `Item`

Only hero units may use items in battle.

#### Generic-unit command baseline
Generic units may have:
- no skills
- one skill
- two skills
- more than two skills

If a generic unit has:
- 0 skills: show only the relevant baseline commands
- 1 skill: surface that skill directly on the main menu
- 2 skills: surface both directly on the main menu
- more than 2 skills: use a `Skill` submenu

### Secondary menu

There is also a secondary hidden battle menu for:

- `Wait`
- row-change commands
- `Leader` swap command where legal

This is a contextual battle menu, not a separate strategic system.

## Wait and Defend

### Wait

`Wait` consumes the unit's turn and applies:

- `2x` the unit's base Agility
- lasting until that unit's next turn
- removed immediately when that next turn begins

This may be chained repeatedly.

### Defend

`Defend` consumes the unit's turn and applies:

- `+5 Defense`
- physical-only mitigation
- lasting until that unit's next turn
- removed immediately when that next turn begins

This is a provisional baseline pending later balance passes.

## Status duration and timing

- There are no rounds.
- Status duration is tracked by **the affected unit's own turns**.
- A duration such as “lasts X turns” means:
  - it counts down on that unit's own future turns
  - not on global actions
  - not on the caster's turns unless a specific later effect says otherwise

This applies to:
- buffs
- debuffs
- haste / slow
- defend-style temporary effects
- deterministic dodge windows
- other duration-based combat effects

## Randomness and reliability

### Baseline philosophy

Combat should be:

- mostly deterministic
- legible
- strategically calculable
- closer to chess-like CTB planning than to swingy proc-heavy chaos

### Baseline randomness

The only normal baseline randomness is:

- damage roll variance between `MinDamage` and `MaxDamage`

### Not baseline-random by default

The following are **not** random by default:

- hit chance
- dodge chance
- crit chance
- status proc chance
- revive chance
- resistance proc chance

If these effects exist, they should generally be implemented as deterministic passives, triggers, or explicit skill logic unless the design later chooses a rare exception.

## Persistence after battle

### Hero persistence

Heroes:
- keep current HP between battles
- keep current MP between battles
- lose in-battle temporary buffs/debuffs when battle ends
- may leave the roster if still KO'd at battle end according to broader world rules

### Generic persistence

Generic units:
- keep lost `Life` count between battles
- restore current body HP to max after battle
- keep MP between battles
- lose in-battle temporary buffs/debuffs when battle ends

### Overworld / world-applied buffs
World-applied buffs from services, items, or events may last for:
- 1 battle
- 1 day
- 1 week

Those are broader world-system durations, not ordinary in-battle status durations.

## Revival

### Baseline revival philosophy

Revival is:
- valid
- relatively rare
- costly
- not supposed to be the default answer to battle mistakes

### Hero revival

- A KO'd hero may be revived during battle.
- If revived during battle, the hero remains in the fight normally.
- If still KO'd when battle ends, broader world rules apply.

### Generic revival

- Lost generic `Life` may be revived during battle.
- A revive effect may restore:
  - a fully wiped stack
  - a partially damaged stack
- The amount restored depends on the power of the revive effect.

Stack count not restored before battle ends remains lost.

## Battle UI expectations

Battle UI should clearly show:

- active unit
- target selection
- turn order
- HP / MP / Life
- action menu
- target legality
- row / leader readability
- min/max damage preview
- min/max KO / kill preview

### Min/max preview rule

When selecting a target:

- show min/max damage
- show min/max KO / kill result
- if min and max are identical, show a single value instead of a range

This applies to both physical and magic actions.

### Turn-order readability

- Show the resulting turn-order change immediately when the player highlights an action or target.
- Do not require exposing internal CTB formulas.
- Showing the actual reordered turn timeline is enough.

### Penalty readability

Agility-penalty feedback should be clearly visible during target selection, preferably through cursor or target-state cues rather than heavy formula text.

## Out-of-battle battle setup implications

Although this file is battle-specific, battle assumes these broader truths:

- active-party leadership exists before battle begins
- leader assignment can also be changed outside battle
- active positions persist while units remain in the active party
- if an active unit is removed to reserve and later returned, it normally gets recalculated to a best position unless directly replacing another unit
- battle outcome writes back into persistent roster state

Those broader rules are defined in `docs/core_loop_rules.md` and `docs/game_vision_complete.md`.

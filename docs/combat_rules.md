# Combat Rules

This document is the battle-system source of truth for Ashvale's current vision.

It defines the intended battle model, baseline formulas, persistence rules, and UI expectations. Balance values may still be tuned later, but the core structure in this file should be treated as the design baseline.

## Battle format

- Battles are true turn-based CTB battles.
- The same battle module is used by both overworld and location encounters.
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
- Only hero units are leader-capable.
- If reduced to 0 HP, a hero is KO'd.
- A KO'd hero can be revived during battle.
- If a non-player hero is still KO'd when battle ends, that hero leaves the party and is treated as recovering.
- If the player character is still KO'd when battle ends but the allied team wins, the player character returns to the party at 1 HP.
- Heroes that leave the party are not considered permanently dead. They may be recovered or re-recruited later through scenario content and hero-recruitment services.

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

3. Convert delta into multiplier using the same rule as physical damage:

- if `statDelta > 0`, increase damage by 10% per point
- if `statDelta < 0`, decrease damage by 5% per point
- cap the total adjustment at `+200%` maximum and `-90%` minimum

4. Final magic damage:

`finalMagicDamage = max(1, floor(baseMagic * damageMultiplier))`

### Damage randomness and reliability

- Damage roll variance is the default source of RNG in battle.
- Hit/miss, dodge, crit-like effects, status application, and similar mechanics should default to deterministic rules if added.
- Example: a passive that dodges the first physical attack each turn is acceptable because it is deterministic.

## Applying damage to generic stacks

For generic stack units:

- damage reduces current HP on the current active body
- if current HP reaches 0, lose 1 Life
- excess damage bleeds into the next body in the stack
- one action can remove multiple lives if damage is high enough
- if Life reaches 0, the stack is defeated and removed unless revived before battle ends

### Generic stack persistence after battle

- current HP is restored to full for surviving generic stacks after battle
- lost Life persists
- MP persists
- temporary battle-only effects are removed

## Applying damage to heroes

For hero units:

- if HP reaches 0, the hero is KO'd
- a KO'd hero takes no further actions unless revived
- if the KO'd hero is the assigned leader, the leader aura is removed immediately

### Hero persistence after battle

- surviving hero HP persists after battle
- hero MP persists after battle
- temporary battle-only effects are removed
- if a non-player hero is still KO'd when battle ends, that hero leaves the party and is treated as recovering
- if the player character is still KO'd when battle ends but the allied team wins, the player character returns at 1 HP

## Revive rules

- Revive is intended to be rare and costly.
- Revive may come from skills or items.
- Both heroes and generic stacks may be revived in battle.
- Reviving a generic stack can restore lost Life if the revive effect is strong enough.
- Revive can also top up a partially damaged stack.
- Any unit or stack still unresolved at battle end follows normal post-battle persistence rules.

## Actions and battle command structure

### Baseline actions

Every unit should support the battle flow expected from an FF-style command menu.

Baseline concepts:

- `Attack`
- `Defend`
- `Skill`
- `Item`
- `Wait`
- `Position`

### Hero command structure

Hero units use the standard battle menu structure:

- `Attack`
- `Defend`
- `Skill`
- `Item`

Where relevant:

- `Skill` opens a submenu of available skills
- `Item` opens a submenu of available items
- only hero units may use items in battle

### Generic command structure

Generic units may have zero or more skills.

- If a generic unit has no skills, its main menu may simply show its directly available baseline actions.
- If a generic unit has 1 or 2 skills, those skills should appear directly on the main menu instead of using a nested `Skill` command.
- If a generic unit has more than 2 skills, it should use a nested `Skill` command.
- Generic units do not use items.

### Secondary command menu

In addition to the visible main command menu, battle should support a secondary command menu for positional utility.

This secondary menu contains:

- `Wait`
- legal position-change commands for the acting unit

Examples:

- a unit currently in `Front` might see `Middle ->` and `Back ->`
- a non-leader hero may also see `Leader ->` if legal
- the current leader does not get another `Leader` option while already assigned as leader

### Skill categories

Skills may include:

- direct damage
- utility
- buff
- debuff
- healing
- revive
- stealing
- position manipulation
- CTB manipulation

## Wait, Defend, and status duration

### Wait

- `Wait` consumes the acting unit's turn.
- `Wait` doubles the unit's base Agility until that unit's next turn.
- When that next turn begins, the temporary Wait agility bonus is removed immediately.
- Wait can be chained repeatedly.

### Defend

`Defend` is currently a provisional baseline pending a later balance pass.

Current baseline:

- `Defend` consumes the acting unit's turn.
- `Defend` grants `+5 Defense`.
- This only affects physical damage.
- The effect lasts until that unit's next turn.
- When that next turn begins, the temporary Defense bonus is removed immediately.
- `Defend` does not directly change CTB timing.

### Buff and debuff duration

- There are no rounds.
- If an in-battle effect lasts for `X turns`, it counts down on the **affected unit's own turns**.
- Effects applied in battle are removed when battle ends unless a specific rule says otherwise.

### Overworld-applied buffs

Overworld or service buffs may have explicit non-battle durations such as:

- `1 battle`
- `1 day`
- `1 week`

These durations should be handled by the broader world-state rules, not by battle-round logic.

## CTB requirements and readability

- Battle UI must show upcoming turn order.
- Turn order must always reflect the real current CTB state.
- If the player previews an action that changes turn order, the turn-order display should update immediately.
- If the player cancels or changes the selection, the preview should update immediately to match the current choice.
- If a unit is defeated, revived, repositioned, hasted, slowed, or otherwise affects CTB, the turn-order display should update immediately.
- The game should not expose low-level delay math directly if the visible turn-order bar already communicates the outcome clearly.

### Damage and KO preview

When selecting a target, the player should see as much tactical information as is practical, including:

- min-max damage preview
- min-max KO / kill preview
- resulting turn-order preview

If min and max are identical, the UI may show a single value instead of a range.

## Persistent enemy attrition

Persistent enemy teams follow the same broad durability rules as the player's team.

This means persistent enemy groups may retain:

- lost generic stack count
- lost hero participants that are still unresolved or recovering
- spent MP
- surviving hero HP state

Persistent enemy groups do **not** retain:

- temporary battle-only buffs or debuffs

Enemy groups may also recover through broader world rules, such as resting across days or occupying healing services, if the scenario simulation supports that behavior.

## Win/loss conditions

### Win

- all enemy units are removed or otherwise defeated according to battle rules

### Loss

- all allied units are removed or incapacitated

# Combat Rules

## Battle format

- true turn-based CTB battle
- one battle module used by both overworld and location encounters
- each side can field up to 5 units

## Unit categories

### Leader
- Must be the player character or a hero unit
- The leader's Attack, Defense, Magic and Resistance stats are directly added to the stats of all units on their team.
- The leader position is always "Leader" (furthest back).
- As long as the leader's turn is active, the leader can choose to cast a spell or wait.
- After the leader has cast a spell, the leader's Agility decidec when he/she can cast a spell again.
- If all units of the leader is defeated the leaders enters battle

### Hero units
- unique named characters
- do not stack
- have personality and story relevance
- if reduced to 0 HP, they leave after battle unless they are revivied before battle ends
- they can be rediscovered/recruited later if lost

### Generic units
- stackable
- when defeated, they are permanently lost

## Stats

Each unit has:
- Attack
- Defense
- Magic
- Resistance
- MinDamage
- MaxDamage
- HP
- MP
- Agility
- Life
- Position
- Range (Melee, Ranged or Long Ranged)

## Interpretation

- HP = current health of one active body/unit
- MP = magic resource
- Agility = how quickly turns come up in CTB
- Life:
  - for generic units, this is stack count
  - for heroes, Life is effectively 1
- Magic = Power of magic attacks. Affects damage or how long buffs/debuffs last. Does not affect turn order.
- Resistance = reduces magic damage taken. Does not affect turn order.
- MinDamage and MaxDamage are used to calculate base damage for physical attacks. They do not affect turn order.
- Attack and Defense affect physical damage.
- Position can be: Front, Middle or Back. Can negatively affect turn order of unit dependant on Range.
  - Front: Best for Melee units.
  - Middle: Best for Ranged units.
  - Back: Best for Long Ranged units.

## Prototype formulas

These are prototype formulas for vertical slice and may be revised later.

### Agility penalty based on Position and Range
- Position and range effects turn order.
- There is either 0 Agility penalty or penalty based on position and range of attacking unit.
- Ragne values, Melee = 0, Ranged = 1, Long Ranged = 3
- Position values, Front = 0, Middle = 1, Back = 2, leader = 5
- Agility Penalty = (Attacking units range - Defending units position)
- Agility Penalty is 0 if  Agility Penalty is positive.
- Final Agility Penalty is 50% if Agility Penalty is 1
- Final Agility Penalty is 75% if Agility Penalty is 2 or more
- Agility can not go below 1 after applying penalty.
- Agility Penalty is always reset on the next turn, so it does not permanently affect the unit.
When the last unit in a position is defeated, the positions of all remaining units on the team are shifted accordingly, but only forward. For example, if the front unit is defeated, the middle unit becomes the new front and the back unit becomes the new middle.

### Physical damage
baseDamage = random(MinDamage, MaxDamage) * Life
attackModifier = Ataccking unit's Attack stat - target unit's Defense stat
If attackModifier is positive, baseDamege increases 10 percent per point of attackModifier.
If attackModifier is negative, baseDamage decreases 5 percent per point of attackModifier.
attackModifier is maximum +200% and minimum -90%
modifiedDamage = baseDamage * attackModifier
finalDamage = max(1, modifiedDamage)

### Magic damage
baseMagic = Magic * (skillPower of magic used)
attackModifier = Attacking unit's Magic stat - target unit's Resistance stat
If attackModifier is positive, baseMagic increases 10 percent per point of attackModifier.
If attackModifier is negative, baseMagic decreases 5 percent per point of attackModifier.
modifiedMagic = baseMagic * attackModifier
finalMagicDamage = max(1, modifiedMagic)

If the implementation needs a simpler prototype, document the simplification.

## Applying damage to generic stacks

For generic stack units:
- damage reduces current HP
- if HP reaches 0, lose 1 Life
- damage bleeds over to next unit in the stack and can potentially remove multiple units (lives) if the damage is high enough
- if Life reaches 0, the unit is in defeated state and removed from the team if not revived before battle ends


## Applying damage to heroes

For hero units:
- if HP reaches 0, the hero is KO'd
- after battle, and still on KO status, remove the hero from the active party and flag them as lost

## Minimum prototype actions

- Attack
- Defend
- Wait (waiting makes the unit act faster)
- Skill 1
- Skill 2
- Item use may be stubbed or deferred


## CTB requirements

- battle UI must show upcoming turn order
- agility influences how frequently units act
- system should be deterministic apart from intentional RNG

## Win/loss conditions

Win:
- all enemy units removed

Loss:
- all allied units removed or incapacitated

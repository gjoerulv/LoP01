# Combat Rules

## Battle format

- true turn-based CTB battle
- one battle module used by both overworld and location encounters
- each side can field up to 5 units

## Unit categories

### Leader
- Leader-capable units are units with category `Leader` or `Hero`.
- The player team must always have exactly one assigned leader in the active battle party.
- Enemy teams may have zero or one assigned leader.
- If a team has an assigned leader, that unit is a normal battle participant from turn 1.
- The assigned leader occupies the `Leader` position (furthest back).
- The assigned leader's Attack, Defense, Magic, and Resistance stats are directly added to the stats of all units on their team, including the leader, while the leader is alive.
- The leader aura is removed immediately when the assigned leader reaches 0 HP.
- The player character is the default assigned leader whenever present in the active battle party.
- For the player team, moving the current leader out of the active party is illegal if it would leave the active party without a legal leader.

### Hero units
- unique named characters
- do not stack
- have personality and story relevance
- are leader-capable
- if reduced to 0 HP, they are KO'd
- if a non-player hero is still KO'd when battle ends, they leave the party unless revived before battle ends
- if the player character is still KO'd when battle ends but the team wins, the player character returns to the party at 1 HP
- non-player heroes that leave the party can be rediscovered or re-recruited later

### Generic units
- stackable
- when defeated (stack count/life = 0), they are permanently lost, unless revived before battle ends
- stacks do not refill automatically
- stack can fill up by recruiting more units. Recruiting costs gold, and, sometimes additional resources pr unit.

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
- if HP reaches 0, the hero is KO'd and their battle participation ends for that battle
- if the KO'd hero is the assigned leader, the leader aura is removed immediately
- after battle, unresolved KO is handled by post-battle recovery rules:
  - non-player heroes leave the party unless revived before battle ends
  - the player character returns to the party at 1 HP if the team wins
  - on defeat/loss, the normal defeat recovery flow applies

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
- player action, shuch as waiting or using skills, can influence turn order, and this should be communicated to the player by updateing the turn order UI immediately after the player action is selected, and reset if actions are cancelled or changed, but be mainteaned if the player goes through with the action
- turn order UI should always update according to what happens in battle, and reflect the actual turn order, even if the player does not select an action that influences turn order. For example, if a unit is defeated, the turn order should update immediately to reflect the change in units and their turn order.
- system should be deterministic apart from intentional RNG

## Win/loss conditions

Win:
- all enemy units removed

Loss:
- all allied units removed or incapacitated

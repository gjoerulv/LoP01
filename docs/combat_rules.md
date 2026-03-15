# Combat Rules

## Battle format

- true turn-based CTB battle
- one battle module used by both overworld and location encounters
- each side can field up to 5 units

## Unit categories

### Hero units
- unique named characters
- do not stack
- have personality and story relevance
- if reduced to 0 HP, they leave the party
- they must be rediscovered/recruited later

### Generic units
- stackable
- when defeated, they are permanently lost

## Stats

Each unit has:
- Attack
- Defense
- Magic
- MinDamage
- MaxDamage
- HP
- MP
- Agility
- Life

## Interpretation

- HP = current health of one active body/unit
- MP = magic resource
- Agility = how quickly turns come up in CTB
- Life:
  - for generic units, this is stack count
  - for heroes, Life is effectively 1

## Prototype formulas

These are prototype formulas for vertical slice and may be revised later.

### Physical damage
baseDamage = random(MinDamage, MaxDamage)
modifiedDamage = baseDamage + Attack - floor(Defense / 2)
finalDamage = max(1, modifiedDamage)

### Magic damage
baseMagic = Magic * skillPower
finalMagicDamage = max(1, baseMagic - floor(targetMagicResistPlaceholder))

If the implementation needs a simpler prototype, document the simplification.

## Applying damage to generic stacks

For generic stack units:
- damage reduces current HP
- if HP reaches 0, lose 1 Life
- if Life remains, HP resets to base HP for the next unit in the stack
- if Life reaches 0, the stack is removed

## Applying damage to heroes

For hero units:
- if HP reaches 0, the hero is KO'd and removed from battle
- after battle, remove the hero from the active party and flag them as lost

## Minimum prototype actions

- Attack
- Defend
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

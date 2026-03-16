# Vertical Slice Content Scope

## Region count

Implement exactly 1 overworld region for v0.

## Overworld destinations

Target 10 to 20 destinations:
- home/base
- abandoned town center
- inn
- shop/recruit point
- mine entrance
- dungeon/ruin
- service node
- optional threat/combat node
- combat locations blocking path to other destinations

## Locations

Implement:
- 1 or 2 town-style location per overworld region
- 1, 2 or 3 dungeon-style location per overworld region
- 1 home/base interior per overworld region

## Party content

- 1 player character
- 3 hero characters total
- 8 generic unit types
- 8, 9 or 10 enemy groups

## Systems

Must exist in v0:
- time/day progression
- travel time
- location time costs
- sleeping
- wake-up penalty
- battle module
- recruitment
- shop
- save/load
- opening story sequence
- 2 simple quests

## Mine

For v0:
- implement 10 floors only
- procedural or semi-random floor generation is acceptable
- floor 5 acts as first checkpoint, floor 10 acts as second checkpoint
- A checkpoint happens every fifth floor
- The player can choose to exit the mine from each floor entrance
- From the main entrance to the main, the player can choose to enter from floor 0 or any checkpoint floor
- Even when returning to a checkpoint, floors are regenerated

## UI

Must show:
- current day
- current time
- gold
- current location/region
- party summary
- battle turn order

# Core Loop Rules

## Day structure

- Day starts at 06:00
- Day ends at 02:00 the following night
- If the player fails to sleep in a valid bed before 02:00, apply wake-up penalty next day
- Gold can never go below 0

## Wake-up penalty

If the player:
- is fully defeated
- or fails to reach a bed before 02:00

then:
- next day starts at 11:00
- player wakes in nearest hospital or jail depending on context
- player loses 1000 gold, but not below 0

## Valid sleeping locations

- player home/base
- inns

## Overworld selection mode

- player chooses current overworld region
- switching region costs 1 full day
- region transfer must be initiated before 11:00
- region transfer must be discovered/unlocked first

## Overworld mode

- player does not move freely on the map
- player selects a destination
- travel time is based on distance
- travel time is quantized to 15-minute steps
- shortest travel time: 15 minutes
- longest travel time: 4 hours

Destinations can be:
- enterable locations
- single-use service locations
- recruit points
- buff points
- combat locations
- dungeons

## Location mode

Walking is free.

Time costs:
- opening a door = 1 minute
- each chosen dialogue option = 1 minute
- shopping transaction = 5 minutes
- recruiting = 10 minutes

Location mode exits back to the overworld entrance point.

The player can open a menu here for:
- party
- inventory
- skills

## Defeat handling

If the player party is fully defeated:
- wake at hospital or jail
- lose 1000 gold, min 0
- next day starts at 11:00

If the player character is KO'd in battle but the party still wins:
- set player HP to 1 after battle

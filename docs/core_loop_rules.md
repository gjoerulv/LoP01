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
- player selects a destination node
- travel time is based on distance
- travel time is quantized to 15-minute steps
- shortest travel time: 15 minutes
- longest travel time: 4 hours
- Entering/using same location as the player is already in, costs 0 minutes
- If 1 or more enmies are blocking a destination node, the player can't pass without defeating them first

Destination nodes can be:
- enterable locations
- single-use service locations
- recruit points
- buff points
- combat locations
- dungeons
- multi-purpose location containing 2-3 single-purpose serveices. The user can select between them, and optionally use all
- Placeholder nodes that may be empty, contain a single resource/item, or a single fight
- Picking up resources/items are optional. If picked up, node becomes empty
- Winning a fight at a fighting node, clears the node and makes it empty
- Enemies may spawn on placeholder nodes at the start of day with a low probability (~3%)

## Location mode

Walking is free.

Time costs:
- opening a door/entering a building = 1 minute
- scene transitions = 1 - 5 minutes, depending on the transition
- each chosen dialogue option = 1 minute
- shopping transaction = 5 minutes
- recruiting = 10 minutes
- cutscenes: varies, may take 1 minute to several days depending on the scene.

Location mode exits back to the overworld entrance point by walking into it.

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

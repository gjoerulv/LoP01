# Core Loop Rules

This document mixes **current slice rules** and **medium-term target behavior**.
When a detail conflicts with the current codebase or the active milestone doc, use the codebase and active milestone doc as the source of truth.

## Day structure

- Day starts at 06:00
- Day ends at 02:00 the following night
- If the player fails to sleep in a valid bed before 02:00, apply wake-up penalty next day
- Gold can never go below 0

## Wake-up penalty

If the player:
- is fully defeated
- or fails to reach valid sleep before 02:00

then:
- next day starts at 11:00
- player wakes at the current slice fallback recovery location for the context
- player loses 1000 gold, but not below 0

## Valid sleeping locations

Current slice baseline:
- player home/base
- inns

Longer-term direction:
- inns should generally be usable if present and should cost money
- home-base rest should remain free

## World map / region select mode

- the player may consult the world map at any time for information
- the world map is the scenario-level region selection and planning layer
- the player may travel between overworlds/regions only once per day
- region travel must be initiated before 11:00
- region travel uses that day's one region-travel allowance
- after region travel is confirmed, the player character arrives in the destination region at 11:00
- if the current time is 11:00 or later, region travel is not allowed until the next day
- some regions may be locked and require discovery or progression before travel is allowed

## Overworld mode

- player does not move freely on the map
- player selects a destination node
- travel follows the authored route graph rather than simple index distance
- travel time is quantized to 15-minute steps in the current slice
- entering/using the same location the player is already in costs 0 minutes
- the player cannot confirm travel that would arrive past 02:00
- blocker nodes can prevent transit until their gating condition is cleared
- if the player is trapped only because of the late-night cutoff, the wake-penalty flow should resolve the dead-end rather than soft-locking the loop

Destination nodes can be:
- enterable locations
- inns or service locations
- recruit points
- buff points
- combat locations
- dungeons
- multi-purpose locations containing 2-3 single-purpose services
- placeholder nodes that may be empty, contain a single resource/item, or a single fight

Slice and future-facing examples:
- picking up a resource/item may be optional and may empty the node after use
- winning a fight at a combat node may clear the node and change routing/world state
- enemies may spawn on some placeholder nodes later if that becomes part of the design

## Location mode

Walking is free.

Time costs:
- opening a door / entering a building = 1 minute
- scene transitions = 1 - 5 minutes, depending on the transition
- each chosen dialogue option = 1 minute
- shopping transaction = 5 minutes
- recruiting = 10 minutes
- cutscenes: varies, may take 1 minute to several days depending on the scene

Location mode exits back to the overworld entrance point by walking into it.

The player can open a menu here for:
- party
- inventory
- skills

## Defeat handling

If the player party is fully defeated:
- wake at the slice-appropriate fallback recovery location
- lose 1000 gold, min 0
- next day starts at 11:00

If the player character is KO'd in battle but the party still wins:
- set player HP to 1 after battle

## Cross-region party state

- only the player character travels between regions
- each region remembers its own party composition
- units recruited or retained in one region remain in that region until the player returns there

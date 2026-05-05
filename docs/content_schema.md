# Content Schema

This document defines the intended long-term authored data model for Ashvale.

It is a conceptual schema guide, not a complete JSON Schema implementation.

Use this document when designing or modifying:
- content JSON
- content loaders
- editor data models
- validators
- mod loading
- Scenario / Region / Location schemas
- event, condition, and action schemas

Related docs:
- `docs/scenario_authoring.md`
- `docs/validation_system.md`
- `docs/game_vision.md`
- `docs/core_loop_rules.md`
- `docs/terminology_map.md`
- `README_DECISIONS.md`

---

## 1. Schema goals

Ashvale content should be:

- JSON-based
- hand-readable
- editor-managed
- mod-friendly
- schema-versioned
- validation-friendly
- deterministic to load
- clear enough for AI agents to edit safely

Content data configures supported systems.

Content data should not create arbitrary new mechanics outside the typed systems implemented in code.

---

## 2. Top-level file shape

Every top-level JSON content file should include:

```json
{
  "schemaVersion": 1,
  "kind": "Scenario",
  "id": "scenario_intro"
}
```

Required fields:

- `schemaVersion`
- `kind`
- `id`

Rules:

- `schemaVersion` is an integer.
- `kind` identifies the top-level content type.
- `id` is the stable authored content identity.
- `id` should be an opaque stable string, not player-facing text.
- display names must be separate from ids.

Examples of `kind` values:

- `Campaign`
- `Scenario`
- `Region`
- `Location`
- `UnitDefinition`
- `ItemDefinition`
- `ArtifactDefinition`
- `Recipe`
- `ArtifactCombinationRecipe`
- `ServiceDefinition`
- `TeamTemplate`

---

## 3. Localized text

Player-facing text should use localized text objects from the start.

Example:

```json
{
  "name": {
    "en": "Old Market",
    "nb": "Gammelt marked"
  },
  "description": {
    "en": "A worn-down trading stall.",
    "nb": "En slitt handelsbod."
  }
}
```

Rules:

- language codes are keys
- values are strings
- supported languages should be validated against game/mod metadata
- fallback language rules should be explicit in the loader

Recommended fallback:

1. requested language
2. default content language
3. English if present
4. first available language
5. validation warning if fallback is needed for release content

Do not use localized display text as ids.

---

## 4. File organization

Recommended official content layout:

```text
content/
  campaigns/
  scenarios/
  regions/
  locations/
  units/
  items/
  artifacts/
  recipes/
  services/
  templates/
  mods/
```

The exact directory structure may evolve, but content should stay grouped by domain.

---

## 5. Mods

Mods live under:

```text
content/mods/<modName>/
```

Example:

```text
content/mods/mymod/metadata.json
content/mods/mymod/campaigns/campaign_mycampaign.json
content/mods/mymod/scenarios/scenario_my_scenario.json
```

Each mod folder must contain:

```text
metadata.json
```

Recommended `metadata.json` fields:

```json
{
  "schemaVersion": 1,
  "kind": "ModMetadata",
  "id": "mymod",
  "name": {
    "en": "My Mod"
  },
  "author": "Author Name",
  "modVersion": "1.0.0",
  "targetGameVersion": "0.1.0",
  "supportedLanguages": ["en"]
}
```

### Mod load order

Default mod load order:

1. official content
2. installed mods in alphabetical folder order

A load-order text file inside `content/mods/` may override mod order.

Recommended name:

```text
content/mods/load_order.txt
```

Format:

```text
mymod
othermod
```

Rules:

- listed mods load in listed order
- installed mods not listed load afterward in alphabetical order
- latest loaded content wins on conflicts

### Mod override identity

A mod overrides content by matching top-level:

- `kind`
- `id`

Filename is a convention, not the true identity.

Rules:

- if a mod provides the same `kind + id` as earlier-loaded content, it overrides that content
- if a mod provides a new `kind + id`, it adds new content
- if filename and `id` disagree, validation should report it
- if file directory and `kind` disagree, validation should report it

Example override:

```text
content/scenarios/scenario_intro.json
content/mods/mymod/scenarios/scenario_intro.json
```

The mod file overrides the official Scenario only if it also contains:

```json
{
  "kind": "Scenario",
  "id": "scenario_intro"
}
```

---

## 6. References

Use plain id strings when the field name gives the reference type.

Example:

```json
{
  "unitId": "unit_bandit",
  "itemId": "item_potato",
  "regionId": "region_mushville"
}
```

Use typed references only in heterogeneous lists.

Example:

```json
{
  "target": {
    "kind": "Service",
    "id": "svc_old_market"
  }
}
```

Event actions and conditions should use typed fields rather than ambiguous generic ids.

Example:

```json
{
  "type": "giveItem",
  "itemId": "item_potato",
  "quantity": 1
}
```

---

## 7. Resources

Resources are enum values, not arbitrary mod-defined resource definitions.

Baseline resource enum:

- `Gold`
- `Wood`
- `Stone`
- `Steel`
- `Fiber`
- `Clay`
- `Gems`

Resource enum validation should be strict.

Mods may change content that uses resources, but should not add new resource types unless the code/schema explicitly expands the resource enum.

---

## 8. Campaign schema

A Campaign owns:

- id
- localized name
- localized description
- Scenario list
- transition graph
- carry-over rules
- campaign story flags
- campaign variables where needed

Conceptual shape:

```json
{
  "schemaVersion": 1,
  "kind": "Campaign",
  "id": "campaign_ashvale",
  "name": { "en": "Ashvale" },
  "description": { "en": "A campaign." },
  "scenarios": [
    {
      "scenarioId": "scenario_intro",
      "nextScenarioIds": ["scenario_second"],
      "carryOverRuleId": "carry_intro_to_second"
    }
  ],
  "campaignFlags": [
    "flag_intro_complete"
  ],
  "carryOverRules": []
}
```

Standalone Scenarios are legal and do not require a Campaign.

If a Scenario belongs to a Campaign, it should not be shown as a standalone selectable Scenario unless explicitly authored as standalone-selectable.

---

## 9. Scenario schema

A Scenario owns:

- id
- localized name
- localized description
- World Map data
- Region references
- Scenario Region Contexts
- team setup
- victory conditions
- defeat conditions
- global events
- story flags
- Scenario variables
- hero pool
- resource defaults
- banned skills
- banned artifacts
- validation suppressions
- standalone/campaign selection metadata

Conceptual shape:

```json
{
  "schemaVersion": 1,
  "kind": "Scenario",
  "id": "scenario_intro",
  "name": { "en": "Intro Scenario" },
  "description": { "en": "The first Scenario." },
  "standaloneSelectable": true,
  "worldMap": {},
  "regions": [
    {
      "regionId": "region_mushville",
      "initialState": "unlocked",
      "context": {
        "variables": {
          "winter_mode": true
        }
      }
    }
  ],
  "storyFlags": [
    "flag_market_built"
  ],
  "variables": {
    "bridge_repaired": false
  },
  "heroPool": [
    "hero_jon"
  ],
  "bannedSkills": [],
  "bannedArtifacts": [],
  "teams": [],
  "victoryConditions": [],
  "defeatConditions": [],
  "events": [],
  "validationSuppressions": []
}
```

---

## 10. Scenario Region Context

Regions are reusable structural definitions.

A Scenario controls how a Region behaves by passing Scenario context to it.

Use **Scenario Region Context** rather than arbitrary shallow patching as the primary model.

Scenario Region Context may include:

- initial Region state
- Scenario variables visible to the Region
- Scenario flags visible to the Region
- availability rules from the Scenario
- allowed/banned artifacts, skills, units, items, or heroes where needed
- initial ownership overrides where explicitly supported
- initial reveal or unlock state where explicitly supported

Rules:

- Region files should not hard-own Scenario-level availability rules.
- Scenario owns hero pools, banned artifacts, banned skills, team setup, and Scenario variables.
- Region events, services, and conditions may read Scenario variables and flags.
- Validation evaluates a Region in the Scenario context that loads it.

Avoid broad arbitrary patches as the default authoring model.

If a future patch/override system is needed, it should be typed and validated.

---

## 11. World Map schema

The World Map is part of the Scenario.

It owns:

- visual map data
- Region markers / positions
- manual Region adjacency
- initial visibility/unlock state
- enterable state
- authored Region descriptions or summaries where needed

Conceptual shape:

```json
{
  "worldMap": {
    "regions": [
      {
        "regionId": "region_mushville",
        "position": { "x": 10, "y": 20 },
        "initialVisibility": "visible",
        "initialEnterable": true,
        "description": {
          "en": "A large mushroom-filled region."
        }
      }
    ],
    "adjacency": [
      {
        "fromRegionId": "region_mushville",
        "toRegionId": "region_old_road"
      }
    ]
  }
}
```

World Map adjacency is bidirectional.

The data may store one authored edge, but validation/runtime should treat it as bidirectional unless the design later explicitly supports directed World Map travel.

---

## 12. Region schema

A Region owns:

- id
- localized name
- localized description
- nodes
- routes
- arrival node
- default reveal data
- placed node content
- placed Region services
- neutral encounters
- stationed guards
- Region-level events
- Region display metadata

Conceptual shape:

```json
{
  "schemaVersion": 1,
  "kind": "Region",
  "id": "region_mushville",
  "name": { "en": "Mushville" },
  "description": { "en": "A strange fungal valley." },
  "arrivalNodeId": "node_arrival",
  "nodes": [],
  "routes": [],
  "services": [],
  "events": []
}
```

Region size label is derived from node count.

Regions do not directly own Scenario-level availability rules.

---

## 13. Node schema

Nodes are travel points.

A node may have:

- id
- optional localized display name
- position
- arrival flag where relevant
- main node content
- event references
- reveal metadata where needed

Display name is optional.

If a node has Service content, the Service name should usually take priority for display.

Conceptual shape:

```json
{
  "id": "node_old_farm",
  "name": { "en": "Old Farm" },
  "position": { "x": 12, "y": 8 },
  "content": {
    "type": "service",
    "serviceId": "svc_old_farm"
  },
  "eventRefs": [
    {
      "eventId": "evt_old_farm_warning",
      "priority": "beforeContent"
    }
  ]
}
```

---

## 14. Node content schema

Node content is a discriminated union.

A node may contain at most one main content item.

Examples:

```json
{
  "type": "resourcePickup",
  "resource": "Wood",
  "amount": 5
}
```

```json
{
  "type": "artifactPickup",
  "artifactId": "artifact_old_ring",
  "quantity": 1
}
```

```json
{
  "type": "service",
  "serviceId": "svc_old_market"
}
```

```json
{
  "type": "neutralEnemy",
  "encounterId": "enc_bandits_01"
}
```

```json
{
  "type": "special",
  "specialId": "special_glowing_stone"
}
```

Events are not main node content.

Events are attached separately through `eventRefs`.

---

## 15. Node event references

Node event references live on the node.

Region owns event definitions.

Event priority lives on the event attachment, not the event definition.

Allowed priority values:

- `beforeContent`
- `afterContent`
- `replacesContent`

Default:

- `beforeContent`

Example:

```json
{
  "eventRefs": [
    {
      "eventId": "evt_gate_warning",
      "priority": "beforeContent"
    }
  ]
}
```

---

## 16. Route schema

Routes are explicit Region objects with ids.

Region routes are bidirectional.

Conceptual shape:

```json
{
  "id": "route_arrival_to_farm",
  "fromNodeId": "node_arrival",
  "toNodeId": "node_old_farm",
  "road": true,
  "terrain": "meadow",
  "initialState": "active"
}
```

Fields:

- `id`
- `fromNodeId`
- `toNodeId`
- `road`
- `terrain`
- `initialState`

Travel time and Energy cost are computed from route quality and distance.

Routes may be authored as hidden/inactive and later restored or activated by events.

Events should not create new route definitions from nothing at runtime.

---

## 17. Location schema

A Location owns:

- id
- localized name
- localized description
- screens/maps
- event sprites
- collision/layer data
- triggers
- local service instances where needed
- service calls
- event pipes
- visual state definitions

Conceptual shape:

```json
{
  "schemaVersion": 1,
  "kind": "Location",
  "id": "location_old_house",
  "name": { "en": "Old House" },
  "screens": [],
  "services": [],
  "events": []
}
```

Location mode is event-driven.

Location interactables are event sprites, not hard-wired Region services.

---

## 18. Service instance schema

Region and Location service calls should use the same Service instance schema where possible.

A Service instance has:

- id
- serviceType
- localized name where needed
- type-specific settings
- authored initial state
- destroyable/restorable flags where applicable
- ownership/stationed guard data where applicable
- availability rules where applicable

Conceptual shape:

```json
{
  "id": "svc_old_farm",
  "serviceType": "farming",
  "name": { "en": "Old Farm" },
  "initialState": "active",
  "destroyable": true,
  "restorable": true,
  "settings": {
    "maxSeedQuantity": 999,
    "allowFertilizer": true
  }
}
```

`settings` is a type-specific payload.

Service state in content is initial state.

Runtime state belongs in save data.

---

## 19. Region Services versus Location service calls

Region services are placed as node content.

Example:

```json
{
  "content": {
    "type": "service",
    "serviceId": "svc_old_farm"
  }
}
```

Location service calls are event actions.

Example:

```json
{
  "type": "callService",
  "serviceId": "svc_location_market_small"
}
```

Rules:

- Location service calls use Service instances.
- Location-called services use the same rules as Region services where applicable.
- Only whitelisted service types are callable from Locations.
- One-time / single-use services should not be callable from Location events.
- Location-owned service instances may live inside the Location file or in a Scenario-local service list if later needed.
- Validation must ensure the referenced service is callable from Location context.

---

## 20. Event schema

An Event has:

- id
- trigger
- eligibility
- condition
- priority where applicable
- repeat structure
- actions

Conceptual shape:

```json
{
  "id": "evt_build_market",
  "trigger": {
    "type": "locationConfirm",
    "objectId": "obj_magic_stone"
  },
  "eligibility": {
    "teamKinds": ["human"]
  },
  "condition": {
    "type": "always"
  },
  "repeat": {
    "mode": "once"
  },
  "actions": []
}
```

Event-level message is allowed only as an optional shorthand for design convenience.

All messages should ultimately call the same message-display system as `showMessage`.

---

## 21. Event triggers

Trigger is a typed object.

Examples:

```json
{
  "type": "regionNodeEntry",
  "nodeId": "node_old_gate"
}
```

```json
{
  "type": "locationCollision",
  "objectId": "obj_boy"
}
```

```json
{
  "type": "locationConfirm",
  "objectId": "obj_magic_stone"
}
```

```json
{
  "type": "startOfDay"
}
```

```json
{
  "type": "neutralEncounterDefeated",
  "encounterId": "enc_bandits_01"
}
```

---

## 22. Eligibility schema

Eligibility controls who may trigger or participate.

Conceptual shape:

```json
{
  "teamColors": ["Green"],
  "teamKinds": ["human"],
  "requiredHeroIds": ["hero_jon"],
  "timeWindow": {
    "startDay": 1,
    "endDay": 10
  }
}
```

Eligibility is not the same as condition.

Use eligibility for who may participate.

Use condition for whether the world-state requirement is satisfied.

---

## 23. Condition schema

Conditions are typed objects.

Conditions support composition:

```json
{
  "all": [
    {
      "type": "teamHasResource",
      "resource": "Wood",
      "amount": 10
    },
    {
      "type": "teamHasHero",
      "heroId": "hero_jon"
    }
  ]
}
```

```json
{
  "any": [
    { "type": "storyFlagSet", "flag": "flag_bridge_repaired" },
    { "type": "teamHasItem", "itemId": "item_bridge_key" }
  ]
}
```

```json
{
  "not": {
    "type": "serviceDestroyed",
    "serviceId": "svc_old_bridge"
  }
}
```

Leaf conditions must include `type`.

Composition conditions use `all`, `any`, or `not`.

---

## 24. Action schema

Actions are typed objects.

Every action has a `type`.

Example:

```json
{
  "type": "takeResource",
  "resource": "Wood",
  "amount": 10
}
```

```json
{
  "type": "giveItem",
  "itemId": "item_potato",
  "quantity": 1
}
```

Actions should be small, explicit, and validateable.

Avoid generic script strings.

Action failure behavior is normally determined by the target system rule, not by per-action configuration.

---

## 25. If / Else action schema

`if` is an action type.

Example:

```json
{
  "type": "if",
  "condition": {
    "type": "teamHasResource",
    "resource": "Wood",
    "amount": 10
  },
  "then": [
    {
      "type": "takeResource",
      "resource": "Wood",
      "amount": 10
    },
    {
      "type": "showMessage",
      "text": {
        "en": "The market is rebuilt."
      }
    }
  ],
  "else": [
    {
      "type": "showMessage",
      "text": {
        "en": "You need more Wood."
      }
    }
  ]
}
```

Branches may be nested.

Branches replace optional-action flags.

---

## 26. Message action schema

Messages should use `showMessage` where practical.

Example:

```json
{
  "type": "showMessage",
  "text": {
    "en": "Jon joins you."
  },
  "portraitId": "portrait_jon"
}
```

Quest-service message fields and event-level shorthand messages may exist for editor convenience, but they should call the same message-display code path.

---

## 27. Repeat schema

Use structured repeat data.

Examples:

```json
{
  "repeat": {
    "mode": "once"
  }
}
```

```json
{
  "repeat": {
    "mode": "always"
  }
}
```

```json
{
  "repeat": {
    "mode": "everyNDays",
    "intervalDays": 7
  }
}
```

Automatic event priority is a number.

Lower number means earlier.

Priority must be unique within the same automatic trigger group.

---

## 28. Quest service schema

Quest services are Service instances with `serviceType: "questService"`.

Conceptual shape:

```json
{
  "id": "svc_old_man_quest",
  "serviceType": "questService",
  "settings": {
    "emptyMessage": {
      "text": {
        "en": "This place seems abandoned."
      }
    },
    "quests": []
  }
}
```

A quest service may have zero quests.

Zero quests are valid.

---

## 29. Quest entry schema

A quest entry should contain:

- id
- eligibility
- objective condition
- starting message
- progress message
- completion prompt
- completion actions
- repeatability where applicable

Conceptual shape:

```json
{
  "id": "quest_bring_wood",
  "eligibility": {
    "teamKinds": ["human"]
  },
  "objective": {
    "type": "teamHasResource",
    "resource": "Wood",
    "amount": 10
  },
  "startingMessage": {
    "text": {
      "en": "Bring me 10 Wood."
    },
    "portraitId": "portrait_old_man"
  },
  "progressMessage": {
    "text": {
      "en": "Still looking for Wood?"
    },
    "portraitId": "portrait_old_man"
  },
  "completionPrompt": {
    "text": {
      "en": "Will you give 10 Wood?"
    },
    "portraitId": "portrait_old_man",
    "yesActions": [
      {
        "type": "takeResource",
        "resource": "Wood",
        "amount": 10
      }
    ],
    "noActions": []
  }
}
```

Quest message fields are editor-facing shortcuts.

They should use the same message-display function as `showMessage`.

---

## 30. Victory and defeat condition schema

Victory and defeat conditions use the shared condition model.

Victory conditions are OR-based.

Defeat conditions are OR-based.

Example:

```json
{
  "id": "victory_defeat_red",
  "condition": {
    "type": "teamDefeated",
    "teamColor": "Red"
  },
  "eligibleTeamColors": ["Green"],
  "actions": [
    {
      "type": "triggerVictory"
    }
  ]
}
```

If no victory condition is authored, runtime/validation expands the default:

- defeat all enemy teams

This expansion should produce an info validation message.

---

## 31. Team schema

A Team owns:

- id or team color
- controller
- playerCharacterHeroId for human teams
- start Region
- start node
- active party
- reserve
- resources
- item inventory
- artifact inventory
- revealed nodes where authored
- AI settings where applicable
- alliances

Conceptual shape:

```json
{
  "id": "team_green",
  "color": "Green",
  "controller": "human",
  "playerCharacterHeroId": "hero_player",
  "startRegionId": "region_mushville",
  "startNodeId": "node_arrival",
  "activeParty": [],
  "reserve": [],
  "resources": {
    "Gold": 1000,
    "Wood": 0,
    "Stone": 0,
    "Steel": 0,
    "Fiber": 0,
    "Clay": 0,
    "Gems": 0
  },
  "items": [],
  "artifacts": [],
  "alliances": ["Blue"]
}
```

Controller values:

- `human`
- `ai`

AI teams add:

```json
{
  "ai": {
    "personality": "Builder",
    "aggression": "Careful",
    "patrol": {
      "enabled": true,
      "centerNodeId": "node_camp",
      "radius": 3
    }
  }
}
```

---

## 32. Unit stack and hero instance schema

Generic unit stack:

```json
{
  "unitId": "unit_bandit",
  "quantity": 12
}
```

Generic units may have skills if their unit definition supports them.

Hero instance:

```json
{
  "heroId": "hero_jon",
  "level": 5,
  "experience": 1200,
  "stats": {},
  "skills": [],
  "passiveSkills": [],
  "equippedArtifacts": {}
}
```

Hero id is enough as stable identity because heroes are unique pool entities.

Do not create a separate hero instance id unless the design later supports duplicate hero identities.

---

## 33. Item definition schema

Item definitions include:

- id
- localized name
- localized description
- icon
- subtype
- stack cap
- usable context
- base value
- effects where applicable

Conceptual shape:

```json
{
  "schemaVersion": 1,
  "kind": "ItemDefinition",
  "id": "item_fish_soup",
  "name": { "en": "Fish Soup" },
  "description": { "en": "A warm meal." },
  "icon": "icon_fish_soup",
  "subtype": "food",
  "stackCap": 999,
  "usableContext": "field",
  "baseValue": 50,
  "effects": []
}
```

Food is a normal item subtype.

Food must be field-use only and hero-consumed.

---

## 34. Effects schema

Items should use shared typed effects.

Artifacts should use typed modifiers and special effect enums.

Example item effect:

```json
{
  "type": "recoverHp",
  "target": "hero",
  "amount": 50
}
```

Example artifact modifier:

```json
{
  "type": "statBonus",
  "stat": "Attack",
  "amount": 2
}
```

Example artifact special effect:

```json
{
  "type": "specialEffect",
  "effect": "DisableAllUsableSkillsInBattleForBothTeams"
}
```

Special effects are enum-driven and handled by code.

Artifacts may have multiple bonuses and effects.

---

## 35. Artifact definition schema

Artifact definitions include:

- id
- localized name
- localized description
- icon
- allowed slot types
- effects
- rarity / tier
- base value
- combinable / upgradable flag

Conceptual shape:

```json
{
  "schemaVersion": 1,
  "kind": "ArtifactDefinition",
  "id": "artifact_iron_sword",
  "name": { "en": "Iron Sword" },
  "description": { "en": "A simple weapon." },
  "icon": "icon_iron_sword",
  "allowedSlots": ["Attack"],
  "rarity": "minor",
  "tier": 1,
  "baseValue": 250,
  "combinable": true,
  "effects": [
    {
      "type": "statBonus",
      "stat": "Attack",
      "amount": 2
    }
  ]
}
```

Ultimate or final-form artifacts should set `combinable: false`.

---

## 36. Recipe schema

Cooking recipes include:

- id
- localized name
- input ingredients
- output food item
- output quantity
- time cost
- required passive skills
- passive output modifiers where applicable

Conceptual shape:

```json
{
  "schemaVersion": 1,
  "kind": "Recipe",
  "id": "recipe_fish_soup",
  "name": { "en": "Fish Soup" },
  "ingredients": [
    { "itemId": "item_potato", "quantity": 1 },
    { "itemId": "item_tomato", "quantity": 1 },
    { "itemId": "item_fish", "quantity": 2 }
  ],
  "outputItemId": "item_fish_soup",
  "outputQuantity": 1,
  "timeMinutes": 60,
  "requiredPassiveSkills": []
}
```

The output item must be food subtype.

---

## 37. Artifact combination recipe schema

Artifact combination recipes are always 2 inputs to 1 output.

Inputs may be:
- two of the exact same artifact
- one artifact plus one other artifact type

Conceptual shape:

```json
{
  "schemaVersion": 1,
  "kind": "ArtifactCombinationRecipe",
  "id": "combo_iron_sword_plus_iron_sword",
  "inputs": [
    { "artifactId": "artifact_iron_sword", "quantity": 1 },
    { "artifactId": "artifact_iron_sword", "quantity": 1 }
  ],
  "outputArtifactId": "artifact_steel_sword"
}
```

A specific artifact handler service may deny selected recipes.

The global recipe remains valid unless the recipe itself is invalid.

---

## 38. Validation message schema

Validation messages should be first-class structured objects.

Conceptual shape:

```json
{
  "severity": "error",
  "code": "VAL-REF-001",
  "path": "Scenario:scenario_intro/Region:region_mushville/Node:node_old_gate",
  "message": "Referenced item `item_key_rusty` does not exist.",
  "suggestion": "Select a valid item id or remove the requirement.",
  "related": [
    {
      "kind": "QuestService",
      "id": "svc_old_gate"
    }
  ]
}
```

Required fields:

- severity
- code
- path
- message

Optional fields:

- suggestion
- related

---

## 39. Validation suppression schema

Validation suppressions live at Scenario level.

Conceptual shape:

```json
{
  "validationSuppressions": [
    {
      "code": "VAL-SOFTLOCK-LOW_SAFE_ANCHOR",
      "path": "Scenario:scenario_intro",
      "reason": "This Scenario is intentionally harsh and has no safe anchor."
    }
  ]
}
```

Suppression should match:

- validation code
- authored object path

Suppression must include a reason.

Suppressing one warning must not suppress unrelated future warnings.


---

## 40. Player character schema

The player character is a normal unique hero unit with special human-team rules.

For human teams, `playerCharacterHeroId` lives on the Team object.

Example:

```json
{
  "id": "team_green",
  "color": "Green",
  "controller": "human",
  "playerCharacterHeroId": "hero_player"
}
```

Rules:
- `playerCharacterHeroId` references a valid hero definition / hero identity.
- the player character must be leader-capable.
- the player character must be in that human team's traveling party.
- the player character may be active or reserve.
- the player character is not part of the recruitable hero pool.
- the player character must not appear in AI templates, recruit services, sealed hero services, neutral encounters, or AI-owned rosters.

### Character creation data
Before a Scenario or Campaign starts, the player creates the player character.

Character creation fills the stable player-character hero identity with:
- name
- sex
- simple graphical representation
- starting stats
- starting skills
- starting preset/template

Example:

```json
{
  "playerCharacter": {
    "heroId": "hero_player",
    "name": "Asha",
    "sex": "female",
    "appearance": {
      "body": "body_01",
      "hair": "hair_03",
      "palette": "palette_02"
    },
    "startingPreset": "Warrior",
    "stats": {},
    "skills": [],
    "passiveSkills": []
  }
}
```

Starting presets such as Warrior, Builder, and Explorer are presets only. They are not permanent class restrictions.

### Campaign carry-over
The player-character identity always carries over in Campaigns.

Campaign transition rules may still affect progression details such as level, skills, passive skills, equipment, or artifacts according to the normal carry-over rules.

### Runtime boundary
Player-character authored identity and creation defaults belong in content / campaign-start data.

Current HP, KO recovery, equipment state, level changes, learned skills, and other playthrough state belong in save data.

---

## 41. Runtime save data boundary

Authored content defines initial state.

Save data owns runtime state.

Examples of runtime state:

- current team resources
- active/reserve roster state
- current Energy
- revealed nodes
- destroyed/restored Services
- ongoing farming process
- quest visibility and completion
- event fired state
- route destroyed/restored state
- current ownership
- stationed guards
- Temporarily Unavailable heroes

Do not write runtime progression state back into authored content files.

Editor tools may edit authored initial state, not live save state, unless explicitly in a save-editor mode.

---

## 42. Agent / implementation guidance

For AI agents and future implementation work:

- prefer typed JSON objects over generic script strings
- keep `schemaVersion`, `kind`, and `id` on top-level files
- use localized text objects for player-facing text
- use plain ids when field names provide type context
- use typed refs only for heterogeneous references
- use discriminated unions for node content
- keep Region services and Location-called services mechanically shared
- keep service runtime state separate from authored initial state
- use Scenario Region Context instead of broad arbitrary Region patches
- validate by `kind + id`, not filename alone
- keep mod override behavior deterministic
- keep event conditions/actions explicit and small
- use `showMessage` as the shared message-display action path
- keep validation suppressions Scenario-level

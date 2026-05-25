# Content Schema

This document defines the intended long-term authored data model for Ashvale. It is a conceptual schema guide, not a complete JSON Schema implementation.

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
- `docs/presentation_game_feel.md`
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

Content data configures supported systems. Content data should not create arbitrary new mechanics outside the typed systems implemented in code.

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

Examples of long-term `kind` values:

- `Campaign`
- `Scenario`
- `ScenarioOutcome`
- `Region`
- `Location`
- `UnitDefinition`
- `ItemDefinition`
- `ArtifactDefinition`
- `Recipe`
- `ArtifactCombinationRecipe`
- `ServiceDefinition`
- `TeamTemplate`

Not every listed kind has a loader today. Future phases add loaders when the corresponding runtime system exists.

---

## 3. Localized text

Player-facing text should use localized text objects from the start.

Example:

```json
{
  "name": { "en": "Old Market", "nb": "Gammelt marked" },
  "description": { "en": "A worn-down trading stall.", "nb": "En slitt handelsbod." }
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

Current bounded-slice exception: M12 uses a single optional `content/scenario_outcome.json` file instead of a full per-Scenario directory structure.

---

## 5. Mods

Mods live under:

```text
content/mods/<mod-id>/
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
  "name": { "en": "My Mod" },
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
  "target": { "kind": "Service", "id": "svc_old_market" }
}
```

Event actions and conditions should use typed fields rather than ambiguous generic ids.

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

Resource enum validation should be strict. Mods may change content that uses resources, but should not add new resource types unless the code/schema explicitly expands the resource enum.

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
  "campaignFlags": ["flag_intro_complete"],
  "carryOverRules": []
}
```

Standalone Scenarios are legal and do not require a Campaign. If a Scenario belongs to a Campaign, it should not be shown as a standalone selectable Scenario unless explicitly authored as standalone-selectable.

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
        "variables": { "winter_mode": true }
      }
    }
  ],
  "storyFlags": ["flag_market_built"],
  "variables": { "bridge_repaired": false },
  "heroPool": ["hero_jon"],
  "bannedSkills": [],
  "bannedArtifacts": [],
  "teams": [],
  "victoryConditions": [],
  "defeatConditions": [],
  "events": [],
  "validationSuppressions": []
}
```

Current implementation note: the bounded slice does not yet load a full top-level `Scenario` content type. M12 outcome authoring is provided by `content/scenario_outcome.json`.

---

## 10. Scenario Region Context

Regions are reusable structural definitions. A Scenario controls how a Region behaves by passing Scenario context to it. Use **Scenario Region Context** rather than arbitrary shallow patching as the primary model.

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
- Scenario context should not mutate the reusable Region file directly.
- Save data owns runtime state changes after play begins.

---

## 11. World Map schema

A World Map owns:

- id
- localized name
- Region references
- Region adjacency / route graph
- unlock state
- travel metadata

World Map implementation is future scope. The current codebase is still a single-Region slice.

---

## 12. Region schema

A Region owns:

- id
- localized name
- localized description
- nodes
- routes / links
- arrival node
- local Region services
- local Locations
- optional presentation metadata

Region node ids should be stable and opaque.

---

## 13. Region node schema

A Region node owns:

- id
- localized name
- node type / content type
- optional content reference
- position/presentation hints
- flags such as arrival/safe-anchor/blocker/service/location where applicable

Node runtime state belongs in save data, not authored Region content.

---

## 14. Region node content schema

Node content should be a typed discriminated object.

Example service node:

```json
{
  "content": { "type": "service", "serviceId": "svc_old_farm" }
}
```

Example Location node:

```json
{
  "content": { "type": "location", "locationId": "location_old_house" }
}
```

Rules:

- content type controls which reference field is valid
- a node should not stack unrelated major roles unless a future rule explicitly supports it
- arrival behavior is a node flag, not a separate node type

---

## 15. Region blocker schema

Blockers are authored node or route constraints that prevent movement until a condition is satisfied.

Examples:

- key/password gate
- gold/resource payment gate
- quest-state gate
- hostile occupation at runtime

Runtime hostile occupation is not authored blocker content; it is derived from enemy-team state.

---

## 16. Region route schema

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

Travel time and Energy cost are computed from route quality and distance. Routes may be authored as hidden/inactive and later restored or activated by events. Events should not create new route definitions from nothing at runtime.

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

Location mode is event-driven. Location interactables are event sprites, not hard-wired Region services.

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
  "settings": { "maxSeedQuantity": 999, "allowFertilizer": true }
}
```

`settings` is a type-specific payload. Service state in content is initial state. Runtime state belongs in save data.

---

## 19. Region Services versus Location service calls

Region services are placed as node content.

Example:

```json
{
  "content": { "type": "service", "serviceId": "svc_old_farm" }
}
```

Location service calls are future event actions. Example:

```json
{
  "type": "callService",
  "serviceId": "svc_location_market_small"
}
```

Rules:

- Location service calls use Service instances.
- Location-called services use the same rules as Region services where applicable.
- Only whitelisted service types are callable from Location context.
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
  "trigger": { "type": "locationConfirm", "objectId": "obj_magic_stone" },
  "eligibility": { "teamKinds": ["human"] },
  "condition": { "type": "always" },
  "repeat": { "mode": "once" },
  "actions": []
}
```

Event-level message is allowed only as an optional shorthand for design convenience. All messages should ultimately call the same message-display system as `showMessage`.

---

## 21. Event triggers

Trigger is a typed object.

Examples:

```json
{ "type": "regionNodeEntry", "nodeId": "node_old_gate" }
```

```json
{ "type": "locationCollision", "objectId": "obj_boy" }
```

```json
{ "type": "locationConfirm", "objectId": "obj_magic_stone" }
```

```json
{ "type": "startOfDay" }
```

```json
{ "type": "neutralEncounterDefeated", "encounterId": "enc_bandits_01" }
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
  "timeWindow": { "startDay": 1, "endDay": 10 }
}
```

Eligibility is not the same as condition. Use eligibility for who may participate. Use condition for whether the world-state requirement is satisfied.

---

## 23. Condition schema

Conditions are typed objects.

### Currently implemented leaves

The current event/outcome condition evaluator supports exactly these leaf types:

- `always`
- `teamHasResource`
- `teamHasHero`
- `storyFlagSet`

It also supports the composite forms:

- `all`
- `any`
- `not`

Example implemented condition:

```json
{
  "all": [
    { "type": "teamHasResource", "resource": "Wood", "amount": 10 },
    { "type": "teamHasHero", "heroId": "hero_jon" }
  ]
}
```

```json
{
  "any": [
    { "type": "storyFlagSet", "flag": "flag_bridge_repaired" },
    { "type": "always" }
  ]
}
```

```json
{
  "not": { "type": "storyFlagSet", "flag": "ashvale_lost" }
}
```

Leaf conditions must include `type`. Composition conditions use `all`, `any`, or `not`.

### Future condition leaves

Future phases may add leaves such as `teamHasItem`, `serviceDestroyed`, route state, ownership, time limits, unit counts, Region revealed, or scenario variable checks. These are design targets only until code and validation explicitly add them.

---

## 24. Action schema

Actions are typed objects. Every action has a `type`.

Example:

```json
{ "type": "takeResource", "resource": "Wood", "amount": 10 }
```

Actions should be small, explicit, and validateable. Avoid generic script strings. Action failure behavior is normally determined by the target system rule, not by per-action configuration.

Event action handlers should fail explicitly when required runtime context or required arguments are missing. Silent no-ops are not acceptable for implemented action types.

### Currently implemented action types

The current event action executor/validator recognizes these action types:

- `showMessage`
- `giveResource`
- `takeResource`
- `setStoryFlag`
- `clearStoryFlag`
- `if`
- `spawnTeam`
- `removeTeam`
- `changeAlliance`

### Future action types

Future phases may add actions such as `giveItem`, `takeItem`, `giveArtifact`, `triggerVictory`, `triggerDefeat`, `callService`, route mutation, service destruction/restoration, ownership changes, or AI behavior changes. These are not current M12-supported action types unless code explicitly implements them.

### Enemy-team lifecycle actions

The following action types are implemented for enemy-team runtime mutations. They mutate runtime state, not authored content. Runtime changes must be persisted by save data.

#### `spawnTeam`

Creates or reactivates a runtime enemy team at a Region node.

```json
{
  "type": "spawnTeam",
  "teamColor": "Red",
  "nodeId": "node_forest_camp"
}
```

Required fields:

- `teamColor` — target team identity/color
- `nodeId` — Region node where the team should appear

Rules:

- the action requires a team-mutation context
- missing `teamColor` or `nodeId` is an explicit action failure
- validation should ensure `nodeId` exists in the relevant Region when enough context is available

#### `removeTeam`

Deactivates a runtime enemy team.

```json
{
  "type": "removeTeam",
  "teamColor": "Red"
}
```

Required fields:

- `teamColor` — target team identity/color

Rules:

- the action requires a team-mutation context
- missing `teamColor` is an explicit action failure
- removal affects runtime team state and must survive save/load

#### `changeAlliance`

Adds or removes an alliance relationship for a runtime team.

```json
{
  "type": "changeAlliance",
  "teamColor": "Red",
  "allyColor": "Blue",
  "add": true
}
```

Required fields:

- `teamColor` — team to mutate
- `allyColor` — team color to add/remove from the alliance set
- `add` — `true` to add the alliance, `false` to remove it

Rules:

- the action requires a team-mutation context
- missing `teamColor`, `allyColor`, or `add` is an explicit action failure
- changed alliances are runtime state and must survive save/load

---

## 25. If / Else action schema

`if` is an action type.

Example:

```json
{
  "type": "if",
  "condition": { "type": "teamHasResource", "resource": "Wood", "amount": 10 },
  "then": [
    { "type": "takeResource", "resource": "Wood", "amount": 10 },
    { "type": "showMessage", "text": { "en": "The market is rebuilt." } }
  ],
  "else": [
    { "type": "showMessage", "text": { "en": "You need more Wood." } }
  ]
}
```

Branches may be nested. Branches replace optional-action flags.

---

## 26. Message action schema

Messages should use `showMessage` where practical.

Example:

```json
{
  "type": "showMessage",
  "text": { "en": "Jon joins you." },
  "portraitId": "portrait_jon"
}
```

Quest-service message fields and event-level shorthand messages may exist for editor convenience, but they should call the same message-display code path.

---

## 27. Repeat schema

Use structured repeat data.

Examples:

```json
{ "repeat": { "mode": "once" } }
```

```json
{ "repeat": { "mode": "always" } }
```

```json
{ "repeat": { "mode": "everyNDays", "intervalDays": 7 } }
```

Automatic event priority is a number. Lower number means earlier. Priority must be unique within the same automatic trigger group.

---

## 28. Quest service schema

Quest services are Service instances with `serviceType: "questService"`.

Conceptual shape:

```json
{
  "id": "svc_old_man_quest",
  "serviceType": "questService",
  "settings": {
    "emptyMessage": { "text": { "en": "This place seems abandoned." } },
    "quests": []
  }
}
```

A quest service may have zero quests. Zero quests are valid.

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
  "eligibility": { "teamKinds": ["human"] },
  "objective": { "type": "teamHasResource", "resource": "Wood", "amount": 10 },
  "startingMessage": { "text": { "en": "Bring me 10 Wood." }, "portraitId": "portrait_old_man" },
  "progressMessage": { "text": { "en": "Still looking for Wood?" }, "portraitId": "portrait_old_man" },
  "completionPrompt": {
    "text": { "en": "Will you give 10 Wood?" },
    "portraitId": "portrait_old_man",
    "yesActions": [
      { "type": "takeResource", "resource": "Wood", "amount": 10 }
    ],
    "noActions": []
  }
}
```

Quest message fields are editor-facing shortcuts. They should use the same message-display function as `showMessage`.

---

## 30. Victory and defeat condition schema

Victory and defeat conditions use the shared condition model.

### Current authored shape (M12)

Current implementation accepts an optional `content/scenario_outcome.json`.

The file is a single document with the standard `schemaVersion` / `kind` / `id` identity fields plus two arrays. Each array element is an `EventCondition` tree using the same shape as event conditions (§23).

```json
{
  "schemaVersion": 1,
  "kind": "ScenarioOutcome",
  "id": "scenario_outcome",
  "victoryConditions": [
    { "type": "storyFlagSet", "flag": "ashvale_cleansed" }
  ],
  "defeatConditions": [
    { "type": "storyFlagSet", "flag": "ashvale_lost" }
  ]
}
```

Rules:

- Missing or empty file is legal — default victory fires when no hostile teams remain.
- Empty `victoryConditions` is legal — default victory remains active.
- A non-empty `victoryConditions` list **disables default victory entirely**; designers must satisfy one of the authored conditions to win.
- `victoryConditions` is OR-structured: any one match ends the scenario in victory, unless a defeat condition also matches.
- `defeatConditions` is OR-structured: any one match ends the scenario in defeat.
- Defeat wins over victory if both match in the same evaluation.
- Supported condition leaves are the four shared with events: `always`, `teamHasResource`, `teamHasHero`, `storyFlagSet`, plus the `all` / `any` / `not` composites.
- Richer outcome leaves are deferred.
- Per-scenario authoring through a full top-level `ScenarioDefinition` content kind with embedded victory/defeat blocks is deliberately deferred until campaigns or multi-scenario play arrive.

### Long-term victory/defeat model

Long-term Scenario definitions may embed richer victory/defeat blocks, eligible teams, affected teams, and victory/defeat event chains. Those are design targets only. M12 only latches a terminal outcome and shows placeholder status feedback; it does not run victory event chains.

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
    "patrol": { "enabled": true, "centerNodeId": "node_camp", "radius": 3 }
  }
}
```

Authored team data defines initial state. Runtime fields such as current node, active/inactive state, cooldown, energy, and changed alliances belong in save data.

---

## 32. Unit stack and hero instance schema

Generic unit stack:

```json
{ "unitId": "unit_bandit", "quantity": 12 }
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

Hero id is enough as stable identity because heroes are unique pool entities. Do not create a separate hero instance id unless the design later supports duplicate hero identities.

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

Food is a normal item subtype. Food must be field-use only and hero-consumed.

---

## 34. Effects schema

Items should use shared typed effects. Artifacts should use typed modifiers and special effect enums.

Example item effect:

```json
{ "type": "recoverHp", "target": "hero", "amount": 50 }
```

Example artifact modifier:

```json
{ "type": "statBonus", "stat": "Attack", "amount": 2 }
```

Example artifact special effect:

```json
{ "type": "specialEffect", "effect": "DisableAllUsableSkillsInBattleForBothTeams" }
```

Special effects are enum-driven and handled by code. Artifacts may have multiple bonuses and effects.

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
    { "type": "statBonus", "stat": "Attack", "amount": 2 }
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

A specific artifact handler service may deny selected recipes. The global recipe remains valid unless the recipe itself is invalid.

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
    { "kind": "QuestService", "id": "svc_old_gate" }
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

Suppression must include a reason. Suppressing one warning must not suppress unrelated future warnings.

---

## 40. Player character schema

The player character is a normal unique hero unit with special human-team rules. For human teams, `playerCharacterHeroId` lives on the Team object.

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

Before a Scenario or Campaign starts, the player creates the player character. Character creation fills the stable player-character hero identity with:

- name
- sex
- simple graphical representation
- starting stats
- starting skills
- starting preset/template

Starting presets such as Warrior, Builder, and Explorer are presets only. They are not permanent class restrictions.

### Campaign carry-over

The player-character identity always carries over in Campaigns. Campaign transition rules may still affect progression details such as level, skills, passive skills, equipment, or artifacts according to the normal carry-over rules.

### Runtime boundary

Player-character authored identity and creation defaults belong in content / campaign-start data. Current HP, KO recovery, equipment state, level changes, learned skills, and other playthrough state belong in save data.

---

## 41. Runtime save data boundary

Authored content defines initial state. Save data owns runtime state.

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
- temporarily unavailable heroes
- enemy-team current node, active/inactive state, energy, cooldown, and runtime alliances
- latched scenario outcome

Do not write runtime progression state back into authored content files. Editor tools may edit authored initial state, not live save state, unless explicitly in a save-editor mode.

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
- make implemented event actions fail explicitly when required context/arguments are missing
- do not document future conditions/actions as implemented until code and validation actually support them
- follow `docs/presentation_game_feel.md` for presentation asset ids and presentation event-action boundaries

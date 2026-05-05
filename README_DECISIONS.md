# README_DECISIONS

## Current design decisions

### 1) Keep architecture explicit and small

Decision:
- Use a small, explicit state-machine scaffold with a thin app shell.
- Preserve the current controller / mapper / renderer split.
- Keep shared HUD and debug overlays small and readable.

Why:
- Avoids overengineering while keeping the project extendable.
- Keeps gameplay flow readable and testable.
- Makes AI-assisted iteration safer.

### 2) Keep pure gameplay logic separate from rendering and input

Decision:
- Keep gameplay rules and state transitions out of rendering code and raylib glue.
- Prefer pure gameplay modules for rules, legality, progression, travel, combat outcomes, and economy.
- Treat rendering as presentation and app code as orchestration.

Why:
- Makes the project easier to test.
- Reduces accidental coupling.
- Helps future milestone work remain incremental.

### 3) Use current design terminology as the source of truth

Decision:
- Use current design terms consistently:
  - **World Map**
  - **Region**
  - **Location**
  - **Service**
  - **Traveling party**
  - **Stored units**
  - **Temporarily Unavailable**
- Treat `docs/terminology_map.md` as the terminology source of truth.
- Treat older runtime/content names as legacy compatibility language, not design truth.

Why:
- Prevents confusion between current design and older implementation history.
- Helps future agent work avoid reintroducing outdated terms.

### 4) Keep battle as static formation CTB

Decision:
- Battle is static formation-based CTB, not free battlefield movement.
- Positions are:
  - Front
  - Middle
  - Back
  - Leader
- Position changes are explicit actions that consume the acting unit’s turn.

Why:
- Supports readable, deterministic-feeling tactics.
- Keeps the battle model clean and content-driven.

### 5) Keep the leader inside the active 5-unit party

Decision:
- The active party size is 5.
- The leader occupies one of those 5 slots.
- Only hero units may be leaders in intended design.
- The player team must always have a legal leader.
- Enemy teams may be leaderless.

Why:
- Keeps roster, battle, and leadership rules coherent.
- Makes leader choice strategically meaningful without inflating party size.

### 6) Keep battle readable and mostly deterministic

Decision:
- Only damage roll randomness is core by default.
- Show turn order, target preview, min/max damage, and min/max KO preview.
- Prefer deterministic passives and effects over hidden proc-heavy randomness.

Why:
- Supports tactical planning.
- Keeps combat legible and fair.

### 7) Keep roster structure as active + reserve + stored

Decision:
- **Active party** = current battle-legal party, up to 5.
- **Reserve** = traveling non-active roster, up to 7.
- **Stored units** = units assigned to a specific storage Service, up to 7 per storage.
- **Traveling party** = active + reserve.

Why:
- Gives clear logistics and party-management structure.
- Supports strategic travel decisions without overcomplicating the model.

### 8) Keep cross-Region travel costly and consequential

Decision:
- Region-to-Region travel happens from the **World Map**.
- It requires **1000 Energy** paid once when travel begins.
- It consumes days based on shortest valid Region path distance.
- It must begin before 11:00.
- Heroes in the traveling party travel between Regions.
- Generic units in the traveling party do not survive Region change unless stored first.

Why:
- Makes Region travel strategic.
- Forces real roster and logistics decisions.
- Prevents Regions from feeling like free menus.

### 9) Keep Energy as a shared traveling-party resource

Decision:
- Energy belongs to the traveling party, not to individual units.
- Daily starting Energy is:
  - `1000 + (lowest traveling-party agility × 100) + leader passive bonus + leader item bonus`
- Energy is restored by rest, Services, items, and events.
- Rest is a deliberate travel action:
  - 3 hours
  - 300 Energy

Why:
- Creates a clear HoMM-like movement resource without per-unit bookkeeping.
- Makes party composition matter outside battle.

### 10) Keep Regions as authored node graphs with systemic rules on top

Decision:
- A Region is an authored node graph.
- The world rules layered on top are systemic:
  - movement
  - route quality
  - Energy cost
  - node occupation
  - enemy-team behavior
  - ownership
  - service use
  - event-driven progression
- Keep this “authored structure + systemic rules” model.

Why:
- Gives Scenarios strong authored identity.
- Still allows replayable and emergent world pressure.

### 11) Use a node-content model

Decision:
- Region nodes are fundamentally travel points.
- A node's gameplay behavior is determined by its main node content and any attached events.
- A node may contain at most one main content item, such as:
  - resource pickup
  - artifact pickup
  - Service
  - neutral enemy
  - one-time special content
- There is no dedicated permanent combat-node type.
- Blocker behavior is usually created by content, such as a gate service, neutral enemy, hostile team occupation, or authored rule.
- Arrival is a flag, not a node type.

Why:
- Keeps Region structure understandable.
- Avoids a bloated fixed node-type hierarchy.
- Keeps blockers, pickups, Services, and neutral encounters validateable as content.

### 12) Keep Location and direct Service distinct

Decision:
- **Direct Service node** = quick functional interaction on the Region layer.
- **Location** = entered authored place with NPCs, screens, multiple Services, or dungeon-like content.
- Dungeons are a type of Location.
- Entering and exiting a Location does not cost time.

Why:
- Preserves a clear difference between strategic map play and authored place exploration.
- Avoids bloating the Region layer with pseudo-locations.

### 13) Define safe anchor narrowly

Decision:
- A Location is a **safe anchor** if it provides:
  - free rest
  - guaranteed rest
- Nothing else is required.

Why:
- Keeps the concept sharp.
- Avoids turning “Home Base” into a fuzzy universal category.
- Supports authored variety across Scenarios.

### 14) Protect arrival nodes

Decision:
- Every Region must have an arrival node.
- Arrival nodes may also be Location or single Service nodes.
- Enemies may not:
  - spawn there
  - occupy there
  - block arrival there

Why:
- Makes Region travel reliable.
- Prevents frustrating arrival traps.

### 15) Keep enemy teams as authored setups with systemic behavior

Decision:
- Enemy teams are AI-controlled traveling parties on the Region layer.
- Designers author:
  - team color
  - starting node
  - template
  - alliances
  - patrol radius
  - personality
  - aggression level
- The game system then handles:
  - movement
  - occupation
  - service use
  - recruitment
  - sabotage
  - attacks
  - world competition

Why:
- Gives strong authored control without hardcoding each team’s full behavior.
- Fits the broader Region design philosophy.

### 16) Use fixed enemy action order and one-action enemy phases

Decision:
- After each player action that costs time, eligible enemy teams in the same Region act.
- Enemy action order is fixed by team color.
- Each eligible enemy team gets exactly one action in that phase.
- Enemy actions do not advance the real game clock.
- Enemy cooldowns, such as rest, are measured against the real game clock.

Why:
- Keeps the simulation understandable.
- Prevents runaway enemy activity after longer player actions.
- Supports deterministic-feeling world behavior.

### 17) Keep enemy teams under the same broad world rules where practical

Decision:
- Enemy teams use the same broad rules for:
  - Energy
  - movement legality
  - recruitment competition
  - hero-pool competition
  - service use where applicable
- Enemy teams do not:
  - travel between Regions
  - enter Locations
  - suffer sleep / wake-up penalties

Why:
- Makes the world feel fair and systemic.
- Preserves a useful distinction between player-specific and AI-specific rules.

### 18) Use personality + aggression + patrol as the main AI behavior knobs

Decision:
- Baseline AI personalities:
  - Warrior
  - Builder
  - Explorer
- Baseline aggression levels:
  - Berserk
  - Reckless
  - Opportunistic
  - Careful
  - Coward
  - Pacifist
- Patrol radius is an authored movement constraint.

Why:
- Gives enough expressiveness for varied enemy behavior.
- Avoids prematurely overcomplicating the AI model.

### 19) Keep team relations binary: ally or enemy

Decision:
- Teams are either allies or enemies.
- There is no middle diplomacy state.
- Team relations may change only through authored events.

Why:
- Keeps world simulation and ownership rules simpler.
- Makes Scenario-authored political shifts more legible.

### 20) Let allied teams share nodes, but not ownership

Decision:
- Allied teams may share a node.
- Allied teams do not automatically share:
  - node ownership
  - service ownership
  - service-use rights
  - recruit ownership
  - storage ownership

Why:
- Keeps alliances useful but competitive.
- Prevents ownership systems from becoming muddy.

### 21) Resolve shared-node combat one battle at a time

Decision:
- When multiple allied teams share a node and a hostile team attacks:
  - resolve one battle at a time
  - attacker chooses target order
  - attacker only enters the node if all remaining hostile teams there are defeated or allied
- Otherwise the attacker remains on the previous node.

Why:
- Keeps multi-team nodes tractable.
- Avoids inventing multi-party battle rules too early.

### 22) Keep stationary hostile encounters neutral

Decision:
- Temporary hostile node encounters are always neutral.
- They are not owned by a colored team.
- Once cleared, the node becomes an empty travel node.
- Defeating one may also trigger an event-action chain if authored to do so.

Why:
- Separates world-neutral danger from team-owned territorial conflict.
- Makes neutral encounters usable as progression hooks.

### 23) Support sabotage and denial as core Region-layer strategy

Decision:
- Region-layer competition includes:
  - attacking teams
  - occupying nodes
  - blocking access
  - denying recruits
  - capturing mines and owned assets
  - attacking storage gates
  - destroying destroyable Region Services
- Sabotage is not a separate rules layer; it is part of trying to win the Scenario.

Why:
- Makes the Region layer feel alive and contested.
- Gives enemy teams meaningful strategic pressure beyond direct battle.

### 24) Keep service destruction a Region-layer rule only

Decision:
- Only Region Services are part of the default systemic destruction model.
- Services inside entered Locations are not systemically destroyable by default.
- A destroyable Region Service may be destroyed only by the occupying team, unless a special ranged destroy effect explicitly overrides that.
- Destruction costs:
  - 1000 Energy
  - 1 hour

Why:
- Keeps sabotage readable and bounded.
- Prevents Location content from becoming too fragile by default.

### 25) Queue service restoration until the next day

Decision:
- Restoring a service costs resources, but no Energy.
- Restoration does not complete immediately.
- The service is flagged for restoration and is restored automatically at the start of the next day.
- Queued restoration may be cancelled by destroying the service again before the next day begins.

Why:
- Makes restoration feel like local rebuilding, not magical instant repair.
- Adds interesting pressure around service denial.

### 26) Keep sanctuary as protected non-combat Region service

Decision:
- Sanctuary is a single Service node.
- Combat may not be initiated on a sanctuary node.
- Any team occupying a sanctuary node may not be attacked there.
- Sanctuary may still be destroyable if flagged as destroyable.

Why:
- Creates meaningful protected spaces on the Region layer.
- Adds route tension and positional play without requiring a full Location.

### 27) Treat storage gates as strategic assets, not passive storage menus

Decision:
- Direct Region storage nodes are defensible gates.
- Stored units are the defenders if attacked while no traveling party is present.
- If a traveling party is present, its active party defends.
- If the storage is lost, all stored units are dismissed.
- Stored heroes become Temporarily Unavailable through the normal pipeline.

Why:
- Makes storage a real strategic choice.
- Gives the world meaningful logistical vulnerability.

### 28) Keep ownership transfer immediate

Decision:
- Ownership of owned nodes or Services transfers immediately after capture.
- Current intended ownership examples include:
  - storage gates
  - mines
  - other authored owned Region assets

Why:
- Keeps control readable.
- Supports fast-changing strategic pressure.

### 29) Keep mines as capturable daily-resource assets

Decision:
- Mines and similar nodes generate resources over time.
- They may be free or guarded.
- Guarded mines require defeating their guardians to take control.
- Teams may station units there.
- Mines do not inherently block movement.

Why:
- Adds persistent strategic objectives to Regions.
- Supports territorial competition without overcomplicating node content rules.

### 30) Keep recruitment and hero availability globally competitive inside a Scenario

Decision:
- Generic-unit recruitment pools belong to the Service itself and may accumulate up to 4 weeks of growth.
- Hero-recruit Services draw from the shared Scenario hero pool.
- If any team recruits a hero, that hero becomes unavailable to all other teams until lost and returned to the pool.

Why:
- Makes recruitment a real competitive layer.
- Reinforces that other teams are active participants in the world.

### 31) Support Sealed / Frozen Hero services as one-time roster rewards

Decision:
- A Sealed Hero / Frozen Hero service is a one-time Region Service that frees a hero in a special Sealed state.
- Freeing costs:
  - 500 Energy
  - 1 hour
- The move is illegal if the acting team does not have room for the hero.
- The freed hero joins using the same placement rules as normal hero recruitment.
- The node becomes an empty travel node afterward.

Why:
- Adds authored hero-discovery moments to the Region layer.
- Supports Scenario identity and competitive hero access.

### 32) Keep player-involved Region battles as auto-resolve-first

Decision:
- Region-layer battles involving the player first produce an auto-resolve result.
- The player may accept it or replay the battle manually.
- The player may retry from the original pre-battle state.

Why:
- Keeps multi-team world play from becoming too slow.
- Preserves tactical control when the player wants it.

### 33) Keep AI-vs-AI battles auto-resolved and information-limited

Decision:
- AI-vs-AI battles are always auto-resolved.
- Their results are not shown directly to the player.
- Enemy movement and AI-vs-AI outcomes are visible only in revealed parts of the Region.

Why:
- Supports fog of war.
- Keeps pacing manageable.

### 34) Keep defeat and hero-loss rules symmetric where possible

Decision:
- Enemy teams follow the same broad hero availability rules as the player side.
- Heroes are not permanently tied to one team.
- If a team loses a hero, that hero may later return to the shared pool through the normal Temporarily Unavailable rules.
- Enemy teams may remain leaderless if allowed.
- The player character is a special exception and follows player-character recovery and defeat rules instead of the normal Temporarily Unavailable / recruitable hero-pool loop.

Why:
- Keeps the simulation coherent.
- Prevents separate hero economies from forming for player and AI unless explicitly designed later.

### 35) Treat events as the universal progression engine

Decision:
- Use **events** as the core trigger / condition / effect system for Scenario progression.
- Quests, guidance, alliance changes, Region unlocks, node/world-state changes, victories, defeats, and many authored consequences should be expressible through typed event actions.

Why:
- Gives one coherent progression backbone.
- Prevents scenario logic from being split across too many unrelated systems.

### 36) Keep quests as one specific authored service structure, not the universal progression model

Decision:
- A **quest** is a single authored task.
- A **quest service** is a map service that exposes at most one currently available quest from a chain at a time.
- Quest chains are ordered lists of quests inside one quest service.
- Quest-service quests are always turn-in quests.
- Quests are optional by default unless a victory condition depends on them.

Why:
- Keeps quests concrete and readable.
- Avoids overloading the quest system with all progression logic.

### 37) Separate quest, objective, victory condition, and event as distinct concepts

Decision:
- **Quest** = a single authored task
- **Objective** = a typed requirement used by a quest or victory condition
- **Victory condition** = a Scenario-level win rule
- **Defeat condition** = a Scenario-level loss rule
- **Event** = the trigger / condition / effect system that changes world state

Why:
- Reduces terminology confusion.
- Gives the progression model cleaner boundaries.

### 38) Keep eligibility and condition as separate concepts

Decision:
- **Eligibility** determines who is allowed to participate or trigger.
- **Condition** determines whether the actual quest, event, or victory requirement is satisfied.

Why:
- Makes quest services and events much easier to reason about.
- Prevents “who may interact” from being confused with “what must be true.”

### 39) Keep quest-service messages and turn-in choices explicit

Decision:
- Quest-service quests use:
  - Starting message
  - Progress message
  - Completion message
- The completion message includes a **Yes / No** choice.
- **Yes** completes the quest and triggers its event-action chain.
- **No** leaves the quest unfinished for now.

Why:
- Makes turn-in an explicit player/team choice.
- Supports more deliberate pacing and quest handling.

### 40) Keep quest services competitive between teams

Decision:
- Any eligible team may complete a quest service before another.
- Unless the quest service is a repeatable blocker/toll-style quest guard, a completed quest is gone permanently.
- This means enemy teams may invalidate player-relevant quests by completing them first.

Why:
- Makes Scenarios feel contested.
- Prevents quest content from behaving like private single-player scripting unless explicitly authored that way.

### 41) Keep repeatable quest guards as an intentional authored special case

Decision:
- Only blocker-style quest guards are repeatable by default design.
- Repeatable quest guards repeat:
  - the same condition
  - the same message
  - the same event actions
- This is a deliberate authored choice and may function like a toll or farm.

Why:
- Keeps ordinary quest services finite.
- Preserves a useful HoMM-like guard/toll structure.

### 42) Keep player-facing quest states simple

Decision:
- Technical quest state is primarily completed or not completed.
- Player-facing quest-log states are:
  - Undiscovered
  - Visible in log
  - Completed
  - Failed
- Failed quests remain visible in a failed section of the log.

Why:
- Keeps system logic simple while still giving the player useful status feedback.

### 43) Keep objective types strongly typed and finite

Decision:
- Quest and victory requirements should use an explicit typed list of conditions.
- Avoid turning objectives into a free-form condition language.

Why:
- Makes content safer to author.
- Keeps AI-agent work more predictable.

### 44) Keep victory conditions outside the quest system

Decision:
- Victory conditions are Scenario-level rules, not quests.
- A victory condition may depend on a quest service, but it is still structurally separate.
- A Scenario may have one or more victory conditions.
- Only one victory condition needs to be satisfied to win.

Why:
- Keeps win logic clear.
- Prevents quest framing from over-defining Scenario structure.

### 45) Keep defeat conditions outside the quest system and OR-based

Decision:
- Defeat conditions are Scenario-level loss rules.
- If any defeat condition becomes true, that team loses.
- A Scenario may have no special defeat conditions, or several.

Why:
- Makes failure rules easy to reason about.
- Keeps defeat logic parallel with victory logic.

### 46) Allow authored guidance to be bypassed by true victory

Decision:
- Guidance text, quest chains, and intended story paths may be bypassed if a real victory condition is satisfied directly.

Why:
- Keeps Scenario design flexible.
- Supports emergent or skillful wins without requiring players to follow all intended breadcrumbs.

### 47) Keep guidance, journal, and formal victory/defeat info separate

Decision:
- **Scenario Info screen** = formal victory and defeat conditions
- **Quest log / journal** = discovered quest-service tasks and status
- **Guidance text** = event-driven directional hint layer that persists until changed

Why:
- Gives the player three distinct information channels with different purposes.
- Prevents objective clarity from depending on only one UI surface.

### 48) Keep event actions immediate

Decision:
- When an event triggers, its event-action chain resolves immediately unless a specific action itself explicitly schedules future behavior.

Why:
- Makes progression consequences readable.
- Keeps cause and effect clear.

### 49) Keep manual and automatic events as the two main event families

Decision:
- **Manual events** are triggered by an eligible team entering or using a specific node or service.
- **Automatic events** trigger either:
  - on specified days
  - or when typed conditions are checked at the start of the day
- Manual events are one-shot by default unless explicitly marked repeatable.

Why:
- Provides a clean event taxonomy.
- Covers most authored scenario needs without overcomplication.

### 50) Keep campaign carry-over authored per transition

Decision:
- Carry-over is authored per Scenario transition from a fixed allowed list.
- Candidate carry-over types include:
  - heroes
  - generic troops from the traveling party
  - items
  - resources
  - story flags
  - hero level + attributes
  - hero skills
  - hero passive skills
  - equipment / artifacts
- Story flags are always valid carry-over data.

Why:
- Keeps campaigns flexible.
- Prevents one rigid carry-over rule from constraining all future campaigns.

### 51) Keep a shared team-global economy within each Scenario

Decision:
- Resources belong to the team globally, not to individual units or locations.
- Resources are shared across all Regions inside a Scenario.
- Resources may or may not carry over to the next Scenario depending on authored transition rules.

Why:
- Keeps economic state easy to reason about.
- Supports Region travel without fragmenting the economy.

### 52) Use a fixed baseline resource set

Decision:
- The baseline resource set is:
  - Gold
  - Wood
  - Stone
  - Steel
  - Fiber
  - Clay
  - Gems

Why:
- Gives the economy a stable content vocabulary.
- Supports authored scarcity, trader behavior, quests, and owned-resource services.

### 53) Treat gold as a normal resource with special practical importance

Decision:
- Gold is structurally a normal resource.
- In practice it is gathered at a much larger scale than the other resources and acts as the main universal currency.

Why:
- Keeps the model simple.
- Still supports gold’s special strategic role.

### 54) Use battle loss, surrender, and theft as part of the economy

Decision:
- Team-vs-team battle victory grants:
  - all defeated-team items
  - all defeated-team artifacts
  - 1/4 of all defeated-team non-gold resources
- Gold may be stolen directly by specific battle skills.
- Consumable items may also be stolen by specific skills.
- Escape and surrender are valid alternatives to full defeat and have their own economic consequences.

Why:
- Makes battle matter economically.
- Connects the Region layer to roster and inventory consequences.

### 55) Keep trader services specialized, not generic

Decision:
- Use distinct economy services with one primary role each:
  - **Trading Post**
  - **Market**
  - **Freelancer’s Guild**
  - **Black Market**
- Avoid collapsing all economy functionality into one abstract merchant.

Why:
- Makes authored world services more flavorful.
- Keeps each service role easy to understand.

### 56) Use two separate trade systems at Trading Posts

Decision:
- Trading Posts use:
  - a **gold-based trade model**
  - a separate **resource-for-resource barter model**
- Do not derive all barter through gold.
- Allow Scenario-level defaults to be overridden by specific Trading Posts.

Why:
- Produces cleaner and more controllable exchange behavior.
- Allows authored economy variety without breaking the overall model.

### 57) Keep gold trade intentionally lossy by default

Decision:
- Use updated base values:
  - Wood / Stone = 100 gold
  - Steel / Fiber / Clay = 200 gold
  - Gems = 500 gold
- Default gold buy rate = 5× base value
- Default gold sell rate = 1/5 base value

Why:
- Makes direct gold conversion expensive by design.
- Preserves resource scarcity and strategic tradeoffs.

### 58) Use tier-based default barter rates for non-gold resource exchange

Decision:
- Treat:
  - Wood / Stone as Tier 1
  - Steel / Fiber / Clay as Tier 2
  - Gems as Tier 3
- Default buy costs follow the intended punitive barter model:
  - buying 1 Tier 1 costs 10 / 5 / 2 in Tier 1 / 2 / 3
  - and the full table is derived consistently from that structure
- A resource cannot be exchanged for itself.

Why:
- Gives a readable, harsh default barter economy.
- Loosely follows the gold-value structure without collapsing into it.

### 59) Charge time, not Energy, for trade actions

Decision:
- Trading actions consume time but not Energy.
- If at least one transaction happened before exiting the service screen, 20 minutes pass.
- If nothing was exchanged, no time passes.

Why:
- Keeps trade meaningful without turning it into a movement action.
- Prevents free high-frequency market abuse.

### 60) Keep team-to-team resource transfer instant at Trading Posts

Decision:
- Resources sent to another team through a Trading Post arrive instantly.
- If the receiving team is human-controlled, the game should show a message about the transfer.

Why:
- Keeps diplomacy and economic aid easy to understand.
- Avoids unnecessary delivery simulation.

### 61) Split inventory into item bag and artifact bag

Decision:
- Use:
  - a shared **item inventory**
  - a shared **artifact inventory**
  - per-hero equipped artifact slots

Why:
- Keeps consumables/utilities separate from hero gear.
- Makes inventory rules easier to understand.

### 62) Keep artifacts as the only true equipment layer

Decision:
- There is no separate generic “equipment” category.
- Equippable hero gear is handled entirely through **artifacts**.

Why:
- Simplifies inventory and reward structure.
- Lets legendary weapons, armor-like objects, and special relics all live in one system.

### 63) Make artifacts hero-only and slot-based

Decision:
- Only heroes may equip artifacts.
- Heroes have:
  - 1 Attack slot
  - 1 Defense slot
  - 3 Misc slots
- Artifact effects apply while equipped.

Why:
- Makes hero customization meaningful without spreading gear rules across all unit types.
- Keeps artifact design expressive but bounded.

### 64) Distinguish general item subtypes clearly

Decision:
- General items include:
  - consumables
  - stackable utility / trigger items
  - seeds
  - ingredients
  - food
- Consumables may be battle-use, field-use, or both.
- Food is a hero-consumed buff/recovery category.
- Seeds and ingredients support farming / food systems.

Why:
- Gives the item system enough structure for future expansion.
- Avoids lumping every non-artifact object into one vague category.

### 65) Keep inventory effectively unlimited with per-type caps

Decision:
- Team inventory is effectively unlimited overall.
- Per-type limits still apply:
  - consumables: at most 1 per identical type
  - artifacts: stack to 999 per identical type
  - other stackable items: stack to 999 per identical type

Why:
- Avoids tedious bag-slot management.
- Preserves item-identity rules where useful.

### 66) Keep artifact combination irreversible

Decision:
- Artifacts may be combined into stronger artifacts at a dedicated service.
- Combination is irreversible.
- The source artifacts are consumed permanently.

Why:
- Makes artifact progression meaningful.
- Prevents easy crafting reversals and exploit loops.

### 67) Let heroes keep equipped artifacts when leaving the team for non-team-loss reasons

Decision:
- If a hero leaves the team for a reason other than being defeated by another team, the artifacts equipped on that hero leave with them.
- If a team is defeated by another team, the winner gets the losing team’s artifacts and items.

Why:
- Keeps hero identity and equipment tied together.
- Makes team-vs-team defeat a major economic consequence.

### 68) Allow discarding and permanent consumption where authored rules permit

Decision:
- Consumables are gone immediately when used.
- Artifacts used in combination are gone permanently.
- Artifacts may be discarded unless doing so would violate Scenario-specific hard requirements such as victory conditions.

Why:
- Keeps inventory management flexible.
- Supports authored scenario constraints without banning discard globally.

### 69) Treat mines as the baseline owned resource service

Decision:
- Use **owned resource service** as the broader term.
- Mines are the baseline example for now.
- Default mine payouts are:
  - Wood = 2/day
  - Stone = 2/day
  - Gold = 1000/day
  - Steel / Fiber / Clay = 1/day
  - Gems = 1/day
- Specific mines may override these defaults by authored design.

Why:
- Gives the economy a clear foundational owned-node model.
- Leaves room for future owned-resource service variants.

### 70) Keep stationed mine units defensive only

Decision:
- Units stationed at a mine affect defense only, not production.

Why:
- Keeps production rules simple.
- Prevents defensive assignment from becoming an economic multiplier system.

### 71) Let resources, items, and artifacts participate directly in progression

Decision:
- Quests, events, and victory/defeat conditions may:
  - require resources
  - consume resources
  - require items
  - consume items
  - require artifacts
  - consume or deliver artifacts where authored

Why:
- Makes the economy truly part of Scenario identity.
- Supports legendary-objective and logistics-driven content.

### 72) Keep Scenario-defining items and artifacts as a first-class design tool

Decision:
- Some items and artifacts should be allowed to be central to Scenario identity, quests, or victory conditions.

Why:
- Supports memorable authored scenarios.
- Lets economy and story interact meaningfully.

### 73) Keep PvP separate from the main single-player design path for now

Decision:
- PvP may exist later as a separate mode.
- It should not currently drive the main single-player architecture or design wording unless a task explicitly focuses on PvP.

Why:
- Keeps current design work focused.
- Avoids mixing alternate timing and battle-flow assumptions into the main loop too early.


### 74) Keep Location construction event-driven rather than hard-wired

Decision:
- In **Location mode**, service construction, restoration, and upgrade should primarily happen through **events**.
- A Location interaction may build a new service, restore a ruined one, upgrade an existing one, or reveal/enable a service.
- Do not assume one universal hard-wired construction subsystem for Locations.

Why:
- Preserves the distinction between strategic Region services and authored Location interactions.
- Lets Locations feel like evolving places rather than small adventure-map overlays.

### 75) Keep Region services more explicit and Location services more flexible

Decision:
- **Region-mode services** are usually explicit placed services selected from a predefined service list.
- **Location-mode services** are usually event-triggered and may call default service flows from authored interactions.

Why:
- Preserves the intended split between:
  - HoMM-like strategic Region interaction
  - FF-like authored Location interaction

### 76) Treat Location-built and Location-restored services as persistent world-state

Decision:
- When a Location event builds, restores, or upgrades a service, that result persists in the Scenario until changed again by later events.
- The result is shared world-state and must save/load like other meaningful world-state changes.

Why:
- Makes Location evolution meaningful.
- Prevents Location improvements from feeling temporary or cosmetic.

### 77) Keep Location construction effectively human-only

Decision:
- Only human teams interact with Location construction/restoration/upgrade flows, because AI teams do not enter Locations.

Why:
- Matches the current location-access model.
- Avoids implying a hidden AI Location simulation that does not exist.

### 78) Keep farming as both a Region service and a Location event-driven flow

Decision:
- Farming exists in two forms:
  - a default **Region farming service**
  - **Location farming** triggered through events that call the same broader farming flow

Why:
- Supports risky strategic farming on the Region layer.
- Also supports safer or more authored farming inside Locations.

### 79) Make Region farming contested and risky by design

Decision:
- Region farming services are not private production spaces.
- They may be:
  - guarded
  - sabotaged
  - contested
  - stolen from if left exposed
- Another team may collect finished crops if it gains access to the service.

Why:
- Makes farming part of Region-level economic pressure rather than a passive background timer.

### 80) Limit one farming process to one seed type at a time

Decision:
- A farming service may process only one seed type at a time.
- The process quantity is authored, with a high default cap rather than a tiny fixed limit.

Why:
- Keeps the farming flow readable.
- Avoids unnecessary mixed-crop bookkeeping inside a single process.

### 81) Keep fertilization deterministic and chosen only at planting

Decision:
- Fertilization is:
  - optional by default
  - applied only when planting
  - not added later if skipped
  - one fertilizer per process
- Seed choice plus fertilizer choice determine outcome.

Why:
- Keeps farming predictable and strategic.
- Avoids fiddly mid-process intervention systems.

### 82) Keep crop care as once-per-day service support, not a consumable cost

Decision:
- Watering/care costs time, not resources.
- It is allowed once per day per farming service, shared across teams.
- It improves the next day’s growth progress rather than permanently modifying the crop.

Why:
- Keeps farming support simple.
- Makes care a meaningful logistical choice instead of another inventory sink.

### 83) Keep finished crops persistent until collected

Decision:
- Finished farming output remains in the service until collected.
- It does not expire by default.

Why:
- Keeps farming readable and less punitive.
- Makes guarding and timing matter more than arbitrary spoilage.

### 84) Keep cooking as a party-menu system, not a world-service dependency

Decision:
- Cooking is available anywhere outside battle while inside a Scenario whenever the party menu is available.
- Do not require a map service or Location interaction just to cook.

Why:
- Keeps cooking integrated with travel logistics.
- Avoids excessive friction for a basic preparation system.

### 85) Keep recipes globally visible, but filterable by current feasibility

Decision:
- Recipes are globally visible from the start.
- The player may filter to see only currently makeable recipes.
- Some recipes may still require passive or secondary skills from the traveling team.

Why:
- Reduces discovery ambiguity.
- Still allows meaningful preparation gating through ingredients and skills.

### 86) Keep food as a hero-facing field-use layer

Decision:
- Food is consumed only by heroes.
- Food is a field-use system, not a battle-item system.
- Food may provide:
  - recovery
  - next-battle buffs
  - day/week duration buffs
  - combinations of these

Why:
- Keeps food distinct from ordinary battle consumables.
- Makes food part of planning and logistics rather than in-combat micromanagement.

### 87) Keep cooking and artifact combination as the only crafting systems for now

Decision:
- The only intended crafting systems at this stage are:
  - **cooking**
  - **artifact combination**
- Do not expand into broad generalized crafting by default.

Why:
- Keeps system scope controlled.
- Supports meaningful depth without opening a full crafting sandbox.

### 88) Keep artifact handling narrowly focused

Decision:
- Artifact-handling services exist only to combine artifacts.
- They do not dismantle, repair, or broadly craft other item types.

Why:
- Keeps artifact progression clean.
- Avoids feature creep in the artifact layer.

### 89) Keep artifact combination recipes globally fixed but service-filterable

Decision:
- Artifact combination recipes are globally fixed.
- A specific artifact-handling service may allow all recipes or deny selected recipes by authored design.

Why:
- Gives a stable artifact-combination language.
- Still allows Scenario- or Location-specific restriction.

### 90) Keep artifact combination irreversible

Decision:
- Artifact combination permanently consumes the source artifacts.
- There is no dismantling back into inputs.

Why:
- Makes combination a meaningful commitment.
- Prevents infinite combine/dismantle loops.

### 91) Allow artifact-handling services in both Regions and Locations

Decision:
- Artifact combination may exist as:
  - a direct Region service
  - or a Location event/service call

Why:
- Preserves flexibility in world authorship.
- Fits the broader distinction between Region hard-wiring and Location event-driven flows.

### 92) Keep these systems optional authored layers, not universal scenario requirements

Decision:
- Location building, farming, cooking, and artifact handling are optional authored systems.
- Scenarios may use none, some, or many of them.

Why:
- Prevents the game from requiring every economic and support subsystem in every Scenario.
- Supports strong Scenario identity and variety.

### 93) Use medium-depth deterministic AI, not deep minimax

Decision:
- Enemy-team AI should use medium-depth weighted behavior.
- It should not attempt deep minimax planning.
- AI world behavior should be deterministic from hidden Scenario-start AI seed data.

Why:
- Keeps AI comprehensible and performant.
- Supports repeatable debugging while still allowing varied Scenario starts.

### 94) Give AI the same fog-of-war knowledge limits as humans

Decision:
- AI teams may act only on what they have explored or revealed through normal rules.
- AI should not cheat by using hidden Region information.

Why:
- Preserves fairness.
- Makes scouting and map control meaningful for all teams.

### 95) Use a priority pipeline for AI action selection

Decision:
- AI action selection should use a priority pipeline rather than one flat score.
- The broad priority order is:
  - victory opportunity
  - survival / defeat avoidance
  - urgent tactical or logistical needs
  - personality-shaped goals

Why:
- Prevents AI from looping on low-value tasks.
- Ensures victory and survival remain strategically meaningful.

### 96) Let personality and aggression shape AI risk and movement style

Decision:
- Personality affects both goal priority and movement style.
- Aggression affects combat risk tolerance.
- Victory opportunity usually overrides personality, but aggression affects how much danger the AI accepts.

Why:
- Makes enemy teams feel distinct without requiring custom scripting for each team.

### 97) Make patrol radius a hard AI constraint

Decision:
- Patrol radius is forced.
- AI teams may not move outside their patrol radius unless events remove, replace, or expand it.

Why:
- Gives designers reliable containment tools.
- Avoids AI breaking authored Region structure.

### 98) Avoid direct arbitrary AI objectives

Decision:
- Designers should assign personality, aggression, patrol, gates, events, team setup, and spawns.
- Designers should not normally assign arbitrary direct objectives such as “go attack this exact node now.”

Why:
- Keeps AI systemic.
- Encourages story-like behavior through authored world structure rather than one-off scripts.

### 99) Allow events to change AI parameters

Decision:
- Events may change AI personality, aggression, patrol, alliances, and team spawning.

Why:
- Supports story progression and changing world pressure while keeping AI systemic.

### 100) Let AI use all legal Region services and relevant party systems

Decision:
- AI teams may use all legal Region services.
- AI may also use party-level systems such as cooking and artifact management.
- AI still does not enter Locations.

Why:
- Keeps AI economically and strategically competitive.
- Preserves the current Location-access rule.

### 101) Prevent AI team-to-team resource sending

Decision:
- AI may use Trading Posts for resource exchange.
- AI should not send resources to other teams through Trading Posts.
- Team-to-team resource sending is a human-facing activity.

Why:
- Avoids unnecessary diplomacy/economy complexity.
- Keeps AI trading focused on self-improvement.

### 102) Let AI manage artifacts and cooking

Decision:
- AI should try to equip the best available artifacts.
- AI may combine artifacts when useful.
- AI may cook when available food would benefit the team.

Why:
- Keeps AI competitive under the same economy and preparation rules as human teams.

### 103) Keep service destruction and restoration personality-sensitive

Decision:
- AI may destroy services for denial when legal.
- Warrior AI is most likely to sabotage.
- AI may restore useful services when it can afford to do so.
- Builder AI is most likely to restore.

Why:
- Makes sabotage and rebuilding feel tied to team identity.

### 104) Separate threat preview from auto-resolve result

Decision:
- Threat preview is a rough, color-coded estimate.
- Threat preview is not a full battle simulation.
- Auto-resolve result is the actual deterministic backend battle result.

Why:
- Keeps pre-battle readability cheap.
- Prevents the preview from being confused with the actual simulated outcome.

### 105) Use full backend CTB simulation for auto-resolve

Decision:
- Auto-resolve should play out a full backend CTB battle using AI battle choices.
- It should use the same battle rules as manual battle.
- It should be deterministic from battle seed and starting state.

Why:
- Keeps auto-resolve fair and consistent with real battle rules.
- Makes AI-vs-AI outcomes credible.

### 106) Share the battle AI controller between auto-resolve and auto-combat

Decision:
- Manual battle may support toggleable auto-combat.
- Auto-combat should use the same AI battle controller as auto-resolve.
- Same pre-battle state, seed, and settings should produce the same result.

Why:
- Avoids parallel automated-battle implementations.
- Makes automated results easier to test.

### 107) Restrict human auto-resolve skill/item use by setting

Decision:
- Human-team auto-resolve / auto-combat may disable usable skills and items.
- By default, usable skills and items are disabled for human auto-resolve / auto-combat.
- This does not disable leader aura, passive skills, food buffs, artifacts, or other always-on effects.
- AI teams may use everything available.

Why:
- Prevents auto-resolve from wasting player MP and items by default.
- Keeps always-on build choices meaningful.

### 108) Make battle retry deterministic

Decision:
- Retrying a battle restores the exact pre-battle state and deterministic battle seed.
- The same choices should produce the same result.
- Different choices may produce different results.

Why:
- Keeps retries fair and debuggable.
- Prevents retry randomness from becoming the main tactical tool.

### 109) Use persistent live fog-of-war reveal

Decision:
- Unrevealed areas are hidden.
- Revealed areas stay revealed forever.
- Revealed information is live rather than stale.

Why:
- Matches the intended HoMM-like map readability.
- Avoids more complex stale-information bookkeeping.

### 110) Give both human and AI teams their own vision

Decision:
- Each team has its own revealed map knowledge.
- Vision may come from team position, allied teams, scouting, stationed guards, owned services, or reveal services.

Why:
- Makes scouting, ownership, and allies strategically meaningful.
- Keeps AI knowledge fair.

### 111) Show visible AI activity, not unseen activity

Decision:
- Visible AI movement should be animated, with speed settings from very slow to instant.
- Unseen AI actions should not produce generic messages by default.
- Authored events may still announce important unseen outcomes.

Why:
- Maintains pacing and fog-of-war tension.
- Keeps major story or victory events explainable.

### 112) Use scouting to control enemy inspection depth

Decision:
- Default visible enemy information includes color, leader, threat color, estimated unit quantity ranges, and hero level ranges.
- Highest scouting may reveal full details, including resources, items, and artifacts.

Why:
- Gives players enough information to make decisions while preserving scouting value.

### 113) Use hybrid UI density

Decision:
- Ashvale UI should be clean by default and detailed on hover, select, inspect, or context action.

Why:
- Supports both JRPG readability and HoMM-like strategic clarity.
- Prevents dense systems from becoming visually overwhelming.

### 114) Keep the whole game controller-friendly and touch-aware

Decision:
- The whole game should be controller-friendly.
- Mouse/keyboard and controller remain primary targets.
- UI should also be touch-friendly where practical and avoid future-mobile-hostile assumptions.

Why:
- Supports planned PC play while keeping future smartphone/tablet support feasible.

### 115) Avoid hover-only information

Decision:
- Information shown by hover must have select/tap/controller equivalents.

Why:
- Keeps controller and touch support viable.
- Prevents important information from being mouse-only.

### 116) Use exact numbers only when information is known

Decision:
- Show exact values for the player's own known state.
- Show estimates when fog, scouting, or hidden information limits certainty.

Why:
- Keeps information fair and consistent with fog/scouting systems.

### 117) Use Adventure button strip and Scenario Info screen as separate terms

Decision:
- **Adventure button strip** means the left-side icon menu in Region / World Map UI.
- **Scenario Info screen** means the formal victory/defeat-condition screen.

Why:
- Avoids terminology conflict between a UI button strip and formal scenario-rule display.

### 118) Keep Region HUD stable and strategic

Decision:
- Region mode should have a persistent top bar, bottom resource bar, minimap, Adventure button strip, contextual info panel, and temporary news area.

Why:
- Gives players stable access to strategic information while moving around a contested Region.

### 119) Use select-then-confirm for Region and World Map movement

Decision:
- First click/tap/select previews.
- Second click/tap/confirm executes where legal.

Why:
- Prevents accidental travel.
- Gives route, cost, and legality information before commitment.

### 120) Show shortest legal route and blocker feedback

Decision:
- Route preview should show the shortest legal route where one exists.
- Blockers should be outlined red when they make a path illegal.
- Time and Energy blockers should show red values in tooltip.

Why:
- Makes movement legality understandable without revealing alternate optimal strategies too aggressively.

### 121) Represent teams through color, banner, and portrait

Decision:
- Team tokens should show team color outline, color banner, and leader portrait if available.
- If no leader exists, use the strongest unit as representative portrait.

Why:
- Keeps team identity readable at map scale.

### 122) Show owned and guarded services visually

Decision:
- Owned nodes/services use flag icons.
- Guarded services show strongest-guard portrait and threat information.

Why:
- Makes ownership and danger readable without opening a full panel.

### 123) Make farming state visible on Region nodes

Decision:
- Farming services should visually communicate growing, watered, and ready-to-harvest states.

Why:
- Farming is a contested world process, so its state must be visible and inspectable.

### 124) Keep Location UI JRPG-like and event-driven

Decision:
- Location mode should use sprite-based walking exploration.
- Location interactions are authored events, not a standard minimap/service-list interface.

Why:
- Preserves the intended FF-like Location feel.

### 125) Keep Location services represented by event-state changes

Decision:
- Built/restored/upgraded Location services should appear through changed sprites, collision, layers, objects, and interactables.

Why:
- Makes Location development feel authored and visual rather than abstract.

### 126) Use a clear JRPG battle layout

Decision:
- Party right, enemies left, CTB bar top, help panel below CTB, formation center, party status bottom right, command menu beside status.

Why:
- Supports readable CTB battles and clear target/action feedback.

### 127) Show positions visually, not as constant row labels

Decision:
- Formation positions should be readable by sprite placement and distance from screen center.
- Targeting help may show actual position, but rows are not always displayed as text labels.

Why:
- Keeps battle presentation clean while preserving tactical information.

### 128) Use live CTB preview without formula explanation

Decision:
- Hovering/selecting actions and targets should update CTB order live.
- The UI should show the resulting order, not internal CTB math.

Why:
- Supports tactical planning without overwhelming the player.

### 129) Use one battle result screen for manual and auto-resolve

Decision:
- Manual battle and auto-resolve use the same result screen.

Why:
- Reduces duplicated UI and keeps battle outcomes consistent.

### 130) Keep party menu centered on active party, reserve, and right-side actions

Decision:
- Party menu layout should include a large active-party panel, bottom reserve slots, and right-side menu actions for Items, Artifacts, Cooking, and Position.

Why:
- Keeps the core roster-management flow visible and controller-navigable.

### 131) Document generic stack mouse shortcuts now

Decision:
- Support and document Ctrl/Alt/Shift mouse shortcuts for splitting and merging generic stacks.
- Provide controller context-menu equivalents.

Why:
- Makes stack management efficient without losing controller support.

### 132) Keep Temporarily Unavailable heroes hidden until return

Decision:
- Temporarily Unavailable heroes are not shown in normal party/roster UI until they return.

Why:
- Keeps the roster display focused on usable units.

### 133) Use flat item inventory with sorting

Decision:
- Item inventory is a flat list with icon, name, and quantity.
- Sorting supports manual, name, type, least, and most.

Why:
- Avoids overcomplicating the item inventory with categories.

### 134) Manage equipped artifacts through the hero artifact interface

Decision:
- Equipped artifacts are managed through the Artifacts menu after selecting a hero.

Why:
- Keeps artifact equipment tied to hero identity and stat preview.

### 135) Show cooking ingredients inline with missing counts marked red

Decision:
- Recipes always show required ingredients and held/required counts.
- Missing quantities are marked red inline.
- There is no separate missing-ingredient list.

Why:
- Keeps recipe requirements clear without redundant UI.

### 136) Make artifact combination immediate when legal

Decision:
- Selecting a legal artifact combination performs it immediately.
- No extra irreversible-warning confirmation is required.
- Inputs and output must be clearly shown before activation.

Why:
- Artifact combination may happen frequently and should not be slowed by repeated warnings.

### 137) Do not force all services into one universal screen

Decision:
- Services may have custom interfaces.
- Applicable costs, availability, guards, and threat must still be shown.

Why:
- Supports variety while preserving essential clarity.

### 138) Confirm only high-risk service actions

Decision:
- Confirm Region travel that loses generics, surrender, escape, discarding artifacts, and destroying services.
- Artifact combination does not need a separate warning prompt.

Why:
- Keeps dangerous actions safe while avoiding intrusive confirmations for frequent upgrades.

### 139) Keep Quest Log, Scenario Info, and Guidance separate

Decision:
- Quest Log lists discovered quest-service tasks.
- Scenario Info shows victory/defeat conditions.
- Guidance is event-driven directional text.

Why:
- Prevents objective, rule, and hint surfaces from blending together.

### 140) Use black animated fog without silhouettes

Decision:
- Unrevealed Region areas are black fog with a smooth animated edge.
- Nodes are shown only when revealed.

Why:
- Keeps fog clear and strongly readable.

### 141) Tie enemy inspection depth to scouting

Decision:
- Default enemy inspection is limited.
- Basic, Advanced, and Expert scouting progressively reveal more.
- Expert reveals exact stacks, hero levels, leader stats, items, and artifacts.

Why:
- Makes scouting strategically valuable.

### 142) Maintain a temporary popup area and full history log

Decision:
- Important events appear as temporary news popups.
- The latest four can be expanded.
- Clicking opens full log/history with date information.

Why:
- Keeps the player informed without forcing constant modal interruptions.

### 143) Let animation speed be configurable

Decision:
- AI and human movement / animation speed should be configurable from very slow to instant.

Why:
- Supports both readability and fast repeated play.

### 144) Keep authored game content data-driven

Decision:
- Scenarios, Regions, Locations, nodes, Services, teams, quests, events, victory/defeat conditions, resource amounts/costs/payouts, units, items, artifacts, and recipes should be editable through a designer tool.
- The resource enum itself remains code/schema-defined unless explicitly expanded later.

Why:
- Keeps Scenario creation flexible.
- Makes long-term modding and iteration practical.

### 145) Keep mechanics typed and code-defined

Decision:
- Code defines allowed system types, formulas, runtime behavior, validation rules, and legal enum values.
- Content configures those systems within legal limits.
- Content should not invent arbitrary new mechanics without code support.

Why:
- Preserves validation and runtime predictability.
- Avoids uncontrolled scripting complexity.

### 146) Allow saving invalid work-in-progress content but block play

Decision:
- Designers may save invalid or incomplete maps for later work.
- Invalid maps should not be playable in-game.

Why:
- Supports real authoring workflow without letting broken content reach runtime.

### 147) Distinguish invalid content from harsh legal content

Decision:
- Validation should reject structurally invalid content.
- Validation should allow harsh, unfair, or obscure content if it is structurally valid.

Why:
- Preserves designer freedom.
- Prevents validation from becoming taste enforcement.

### 148) Treat nodes as travel points with authored content

Decision:
- Nodes are fundamentally empty travel points.
- Node behavior is determined by node content and attached events.
- Blocker behavior usually comes from content, not from an intrinsic node type.

Why:
- Keeps Region authoring flexible.
- Avoids a bloated fixed node-type hierarchy.

### 149) Use manually authored World Map adjacency

Decision:
- World Map Region adjacency is manually authored.
- Automatic polygon/border detection is not the source of truth.

Why:
- Gives designers explicit control over Region travel paths.

### 150) Keep routes authored but stateful

Decision:
- Routes are authored data with road flag and terrain enum.
- Events may destroy, restore, reveal, or activate routes.
- Events do not create entirely new route definitions from nothing at runtime.

Why:
- Supports authored structural changes while keeping validation possible.

### 151) Make service validation type-specific and strict

Decision:
- Each service type has its own legal settings and validation rules.
- Defaults are global data, not per-Scenario assumptions.

Why:
- Keeps service authoring safe and predictable.

### 152) Use reusable service definitions for Location service calls

Decision:
- Location events call reusable service definitions where legal.
- Only selected service types should be callable from Locations.

Why:
- Keeps Location service flow flexible without creating incompatible service systems.

### 153) Keep event authoring typed

Decision:
- Events should use typed triggers, eligibility, conditions, branches, actions, repeatability, and priority.
- Events should not become arbitrary free-form scripts.

Why:
- Keeps authoring expressive but validateable.

### 154) Support nested If / Else event branches

Decision:
- Events may support nested If / Else branches.
- Branches replace optional-action flags.

Why:
- Gives designers explicit fallback control without adding optional-action complexity.

### 155) Use non-atomic ordered event action chains

Decision:
- Event actions execute in authored order.
- Previous successful actions are not rolled back if a later action fails.
- Designers must author safe flows using conditions and branches.

Why:
- Keeps event execution understandable and avoids complex rollback semantics.

### 156) Show and log runtime event-action failures when reasonable

Decision:
- Event actions should not fail in normal intended play.
- If an event action fails anyway, fail safely and show/log a clear reason when reasonable.

Why:
- Helps designers and testers detect broken authored logic.
- Avoids silent state corruption.

### 157) Make take actions fail hard when unavailable

Decision:
- Actions that take resources/items re-check availability at runtime.
- They fail hard if unavailable.
- They never clamp or create negative resources.

Why:
- Prevents invalid economy state.

### 158) Let give actions obey target overflow rules

Decision:
- Give actions obey inventory and roster capacity rules.
- Excess may be capped or discarded where the target system defines that behavior.
- The player should receive feedback when reasonable.

Why:
- Keeps overflow behavior consistent with item/roster systems.

### 159) Use authored priority for automatic events

Decision:
- Automatic events use authored priority.
- The designer tool should allow moving events up/down.
- New events are added to the bottom by default.

Why:
- Makes start-of-day and condition-event ordering predictable.

### 160) Share typed condition structures across quests, events, victory, and defeat

Decision:
- Quest objectives, event conditions, victory conditions, and defeat conditions should use the same broad typed condition model where practical.

Why:
- Reduces duplicate logic.
- Makes validation and tooling easier.

### 161) Validate for likely softlocks with best-effort analysis

Decision:
- Validation should attempt graph and dependency checks for reachability and required progression.
- Validation does not need to prove every Scenario is beatable.

Why:
- Catches obvious structural impossibilities without pretending to solve authored design.

### 162) Treat designer judgment as part of the authoring model

Decision:
- Designers may create harsh or obscure Scenarios.
- Validation catches structural impossibility, not bad taste.
- Intentional softlocks should be modeled as defeat states, not accidental broken states.

Why:
- Preserves creative freedom while still discouraging accidental broken content.

### 163) Keep content hand-editable where practical

Decision:
- The designer tool is the primary long-term authoring flow.
- Data files should remain reasonably human-readable and hand-editable where practical.

Why:
- Supports debugging, modding, review, and AI-assisted editing.

### 164) Run validation in editor, startup, CI, and packaging workflows

Decision:
- Validation should run on editor save, game startup, automated tests/CI, and before packaging release content.

Why:
- Catches broken content early and repeatedly.

### 165) Use Save, Playable, and Release validation levels

Decision:
- Validation has three levels:
  - Save validation
  - Playable validation
  - Release validation
- Save validation never blocks saving.
- Playable validation blocks play on errors.
- Release validation requires warnings to be resolved, acknowledged, or suppressed with a reason.

Why:
- Supports real authoring workflow without allowing broken content into play.

### 166) Use Error, Warning, and Info validation severities

Decision:
- Validation messages use:
  - Error
  - Warning
  - Info

Why:
- Separates structural invalidity from suspicious design and helpful authoring notes.

### 167) Require validation messages to identify exact authored objects

Decision:
- Every validation message should include a stable code and exact authored-object path.
- Editor output should make object paths clickable where practical.

Why:
- Makes validation actionable for designers and AI agents.

### 168) Keep deletion cleanup explicit and validation-authoritative

Decision:
- The editor may offer safe reference cleanup when deleting content.
- Cleanup must be visible and reviewable.
- Validation remains the authority.
- If no safe default exists, a validation error remains.

Why:
- Prevents silent ambiguous content changes.

### 169) Enforce one main node content item per node

Decision:
- Validation should enforce one main node content item per Region node.
- Attached events are allowed separately.

Why:
- Preserves the node-content model and prevents muddy node behavior.

### 170) Require explicit node event priority when content and event coexist

Decision:
- If a node has both attached event and main content, event priority should be explicit:
  - beforeContent
  - afterContent
  - replacesContent
- Default is beforeContent.

Why:
- Prevents ambiguity about what happens when a team arrives on a node.

### 171) Validate event cycles as errors

Decision:
- Validation should detect unbounded event cycles and same-day repeat loops.
- These are errors unless a safe boundary can be proven.

Why:
- Prevents runaway event execution.

### 172) Treat artifact-combination cycles as warnings

Decision:
- Artifact-combination cycles are warnings, not errors.
- They may represent poor design but remain legal if intentional and acknowledged.

Why:
- Preserves designer freedom while surfacing likely bad upgrade loops.

### 173) Support warning suppression with reasons

Decision:
- Release validation may allow warnings only when acknowledged or suppressed with a reason.

Why:
- Keeps intentional harsh or unusual content explicit and reviewable.

### 174) Keep validation pure and headless

Decision:
- Validation should be testable without rendering.
- It should run in editor, startup, CI, and packaging/release workflows.

Why:
- Makes validation reliable, automated, and agent-friendly.

### 175) Use JSON content files with schemaVersion, kind, and id

Decision:
- Authored content uses JSON.
- Top-level content files include `schemaVersion`, `kind`, and `id`.

Why:
- Makes content validation, migration, mod loading, and AI-assisted editing predictable.

### 176) Use localized text objects for player-facing text

Decision:
- Player-facing text uses language-code objects from the start.

Why:
- Keeps language support built into content instead of retrofitted later.

### 177) Use `kind + id` as mod override identity

Decision:
- Mods override content by matching top-level `kind + id`, not filename alone.

Why:
- Keeps mod loading deterministic and validation-friendly.

### 178) Use Scenario Region Context instead of broad Region patches

Decision:
- Reusable Regions read Scenario-provided context.
- Avoid arbitrary shallow patching as the primary override model.

Why:
- Keeps reusable Regions validateable and agent-readable.

### 179) Use discriminated union node content

Decision:
- Node content is represented as one discriminated union field.

Why:
- Enforces one main node content item and reduces ambiguity.

### 180) Keep authored initial state separate from runtime save state

Decision:
- Content files define initial state.
- Save data owns runtime progression state.

Why:
- Prevents authored data from being polluted by playthrough state.

### 181) Treat the player character as a unique hero with special human-team rules

Decision:
- The player character is a normal unique hero unit with special human-team rules.
- The player character is identified by a stable hero id such as `hero_player`.

Why:
- Keeps player-character behavior integrated with the hero system while preserving required protagonist rules.

### 182) Define player character identity on the human team

Decision:
- Human teams define `playerCharacterHeroId`.
- Single-player Scenarios have exactly one player character.
- Multi-human / PvP Scenarios may have one player character per human team.

Why:
- Team-level ownership supports both single-player and future multi-human structures.

### 183) Require character creation before play

Decision:
- Before a Scenario or Campaign starts, the player creates the player character.
- Character creation fills name, sex, simple appearance, starting stats, starting skills, and starting preset/template.
- Templates such as Warrior, Builder, and Explorer are starting presets only.

Why:
- Gives the player authored identity without adding permanent class restrictions.

### 184) Keep the player character in the traveling party

Decision:
- The player character must remain in the human team's traveling party.
- The player character may be active or reserve.
- The player character cannot be stored, dismissed, Temporarily Unavailable, recruited, sealed/frozen, neutral, AI-owned, or placed in AI templates.

Why:
- Prevents protagonist loss and AI ownership edge cases while preserving party-management flexibility.

### 185) Restore player character KO at battle end

Decision:
- If the player character has KO status at battle end, restore them to 1 HP.
- Player-character KO is not automatic Scenario defeat unless recovery is impossible or a Scenario-specific defeat condition says so.

Why:
- Keeps the player character persistent while allowing explicit defeat conditions.

### 186) Leader initiates escape and surrender

Decision:
- Escape and surrender may only be initiated by the current leader.
- The resulting escape/surrender outcome still applies to the team according to the normal escape/surrender rules.

Why:
- Keeps battle command authority tied to the leader without reverting to the old “only leader escapes” outcome.

### 187) Preserve heroes and reserve on escape

Decision:
- When a human team escapes, all hero units and all reserve units survive.
- Active generic units are lost.

Why:
- Prevents escape from accidentally deleting the player character or reserve roster.

### 188) Do not allow events to replace player-character identity

Decision:
- Events may modify player-character stats, skills, passives, appearance, name, equipment, and authored properties.
- Events may not replace the player-character identity with a different hero id during a Scenario.

Why:
- Keeps Scenario identity stable and validation tractable.


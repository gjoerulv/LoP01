# Technical Direction

## Stack

- C++20
- raylib
- CMake
- nlohmann/json
- Catch2

## Terminology reference

This document follows the terminology in `docs/terminology_map.md`.

Use current design terms such as **World Map**, **Region**, **Location**, **Service**, **Traveling party**, **Stored units**, and **Temporarily Unavailable** when discussing architecture and future work.

If existing source files, content keys, or serialized values still use older names, treat `docs/terminology_map.md` as the interpretation layer between legacy implementation terminology and current design terminology.

## Architectural principles

- Prefer a simple, explicit gameplay state machine.
- Keep systems explicit rather than overly abstract.
- Keep content data-driven in JSON.
- Separate gameplay rules from presentation.
- Keep rendering models and mappers separate from gameplay state.
- Make pure logic testable without raylib dependency where possible.
- Preserve strong separation of concerns: input, gameplay, mapping, and rendering should not collapse into one layer.
- Prefer authored structures with reusable systemic rules on top, rather than large procedural frameworks.

## Module layout

- `src/app`
  - main loop
  - application shell
  - mode transitions
  - input/update/draw integration
  - controllers and render-model mappers
  - World Map / Region / Location / battle mode coordination

- `src/core`
  - time
  - ids
  - save/load

- `src/data`
  - JSON loading
  - content repositories
  - content definitions and typed gameplay-facing data mapping where needed
  - authored Region graphs, nodes, routes, Locations, Services, unit definitions, and scenario data

- `src/gameplay`
  - World Map and Region rules
  - Region graph traversal, route quality, and Energy use
  - Locations and their internal interactions
  - Services, storage, and authored node-state transitions
  - party, reserve, storage, and roster rules
  - enemy traveling-team behavior
  - battle
  - quests
  - penalties
  - progression
  - gameplay session

- `src/rendering`
  - renderers
  - render models
  - HUD / overlays

## Region / Location / Service architecture guidance

- Treat a **Region** as an authored node graph with reusable systemic rules layered on top.
- Use the node-content model: nodes are travel points whose gameplay behavior comes from main node content plus event attachments.
- Keep node content single-purpose at the content-definition level.
- Do not introduce a dedicated combat-node abstraction. Hostile encounters, blocker behavior, and one-time resource pickups should be authored as node content and usually resolve back into empty travel nodes when cleared.
- Treat **arrival** as a flag on a node, not as a separate node type.
- Keep **Location** as a distinct entered-space concept rather than collapsing it into node-level services.
- Treat **direct Service nodes** as quick functional interactions on the Region layer.
- Allow storage to exist either inside a Location or as a direct Region service, but keep both under a common gameplay concept.
- Keep enemy traveling teams as Region-layer systems. They may interact with Region nodes and Region services, but they do not enter Locations.
- Keep World Map travel separate from Region travel. World Map travel is scenario-level region selection; Region travel is node-to-node movement inside the active Region.

## Content architecture guidance

- Treat authored JSON as static source-of-truth content, not mutable runtime state.
- Use stable ids and explicit fields so future tooling can create/update/delete content safely.
- Prefer typed gameplay-facing definitions rather than passing loosely shaped JSON through gameplay systems.
- Keep content semantics in data whenever practical instead of hiding them in renderer or input code.
- Extend schemas incrementally and keep migrations obvious.
- Prefer explicit authored fields over inferred magic, especially for:
  - Region nodes and routes
  - route terrain / route quality
  - Location entry
  - Services
  - arrival-node flags
  - blocker requirements
  - enemy-team starting positions and movement constraints

## Performance and correctness guidance

- Favor RAII and clear ownership to avoid leaks.
- Avoid raw owning pointers unless ownership is truly external and obvious.
- Do not parse content repeatedly in the frame loop.
- Avoid unnecessary per-frame allocations and string churn in hot paths.
- Avoid blocking work in the main loop.
- Keep input handling responsive and separate from rendering.
- Keep rendering lightweight and avoid moving gameplay decisions into render code.
- Keep pathfinding, travel legality, node-state transitions, and Energy calculations deterministic and testable.

## Implementation guidance

- Do not build a generic engine.
- Do not build full editor tooling yet.
- Do not add networking.
- Do not add mod support yet.
- Keep the battle simulator mostly independent from rendering.
- Prefer explicit mode-entry helpers over generic chained transitions for gameplay flow.
- Favor compilable, testable milestones.
- Extend the current playable slice incrementally instead of broadening scope prematurely.
- Prefer small, explicit gameplay models over broad meta-systems.
- When adding Region / Location / Service behavior, prefer content-driven schemas and lightweight runtime state over broad framework layers.
- When terminology in runtime code still reflects older naming, do not assume the older name is the current design intent.

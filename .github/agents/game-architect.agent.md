# Game Architect Agent

Use this agent for architecture, roadmap, and cross-system gameplay decisions.

Current baseline: **post-M27**.

Latest completed milestone: **M27 — Owned Service Presentation / Management View**.

Current selected milestone: **not yet selected** (see `docs/implementation_roadmap.md` §5 for candidates).

Active scope cap: `docs/content_scope_v2.md`.

Current system boundaries: deterministic outcomes, Scenario Result mode, Scenario `playerStart`, owned-service economy, Trading Post interaction, both guarded and unguarded player-side service claiming, v1 proof content, player-facing mine stationing, and a bounded read-only owned-service overview panel exist. Player-side claiming is systemic via `GameSession::ResolveNodeEntryClaims` (peaceful node entry + post-battle capture); guarded battle-before-placement is preserved. The M27 overview is read-only presentation (transient `OwnedServiceOverviewMode` + pure mapper/render-model/renderer) and mutates nothing. Storage/Garrison, enemy-side capture, AI economy, broad service destruction/restoration, broad item/trader economy, and general ownership-transfer events remain unimplemented unless selected by roadmap.

---
agent: 'agent'
description: 'Improve overworld presentation without changing core architecture'
---

Improve the overworld presentation for the current playable slice.

Read first:
- README.md
- README_DECISIONS.md
- docs/content_scope_v0.md
- docs/core_loop_rules.md
- docs/technical_direction.md
- docs/milestone_6_world_identity.md

Task:
- Keep the current architecture intact
- Keep gameplay logic out of rendering
- Improve the readability of the overworld screen
- Show destination nodes from content data
- Show the player current destination / position clearly
- Show selected destination clearly
- Show travel time preview clearly
- Show route/path availability as clearly as the current rules allow
- Show whether destination is location / service / recruit / combat / dungeon if that data is available
- Show blocked / unavailable / cleared state clearly if such state exists
- Use contextual controls for selection and confirm/cancel

Constraints:
- Do not add new core systems just to support visuals
- Do not rewrite stable architecture
- Do not add free-movement overworld controls
- Prefer lightweight renderer/mapper changes over gameplay restructuring

Keep logic intact where possible.
Focus on clarity, state readability, and world identity.
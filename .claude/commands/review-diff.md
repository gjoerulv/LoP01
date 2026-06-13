# Review diff command context

Review diffs against the active v3 roadmap.

Baseline: post-M31.

Active scope: `docs/content_scope_v3.md`.

Latest completed milestone: **M31 — Shell Entry + Scenario/Campaign Selection**. The next milestone is not yet selected.

Reject changes that silently broaden into unselected v3 candidates: full character creation, full settings/mods/accessibility, save-slot metadata/browsers, Scenario Region Context, fog/reveal, threat preview, AI economy, item economy, battle-depth rewrites, or final service-management UI.

Check that changes preserve the M31 shell semantics: shell screen state stays App-local and unpersisted; content starts go through `GameSession::StartCampaign`/`StartStandaloneScenario` behind the validation gate; standalone selection enforces `standaloneSelectable`; Continue failure never mutates the session; quicksave stays suppressed at the shell.

Check that changes preserve v1/v2 systems unless explicitly scoped:

- stationing/storage one-place-at-a-time invariants;
- M29 travel-loss semantics;
- M30 service defense/capture/destruction/restoration semantics;
- save/load compatibility;
- authored content separated from runtime state;
- gameplay rules outside rendering/input.

Flag stale docs or agent guidance that still says v2 is active or the next milestone is unselected.

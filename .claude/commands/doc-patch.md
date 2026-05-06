# Prepare a focused documentation patch

Prepare a focused documentation patch for the requested design area.

Before editing:
- read `CLAUDE.md`
- read the current docs relevant to the requested area
- identify the single best source-of-truth doc for the detailed rules
- identify only the minimal reference updates needed elsewhere

Patch rules:
- keep one detailed source of truth
- avoid duplicating large rule blocks across docs
- do not bloat `core_loop_rules.md` with shell/UI/presentation content
- do not add manual guidance files to the repo
- keep agent-facing wording explicit and concise
- do not change design scope beyond the user's confirmed decisions
- preserve old docs only if archived or explicitly historical

After editing:
1. summarize files changed
2. identify the new/updated source-of-truth section
3. list any references added
4. call out any possible ambiguity left
5. recommend whether to audit before commit

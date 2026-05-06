# Implement a small approved patch

Implement only the approved small task.

Before editing:
1. read `CLAUDE.md`
2. read only the docs relevant to this task
3. inspect the files you intend to modify
4. summarize the plan
5. list files you intend to change

During implementation:
- keep the patch small
- do not alter unrelated docs or design rules
- do not perform broad refactors
- keep gameplay logic separated from rendering/input
- prefer pure helper functions for rule logic
- add or update tests where practical
- preserve existing behavior unless the task explicitly changes it

After implementation:
1. summarize changed files
2. explain how the patch follows the docs
3. list tests run and results
4. list any tests not run and why
5. call out any follow-up work

Stop and ask before:
- deleting files
- renaming public concepts
- changing architecture broadly
- changing dependencies
- changing generated content formats
- modifying archived docs
- resolving doc/code conflicts by guessing

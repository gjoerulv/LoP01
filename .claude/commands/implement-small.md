# Implement a small approved patch

Implement only the approved small task.

## Before editing

1. read `CLAUDE.md`;
2. read `docs/technical_direction.md`;
3. read `docs/content_scope_v1.md` when content scope or milestone scope is touched;
4. read only the other docs relevant to this task;
5. inspect the files you intend to modify;
6. summarize the plan;
7. list files you intend to change.

## During implementation

- keep the patch small;
- do not alter unrelated docs or design rules;
- do not perform broad refactors;
- keep gameplay logic separated from rendering/input;
- prefer pure helper functions for rule logic;
- add or update tests where practical;
- preserve existing behavior unless the task explicitly changes it;
- avoid demo-specific source branches;
- avoid repeated parsing, repeated graph rebuilding, large needless copies, and per-frame scans;
- preserve save/load compatibility unless migration is explicitly in scope.

## After implementation

1. summarize changed files;
2. explain how the patch follows the docs;
3. list tests run and results;
4. list any tests not run and why;
5. call out any follow-up work.

## Stop and ask before

- deleting files;
- renaming public concepts;
- changing architecture broadly;
- changing dependencies;
- changing generated content formats;
- modifying archived docs;
- resolving doc/code conflicts by guessing.

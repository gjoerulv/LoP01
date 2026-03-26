---
applyTo: "src/**/*.cpp,src/**/*.h,src/**/*.hpp,tests/**/*.cpp,CMakeLists.txt"
---

# C++ and project structure instructions

- Target modern C++20, but keep the code straightforward, explicit, and readable.
- Favor value types and RAII.
- Avoid macros unless clearly justified.
- Prefer standard library containers and algorithms.
- Use `enum class` instead of plain enums.
- Use explicit constructors where appropriate.
- Keep headers lean; avoid unnecessary includes.
- Prefer forward declarations when practical.
- Minimize global mutable state.
- Separate pure gameplay logic from raylib drawing/input glue.
- Keep update logic and draw logic distinct.
- Use small focused classes rather than giant managers.
- When adding files, also update `CMakeLists.txt`.
- For logic-heavy systems, add or update focused unit tests.
- For save data and content definitions, prefer explicit JSON serialization/deserialization.

# Change-scope rules

- Keep patches narrowly scoped to the requested task.
- Do not refactor unrelated code while implementing a bounded change.
- Prefer extending existing code in place over introducing new abstractions unless a new seam is clearly justified.
- Do not move business logic into rendering, input, or mapper code.
- Preserve existing architecture unless the task explicitly requires an architectural change.
- When fixing a bug or completing a milestone slice, do not start follow-on work in the same patch unless explicitly asked.

# Validation rules

- For small scoped changes, run the narrowest relevant tests first.
- Run the full test suite only when the change affects shared systems, persistence, content loading, or core gameplay flow, or when explicitly requested.
- Do not claim a task is complete if the project does not build or the relevant tests were not run.

# Agent behavior rules

- Treat instruction files as constraints, not as separate work items.
- Do not repeatedly reread instruction files unless the task meaningfully changed.
- Do not broaden into repo-wide audits unless explicitly asked.
- If a change becomes partial or leaves the repo broken, switch to recovery mode and restore a clean build/test state before continuing milestone work.
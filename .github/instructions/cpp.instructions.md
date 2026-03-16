---
applyTo: "src/**/*.cpp,src/**/*.h,src/**/*.hpp,tests/**/*.cpp,CMakeLists.txt"
---

# C++ and project structure instructions

- Target modern C++20 but keep the code straightforward and readable.
- Favor value types and RAII.
- Avoid macros unless clearly justified.
- Prefer standard library containers and algorithms.
- Use enum class instead of plain enums.
- Use explicit constructors where appropriate.
- Keep headers lean; avoid unnecessary includes.
- Prefer forward declarations when practical.
- Minimize global mutable state.
- Separate pure gameplay logic from raylib drawing/input glue.
- Keep update logic and draw logic distinct.
- Use small focused classes rather than giant managers.
- When adding files, also update CMakeLists.txt.
- For logic-heavy systems, propose unit tests.
- For save data and content definitions, prefer explicit JSON serialization/deserialization.
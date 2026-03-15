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

## Naming

| Construct | Convention | Example |
|-----------|-----------|---------|
| Types / classes | `PascalCase` | `LocationManager` |
| Member functions | `PascalCase` | `GetName()` |
| Free functions | `PascalCase` | `LoadLocations()` |
| Private members | `m_camelCase` | `m_currentLocation` |
| Local variables | `camelCase` | `exitDirection` |
| Constants / enumerators | `PascalCase` | `Direction::North` |
| Files | `PascalCase` | `Location.h` / `Location.cpp` |

## File structure

- Each class lives in its own header + implementation file pair (`Foo.h` / `Foo.cpp`).
- Headers use `#pragma once`.
- Include order: own header → project headers → standard library headers.

## Memory & ownership

- Prefer value semantics and stack allocation.
- Use `std::unique_ptr` for single ownership of heap objects.
- Use `std::shared_ptr` only when shared ownership is genuinely required.
- Avoid raw `new` / `delete`.

## Error handling

- Throw `std::invalid_argument` or `std::runtime_error` for programming errors.
- Use `std::optional<T>` for values that may legitimately be absent.
- Do **not** use exceptions for normal control flow.

## General style

- Keep functions short (≤ 40 lines is a good target).
- Prefer `const` member functions and `const` references where applicable.
- Do not use `using namespace std;` in headers.

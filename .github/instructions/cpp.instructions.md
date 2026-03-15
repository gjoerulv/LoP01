---
applyTo: "**/*.cpp,**/*.h"
---

# C++ Coding Instructions

## Standard and compiler

- Use **C++17** throughout. Do not rely on C++20 or later features.
- Target MSVC (Visual Studio 2022), GCC ≥ 11 and Clang ≥ 14.
- Enable warnings: `/W4` on MSVC, `-Wall -Wextra -Wpedantic` on GCC/Clang.

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

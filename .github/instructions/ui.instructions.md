---
applyTo: "src/**"
---

# UI / Console Instructions

## Output model

- **All** text output is performed exclusively in `main.cpp`.
- Game logic returns `std::string`; it never calls `std::cout` / `printf`.
- The `Scene::Render()` method produces the full display string for a given game state.

## Scene formatting conventions

```
=== Location Name ===
<blank line>
Description paragraph(s).
<blank line>
Exits: north, south, east
```

- Section headers use `=== Title ===` surrounded by blank lines.
- Exit lists are comma-separated on a single line.
- Full-width dividers use 32 `=` characters: `================================`.

## Player prompt

- The command prompt is `\n> ` (newline, `>`, space).
- After every command response print a blank line before the next prompt.

## Text guidelines

- Use British English spelling throughout (colour, armour, harbour).
- Keep descriptions to 3–5 lines; avoid walls of text.
- Write in second-person present tense: *"The guard eyes you suspiciously."*

## Accessibility

- Do not rely on colour alone to convey information (this is a plain-text game).
- Descriptions must make sense when read by a screen reader.

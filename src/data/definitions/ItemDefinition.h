#pragma once

#include <string>

namespace data {

// Item subtypes per docs/core_loop_rules.md §22 and docs/content_schema.md §33.
// M13 implements the subtype enum and `consumable` stack-of-1 rule (at runtime,
// M13-b). No item effects are implemented in M13: an authored `effects` field
// produces an explicit validation error rather than a silent accept.
enum class ItemSubtype {
    Consumable,
    Quest,
    Seed,
    Ingredient,
    Food,
    Material
};

inline bool ItemSubtypeFromString(const std::string& value, ItemSubtype& out) {
    if (value == "consumable") { out = ItemSubtype::Consumable; return true; }
    if (value == "quest")      { out = ItemSubtype::Quest;      return true; }
    if (value == "seed")       { out = ItemSubtype::Seed;       return true; }
    if (value == "ingredient") { out = ItemSubtype::Ingredient; return true; }
    if (value == "food")       { out = ItemSubtype::Food;       return true; }
    if (value == "material")   { out = ItemSubtype::Material;   return true; }
    return false;
}

struct ItemDefinition {
    std::string id;
    std::string name;          // English name (full localized object deferred)
    std::string icon;
    ItemSubtype subtype = ItemSubtype::Material;
    int stackCap = 999;        // Default per docs §22 "stack up to 999".
    int baseValue = 0;
};

} // namespace data

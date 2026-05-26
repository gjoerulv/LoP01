#pragma once

#include <string>

namespace gameplay {

// Team-shared item inventory entry (M13-b runtime state).
// Consumables are runtime-enforced to quantity == 1; other subtypes stack up
// to the item's authored stackCap from data::ItemDefinition.
struct ItemStackState {
    std::string itemId;
    int quantity = 0;
};

// Team-shared **unequipped** artifact inventory entry (M13-b runtime state).
// Equipped artifacts live exclusively in HeroEquipmentState (declared in
// GameSession.h); an artifact never appears simultaneously in this container
// and a hero slot. Stack cap is 999 per docs/core_loop_rules.md §22.
struct ArtifactStackState {
    std::string artifactId;
    int quantity = 0;
};

} // namespace gameplay

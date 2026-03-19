#include <catch2/catch_test_macros.hpp>

#include "gameplay/world/NodeWorldState.h"

TEST_CASE("NodeWorldState marks combat nodes as cleared idempotently") {
    gameplay::world::NodeWorldState state;

    state.MarkCombatNodeCleared("bridge_checkpoint");
    state.MarkCombatNodeCleared("bridge_checkpoint");

    REQUIRE(state.IsCombatNodeCleared("bridge_checkpoint"));
    REQUIRE(state.ClearedCombatNodeIds().size() == 1);
}

TEST_CASE("NodeWorldState restore applies unique cleared combat node ids") {
    gameplay::world::NodeWorldState state;

    state.RestoreClearedCombatNodeIds({"bridge_checkpoint", "orchard_pass", "bridge_checkpoint"});

    REQUIRE(state.IsCombatNodeCleared("bridge_checkpoint"));
    REQUIRE(state.IsCombatNodeCleared("orchard_pass"));
    REQUIRE(state.ClearedCombatNodeIds().size() == 2);
}

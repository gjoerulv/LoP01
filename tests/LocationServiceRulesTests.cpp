#include <catch2/catch_test_macros.hpp>

#include "gameplay/location/LocationScene.h"
#include "gameplay/location/LocationServiceRules.h"

TEST_CASE("Rest is valid for inn door when location allows sleep") {
    const bool isValid = gameplay::location::IsRestValidForLocation(
        gameplay::location::InteractionType::InnDoor,
        true);

    REQUIRE(isValid);
}

TEST_CASE("Rest is invalid for inn door when location does not allow sleep") {
    const bool isValid = gameplay::location::IsRestValidForLocation(
        gameplay::location::InteractionType::InnDoor,
        false);

    REQUIRE_FALSE(isValid);
}

TEST_CASE("Non-rest interactions are not treated as valid rest") {
    const bool shopAsRest = gameplay::location::IsRestValidForLocation(
        gameplay::location::InteractionType::Shop,
        true);

    const bool npcAsRest = gameplay::location::IsRestValidForLocation(
        gameplay::location::InteractionType::Npc,
        true);

    REQUIRE_FALSE(shopAsRest);
    REQUIRE_FALSE(npcAsRest);
}

#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "gameplay/ResourceState.h"
#include "gameplay/economy/MineProductionRules.h"

using gameplay::ResourceType;
using gameplay::economy::ComputeMineDailyOutput;
using gameplay::economy::MineProductionPassive;
using gameplay::economy::MineResourceOutput;

namespace {

// Finds the resolved amount for a resource in an output vector; -1 if absent.
int AmountOf(const std::vector<MineResourceOutput>& outputs, ResourceType resource) {
    for (const auto& o : outputs) {
        if (o.resource == resource) {
            return o.amount;
        }
    }
    return -1;
}

} // namespace

TEST_CASE("MineProduction - base output is unchanged with no passives") {
    const std::vector<MineResourceOutput> base = {{ResourceType::Stone, 2}};

    const auto result = ComputeMineDailyOutput(base, {});

    REQUIRE(result.size() == 1);
    REQUIRE(result[0].resource == ResourceType::Stone);
    REQUIRE(result[0].amount == 2);
}

TEST_CASE("MineProduction - strongest passive wins for a resource") {
    const std::vector<MineResourceOutput> base = {{ResourceType::Stone, 2}};
    const std::vector<MineProductionPassive> passives = {
        {ResourceType::Stone, 1},  // weaker (Kobold)
        {ResourceType::Stone, 2}   // strongest (Stonewright)
    };

    const auto result = ComputeMineDailyOutput(base, passives);

    // 2 base + strongest (+2), not +1+2: output is 4, not 5.
    REQUIRE(AmountOf(result, ResourceType::Stone) == 4);
}

TEST_CASE("MineProduction - equal-strength passives do not stack") {
    const std::vector<MineResourceOutput> base = {{ResourceType::Stone, 2}};
    const std::vector<MineProductionPassive> passives = {
        {ResourceType::Stone, 2},
        {ResourceType::Stone, 2}
    };

    const auto result = ComputeMineDailyOutput(base, passives);

    // +2 and +2 still gives only +2 -> 4.
    REQUIRE(AmountOf(result, ResourceType::Stone) == 4);
}

TEST_CASE("MineProduction - modifiers resolve independently per resource") {
    const std::vector<MineResourceOutput> base = {
        {ResourceType::Stone, 2},
        {ResourceType::Clay, 1}
    };
    const std::vector<MineProductionPassive> passives = {
        {ResourceType::Stone, 2},
        {ResourceType::Stone, 1},  // weaker stone, ignored
        {ResourceType::Clay, 3}
    };

    const auto result = ComputeMineDailyOutput(base, passives);

    REQUIRE(result.size() == 2);
    REQUIRE(AmountOf(result, ResourceType::Stone) == 4);  // 2 + strongest 2
    REQUIRE(AmountOf(result, ResourceType::Clay) == 4);   // 1 + 3
}

TEST_CASE("MineProduction - passive for a non-produced resource creates no output") {
    const std::vector<MineResourceOutput> base = {{ResourceType::Stone, 2}};
    const std::vector<MineProductionPassive> passives = {
        {ResourceType::Wood, 5}  // mine does not produce Wood
    };

    const auto result = ComputeMineDailyOutput(base, passives);

    REQUIRE(result.size() == 1);
    REQUIRE(AmountOf(result, ResourceType::Stone) == 2);
    REQUIRE(AmountOf(result, ResourceType::Wood) == -1);  // never introduced
}

TEST_CASE("MineProduction - Gold output is representable in pure output data") {
    const std::vector<MineResourceOutput> base = {{ResourceType::Gold, 1000}};
    const std::vector<MineProductionPassive> passives = {
        {ResourceType::Gold, 250}
    };

    const auto result = ComputeMineDailyOutput(base, passives);

    REQUIRE(result.size() == 1);
    REQUIRE(result[0].resource == ResourceType::Gold);
    REQUIRE(result[0].amount == 1250);
}

TEST_CASE("MineProduction - base output order is preserved") {
    const std::vector<MineResourceOutput> base = {
        {ResourceType::Gems, 1},
        {ResourceType::Stone, 2},
        {ResourceType::Wood, 3}
    };

    const auto result = ComputeMineDailyOutput(base, {});

    REQUIRE(result.size() == 3);
    REQUIRE(result[0].resource == ResourceType::Gems);
    REQUIRE(result[1].resource == ResourceType::Stone);
    REQUIRE(result[2].resource == ResourceType::Wood);
}

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <vector>

#include "data/definitions/LocationServiceDefinition.h"
#include "data/definitions/UnitDefinition.h"
#include "gameplay/economy/MineProductionRules.h"
#include "gameplay/economy/StationedProductionRules.h"

using gameplay::ResourceType;
using gameplay::economy::CollectMineProductionPassives;
using gameplay::economy::MineProductionPassive;

namespace {

// Builds a unit definition of `category` carrying a mine-production passive.
data::UnitDefinition MakeUnitWithPassive(const std::string& id,
    data::UnitDefinitionCategory category,
    const std::string& resource, int amount,
    const std::string& target = "mine") {
    data::UnitDefinition u;
    u.id = id;
    u.category = category;
    u.passiveEffects.push_back(data::UnitPassiveEffect{
        data::PassiveEffectKind::MineProduction, resource, target, amount});
    return u;
}

data::UnitDefinition MakePlainUnit(const std::string& id,
    data::UnitDefinitionCategory category) {
    data::UnitDefinition u;
    u.id = id;
    u.category = category;
    return u;  // no passive effects
}

bool HasPassive(const std::vector<MineProductionPassive>& list,
    ResourceType resource, int amount) {
    return std::ranges::any_of(list, [&](const MineProductionPassive& p) {
        return p.resource == resource && p.amount == amount;
    });
}

} // namespace

TEST_CASE("StationedProduction - hero unit passive is recognized when stationed") {
    const auto hero = MakeUnitWithPassive("hero_smith",
        data::UnitDefinitionCategory::Hero, "Gold", 250);
    const std::vector<const data::UnitDefinition*> stationed = {&hero};

    const auto passives = CollectMineProductionPassives(
        stationed, data::LocationServiceKind::Mine);

    REQUIRE(passives.size() == 1);
    REQUIRE(HasPassive(passives, ResourceType::Gold, 250));
}

TEST_CASE("StationedProduction - generic unit passive is recognized when stationed") {
    const auto kobold = MakeUnitWithPassive("kobold",
        data::UnitDefinitionCategory::Generic, "Stone", 1);
    const std::vector<const data::UnitDefinition*> stationed = {&kobold};

    const auto passives = CollectMineProductionPassives(
        stationed, data::LocationServiceKind::Mine);

    REQUIRE(passives.size() == 1);
    REQUIRE(HasPassive(passives, ResourceType::Stone, 1));
}

TEST_CASE("StationedProduction - eligibility is by passive, not by category") {
    // A Leader-category unit with the passive contributes; a Hero-category unit
    // without it does not. Category must never be the deciding factor.
    const auto leaderWith = MakeUnitWithPassive("warlord",
        data::UnitDefinitionCategory::Leader, "Steel", 3);
    const auto heroWithout = MakePlainUnit("bard", data::UnitDefinitionCategory::Hero);
    const std::vector<const data::UnitDefinition*> stationed = {&leaderWith, &heroWithout};

    const auto passives = CollectMineProductionPassives(
        stationed, data::LocationServiceKind::Mine);

    REQUIRE(passives.size() == 1);
    REQUIRE(HasPassive(passives, ResourceType::Steel, 3));
}

TEST_CASE("StationedProduction - units without the passive contribute nothing") {
    const auto plain1 = MakePlainUnit("plain1", data::UnitDefinitionCategory::Generic);
    const auto plain2 = MakePlainUnit("plain2", data::UnitDefinitionCategory::Hero);
    const std::vector<const data::UnitDefinition*> stationed = {&plain1, &plain2};

    const auto passives = CollectMineProductionPassives(
        stationed, data::LocationServiceKind::Mine);

    REQUIRE(passives.empty());
}

TEST_CASE("StationedProduction - null entries are skipped") {
    const auto kobold = MakeUnitWithPassive("kobold",
        data::UnitDefinitionCategory::Generic, "Stone", 1);
    const std::vector<const data::UnitDefinition*> stationed = {nullptr, &kobold, nullptr};

    const auto passives = CollectMineProductionPassives(
        stationed, data::LocationServiceKind::Mine);

    REQUIRE(passives.size() == 1);
    REQUIRE(HasPassive(passives, ResourceType::Stone, 1));
}

TEST_CASE("StationedProduction - collected passives feed ComputeMineDailyOutput strongest-only") {
    const auto kobold = MakeUnitWithPassive("kobold",
        data::UnitDefinitionCategory::Generic, "Stone", 1);
    const auto stonewright = MakeUnitWithPassive("stonewright",
        data::UnitDefinitionCategory::Generic, "Stone", 2);
    const std::vector<const data::UnitDefinition*> stationed = {&kobold, &stonewright};

    const auto passives = CollectMineProductionPassives(
        stationed, data::LocationServiceKind::Mine);
    REQUIRE(passives.size() == 2);

    const std::vector<gameplay::economy::MineResourceOutput> base = {{ResourceType::Stone, 2}};
    const auto output = gameplay::economy::ComputeMineDailyOutput(base, passives);

    REQUIRE(output.size() == 1);
    REQUIRE(output[0].resource == ResourceType::Stone);
    REQUIRE(output[0].amount == 4);  // 2 base + strongest (+2), not +1+2
}

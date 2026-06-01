#include <catch2/catch_test_macros.hpp>

#include "data/definitions/CampaignDefinition.h"
#include "gameplay/campaign/CampaignProgressionRules.h"
#include "gameplay/scenario/ScenarioOutcomeRules.h"

using gameplay::campaign::ResolveNextScenarioId;
using gameplay::scenario::ScenarioOutcomeState;

namespace {

data::CampaignDefinition MakeLinearCampaign() {
    data::CampaignDefinition c;
    c.id = "camp";
    c.startScenarioId = "s1";
    c.scenarios.push_back({"s1", {"s2"}, "rule"});
    c.scenarios.push_back({"s2", {}, ""});   // final scenario, no next
    return c;
}

} // namespace

TEST_CASE("Campaign progression: Victory advances to the next scenario") {
    const auto campaign = MakeLinearCampaign();
    const auto next = ResolveNextScenarioId(campaign, "s1", ScenarioOutcomeState::Victory);
    REQUIRE(next.has_value());
    REQUIRE(*next == "s2");
}

TEST_CASE("Campaign progression: final scenario yields no next on Victory") {
    const auto campaign = MakeLinearCampaign();
    const auto next = ResolveNextScenarioId(campaign, "s2", ScenarioOutcomeState::Victory);
    REQUIRE_FALSE(next.has_value());
}

TEST_CASE("Campaign progression: Defeat never advances") {
    const auto campaign = MakeLinearCampaign();
    REQUIRE_FALSE(ResolveNextScenarioId(campaign, "s1", ScenarioOutcomeState::Defeat).has_value());
}

TEST_CASE("Campaign progression: Ongoing never advances") {
    const auto campaign = MakeLinearCampaign();
    REQUIRE_FALSE(ResolveNextScenarioId(campaign, "s1", ScenarioOutcomeState::Ongoing).has_value());
}

TEST_CASE("Campaign progression: unknown current scenario yields no next") {
    const auto campaign = MakeLinearCampaign();
    REQUIRE_FALSE(ResolveNextScenarioId(campaign, "missing", ScenarioOutcomeState::Victory).has_value());
}

TEST_CASE("Campaign progression: branching picks the first listed next (deterministic)") {
    data::CampaignDefinition c;
    c.id = "camp";
    c.startScenarioId = "s1";
    c.scenarios.push_back({"s1", {"sA", "sB"}, ""});
    const auto next = ResolveNextScenarioId(c, "s1", ScenarioOutcomeState::Victory);
    REQUIRE(next.has_value());
    REQUIRE(*next == "sA");
}

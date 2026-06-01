#pragma once

#include <string>
#include <vector>

namespace data {

// One scenario node in a campaign's transition graph. nextScenarioIds is a list
// to leave room for branching, but M16 progression picks deterministically (the
// first listed next scenario) — branching-choice UI is deferred.
struct CampaignScenarioEntry {
    std::string scenarioId;
    std::vector<std::string> nextScenarioIds;
    std::string carryOverRuleId;   // optional; empty => carry nothing but the player hero
};

// Explicit allow-list carry-over rule (M16). Expressed against the current
// runtime model — no hero-instance system. Campaign flags persist across
// scenarios independently of any rule, so they are not represented here; this
// rule only governs scenario-local runtime carry-over.
//   - carryRoster=false  => keep only the player hero, drop other roster units
//   - carryArtifacts     => governs BOTH unequipped artifacts and hero equipment
//   - carryStoryFlags    => the named scenario story flags to preserve
struct CarryOverRule {
    std::string id;
    bool carryGold = false;
    bool carryRoster = false;
    bool carryItems = false;
    bool carryArtifacts = false;
    std::vector<std::string> carryStoryFlags;
};

// A Campaign sequences thin Scenarios with authored carry-over (M16 / Phase 7).
struct CampaignDefinition {
    std::string id;
    std::string name;
    std::string description;
    std::string startScenarioId;
    std::vector<CampaignScenarioEntry> scenarios;
    std::vector<std::string> campaignFlags;
    std::vector<CarryOverRule> carryOverRules;

    [[nodiscard]] const CampaignScenarioEntry* FindScenarioEntry(const std::string& scenarioId) const {
        for (const auto& entry : scenarios) {
            if (entry.scenarioId == scenarioId) {
                return &entry;
            }
        }
        return nullptr;
    }

    [[nodiscard]] const CarryOverRule* FindCarryOverRule(const std::string& ruleId) const {
        for (const auto& rule : carryOverRules) {
            if (rule.id == ruleId) {
                return &rule;
            }
        }
        return nullptr;
    }
};

} // namespace data

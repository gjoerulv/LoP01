#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include "data/ContentRepository.h"
#include "gameplay/GameSession.h"

// LOP01_PROJECT_ROOT is set by CMakeLists.txt to the absolute repo root so that
// these end-to-end tests can load the actual content/ directory used by the
// running app. If absent the tests degrade to the current working directory,
// which on developer machines is typically the build folder and will fail
// loudly rather than silently.
#ifndef LOP01_PROJECT_ROOT
#define LOP01_PROJECT_ROOT "."
#endif

namespace {

std::filesystem::path RealContentDir() {
    return std::filesystem::path(LOP01_PROJECT_ROOT) / "content";
}

bool HasErrorMessage(const std::vector<ValidationMessage>& msgs) {
    return std::any_of(msgs.begin(), msgs.end(),
        [](const ValidationMessage& m) { return m.severity == Severity::Error; });
}

// Mirrors the wiring App::App() performs for the running game, so the test
// exercises the exact integration the player hits in-game.
gameplay::GameSession MakeRealContentSession(data::ContentRepository& repo) {
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));
    REQUIRE_FALSE(HasErrorMessage(repo.ValidationMessages()));

    gameplay::GameSession session;
    session.InitializeQuestState(repo.QuestDefinitions());
    session.InitializeEventDefinitions(repo.EventDefinitions());
    session.SetScenarioOutcomeDefinition(repo.ScenarioOutcome());
    session.SetItemCatalog(repo.Items());
    session.SetArtifactCatalog(repo.Artifacts());
    session.SetPlayerColor("Green");

    // Recruit the player character into the active party. This matches the
    // App-startup roster path: hero_player is the leader-capable player
    // character and must be on the team before equip operations are legal.
    session.SetLeaderCapableUnitIds({"hero_player"});
    REQUIRE(session.AddOwnedUnit("hero_player", 1));
    REQUIRE(session.TryAddUnitToActiveParty("hero_player"));

    return session;
}

} // namespace

TEST_CASE("End-to-end - real items.json loads the two authored demo items") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));
    REQUIRE_FALSE(HasErrorMessage(repo.ValidationMessages()));

    const auto* ration = repo.FindItemById("item_traveler_ration");
    REQUIRE(ration != nullptr);
    REQUIRE(ration->subtype == data::ItemSubtype::Consumable);
    REQUIRE(ration->stackCap == 1);

    const auto* token = repo.FindItemById("item_ashvale_token");
    REQUIRE(token != nullptr);
    REQUIRE(token->subtype == data::ItemSubtype::Quest);
}

TEST_CASE("End-to-end - real artifacts.json loads the two authored demo artifacts with statBonuses") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));
    REQUIRE_FALSE(HasErrorMessage(repo.ValidationMessages()));

    const auto* sword = repo.FindArtifactById("artifact_iron_sword");
    REQUIRE(sword != nullptr);
    REQUIRE(sword->combinable);
    REQUIRE(sword->allowedSlots.size() == 1);
    REQUIRE(sword->allowedSlots[0] == data::ArtifactSlotKind::Attack);
    REQUIRE(sword->statBonuses.size() == 1);
    REQUIRE(sword->statBonuses[0].stat == data::ArtifactStatBonusStat::Attack);
    REQUIRE(sword->statBonuses[0].amount == 2);

    const auto* charm = repo.FindArtifactById("artifact_journeyman_charm");
    REQUIRE(charm != nullptr);
    REQUIRE_FALSE(charm->combinable);
    REQUIRE(charm->allowedSlots[0] == data::ArtifactSlotKind::Misc);
    REQUIRE(charm->statBonuses[0].stat == data::ArtifactStatBonusStat::Defense);
    REQUIRE(charm->statBonuses[0].amount == 1);
}

TEST_CASE("End-to-end - real events.json wires the supply_cart pickup event") {
    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(RealContentDir()));

    bool hasSupplyCartEvent = false;
    for (const auto& e : repo.EventDefinitions()) {
        if (e.id == "evt_supply_cart_pickup") {
            REQUIRE(e.trigger.type == gameplay::events::EventTriggerType::RegionNodeEntry);
            REQUIRE(e.trigger.targetId == "supply_cart");
            hasSupplyCartEvent = true;
            break;
        }
    }
    REQUIRE(hasSupplyCartEvent);
}

TEST_CASE("End-to-end - visiting supply_cart grants the ration item and iron sword artifact") {
    data::ContentRepository repo;
    auto session = MakeRealContentSession(repo);
    REQUIRE(session.Items().empty());
    REQUIRE(session.Artifacts().empty());

    const auto results = session.NotifyRegionNodeEntry("supply_cart");
    REQUIRE_FALSE(results.empty());
    for (const auto& r : results) {
        REQUIRE(r.success);
    }

    // Ration ends up in items_; iron sword ends up in artifacts_ (unequipped).
    REQUIRE(session.Items().size() == 1);
    REQUIRE(session.Items()[0].itemId == "item_traveler_ration");
    REQUIRE(session.Items()[0].quantity == 1);

    REQUIRE(session.Artifacts().size() == 1);
    REQUIRE(session.Artifacts()[0].artifactId == "artifact_iron_sword");
    REQUIRE(session.Artifacts()[0].quantity == 1);

    // The hero's equipment is still empty until an explicit equip.
    const auto eq = session.HeroEquipment("hero_player");
    REQUIRE(eq.attackArtifactId.empty());
}

TEST_CASE("End-to-end - equipping the iron sword moves it from inventory into the hero's Attack slot") {
    data::ContentRepository repo;
    auto session = MakeRealContentSession(repo);
    static_cast<void>(session.NotifyRegionNodeEntry("supply_cart"));

    const auto result = session.TryEquipArtifact(
        "hero_player",
        gameplay::ArtifactEquipSlot::Attack,
        "artifact_iron_sword");
    REQUIRE(result.success);
    REQUIRE(session.Artifacts().empty());
    REQUIRE(session.HeroEquipment("hero_player").attackArtifactId == "artifact_iron_sword");
}

TEST_CASE("End-to-end - the supply_cart event is once-only (a second visit grants nothing)") {
    data::ContentRepository repo;
    auto session = MakeRealContentSession(repo);
    static_cast<void>(session.NotifyRegionNodeEntry("supply_cart"));
    REQUIRE(session.Items().size() == 1);
    REQUIRE(session.Artifacts().size() == 1);

    // Re-visit. The once-mode event must not duplicate the grants.
    static_cast<void>(session.NotifyRegionNodeEntry("supply_cart"));
    REQUIRE(session.Items().size() == 1);
    REQUIRE(session.Items()[0].quantity == 1);
    REQUIRE(session.Artifacts().size() == 1);
    REQUIRE(session.Artifacts()[0].quantity == 1);
}

TEST_CASE("End-to-end - M12 victory/defeat outcomes still resolve correctly with M13 content in place") {
    // Regression check: M13-c authored content (items, artifacts, supply_cart
    // event) must not interfere with M12 scenario outcome resolution. Visiting
    // sunken_ruin still latches Victory; visiting clocktower_square still
    // latches Defeat — independent of inventory state.
    data::ContentRepository repo;
    auto session = MakeRealContentSession(repo);

    static_cast<void>(session.NotifyRegionNodeEntry("sunken_ruin"));
    REQUIRE(session.IsScenarioEnded());
    REQUIRE(session.Outcome().has_value());
    REQUIRE(session.Outcome()->state == gameplay::scenario::ScenarioOutcomeState::Victory);
}

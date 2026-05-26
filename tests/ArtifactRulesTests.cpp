#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "data/definitions/ArtifactDefinition.h"
#include "gameplay/GameSession.h"
#include "gameplay/events/EventDefinition.h"

using namespace gameplay;

namespace {

data::ArtifactDefinition MakeArtifact(
    const std::string& id,
    std::vector<data::ArtifactSlotKind> allowedSlots,
    std::vector<data::ArtifactStatBonus> bonuses = {})
{
    data::ArtifactDefinition def;
    def.id = id;
    def.name = id;
    def.allowedSlots = std::move(allowedSlots);
    def.statBonuses = std::move(bonuses);
    return def;
}

// Authored start-of-day event that grants the team one or more copies of an
// artifact. Driving setup through real authored events matches the production
// inventory-acquisition path exactly.
events::EventDefinition MakeGiveArtifactEvent(
    const std::string& eventId,
    const std::string& artifactId,
    int amount)
{
    events::EventDefinition def;
    def.id = eventId;
    def.trigger.type = events::EventTriggerType::StartOfDay;
    def.repeat.mode = "once";
    events::EventAction action;
    action.type = "giveArtifact";
    action.args = nlohmann::json{
        {"type", "giveArtifact"},
        {"artifactId", artifactId},
        {"amount", amount}
    };
    def.actions.push_back(action);
    return def;
}

// Builds a session with one player-character hero in the active party, the
// supplied artifact catalog wired up, and (optionally) seeded with one or
// more unequipped copies of `seedArtifactId` via a real start-of-day event.
GameSession MakeSessionWithHero(
    const std::string& heroId,
    std::vector<data::ArtifactDefinition> catalog,
    const std::string& seedArtifactId = "",
    int seedQuantity = 0)
{
    GameSession session;
    session.SetLeaderCapableUnitIds({heroId});
    REQUIRE(session.AddOwnedUnit(heroId, 1));
    REQUIRE(session.TryAddUnitToActiveParty(heroId));
    session.SetArtifactCatalog(catalog);

    if (!seedArtifactId.empty() && seedQuantity > 0) {
        session.InitializeEventDefinitions({
            MakeGiveArtifactEvent("evt_seed", seedArtifactId, seedQuantity)
        });
        const auto results = session.NotifyStartOfDay();
        REQUIRE(results.size() == 1);
        REQUIRE(results[0].success);
    }

    return session;
}

} // namespace

TEST_CASE("ArtifactRules - equip moves artifact from inventory into hero slot") {
    auto sword = MakeArtifact("artifact_sword", { data::ArtifactSlotKind::Attack });
    auto session = MakeSessionWithHero("hero_player", { sword }, "artifact_sword", 1);
    REQUIRE(session.Artifacts().size() == 1);

    const auto result = session.TryEquipArtifact(
        "hero_player", ArtifactEquipSlot::Attack, "artifact_sword");
    REQUIRE(result.success);
    REQUIRE(session.Artifacts().empty()); // moved out of unequipped inventory
    REQUIRE(session.HeroEquipment("hero_player").attackArtifactId == "artifact_sword");
}

TEST_CASE("ArtifactRules - equip fails when slot kind not in allowedSlots") {
    auto sword = MakeArtifact("artifact_sword", { data::ArtifactSlotKind::Attack });
    auto session = MakeSessionWithHero("hero_player", { sword }, "artifact_sword", 1);

    const auto result = session.TryEquipArtifact(
        "hero_player", ArtifactEquipSlot::Defense, "artifact_sword");
    REQUIRE_FALSE(result.success);
    REQUIRE_FALSE(result.message.empty());
    REQUIRE(session.Artifacts().size() == 1); // unchanged
    REQUIRE(session.HeroEquipment("hero_player").attackArtifactId.empty());
    REQUIRE(session.HeroEquipment("hero_player").defenseArtifactId.empty());
}

TEST_CASE("ArtifactRules - Misc-allowed artifact can equip into any Misc slot") {
    auto charm = MakeArtifact("artifact_charm", { data::ArtifactSlotKind::Misc });
    auto session = MakeSessionWithHero("hero_player", { charm }, "artifact_charm", 3);

    REQUIRE(session.TryEquipArtifact("hero_player", ArtifactEquipSlot::Misc1, "artifact_charm").success);
    REQUIRE(session.TryEquipArtifact("hero_player", ArtifactEquipSlot::Misc2, "artifact_charm").success);
    REQUIRE(session.TryEquipArtifact("hero_player", ArtifactEquipSlot::Misc3, "artifact_charm").success);
    REQUIRE(session.Artifacts().empty());
    const auto eq = session.HeroEquipment("hero_player");
    REQUIRE(eq.misc1ArtifactId == "artifact_charm");
    REQUIRE(eq.misc2ArtifactId == "artifact_charm");
    REQUIRE(eq.misc3ArtifactId == "artifact_charm");
}

TEST_CASE("ArtifactRules - equip fails when no unequipped copy exists") {
    auto sword = MakeArtifact("artifact_sword", { data::ArtifactSlotKind::Attack });
    auto session = MakeSessionWithHero("hero_player", { sword });

    const auto result = session.TryEquipArtifact(
        "hero_player", ArtifactEquipSlot::Attack, "artifact_sword");
    REQUIRE_FALSE(result.success);
    REQUIRE(session.HeroEquipment("hero_player").attackArtifactId.empty());
}

TEST_CASE("ArtifactRules - equip fails when hero is not on the traveling team") {
    auto sword = MakeArtifact("artifact_sword", { data::ArtifactSlotKind::Attack });
    GameSession session;
    session.SetArtifactCatalog({ sword });
    // Seed without a hero in the party: drive a startOfDay event to inject one
    // artifact copy, then attempt to equip onto a non-existent hero.
    session.InitializeEventDefinitions({
        events::EventDefinition{
            "evt_seed",
            { events::EventTriggerType::StartOfDay, "" },
            {}, {}, std::nullopt,
            { "once", 0 },
            {
                events::EventAction{ "giveArtifact",
                    nlohmann::json{
                        {"type", "giveArtifact"},
                        {"artifactId", "artifact_sword"},
                        {"amount", 1}
                    }
                }
            }
        }
    });
    static_cast<void>(session.NotifyStartOfDay());

    const auto result = session.TryEquipArtifact(
        "hero_unknown", ArtifactEquipSlot::Attack, "artifact_sword");
    REQUIRE_FALSE(result.success);
    REQUIRE(session.Artifacts().size() == 1);
}

TEST_CASE("ArtifactRules - equip fails when artifact id is unknown") {
    auto session = MakeSessionWithHero("hero_player", {});

    const auto result = session.TryEquipArtifact(
        "hero_player", ArtifactEquipSlot::Attack, "artifact_phantom");
    REQUIRE_FALSE(result.success);
}

TEST_CASE("ArtifactRules - equip fails when slot already occupied (no auto-replace)") {
    auto a = MakeArtifact("artifact_a", { data::ArtifactSlotKind::Attack });
    auto b = MakeArtifact("artifact_b", { data::ArtifactSlotKind::Attack });
    GameSession session;
    session.SetLeaderCapableUnitIds({"hero_player"});
    REQUIRE(session.AddOwnedUnit("hero_player", 1));
    REQUIRE(session.TryAddUnitToActiveParty("hero_player"));
    session.SetArtifactCatalog({ a, b });

    // Seed both artifacts in one startOfDay event with two giveArtifact actions.
    events::EventDefinition seed;
    seed.id = "evt_seed_two";
    seed.trigger.type = events::EventTriggerType::StartOfDay;
    seed.repeat.mode = "once";
    for (const auto& id : {"artifact_a", "artifact_b"}) {
        events::EventAction act;
        act.type = "giveArtifact";
        act.args = nlohmann::json{
            {"type", "giveArtifact"}, {"artifactId", id}, {"amount", 1}
        };
        seed.actions.push_back(act);
    }
    session.InitializeEventDefinitions({seed});
    static_cast<void>(session.NotifyStartOfDay());
    REQUIRE(session.Artifacts().size() == 2);

    REQUIRE(session.TryEquipArtifact("hero_player", ArtifactEquipSlot::Attack, "artifact_a").success);
    const auto blocked = session.TryEquipArtifact(
        "hero_player", ArtifactEquipSlot::Attack, "artifact_b");
    REQUIRE_FALSE(blocked.success);
    REQUIRE(session.HeroEquipment("hero_player").attackArtifactId == "artifact_a");
    // artifact_b is still in unequipped inventory; artifact_a moved out.
    REQUIRE(session.Artifacts().size() == 1);
    REQUIRE(session.Artifacts()[0].artifactId == "artifact_b");
}

TEST_CASE("ArtifactRules - unequip returns artifact to unequipped inventory") {
    auto sword = MakeArtifact("artifact_sword", { data::ArtifactSlotKind::Attack });
    auto session = MakeSessionWithHero("hero_player", { sword }, "artifact_sword", 1);
    REQUIRE(session.TryEquipArtifact("hero_player", ArtifactEquipSlot::Attack, "artifact_sword").success);
    REQUIRE(session.Artifacts().empty());

    const auto result = session.UnequipArtifact("hero_player", ArtifactEquipSlot::Attack);
    REQUIRE(result.success);
    REQUIRE(session.HeroEquipment("hero_player").attackArtifactId.empty());
    REQUIRE(session.Artifacts().size() == 1);
    REQUIRE(session.Artifacts()[0].artifactId == "artifact_sword");
    REQUIRE(session.Artifacts()[0].quantity == 1);
}

TEST_CASE("ArtifactRules - unequip fails on empty slot") {
    auto session = MakeSessionWithHero("hero_player", {});
    const auto result = session.UnequipArtifact("hero_player", ArtifactEquipSlot::Attack);
    REQUIRE_FALSE(result.success);
}

TEST_CASE("ArtifactRules - equipping does not duplicate across inventory and slot") {
    auto sword = MakeArtifact("artifact_sword", { data::ArtifactSlotKind::Attack });
    auto session = MakeSessionWithHero("hero_player", { sword }, "artifact_sword", 2);

    REQUIRE(session.TryEquipArtifact("hero_player", ArtifactEquipSlot::Attack, "artifact_sword").success);
    // Exactly one copy remains in inventory; the other lives in the slot.
    REQUIRE(session.Artifacts().size() == 1);
    REQUIRE(session.Artifacts()[0].quantity == 1);
    REQUIRE(session.HeroEquipment("hero_player").attackArtifactId == "artifact_sword");
}

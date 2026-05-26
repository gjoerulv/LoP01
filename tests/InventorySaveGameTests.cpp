#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

#include "core/SaveGame.h"
#include "data/definitions/ArtifactDefinition.h"
#include "data/definitions/ItemDefinition.h"
#include "gameplay/GameSession.h"
#include "gameplay/events/EventDefinition.h"

using namespace gameplay;

namespace {

data::ItemDefinition MakeItem(
    const std::string& id,
    data::ItemSubtype subtype = data::ItemSubtype::Material,
    int stackCap = 999)
{
    data::ItemDefinition def;
    def.id = id;
    def.name = id;
    def.subtype = subtype;
    def.stackCap = stackCap;
    return def;
}

data::ArtifactDefinition MakeArtifact(
    const std::string& id,
    data::ArtifactSlotKind slot)
{
    data::ArtifactDefinition def;
    def.id = id;
    def.name = id;
    def.allowedSlots = { slot };
    return def;
}

// Drives an inventory state into a session via authored events, then equips
// one artifact, so the round-trip touches all three new SaveData containers.
GameSession SeededSession() {
    GameSession session;
    session.SetLeaderCapableUnitIds({"hero_player"});
    REQUIRE(session.AddOwnedUnit("hero_player", 1));
    REQUIRE(session.TryAddUnitToActiveParty("hero_player"));
    session.SetItemCatalog({
        MakeItem("item_ration", data::ItemSubtype::Consumable, 1),
        MakeItem("item_log", data::ItemSubtype::Material, 50)
    });
    session.SetArtifactCatalog({
        MakeArtifact("artifact_sword", data::ArtifactSlotKind::Attack),
        MakeArtifact("artifact_charm", data::ArtifactSlotKind::Misc)
    });

    events::EventDefinition seed;
    seed.id = "evt_seed";
    seed.trigger.type = events::EventTriggerType::StartOfDay;
    seed.repeat.mode = "once";
    for (const auto& [type, id, amt] : std::vector<std::tuple<std::string, std::string, int>>{
            {"giveItem",     "item_ration", 1},
            {"giveItem",     "item_log",    7},
            {"giveArtifact", "artifact_sword", 1},
            {"giveArtifact", "artifact_charm", 2}
         }) {
        events::EventAction act;
        act.type = type;
        if (type.find("Item") != std::string::npos) {
            act.args = nlohmann::json{{"type", type}, {"itemId", id}, {"amount", amt}};
        } else {
            act.args = nlohmann::json{{"type", type}, {"artifactId", id}, {"amount", amt}};
        }
        seed.actions.push_back(act);
    }
    session.InitializeEventDefinitions({seed});
    static_cast<void>(session.NotifyStartOfDay());

    REQUIRE(session.TryEquipArtifact(
        "hero_player", ArtifactEquipSlot::Attack, "artifact_sword").success);
    return session;
}

} // namespace

TEST_CASE("InventorySaveGame - items, artifacts, and equipment survive in-memory round-trip") {
    GameSession source = SeededSession();
    REQUIRE(source.Items().size() == 2);
    REQUIRE(source.Artifacts().size() == 1); // sword equipped, charm unequipped
    REQUIRE(source.HeroEquipment("hero_player").attackArtifactId == "artifact_sword");

    const auto saveData = source.ToSaveData();
    REQUIRE(saveData.items.size() == 2);
    REQUIRE(saveData.artifacts.size() == 1);
    REQUIRE(saveData.heroEquipment.size() == 1);
    REQUIRE(saveData.heroEquipment[0].heroId == "hero_player");
    REQUIRE(saveData.heroEquipment[0].attackArtifactId == "artifact_sword");

    GameSession restored;
    restored.ApplySaveData(saveData);

    REQUIRE(restored.Items().size() == 2);
    REQUIRE(restored.Artifacts().size() == 1);
    REQUIRE(restored.Artifacts()[0].artifactId == "artifact_charm");
    REQUIRE(restored.Artifacts()[0].quantity == 2);
    REQUIRE(restored.HeroEquipment("hero_player").attackArtifactId == "artifact_sword");
}

TEST_CASE("InventorySaveGame - round-trip via SaveGameRepository file write/read") {
    const std::filesystem::path savePath = "saves/inventory_savegame_test/round.json";
    std::filesystem::create_directories(savePath.parent_path());

    GameSession source = SeededSession();
    const auto saveData = source.ToSaveData();
    core::SaveGameRepository repo;
    REQUIRE(repo.SaveToFile(saveData, savePath.string()));

    const auto loaded = repo.LoadFromFile(savePath.string());
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->items.size() == 2);
    REQUIRE(loaded->artifacts.size() == 1);
    REQUIRE(loaded->artifacts[0].artifactId == "artifact_charm");
    REQUIRE(loaded->artifacts[0].quantity == 2);
    REQUIRE(loaded->heroEquipment.size() == 1);
    REQUIRE(loaded->heroEquipment[0].attackArtifactId == "artifact_sword");

    std::filesystem::remove_all(savePath.parent_path());
}

TEST_CASE("InventorySaveGame - legacy save (no inventory keys) loads as empty inventories") {
    const std::filesystem::path savePath = "saves/inventory_legacy_test/legacy.json";
    std::filesystem::create_directories(savePath.parent_path());

    std::ofstream out(savePath, std::ios::trunc);
    out << R"({
  "schema_version": 5,
  "day": 1,
  "minutes_into_slice_day": 0,
  "gold": 100,
  "mode": "overworld_mode",
  "region_id": "r1",
  "destination_id": "d1",
  "roster_stacks": [],
  "active_slot_stack_ids": ["","","","",""],
  "reserve_slot_stack_ids": ["","","","","","","",""]
})";
    out.close();

    core::SaveGameRepository repo;
    const auto loaded = repo.LoadFromFile(savePath.string());
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->items.empty());
    REQUIRE(loaded->artifacts.empty());
    REQUIRE(loaded->heroEquipment.empty());

    GameSession restored;
    restored.ApplySaveData(*loaded);
    REQUIRE(restored.Items().empty());
    REQUIRE(restored.Artifacts().empty());

    std::filesystem::remove_all(savePath.parent_path());
}

TEST_CASE("InventorySaveGame - equipped artifacts are never duplicated across artifacts_ and hero slot in save data") {
    GameSession source = SeededSession();
    const auto saveData = source.ToSaveData();

    // artifact_sword is equipped; it must not appear in saveData.artifacts.
    for (const auto& a : saveData.artifacts) {
        REQUIRE(a.artifactId != "artifact_sword");
    }
    // It must appear in heroEquipment.
    bool foundInEquipment = false;
    for (const auto& e : saveData.heroEquipment) {
        if (e.attackArtifactId == "artifact_sword") {
            foundInEquipment = true;
            break;
        }
    }
    REQUIRE(foundInEquipment);
}

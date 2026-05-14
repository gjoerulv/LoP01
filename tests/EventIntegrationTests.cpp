#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

#include "core/SaveGame.h"
#include "data/ContentRepository.h"
#include "gameplay/GameSession.h"
#include "gameplay/events/EventDefinition.h"
#include "gameplay/events/EventParser.h"

using namespace gameplay::events;

namespace {

void WriteTextFile(const std::filesystem::path& path, const std::string& content) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path, std::ios::trunc);
    output << content;
}

void WriteMinimalContentDir(const std::filesystem::path& root) {
    std::filesystem::create_directories(root);
    WriteTextFile(root / "regions.json",
        R"({"schemaVersion":1,"kind":"RegionCollection","id":"regions","regions":[]})");
    WriteTextFile(root / "locations.json",
        R"({"schemaVersion":1,"kind":"LocationCollection","id":"locations","locations":[]})");
    WriteTextFile(root / "location_scenes.json",
        R"({"schemaVersion":1,"kind":"LocationSceneCollection","id":"location_scenes","location_scenes":[]})");
    WriteTextFile(root / "units.json",
        R"({"schemaVersion":1,"kind":"UnitCollection","id":"units","units":[]})");
    WriteTextFile(root / "battle_scenarios.json",
        R"({"schemaVersion":1,"kind":"BattleScenarioCollection","id":"battle_scenarios","battle_scenarios":[]})");
    WriteTextFile(root / "enemy_groups.json",
        R"({"schemaVersion":1,"kind":"EnemyGroupCollection","id":"enemy_groups","enemy_groups":[]})");
    WriteTextFile(root / "quests.json",
        R"({"schemaVersion":1,"kind":"QuestCollection","id":"quests","quests":[]})");
    WriteTextFile(root / "location_services.json",
        R"({"schemaVersion":1,"kind":"LocationServiceCollection","id":"location_services","location_services":[]})");
}

EventDefinition MakeStartOfDayEvent(
    const std::string& id,
    std::vector<EventAction> actions = {},
    const std::string& repeatMode = "once")
{
    EventDefinition def;
    def.id = id;
    def.trigger.type = EventTriggerType::StartOfDay;
    def.actions = std::move(actions);
    def.repeat.mode = repeatMode;
    return def;
}

EventAction MakeAction(const nlohmann::json& j) {
    EventAction a;
    a.type = j.value("type", "");
    a.args = j;
    return a;
}

} // namespace

// ---------------------------------------------------------------------------
// ContentRepository — events.json loading
// ---------------------------------------------------------------------------

TEST_CASE("ContentRepository - LoadFromDirectory succeeds when events.json is absent") {
    const std::filesystem::path root = "saves/event_integration_test/no_events";
    WriteMinimalContentDir(root);

    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(root));
    REQUIRE(repo.EventDefinitions().empty());
}

TEST_CASE("ContentRepository - loads event definitions from events.json") {
    const std::filesystem::path root = "saves/event_integration_test/load_events";
    WriteMinimalContentDir(root);
    WriteTextFile(root / "events.json",
        R"({"schemaVersion":1,"kind":"EventCollection","id":"events","events":[)"
        R"({"id":"evt_dawn","trigger":{"type":"startOfDay"},"actions":[]},)"
        R"({"id":"evt_dusk","trigger":{"type":"startOfDay"},"actions":[]})"
        R"(]})");

    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(root));
    REQUIRE(repo.EventDefinitions().size() == 2);
}

TEST_CASE("ContentRepository - EventDefinitions returns correct count and ids") {
    const std::filesystem::path root = "saves/event_integration_test/event_ids";
    WriteMinimalContentDir(root);
    WriteTextFile(root / "events.json",
        R"({"schemaVersion":1,"kind":"EventCollection","id":"events","events":[)"
        R"({"id":"evt_alpha","trigger":{"type":"startOfDay"},"actions":[]},)"
        R"({"id":"evt_beta","trigger":{"type":"startOfDay"},"actions":[]})"
        R"(]})");

    data::ContentRepository repo;
    REQUIRE(repo.LoadFromDirectory(root));
    REQUIRE(repo.EventDefinitions()[0].id == "evt_alpha");
    REQUIRE(repo.EventDefinitions()[1].id == "evt_beta");
}

TEST_CASE("ContentRepository - duplicate event id produces EVENT_ID_DUPLICATE message") {
    const std::filesystem::path root = "saves/event_integration_test/dup_id";
    WriteMinimalContentDir(root);
    WriteTextFile(root / "events.json",
        R"({"schemaVersion":1,"kind":"EventCollection","id":"events","events":[)"
        R"({"id":"evt_same","trigger":{"type":"startOfDay"},"actions":[]},)"
        R"({"id":"evt_same","trigger":{"type":"startOfDay"},"actions":[]})"
        R"(]})");

    data::ContentRepository repo;
    (void)repo.LoadFromDirectory(root);

    const auto& msgs = repo.ValidationMessages();
    const bool hasDupError = std::ranges::any_of(msgs,
        [](const ValidationMessage& m) { return m.code == "EVENT_ID_DUPLICATE"; });
    REQUIRE(hasDupError);
}

TEST_CASE("ContentRepository - duplicate priority for same trigger type produces EVENT_PRIORITY_DUPLICATE") {
    const std::filesystem::path root = "saves/event_integration_test/dup_priority";
    WriteMinimalContentDir(root);
    WriteTextFile(root / "events.json",
        R"({"schemaVersion":1,"kind":"EventCollection","id":"events","events":[)"
        R"({"id":"evt_a","trigger":{"type":"startOfDay"},"priority":1,"actions":[]},)"
        R"({"id":"evt_b","trigger":{"type":"startOfDay"},"priority":1,"actions":[]})"
        R"(]})");

    data::ContentRepository repo;
    (void)repo.LoadFromDirectory(root);

    const auto& msgs = repo.ValidationMessages();
    const bool hasPriorityError = std::ranges::any_of(msgs,
        [](const ValidationMessage& m) { return m.code == "EVENT_PRIORITY_DUPLICATE"; });
    REQUIRE(hasPriorityError);
}

// ---------------------------------------------------------------------------
// SaveData — round-trip tests
// ---------------------------------------------------------------------------

TEST_CASE("SaveData - firedEventIds round-trips through save/load") {
    const std::filesystem::path savePath = "saves/event_integration_test/fired_round_trip.json";
    std::filesystem::create_directories(savePath.parent_path());

    core::SaveData original;
    original.schemaVersion = 4;
    original.day = 1;
    original.mode = "overworld_mode";
    original.regionId = "test_region";
    original.destinationId = "test_dest";
    original.hasCanonicalRoster = true;
    original.activeSlotStackIds = {"", "", "", "", ""};
    original.reserveSlotStackIds = {"", "", "", "", "", "", "", ""};
    original.firedEventIds = {"evt_dawn", "evt_reward"};

    core::SaveGameRepository repo;
    REQUIRE(repo.SaveToFile(original, savePath.string()));
    const auto loaded = repo.LoadFromFile(savePath.string());
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->firedEventIds == std::vector<std::string>{"evt_dawn", "evt_reward"});

    std::filesystem::remove(savePath);
}

TEST_CASE("SaveData - storyFlags round-trips through save/load") {
    const std::filesystem::path savePath = "saves/event_integration_test/flags_round_trip.json";
    std::filesystem::create_directories(savePath.parent_path());

    core::SaveData original;
    original.schemaVersion = 4;
    original.day = 1;
    original.mode = "overworld_mode";
    original.regionId = "test_region";
    original.destinationId = "test_dest";
    original.hasCanonicalRoster = true;
    original.activeSlotStackIds = {"", "", "", "", ""};
    original.reserveSlotStackIds = {"", "", "", "", "", "", "", ""};
    original.storyFlags = {"chapter2_started", "old_flag"};

    core::SaveGameRepository repo;
    REQUIRE(repo.SaveToFile(original, savePath.string()));
    const auto loaded = repo.LoadFromFile(savePath.string());
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->storyFlags == std::vector<std::string>{"chapter2_started", "old_flag"});

    std::filesystem::remove(savePath);
}

TEST_CASE("SaveData - v3 save without event fields loads without error; fields default to empty") {
    const std::filesystem::path savePath = "saves/event_integration_test/v3_compat.json";
    std::filesystem::create_directories(savePath.parent_path());

    std::ofstream out(savePath, std::ios::trunc);
    out << R"({
  "schema_version": 3,
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
    REQUIRE(loaded->firedEventIds.empty());
    REQUIRE(loaded->storyFlags.empty());

    std::filesystem::remove(savePath);
}

// ---------------------------------------------------------------------------
// GameSession — NotifyStartOfDay
// ---------------------------------------------------------------------------

TEST_CASE("GameSession - NotifyStartOfDay fires startOfDay event and returns action results") {
    gameplay::GameSession session;
    session.InitializeEventDefinitions({MakeStartOfDayEvent("evt_dawn", {
        MakeAction({{"type", "showMessage"}, {"text", {{"en", "A new day begins."}}}})
    })});

    const auto results = session.NotifyStartOfDay();
    REQUIRE(results.size() == 1);
    REQUIRE(results[0].success);
    REQUIRE(results[0].message == "A new day begins.");
}

TEST_CASE("GameSession - NotifyStartOfDay skips event with unmet condition") {
    gameplay::GameSession session;

    EventCondition cond;
    cond.kind = EventConditionKind::Leaf;
    cond.leafType = "storyFlagSet";
    cond.leafArgs = {{"type", "storyFlagSet"}, {"flag", "absent_flag"}};

    EventDefinition def = MakeStartOfDayEvent("evt_conditional", {
        MakeAction({{"type", "showMessage"}, {"text", {{"en", "Should not fire."}}}})
    });
    def.condition = cond;

    session.InitializeEventDefinitions({def});
    const auto results = session.NotifyStartOfDay();
    REQUIRE(results.empty());
}

TEST_CASE("GameSession - NotifyStartOfDay once-mode event is added to firedEventIds after firing") {
    gameplay::GameSession session;
    session.InitializeEventDefinitions({MakeStartOfDayEvent("evt_once")});

    (void)session.NotifyStartOfDay();

    const auto saveData = session.ToSaveData();
    REQUIRE(std::ranges::any_of(saveData.firedEventIds,
        [](const auto& id) { return id == "evt_once"; }));
}

TEST_CASE("GameSession - NotifyStartOfDay skips already-fired once-mode event") {
    gameplay::GameSession session;
    session.InitializeEventDefinitions({MakeStartOfDayEvent("evt_once", {
        MakeAction({{"type", "showMessage"}, {"text", {{"en", "Fired once."}}}})
    })});

    const auto firstResults = session.NotifyStartOfDay();
    REQUIRE(firstResults.size() == 1);

    const auto secondResults = session.NotifyStartOfDay();
    REQUIRE(secondResults.empty());
}

TEST_CASE("GameSession - NotifyStartOfDay setStoryFlag action persists to storyFlags_") {
    gameplay::GameSession session;
    session.InitializeEventDefinitions({MakeStartOfDayEvent("evt_flag", {
        MakeAction({{"type", "setStoryFlag"}, {"flag", "chapter2_started"}})
    })});

    (void)session.NotifyStartOfDay();

    const auto saveData = session.ToSaveData();
    REQUIRE(std::ranges::any_of(saveData.storyFlags,
        [](const auto& f) { return f == "chapter2_started"; }));
}

TEST_CASE("GameSession - storyFlags survive ToSaveData / ApplySaveData round-trip") {
    gameplay::GameSession session;
    session.InitializeEventDefinitions({MakeStartOfDayEvent("evt_flag", {
        MakeAction({{"type", "setStoryFlag"}, {"flag", "flag_one"}})
    })});
    (void)session.NotifyStartOfDay();

    const auto saveData = session.ToSaveData();
    REQUIRE(std::ranges::any_of(saveData.storyFlags,
        [](const auto& f) { return f == "flag_one"; }));

    gameplay::GameSession restored;
    restored.ApplySaveData(saveData);
    restored.InitializeEventDefinitions({MakeStartOfDayEvent("evt_flag", {
        MakeAction({{"type", "setStoryFlag"}, {"flag", "flag_one"}})
    })});

    // once-mode event already fired — should not fire again (firedEventIds survived)
    const auto results = restored.NotifyStartOfDay();
    REQUIRE(results.empty());

    // storyFlags also survived into the restored session
    const auto restoredSave = restored.ToSaveData();
    REQUIRE(std::ranges::any_of(restoredSave.storyFlags,
        [](const auto& f) { return f == "flag_one"; }));
}

TEST_CASE("GameSession - NotifyStartOfDay does not fire non-startOfDay events") {
    gameplay::GameSession session;

    EventDefinition regionDef = MakeStartOfDayEvent("evt_region", {
        MakeAction({{"type", "showMessage"}, {"text", {{"en", "Wrong trigger."}}}})
    });
    regionDef.trigger.type = EventTriggerType::RegionNodeEntry;

    session.InitializeEventDefinitions({regionDef, MakeStartOfDayEvent("evt_sod", {
        MakeAction({{"type", "showMessage"}, {"text", {{"en", "Only this fires."}}}})
    })});

    const auto results = session.NotifyStartOfDay();
    REQUIRE(results.size() == 1);
    REQUIRE(results[0].message == "Only this fires.");
}

TEST_CASE("GameSession - NotifyStartOfDay fires events in priority order (lower first)") {
    gameplay::GameSession session;

    // evtPrio2: sets "second_fired", priority 2, always repeating
    EventDefinition evtPrio2 = MakeStartOfDayEvent("evt_prio2", {
        MakeAction({{"type", "setStoryFlag"}, {"flag", "second_fired"}})
    }, "always");
    evtPrio2.priority = 2;

    // evtPrio1: condition = NOT storyFlagSet("second_fired"), sets "first_fired", priority 1
    // If prio1 fires before prio2, "second_fired" is not yet set → condition true → fires
    // If prio2 fires before prio1, "second_fired" is set → condition false → does not fire
    EventCondition notSecond;
    notSecond.kind = EventConditionKind::Not;
    EventCondition leafSecond;
    leafSecond.kind = EventConditionKind::Leaf;
    leafSecond.leafType = "storyFlagSet";
    leafSecond.leafArgs = {{"type", "storyFlagSet"}, {"flag", "second_fired"}};
    notSecond.operands = {leafSecond};

    EventDefinition evtPrio1 = MakeStartOfDayEvent("evt_prio1", {
        MakeAction({{"type", "setStoryFlag"}, {"flag", "first_fired"}})
    }, "always");
    evtPrio1.priority = 1;
    evtPrio1.condition = notSecond;

    // Pass prio2 first in the list to ensure sorting is what determines order
    session.InitializeEventDefinitions({evtPrio2, evtPrio1});

    const auto results = session.NotifyStartOfDay();
    REQUIRE(results.size() == 2);

    const auto saveData = session.ToSaveData();
    REQUIRE(std::ranges::any_of(saveData.storyFlags,
        [](const auto& f) { return f == "first_fired"; }));
    REQUIRE(std::ranges::any_of(saveData.storyFlags,
        [](const auto& f) { return f == "second_fired"; }));
}

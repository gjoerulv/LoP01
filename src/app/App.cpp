#include "app/App.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cctype>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
#include <raylib.h>

#include "gameplay/battle/BattleFactory.h"

namespace app {

namespace {
using ashvale::rendering::DebugLine;
using ashvale::rendering::DebugOverlayModel;
using ashvale::rendering::TitleScreenModel;
} // namespace

App::App() {
    contentLoaded_ = content_.LoadFromDirectory("content");
    statusMessage_ = contentLoaded_ ? "Content loaded" : "Content could not be loaded";
}

void App::Run() {
    InitWindow(1280, 720, "Project Ashvale - Visual Pass");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        Update();

        BeginDrawing();
        Draw();
        EndDrawing();
    }

    CloseWindow();
}

void App::InitializeModeStateIfNeeded(const gameplay::SessionSnapshot& snapshot) {
    if (snapshot.mode == gameplay::GameMode::BattleMode) {
        InitializeBattleIfNeeded(snapshot);
    }

    if (snapshot.mode == gameplay::GameMode::LocationMode) {
        InitializeLocationIfNeeded(snapshot);
    }
}

void App::InitializeBattleIfNeeded(const gameplay::SessionSnapshot& snapshot) {
    if (battleInitialized_) {
        return;
    }

    const std::string scenarioId = ResolveBattleScenarioId(snapshot);
    const auto battle = gameplay::battle::BattleFactory::CreateFromScenario(
        content_,
        scenarioId,
        7);

    if (battle.has_value()) {
        debugBattle_ = std::move(*battle);
        battleControllerState_ = {};
        battleControllerState_.selectedTargetIndex = debugBattle_.FindFirstTargetForActive();
        battleInitialized_ = true;
        statusMessage_ = "Battle started: " + scenarioId;
    }
    else {
        debugBattle_ = gameplay::battle::BattleState{};
        battleControllerState_ = {};
        battleInitialized_ = false;
        statusMessage_ = "Failed to initialize battle scenario: " + scenarioId;
    }

    pendingBattleScenarioId_.clear();
}

void App::InitializeLocationIfNeeded(const gameplay::SessionSnapshot& snapshot) {
    if (locationInitialized_) {
        return;
    }

    if (const auto* location = content_.FindLocationById(snapshot.destinationId)) {
        if (!location->sceneId.empty()) {
            if (const auto* scene = content_.FindLocationSceneById(location->sceneId)) {
                locationScene_.Reset(*scene);
                locationInitialized_ = true;
                statusMessage_ = "Entered location: " + location->name;
                return;
            }

            locationScene_ = gameplay::location::LocationScene{};
            statusMessage_ = "Missing scene definition: " + location->sceneId;
            return;
        }

        locationScene_ = gameplay::location::LocationScene{};
        statusMessage_ = "Location has no scene: " + location->name;
        return;
    }

    locationScene_ = gameplay::location::LocationScene{};
    statusMessage_ = "Unknown location scene target: " + snapshot.destinationId;
}

void App::AdvanceFrontEndModesIfRequested(
    const gameplay::SessionSnapshot& snapshot,
    const input::InputState& input) {
    if (!input.confirm) {
        return;
    }

    if (snapshot.mode == gameplay::GameMode::Title ||
        snapshot.mode == gameplay::GameMode::OpeningSequence ||
        snapshot.mode == gameplay::GameMode::OverworldSelection) {
        session_.AdvanceMode();
    }
}

void App::StartBattleScenario(const std::string& scenarioId, const std::string& statusMessage) {
    pendingBattleScenarioId_ = scenarioId;
    session_.EnterBattleMode();
    battleInitialized_ = false;
    battleControllerState_ = {};
    statusMessage_ = statusMessage;
}

void App::StartLocationMode(const std::string& locationId, const std::string& statusMessage) {
    session_.EnterLocationMode(locationId);
    locationInitialized_ = false;
    statusMessage_ = statusMessage;
}

void App::ResetTransientModeState() {
    pendingBattleScenarioId_.clear();
    battleInitialized_ = false;
    locationInitialized_ = false;
    battleControllerState_ = {};
}

std::string App::ResolveBattleScenarioId(const gameplay::SessionSnapshot& snapshot) const {
    if (!pendingBattleScenarioId_.empty()) {
        return pendingBattleScenarioId_;
    }

    if (const auto* location = content_.FindLocationById(snapshot.destinationId)) {
        if (!location->battleScenarioId.empty()) {
            return location->battleScenarioId;
        }
    }

    return "debug_intro_battle";
}

void App::Update() {
    const gameplay::SessionSnapshot snapshot = session_.Snapshot();
    const input::InputState input = inputTranslator_.Poll();

    if (IsKeyPressed(KEY_F1)) {
        debugOverlayVisible_ = !debugOverlayVisible_;
    }

    InitializeModeStateIfNeeded(snapshot);
    AdvanceFrontEndModesIfRequested(snapshot, input);

    if (snapshot.mode == gameplay::GameMode::OverworldMode) {
        UpdateOverworldMode(input);
    }

    if (snapshot.mode == gameplay::GameMode::LocationMode && locationInitialized_) {
        UpdateLocationScene(input, GetFrameTime());
    }

    if (snapshot.mode == gameplay::GameMode::BattleMode && battleInitialized_) {
        UpdateBattleMode(input);
    }

    if (input.save) {
        const bool saved = saveRepository_.SaveToFile(session_.ToSaveData(), "saves/slot_1.json");
        statusMessage_ = saved ? "Saved to saves/slot_1.json" : "Save failed";
    }

    if (input.load) {
        const auto loaded = saveRepository_.LoadFromFile("saves/slot_1.json");
        if (loaded.has_value()) {
            session_.ApplySaveData(*loaded);
            ResetTransientModeState();
            statusMessage_ = "Loaded saves/slot_1.json";
        } else {
            statusMessage_ = "Load failed";
        }
    }
}

void App::UpdateOverworldMode(const input::InputState& input) {
    const auto snapshot = session_.Snapshot();
    const auto nodes = overworldModelMapper_.BuildNodes(content_, snapshot.regionId);
    if (nodes.empty()) {
        return;
    }

    const int currentIndex =
        overworldModelMapper_.FindNodeIndexById(nodes, snapshot.destinationId);

    const OverworldUpdateResult result =
        overworldController_.Update(
            input,
            static_cast<int>(nodes.size()),
            currentIndex,
            overworldSelectedNodeIndex_);

    overworldSelectedNodeIndex_ = result.selectedNodeIndex;

    if (result.travelCancelled) {
        statusMessage_ = "Travel selection cancelled";
    }

    if (result.travelConfirmed) {
        const auto& destination = nodes[overworldSelectedNodeIndex_];
        if (!destination.travelAvailable) {
            statusMessage_ = destination.label + " is not reachable yet";
            return;
        }

        const int travelMinutes =
            overworldModelMapper_.ComputeTravelPreviewMinutes(currentIndex, overworldSelectedNodeIndex_);
        session_.AddMinutes(travelMinutes);

        session_.SetDestination(destination.id);
        statusMessage_ =
            "Traveled to " + destination.label + " (" + overworldModelMapper_.FormatTravelTime(travelMinutes) + ")";

        if (destination.supportsBattle && !destination.battleScenarioId.empty()) {
            StartBattleScenario(destination.battleScenarioId, "Encounter started at " + destination.label);
            return;
        }

        if (destination.entersLocationMode) {
            StartLocationMode(destination.id, "Entering location: " + destination.label);
        }
    }

    if (result.requestDebugBattle) {
        const auto& destination = nodes[overworldSelectedNodeIndex_];
        StartBattleScenario(
            !destination.battleScenarioId.empty() ? destination.battleScenarioId : "debug_intro_battle",
            "Debug battle requested");
    }
}

void App::UpdateLocationScene(const input::InputState& input, const float deltaTime) {
    const LocationUpdateResult result =
        locationController_.Update(input, deltaTime, locationScene_.HasActiveDialogue());

    if (locationScene_.HasActiveDialogue()) {
        if (result.chooseOption1) {
            const auto choice = locationScene_.ChooseDialogueOption(0);
            if (choice.has_value()) {
                ApplyLocationOutcome(*choice);
            }
        }

        if (result.chooseOption2) {
            const auto choice = locationScene_.ChooseDialogueOption(1);
            if (choice.has_value()) {
                ApplyLocationOutcome(*choice);
            }
        }

        return;
    }

    if (result.moveDx != 0.0f) {
        locationScene_.TryMovePlayer(result.moveDx, 0.0f);
    }

    if (result.moveDy != 0.0f) {
        locationScene_.TryMovePlayer(0.0f, result.moveDy);
    }

    if (!result.interact) {
        return;
    }

    const auto outcome = locationScene_.Interact();
    if (!outcome.has_value()) {
        statusMessage_ = "Nothing to interact with";
        return;
    }

    ApplyLocationOutcome(*outcome);
}

void App::UpdateBattleMode(const input::InputState& input) {
    if (debugBattle_.IsFinished()) {
        return;
    }

    const BattleUpdateResult result =
        battleController_.Update(input, debugBattle_, battleControllerState_);

    battleControllerState_ = result.state;

    if (!result.executeAction) {
        return;
    }

    const bool executed = debugBattle_.ExecuteAction(result.action, result.targetIndex);
    if (executed) {
        statusMessage_ = battleEventTextFormatter_.FormatSummary(debugBattle_.LastEvents());
        battleControllerState_.selectedTargetIndex = debugBattle_.FindFirstTargetForActive();
    }
}

bool App::ApplyLocationOutcome(const gameplay::location::InteractionOutcome& outcome) {
    if (outcome.goldCost > 0 && !session_.TrySpendGold(outcome.goldCost)) {
        statusMessage_ = !outcome.failureText.empty()
            ? outcome.failureText
            : "Interaction failed";
        return false;
    }

    if (outcome.timeCostMinutes > 0) {
        session_.AddMinutes(outcome.timeCostMinutes);
    }

    if (outcome.recruitCount > 0) {
        recruitedUnits_ += outcome.recruitCount;
    }

    if (outcome.exitsLocation) {
        session_.ExitLocationMode();
        locationInitialized_ = false;
    }

    statusMessage_ = outcome.message;
    return true;
}

void App::Draw() const {
    ashvale::rendering::RenderContext context = renderContext_;
    context.screenWidth = GetScreenWidth();
    context.screenHeight = GetScreenHeight();
    context.debugEnabled = debugOverlayVisible_;

    const gameplay::SessionSnapshot snapshot = session_.Snapshot();

    const auto hudModel = hudModelMapper_.Map(snapshot, statusMessage_);

    switch (snapshot.mode) {
    case gameplay::GameMode::Title: {
        TitleScreenModel model;
        model.title = "Project Ashvale";
        model.subtitle = "First Visual Gameplay Pass";
        model.menuItems = {"Start"};
        model.selectedIndex = 0;
        model.footerHint = "Press Enter to continue";
        titleRenderer_.Draw(context, model);
        break;
    }
    case gameplay::GameMode::OpeningSequence: {
        ClearBackground(context.theme.clearColor);
        const Font font = ashvale::rendering::ResolveUiFont(context);
        DrawTextEx(font, "Opening Sequence", {80.0f, 120.0f}, context.titleFontSize, 1.0f, context.theme.highlightTextColor);
        DrawTextEx(font, "You wake in an abandoned town with no memory.", {80.0f, 190.0f}, context.normalFontSize, 1.0f, context.theme.textColor);
        DrawTextEx(font, "Press Enter to proceed to the overworld.", {80.0f, 230.0f}, context.normalFontSize, 1.0f, context.theme.mutedTextColor);
        break;
    }
    case gameplay::GameMode::OverworldSelection:
    case gameplay::GameMode::OverworldMode: {
        const auto model = overworldModelMapper_.Map(
            content_,
            snapshot,
            overworldSelectedNodeIndex_);

        overworldRenderer_.Draw(context, model);
        break;
    }
    case gameplay::GameMode::LocationMode: {
        const auto model = locationModelMapper_.Map(
            snapshot,
            locationScene_,
            statusMessage_);

        locationRenderer_.Draw(context, model);
        break;
    }
    case gameplay::GameMode::BattleMode: {
        const auto model = battleModelMapper_.Map(
            debugBattle_,
            battleControllerState_.selectedActionIndex,
            battleControllerState_.selectedTargetIndex,
            battleEventTextFormatter_.FormatSummary(debugBattle_.LastEvents()));

        battleRenderer_.Draw(context, model);
        break;
    }
    }

    if (snapshot.mode != gameplay::GameMode::Title) {
        hudRenderer_.Draw(context, hudModel);
    }

    DebugOverlayModel debugModel;
    debugModel.visible = debugOverlayVisible_;
    debugModel.lines = {
        DebugLine{"Content loaded", contentLoaded_ ? "true" : "false"},
        DebugLine{"Mode", gameplay::GameSession::ToString(snapshot.mode)},
        DebugLine{"Day", std::to_string(snapshot.day)},
        DebugLine{"Time", snapshot.time},
        DebugLine{"Gold", std::to_string(snapshot.gold)},
        DebugLine{"Region", snapshot.regionId},
        DebugLine{"Destination", snapshot.destinationId},
        DebugLine{"Recruits", std::to_string(recruitedUnits_)},
        DebugLine{"Status", statusMessage_}
    };
    debugOverlayRenderer_.Draw(context, debugModel);
}

} // namespace app

#include "app/App.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cctype>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
#include <raylib.h>

#include "core/GameClock.h"
#include "gameplay/battle/BattleFactory.h"
#include "gameplay/location/LocationServiceRules.h"
#include "gameplay/overworld/OverworldDeadEndRules.h"
#include "gameplay/overworld/OverworldTravelRules.h"

namespace app {

namespace {
using ashvale::rendering::DebugLine;
using ashvale::rendering::DebugOverlayModel;
using ashvale::rendering::TitleScreenModel;

std::vector<std::string> BuildBlockedTransitNodeIds(const std::vector<app::mappers::OverworldNodeMeta>& nodes) {
    std::vector<std::string> blockedNodeIds;
    for (const auto& node : nodes) {
        if (node.blocksTransitUntilCleared && !node.combatNodeCleared) {
            blockedNodeIds.push_back(node.id);
        }
    }

    return blockedNodeIds;
}
} // namespace

App::App() {
    contentLoaded_ = content_.LoadFromDirectory("content");
    if (contentLoaded_) {
        session_.InitializeQuestState(content_.QuestDefinitions());
    }
    statusMessage_ = contentLoaded_ ? "Content loaded" : "Content could not be loaded";
    observedDay_ = session_.Snapshot().day;
}

void App::Run() {
    InitWindow(1280, 720, "Project Ashvale");
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
        session_.ActivePartyUnitIds(),
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
    const gameplay::SessionSnapshot snapshot = session_.Snapshot();
    battleReturnMode_ = snapshot.mode == gameplay::GameMode::LocationMode
        ? gameplay::GameMode::LocationMode
        : gameplay::GameMode::OverworldMode;

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
    musteringInteraction_.Close();

    const gameplay::SessionSnapshot snapshot = session_.Snapshot();
    observedDay_ = snapshot.day;
    restedThisDay_ = false;
}

void App::ApplyMissedSleepPenaltyIfNeeded() {
    const gameplay::SessionSnapshot snapshot = session_.Snapshot();

    if (snapshot.day == observedDay_) {
        return;
    }

    if (!restedThisDay_) {
        ApplyWakePenaltyAndRecover("Missed sleep before day end");
    }

    restedThisDay_ = false;
    observedDay_ = session_.Snapshot().day;
}

std::string App::ResolveSafeFallbackLocationId() const {
    if (content_.FindLocationById("home_base") != nullptr) {
        return "home_base";
    }

    if (content_.FindLocationById("old_inn") != nullptr) {
        return "old_inn";
    }

    const auto& locations = content_.Locations();
    if (!locations.empty()) {
        return locations.front().id;
    }

    return session_.Snapshot().destinationId;
}

void App::ApplyWakePenaltyAndRecover(const std::string& reason) {
    session_.ApplyWakePenalty();

    const std::string fallbackLocationId = ResolveSafeFallbackLocationId();
    session_.SetDestination(fallbackLocationId);
    session_.EnterOverworldMode();
    ResetTransientModeState();

    const gameplay::SessionSnapshot penalized = session_.Snapshot();
    statusMessage_ = reason + ". Wake-up penalty applied (-1000 gold, wake at " + penalized.time + ") and moved to " + fallbackLocationId;
}

void App::ResolveBattleOutcomeIfNeeded() {
    if (!battleInitialized_ || !debugBattle_.IsFinished()) {
        return;
    }

    const gameplay::battle::BattleSummary summary = debugBattle_.Summary();
    battleInitialized_ = false;
    battleControllerState_ = {};

    if (summary.enemiesWon) {
        ApplyWakePenaltyAndRecover("Party defeated in battle");
        return;
    }

    if (summary.alliesWon) {
        const gameplay::SessionSnapshot snapshot = session_.Snapshot();
        const auto* location = content_.FindLocationById(snapshot.destinationId);
        const bool nodeIsCombatType = location != nullptr && location->type == data::LocationType::Combat;
        const bool wasCleared = session_.IsCombatNodeCleared(snapshot.destinationId);
        session_.ApplyOverworldCombatVictoryNodeClear(
            summary.alliesWon,
            summary.enemiesWon,
            battleReturnMode_,
            snapshot.destinationId,
            nodeIsCombatType);
        const bool newlyCleared = !wasCleared && session_.IsCombatNodeCleared(snapshot.destinationId);

        if (newlyCleared) {
            const auto questUpdates = session_.NotifyCombatNodeCleared(snapshot.destinationId);
            if (!questUpdates.empty()) {
                statusMessage_ = summary.playerSetToOneHp
                    ? "Battle won. Player recovered to 1 HP. | " + questUpdates.front()
                    : "Battle won. | " + questUpdates.front();
            }
            else {
                statusMessage_ = summary.playerSetToOneHp
                    ? "Battle won. Player recovered to 1 HP."
                    : "Battle won.";
            }
        }
        else {
            statusMessage_ = summary.playerSetToOneHp
                ? "Battle won. Player recovered to 1 HP."
                : "Battle won.";
        }

        if (battleReturnMode_ == gameplay::GameMode::LocationMode) {
            session_.EnterLocationMode(snapshot.destinationId);
        }
        else {
            session_.EnterOverworldMode();
        }

        return;
    }

    session_.EnterOverworldMode();
    statusMessage_ = "Battle ended.";
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

    session_.RefreshDailyServiceUses(content_.LocationServices());
    session_.RefreshWeeklyRecruitStocks(content_.LocationServices());

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

    ApplyMissedSleepPenaltyIfNeeded();
}

void App::UpdateOverworldMode(const input::InputState& input) {
    const auto snapshot = session_.Snapshot();
    const auto nodes = overworldModelMapper_.BuildNodes(content_, snapshot.regionId, session_.ClearedCombatNodeIds());
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

    const auto& currentNode = nodes[currentIndex];
    const bool hasUsableLocalAction =
        currentNode.supportsBattle && !currentNode.combatNodeCleared && !currentNode.battleScenarioId.empty();

    const auto* region = content_.FindRegionById(snapshot.regionId);
    const std::vector<data::RegionLinkDefinition> emptyLinks;
    const auto& links = region != nullptr ? region->links : emptyLinks;
    const auto blockedTransitNodeIds = BuildBlockedTransitNodeIds(nodes);

    bool hasLegalTravelNow = false;
    bool hasReachableTravelIgnoringCutoff = false;
    for (int i = 0; i < static_cast<int>(nodes.size()); ++i) {
        if (i == currentIndex) {
            continue;
        }

        const auto nowTravel = gameplay::overworld::EvaluateTravel(
            snapshot.destinationId,
            nodes[i].id,
            nodes[i].travelAvailable,
            snapshot.minutesIntoSliceDay,
            links,
            blockedTransitNodeIds);
        if (nowTravel.legal) {
            hasLegalTravelNow = true;
        }

        const auto ignoreCutoffTravel = gameplay::overworld::EvaluateTravel(
            snapshot.destinationId,
            nodes[i].id,
            nodes[i].travelAvailable,
            0,
            links,
            blockedTransitNodeIds);
        if (ignoreCutoffTravel.legal) {
            hasReachableTravelIgnoringCutoff = true;
        }

        if (hasLegalTravelNow && hasReachableTravelIgnoringCutoff) {
            break;
        }
    }

    const bool declinedUsableLocalAction = hasUsableLocalAction && result.travelCancelled;
    const auto deadEndDecision = gameplay::overworld::EvaluateDeadEndWakePenalty(
        snapshot.minutesIntoSliceDay,
        hasLegalTravelNow,
        hasReachableTravelIgnoringCutoff,
        hasUsableLocalAction,
        declinedUsableLocalAction,
        core::GameClock::kMinutesPerSliceDay);
    if (deadEndDecision.shouldApplyWakePenalty) {
        ApplyWakePenaltyAndRecover("No legal remaining action before day end");
        return;
    }

    if (result.travelCancelled) {
        statusMessage_ = "Travel selection cancelled";
    }

    if (result.travelConfirmed) {
        const auto& destination = nodes[overworldSelectedNodeIndex_];
        const auto travel = gameplay::overworld::EvaluateTravel(
            snapshot.destinationId,
            destination.id,
            destination.travelAvailable,
            snapshot.minutesIntoSliceDay,
            links,
            blockedTransitNodeIds);

        if (!travel.legal) {
            if (travel.reason == gameplay::overworld::TravelBlockReason::DestinationUnavailable) {
                statusMessage_ = destination.label + " is not reachable yet";
            }
            else if (travel.reason == gameplay::overworld::TravelBlockReason::NoRouteLink) {
                statusMessage_ = "No route link from current node to " + destination.label;
            }
            else if (travel.reason == gameplay::overworld::TravelBlockReason::ArrivalPastDayEnd) {
                statusMessage_ = "Travel would arrive past 02:00";
            }
            else if (travel.reason == gameplay::overworld::TravelBlockReason::BlockedByUnclearedTransitNode) {
                statusMessage_ = "Route is blocked by an uncleared blocker node";
            }
            else {
                statusMessage_ = "Travel unavailable";
            }

            return;
        }

        const bool hadActiveSupplyPrep = session_.HasActiveSameDayTravelPrep();
        const int appliedTravelMinutes = session_.ApplySameDayTravelPrepToTravelMinutes(travel.minutes);
        session_.AddMinutes(appliedTravelMinutes);

        session_.SetDestination(destination.id);
        std::string travelStatus;
        if (hadActiveSupplyPrep && appliedTravelMinutes < travel.minutes) {
            travelStatus =
                "Traveled to " + destination.label + " (" +
                overworldModelMapper_.FormatTravelTime(appliedTravelMinutes) +
                ") | Supply prep used (-" + std::to_string(travel.minutes - appliedTravelMinutes) + "m)";
        }
        else {
            travelStatus =
                "Traveled to " + destination.label + " (" + overworldModelMapper_.FormatTravelTime(appliedTravelMinutes) + ")";
        }
        statusMessage_ = travelStatus;
        OnDestinationArrived(destination.id);

        if (destination.supportsBattle && !destination.combatNodeCleared && !destination.battleScenarioId.empty()) {
            StartBattleScenario(destination.battleScenarioId, travelStatus + " | Encounter started at " + destination.label);
            return;
        }

        if (destination.entersLocationMode) {
            StartLocationMode(destination.id, travelStatus + " | Entering location: " + destination.label);
        }
    }

    if (result.requestDebugBattle) {
        const auto& destination = nodes[overworldSelectedNodeIndex_];
        StartBattleScenario(
            !destination.battleScenarioId.empty() ? destination.battleScenarioId : "debug_intro_battle",
            "Debug battle requested");
    }
}

void App::OnDestinationArrived(const std::string& destinationId) {
    const gameplay::SessionSnapshot snapshot = session_.Snapshot();
    if (snapshot.destinationId != destinationId) {
        return;
    }

    const auto questUpdates = session_.NotifyDestinationReached(destinationId);
    if (!questUpdates.empty()) {
        statusMessage_ += " | " + questUpdates.front();
    }
}

MusteringCommand App::TranslateMusteringCommand(const input::InputState& input) const {
    if (input.interact) {
        return MusteringCommand::Exit;
    }

    if (input.option1) {
        return MusteringCommand::AddSelectedReserveToActive;
    }

    if (input.option2) {
        return MusteringCommand::RemoveSelectedActive;
    }

    if (input.selectPrev) {
        return MusteringCommand::SelectReservePrev;
    }

    if (input.selectNext) {
        return MusteringCommand::SelectReserveNext;
    }

    if (input.targetPrev) {
        return MusteringCommand::SelectActivePrev;
    }

    if (input.targetNext) {
        return MusteringCommand::SelectActiveNext;
    }

    return MusteringCommand::None;
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

    if (musteringInteraction_.IsActive()) {
        const MusteringCommand command = TranslateMusteringCommand(input);
        const MusteringApplyResult musteringResult = musteringInteraction_.ApplyCommand(command, session_);
        if (!musteringResult.statusText.empty()) {
            statusMessage_ = musteringResult.statusText;
        }

        if (musteringResult.shouldExit) {
            musteringInteraction_.Close();
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

    const auto snapshot = session_.Snapshot();
    const data::LocationServiceDefinition* service =
        content_.FindLocationService(snapshot.destinationId, outcome->zoneId);

    if (service != nullptr) {
        ApplyResolvedLocationService(*service);
        return;
    }

    const bool isServiceInteraction =
        outcome->type == gameplay::location::InteractionType::InnDoor ||
        outcome->type == gameplay::location::InteractionType::Shop ||
        outcome->type == gameplay::location::InteractionType::Recruit;

    if (isServiceInteraction) {
        statusMessage_ = "Service unavailable here.";
        return;
    }

    ApplyLocationOutcome(*outcome);
}

void App::UpdateBattleMode(const input::InputState& input) {
    if (debugBattle_.IsFinished()) {
        ResolveBattleOutcomeIfNeeded();
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

        if (debugBattle_.IsFinished()) {
            ResolveBattleOutcomeIfNeeded();
        }
    }
}

bool App::ApplyResolvedLocationService(const data::LocationServiceDefinition& service) {
    if (gameplay::location::IsRestService(&service)) {
        if (service.goldCost > 0 && !session_.TrySpendGold(service.goldCost)) {
            statusMessage_ = !service.failureText.empty()
                ? service.failureText
                : "Not enough gold to rest";
            return false;
        }

        restedThisDay_ = true;
        session_.RestToNextDayStart();

        const auto afterRest = session_.Snapshot();
        statusMessage_ = service.successText + ". Day " +
            std::to_string(afterRest.day) + " " + afterRest.time;
        return true;
    }

    if (gameplay::location::IsShopService(&service)) {
        if (service.travelPrepDiscountMinutes > 0 && service.travelPrepCharges > 0 &&
            session_.HasActiveSameDayTravelPrep()) {
            statusMessage_ = "Travel prep already active for next travel (use it first or wait for day rollover)";
            return false;
        }

        if (service.dailyUseLimit > 0) {
            const int remainingUses = session_.RemainingDailyServiceUses(service.id, service.dailyUseLimit);
            if (remainingUses <= 0) {
                statusMessage_ = !service.failureText.empty()
                    ? service.failureText
                    : "Service already used today";
                statusMessage_ += " (resets next day)";
                return false;
            }
        }

        if (service.goldCost > 0 && !session_.TrySpendGold(service.goldCost)) {
            statusMessage_ = "Not enough gold";
            return false;
        }

        if (service.dailyUseLimit > 0 && !session_.TryConsumeDailyServiceUse(service.id, service.dailyUseLimit)) {
            statusMessage_ = !service.failureText.empty()
                ? service.failureText
                : "Service already used today";
            statusMessage_ += " (resets next day)";
            return false;
        }

        if (service.timeCostMinutes > 0) {
            session_.AddMinutes(service.timeCostMinutes);
        }

        if (service.travelPrepDiscountMinutes > 0 && service.travelPrepCharges > 0) {
            session_.GrantSameDayTravelPrep(service.travelPrepDiscountMinutes, service.travelPrepCharges);
            statusMessage_ = service.successText + " | Next travel today -" +
                std::to_string(service.travelPrepDiscountMinutes) + "m (expires day end)";
            return true;
        }

        statusMessage_ = service.successText;
        return true;
    }

    if (gameplay::location::IsRecruitService(&service)) {
        const auto result = gameplay::location::TryApplyRecruitService(session_, content_, service);
        statusMessage_ = result.statusMessage;
        return result.success;
    }

    if (gameplay::location::IsMusterService(&service)) {
        musteringInteraction_.Open(session_);
        statusMessage_ = "Opened Home Base mustering";
        return true;
    }

    statusMessage_ = "Unknown service";
    return false;
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
    const auto hudModel = hudModelMapper_.Map(content_, session_, snapshot, statusMessage_, session_.QuestProgress());
    std::optional<mappers::InteractPromptOverride> locationPromptOverride;
    if (snapshot.mode == gameplay::GameMode::LocationMode && musteringInteraction_.IsActive()) {
        locationPromptOverride = mappers::InteractPromptOverride{
            musteringInteraction_.BuildPromptText(session_),
            true
        };
    }
    
    switch (snapshot.mode) {
    case gameplay::GameMode::Title: {
        TitleScreenModel model;
        model.title = "Project Ashvale";
        model.subtitle = "Prototype Vertical Slice";
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
            session_,
            snapshot,
            overworldSelectedNodeIndex_,
            session_.ClearedCombatNodeIds());

        overworldRenderer_.Draw(context, model);
        break;
    }
    case gameplay::GameMode::LocationMode: {
        const auto model = locationModelMapper_.Map(
            content_,
            session_,
            snapshot,
            locationScene_,
            statusMessage_,
            locationPromptOverride);

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
    int completedQuests = 0;
    const auto& questProgress = session_.QuestProgress();
    for (const auto& quest : questProgress) {
        if (quest.status == gameplay::quests::QuestStatus::Completed) {
            ++completedQuests;
        }
    }

    debugModel.visible = debugOverlayVisible_;
    debugModel.lines = {
        DebugLine{"Content loaded", contentLoaded_ ? "true" : "false"},
        DebugLine{"Mode", gameplay::GameSession::ToString(snapshot.mode)},
        DebugLine{"Day", std::to_string(snapshot.day)},
        DebugLine{"Time", snapshot.time},
        DebugLine{"Gold", std::to_string(snapshot.gold)},
        DebugLine{"Region", snapshot.regionId},
        DebugLine{"Destination", snapshot.destinationId},
        DebugLine{"Quests", std::to_string(completedQuests) + "/" + std::to_string(questProgress.size()) + " completed"},
        DebugLine{"Status", statusMessage_}
    };
    debugOverlayRenderer_.Draw(context, debugModel);
}

} // namespace app

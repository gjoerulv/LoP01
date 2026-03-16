#include "app/App.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cctype>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
#include <raylib.h>

namespace app {

namespace {

using ashvale::rendering::BattleActionView;
using ashvale::rendering::BattleRenderModel;
using ashvale::rendering::BattleTeam;
using ashvale::rendering::BattleUnitView;
using ashvale::rendering::DebugLine;
using ashvale::rendering::DebugOverlayModel;
using ashvale::rendering::HudModel;
using ashvale::rendering::LocationDialogueView;
using ashvale::rendering::LocationInteractableType;
using ashvale::rendering::LocationNpcView;
using ashvale::rendering::LocationRenderModel;
using ashvale::rendering::LocationZoneView;
using ashvale::rendering::OverworldNodeType;
using ashvale::rendering::OverworldNodeView;
using ashvale::rendering::OverworldRenderModel;
using ashvale::rendering::TitleScreenModel;

struct OverworldNodeMeta {
    std::string id;
    std::string label;
    std::string type;
    bool enterable = true;
};

std::vector<OverworldNodeMeta> BuildOverworldNodes(const data::ContentRepository& content) {
    std::vector<OverworldNodeMeta> nodes;

    const nlohmann::json& locations = content.Locations();
    if (!locations.contains("locations") || !locations["locations"].is_array()) {
        return nodes;
    }

    for (const auto& location : locations["locations"]) {
        nodes.push_back(OverworldNodeMeta{
            location.value("id", "unknown"),
            location.value("name", "Unknown"),
            location.value("type", "unknown"),
            location.value("overworld_destination", true)
        });
    }

    return nodes;
}

std::string HumanizeId(std::string value) {
    bool nextUpper = true;
    for (char& ch : value) {
        if (ch == '_') {
            ch = ' ';
            nextUpper = true;
            continue;
        }

        if (nextUpper) {
            ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
            nextUpper = false;
        }
    }

    return value;
}

std::string BuildLocationPrompt(const gameplay::location::InteractionType type) {
    switch (type) {
    case gameplay::location::InteractionType::InnDoor:
        return "E: Open Inn Door (+1 min)";
    case gameplay::location::InteractionType::Shop:
        return "E: Shop (+5 min)";
    case gameplay::location::InteractionType::Recruit:
        return "E: Recruit (+10 min)";
    case gameplay::location::InteractionType::Npc:
        return "E: Talk";
    case gameplay::location::InteractionType::Exit:
        return "E: Exit To Overworld";
    }

    return "E: Interact";
}

std::string ToDisplayTypeLabel(const std::string& type) {
    if (type == "home") {
        return "Home";
    }
    if (type == "town") {
        return "Town";
    }
    if (type == "dungeon") {
        return "Dungeon";
    }
    if (type == "service") {
        return "Service";
    }
    if (type == "recruit") {
        return "Recruit";
    }
    if (type == "combat") {
        return "Combat";
    }

    return "Unknown";
}

int FindNodeIndexById(const std::vector<OverworldNodeMeta>& nodes, const std::string& id) {
    for (int i = 0; i < static_cast<int>(nodes.size()); ++i) {
        if (nodes[i].id == id) {
            return i;
        }
    }
    return 0;
}

OverworldNodeType ToNodeType(const std::string& type) {
    if (type == "home") {
        return OverworldNodeType::Home;
    }
    if (type == "town") {
        return OverworldNodeType::Town;
    }
    if (type == "dungeon") {
        return OverworldNodeType::Dungeon;
    }
    if (type == "service") {
        return OverworldNodeType::Service;
    }
    if (type == "recruit") {
        return OverworldNodeType::Recruit;
    }
    if (type == "combat") {
        return OverworldNodeType::Combat;
    }

    return OverworldNodeType::Unknown;
}

std::string FormatTravelTime(const int minutes) {
    if (minutes < 60) {
        return std::to_string(minutes) + " min";
    }

    const int hours = minutes / 60;
    const int remainder = minutes % 60;
    if (remainder == 0) {
        return std::to_string(hours) + "h";
    }

    return std::to_string(hours) + "h " + std::to_string(remainder) + "m";
}

int ComputeTravelPreviewMinutes(const int currentIndex, const int selectedIndex) {
    const int distanceSteps = std::abs(selectedIndex - currentIndex);
    return core::GameClock::QuantizeTravelMinutes(15 + distanceSteps * 15);
}

Rectangle ToRectangle(const gameplay::location::RectF& rect) {
    return Rectangle{rect.x, rect.y, rect.width, rect.height};
}

bool Intersects(const Rectangle& a, const Rectangle& b) {
    return a.x < b.x + b.width &&
           a.x + a.width > b.x &&
           a.y < b.y + b.height &&
           a.y + a.height > b.y;
}

int FindNearLocationZoneIndex(const gameplay::location::LocationScene& scene) {
    const auto& player = scene.Player();
    Rectangle probe{player.x - 20.0f, player.y - 20.0f, player.width + 40.0f, player.height + 40.0f};

    const auto& zones = scene.Zones();
    for (int i = 0; i < static_cast<int>(zones.size()); ++i) {
        if (Intersects(probe, ToRectangle(zones[i].area))) {
            return i;
        }
    }

    return -1;
}

LocationInteractableType ToInteractableType(const gameplay::location::InteractionType type) {
    switch (type) {
    case gameplay::location::InteractionType::InnDoor:
        return LocationInteractableType::Inn;
    case gameplay::location::InteractionType::Shop:
        return LocationInteractableType::Shop;
    case gameplay::location::InteractionType::Recruit:
        return LocationInteractableType::Recruit;
    case gameplay::location::InteractionType::Npc:
        return LocationInteractableType::Npc;
    case gameplay::location::InteractionType::Exit:
        return LocationInteractableType::Exit;
    }

    return LocationInteractableType::Unknown;
}

bool IsUnitAlive(const gameplay::battle::BattleUnit& unit) {
    if (unit.category == gameplay::battle::UnitCategory::Generic) {
        return unit.life > 0;
    }

    return unit.hp > 0;
}

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

void App::Update() {
    const gameplay::SessionSnapshot snapshot = session_.Snapshot();

    if (IsKeyPressed(KEY_F1)) {
        debugOverlayVisible_ = !debugOverlayVisible_;
    }

    if (snapshot.mode == gameplay::GameMode::BattleMode && !battleInitialized_) {
        debugBattle_ = gameplay::battle::BattleState::CreateDebugBattle();
        battleInitialized_ = true;
        battleSelectedActionIndex_ = 0;
        battleSelectedTargetIndex_ = debugBattle_.FindFirstTargetForActive();
        statusMessage_ = "Debug battle started";
    }

    if (snapshot.mode == gameplay::GameMode::LocationMode && !locationInitialized_) {
        locationScene_.Reset();
        locationInitialized_ = true;
        statusMessage_ = "Entered town location";
    }

    if (snapshot.mode == gameplay::GameMode::Title && IsKeyPressed(KEY_ENTER)) {
        session_.AdvanceMode();
    }
    if (snapshot.mode == gameplay::GameMode::OpeningSequence && IsKeyPressed(KEY_ENTER)) {
        session_.AdvanceMode();
    }
    if (snapshot.mode == gameplay::GameMode::OverworldSelection && IsKeyPressed(KEY_ENTER)) {
        session_.AdvanceMode();
    }

    if (snapshot.mode == gameplay::GameMode::OverworldMode) {
        UpdateOverworldMode();
    }

    if (snapshot.mode == gameplay::GameMode::LocationMode && locationInitialized_) {
        UpdateLocationScene(GetFrameTime());
    }

    if (snapshot.mode == gameplay::GameMode::BattleMode && battleInitialized_) {
        UpdateBattleMode();
    }

    if (IsKeyPressed(KEY_F5)) {
        const bool saved = saveRepository_.SaveToFile(session_.ToSaveData(), "saves/slot_1.json");
        statusMessage_ = saved ? "Saved to saves/slot_1.json" : "Save failed";
    }

    if (IsKeyPressed(KEY_F9)) {
        const auto loaded = saveRepository_.LoadFromFile("saves/slot_1.json");
        if (loaded.has_value()) {
            session_.ApplySaveData(*loaded);
            statusMessage_ = "Loaded saves/slot_1.json";
        } else {
            statusMessage_ = "Load failed";
        }
    }
}

void App::UpdateOverworldMode() {
    const auto nodes = BuildOverworldNodes(content_);
    if (nodes.empty()) {
        return;
    }

    const int currentIndex = FindNodeIndexById(nodes, session_.Snapshot().destinationId);
    overworldSelectedNodeIndex_ = std::clamp(overworldSelectedNodeIndex_, 0, static_cast<int>(nodes.size()) - 1);

    if (IsKeyPressed(KEY_LEFT)) {
        overworldSelectedNodeIndex_ = (overworldSelectedNodeIndex_ - 1 + static_cast<int>(nodes.size())) % static_cast<int>(nodes.size());
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        overworldSelectedNodeIndex_ = (overworldSelectedNodeIndex_ + 1) % static_cast<int>(nodes.size());
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        overworldSelectedNodeIndex_ = currentIndex;
        statusMessage_ = "Travel selection cancelled";
    }

    if (IsKeyPressed(KEY_ENTER)) {
        const auto& destination = nodes[overworldSelectedNodeIndex_];
        if (!destination.enterable) {
            statusMessage_ = destination.label + " is not enterable yet";
            return;
        }

        const int travelMinutes = ComputeTravelPreviewMinutes(currentIndex, overworldSelectedNodeIndex_);
        session_.AddMinutes(travelMinutes);

        session_.SetDestination(destination.id);
        statusMessage_ = "Traveled to " + destination.label + " (" + FormatTravelTime(travelMinutes) + ")";

        if (destination.type == "town" || destination.type == "home" || destination.type == "inn") {
            session_.EnterLocationMode(destination.id);
            locationScene_.Reset();
            locationInitialized_ = true;
            statusMessage_ = "Entered location: " + destination.label;
        }
    }

    if (IsKeyPressed(KEY_B)) {
        session_.AdvanceMode();
        session_.AdvanceMode();
        battleInitialized_ = false;
    }
}

void App::UpdateLocationScene(const float deltaTime) {
    if (locationScene_.HasActiveDialogue()) {
        if (IsKeyPressed(KEY_ONE)) {
            const auto choice = locationScene_.ChooseDialogueOption(0);
            if (choice.has_value()) {
                session_.ApplyDialogueChoiceCost();
                statusMessage_ = choice->message;
            }
        }
        if (IsKeyPressed(KEY_TWO)) {
            const auto choice = locationScene_.ChooseDialogueOption(1);
            if (choice.has_value()) {
                session_.ApplyDialogueChoiceCost();
                statusMessage_ = choice->message;
            }
        }
        return;
    }

    const float moveSpeed = 180.0f;
    float dx = 0.0f;
    float dy = 0.0f;

    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        dx -= moveSpeed * deltaTime;
    }
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        dx += moveSpeed * deltaTime;
    }
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
        dy -= moveSpeed * deltaTime;
    }
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
        dy += moveSpeed * deltaTime;
    }

    if (dx != 0.0f) {
        locationScene_.TryMovePlayer(dx, 0.0f);
    }
    if (dy != 0.0f) {
        locationScene_.TryMovePlayer(0.0f, dy);
    }

    if (!IsKeyPressed(KEY_E)) {
        return;
    }

    const auto outcome = locationScene_.Interact();
    if (!outcome.has_value()) {
        statusMessage_ = "Nothing to interact with";
        return;
    }

    switch (outcome->type) {
    case gameplay::location::InteractionType::InnDoor:
        session_.ApplyDoorOpenCost();
        statusMessage_ = outcome->message;
        break;
    case gameplay::location::InteractionType::Shop:
        if (session_.ApplyShopCost(50)) {
            statusMessage_ = "Bought supplies for 50 gold (+5 min)";
        } else {
            statusMessage_ = "Not enough gold for shopping";
        }
        break;
    case gameplay::location::InteractionType::Recruit:
        if (session_.ApplyRecruitCost(120)) {
            ++recruitedUnits_;
            statusMessage_ = "Recruit completed for 120 gold (+10 min)";
        } else {
            statusMessage_ = "Not enough gold to recruit";
        }
        break;
    case gameplay::location::InteractionType::Npc:
        statusMessage_ = outcome->message;
        break;
    case gameplay::location::InteractionType::Exit:
        session_.ExitLocationMode();
        locationInitialized_ = false;
        statusMessage_ = outcome->message;
        break;
    }
}

void App::UpdateBattleMode() {
    if (debugBattle_.IsFinished()) {
        return;
    }

    constexpr int kActionCount = 5;

    if (IsKeyPressed(KEY_LEFT)) {
        battleSelectedActionIndex_ = (battleSelectedActionIndex_ - 1 + kActionCount) % kActionCount;
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        battleSelectedActionIndex_ = (battleSelectedActionIndex_ + 1) % kActionCount;
    }

    const int activeIndex = debugBattle_.ActiveUnitIndex();
    const auto& units = debugBattle_.Units();

    std::vector<int> targetableIndices;
    if (activeIndex >= 0 && activeIndex < static_cast<int>(units.size())) {
        const auto actorSide = units[activeIndex].side;
        for (int i = 0; i < static_cast<int>(units.size()); ++i) {
            if (units[i].side != actorSide && IsUnitAlive(units[i])) {
                targetableIndices.push_back(i);
            }
        }
    }

    if (!targetableIndices.empty()) {
        if (battleSelectedTargetIndex_ < 0 || std::find(targetableIndices.begin(), targetableIndices.end(), battleSelectedTargetIndex_) == targetableIndices.end()) {
            battleSelectedTargetIndex_ = targetableIndices.front();
        }

        auto targetIter = std::find(targetableIndices.begin(), targetableIndices.end(), battleSelectedTargetIndex_);
        int targetPos = static_cast<int>(targetIter - targetableIndices.begin());

        if (IsKeyPressed(KEY_UP)) {
            targetPos = (targetPos - 1 + static_cast<int>(targetableIndices.size())) % static_cast<int>(targetableIndices.size());
            battleSelectedTargetIndex_ = targetableIndices[targetPos];
        }

        if (IsKeyPressed(KEY_DOWN)) {
            targetPos = (targetPos + 1) % static_cast<int>(targetableIndices.size());
            battleSelectedTargetIndex_ = targetableIndices[targetPos];
        }
    }

    if (!IsKeyPressed(KEY_ENTER)) {
        return;
    }

    const gameplay::battle::BattleActionType action =
        battleSelectedActionIndex_ == 0 ? gameplay::battle::BattleActionType::Attack :
        battleSelectedActionIndex_ == 1 ? gameplay::battle::BattleActionType::Defend :
        battleSelectedActionIndex_ == 2 ? gameplay::battle::BattleActionType::Wait :
        battleSelectedActionIndex_ == 3 ? gameplay::battle::BattleActionType::Skill1 :
                                          gameplay::battle::BattleActionType::Skill2;

    const bool needsTarget = action == gameplay::battle::BattleActionType::Attack ||
                             action == gameplay::battle::BattleActionType::Skill1 ||
                             action == gameplay::battle::BattleActionType::Skill2;

    const int targetIndex = needsTarget ? battleSelectedTargetIndex_ : -1;
    const bool executed = debugBattle_.ExecuteAction(action, targetIndex);
    if (executed) {
        statusMessage_ = debugBattle_.LastActionText();
        battleSelectedTargetIndex_ = debugBattle_.FindFirstTargetForActive();
    }
}

void App::Draw() const {
    ashvale::rendering::RenderContext context = renderContext_;
    context.screenWidth = GetScreenWidth();
    context.screenHeight = GetScreenHeight();
    context.debugEnabled = debugOverlayVisible_;

    const gameplay::SessionSnapshot snapshot = session_.Snapshot();

    HudModel hudModel;
    hudModel.modeLabel = gameplay::GameSession::ToString(snapshot.mode);
    hudModel.day = snapshot.day;
    hudModel.timeText = snapshot.time;
    hudModel.gold = snapshot.gold;
    hudModel.primaryAreaLabel = "Region:";
    hudModel.primaryAreaValue = snapshot.regionId;
    hudModel.secondaryAreaLabel = "Location:";
    hudModel.secondaryAreaValue = snapshot.destinationId;
    hudModel.statusText = statusMessage_;
    hudModel.showStatus = true;

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
        const auto nodes = BuildOverworldNodes(content_);
        OverworldRenderModel model;
        model.regionName = "Ashvale Heartland";
        model.hintText = "Left/Right: select destination";
        model.controlsText = "Enter: confirm travel | Esc: cancel selection";

        static const std::array<Vector2, 12> positions = {
            Vector2{110.0f, 460.0f}, Vector2{230.0f, 370.0f}, Vector2{360.0f, 430.0f}, Vector2{470.0f, 300.0f},
            Vector2{610.0f, 410.0f}, Vector2{740.0f, 340.0f}, Vector2{190.0f, 230.0f}, Vector2{350.0f, 210.0f},
            Vector2{520.0f, 200.0f}, Vector2{680.0f, 210.0f}, Vector2{800.0f, 470.0f}, Vector2{730.0f, 520.0f}
        };

        const int currentIndex = FindNodeIndexById(nodes, snapshot.destinationId);
        const int selectedIndex = nodes.empty() ? 0 : std::clamp(overworldSelectedNodeIndex_, 0, static_cast<int>(nodes.size()) - 1);

        for (int i = 0; i < static_cast<int>(nodes.size()); ++i) {
            model.nodes.push_back(OverworldNodeView{
                nodes[i].id,
                nodes[i].label,
                positions[i % static_cast<int>(positions.size())],
                ToNodeType(nodes[i].type),
                true,
                nodes[i].enterable,
                i == currentIndex,
                i == selectedIndex
            });

            if (i > 0) {
                model.links.push_back({i - 1, i});
            }
        }

        if (!nodes.empty()) {
            model.currentNodeLabel = nodes[currentIndex].label;
            model.selectedNodeLabel = nodes[selectedIndex].label;
            model.selectedNodeType = "Type: " + ToDisplayTypeLabel(nodes[selectedIndex].type);
            model.selectedNodeEnterable = std::string("Enterable: ") + (nodes[selectedIndex].enterable ? "Yes" : "No");
            model.travelTimeText = FormatTravelTime(ComputeTravelPreviewMinutes(currentIndex, selectedIndex));
        }

        overworldRenderer_.Draw(context, model);
        break;
    }
    case gameplay::GameMode::LocationMode: {
        LocationRenderModel model;
        model.locationName = "Town: " + HumanizeId(snapshot.destinationId);

        const auto& player = locationScene_.Player();
        model.playerBounds = Rectangle{player.x, player.y, player.width, player.height};

        const auto& blocks = locationScene_.BlockingRects();
        for (int i = 0; i < static_cast<int>(blocks.size()); ++i) {
            const Rectangle rect{blocks[i].x, blocks[i].y, blocks[i].width, blocks[i].height};
            if (i < 4) {
                model.walls.push_back(rect);
            } else {
                model.buildingBlocks.push_back(rect);
            }
        }

        const int nearZone = FindNearLocationZoneIndex(locationScene_);
        const auto& zones = locationScene_.Zones();
        for (int i = 0; i < static_cast<int>(zones.size()); ++i) {
            LocationZoneView zoneView;
            zoneView.bounds = ToRectangle(zones[i].area);
            zoneView.label = HumanizeId(zones[i].id);
            zoneView.type = ToInteractableType(zones[i].type);
            zoneView.highlighted = i == nearZone;
            model.zones.push_back(zoneView);

            if (zones[i].type == gameplay::location::InteractionType::Npc) {
                model.npcs.push_back(LocationNpcView{ToRectangle(zones[i].area), HumanizeId(zones[i].id), i == nearZone});
            }
        }

        model.showInteractPrompt = nearZone >= 0;
        if (locationScene_.HasActiveDialogue()) {
            model.interactPrompt = "Choose dialogue option (1/2)";
        } else if (nearZone >= 0) {
            model.interactPrompt = BuildLocationPrompt(zones[nearZone].type);
        } else {
            model.interactPrompt = "Move near a marker and press E";
        }

        model.dialogue = LocationDialogueView{};
        model.dialogue.visible = locationScene_.HasActiveDialogue();
        model.dialogue.speaker = "Resident";
        model.dialogue.text = statusMessage_;
        model.dialogue.options = locationScene_.ActiveDialogueChoices();

        locationRenderer_.Draw(context, model);
        break;
    }
    case gameplay::GameMode::BattleMode: {
        BattleRenderModel model;
        model.battleTitle = "CTB Battle";
        model.statusText = debugBattle_.LastActionText();

        const auto& units = debugBattle_.Units();
        const int activeIndex = debugBattle_.ActiveUnitIndex();

        int allyColumn = 0;
        int enemyColumn = 0;

        for (int i = 0; i < static_cast<int>(units.size()); ++i) {
            const auto& unit = units[i];
            const bool ally = unit.side == gameplay::battle::TeamSide::Allies;

            const float x = ally
                ? 44.0f + static_cast<float>(allyColumn % 3) * 196.0f
                : 672.0f + static_cast<float>(enemyColumn % 3) * 196.0f;
            const float y = 150.0f + static_cast<float>((ally ? allyColumn : enemyColumn) / 3) * 164.0f;

            if (ally) {
                ++allyColumn;
            } else {
                ++enemyColumn;
            }

            model.units.push_back(BattleUnitView{
                unit.name,
                ally ? BattleTeam::Allies : BattleTeam::Enemies,
                Rectangle{x, y, 184.0f, 146.0f},
                unit.hp,
                unit.stats.maxHp,
                unit.mp,
                unit.stats.maxMp,
                unit.life,
                i == activeIndex,
                IsUnitAlive(unit),
                i == battleSelectedTargetIndex_,
                unit.ko
            });
        }

        const std::vector<int> turnOrder = debugBattle_.UpcomingTurnOrder(6);
        for (const int index : turnOrder) {
            if (index >= 0 && index < static_cast<int>(units.size())) {
                model.turnOrder.push_back(units[index].name);
            }
        }

        static const std::array<std::string, 5> actionLabels = {"Attack", "Defend", "Wait", "Skill 1", "Skill 2"};
        for (int i = 0; i < static_cast<int>(actionLabels.size()); ++i) {
            model.actions.push_back(BattleActionView{actionLabels[i], true, i == battleSelectedActionIndex_});
        }

        if (battleSelectedTargetIndex_ >= 0 && battleSelectedTargetIndex_ < static_cast<int>(units.size())) {
            model.targetHint = "Target: " + units[battleSelectedTargetIndex_].name + "  (Up/Down to switch, Enter confirm)";
        } else {
            model.targetHint = "Choose action with Left/Right, confirm with Enter";
        }

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

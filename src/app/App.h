#pragma once

#include <string>
#include <vector>

#include "core/SaveGame.h"
#include "data/ContentRepository.h"
#include "gameplay/battle/Battle.h"
#include "gameplay/GameSession.h"
#include "gameplay/location/LocationScene.h"
#include "rendering/BattleRenderer.h"
#include "rendering/DebugOverlay.h"
#include "rendering/HudRenderer.h"
#include "rendering/LocationRenderer.h"
#include "rendering/OverworldRenderer.h"
#include "rendering/RenderContext.h"
#include "rendering/TitleRenderer.h"

namespace app {

class App {
public:
    App();

    void Run();

private:
    void Update();
    void Draw() const;

    void UpdateOverworldMode();
    void UpdateLocationScene(float deltaTime);
    void UpdateBattleMode();

    gameplay::GameSession session_;
    data::ContentRepository content_;
    core::SaveGameRepository saveRepository_;

    gameplay::battle::BattleState debugBattle_;
    gameplay::location::LocationScene locationScene_;

    ashvale::rendering::RenderContext renderContext_;
    ashvale::rendering::HudRenderer hudRenderer_;
    ashvale::rendering::DebugOverlay debugOverlayRenderer_;
    ashvale::rendering::TitleRenderer titleRenderer_;
    ashvale::rendering::OverworldRenderer overworldRenderer_;
    ashvale::rendering::LocationRenderer locationRenderer_;
    ashvale::rendering::BattleRenderer battleRenderer_;

    bool battleInitialized_ = false;
    bool locationInitialized_ = false;
    bool contentLoaded_ = false;
    bool debugOverlayVisible_ = false;

    int recruitedUnits_ = 0;
    int overworldSelectedNodeIndex_ = 0;
    int battleSelectedActionIndex_ = 0;
    int battleSelectedTargetIndex_ = -1;

    std::string statusMessage_;
};

} // namespace app

#pragma once

#include <string>
#include <vector>

#include "app/OverworldController.h"
#include "app/LocationController.h"
#include "app/BattleController.h"
#include "app/input/InputTranslator.h"
#include "app/mappers/OverworldModelMapper.h"
#include "app/mappers/LocationModelMapper.h"
#include "app/mappers/HudModelMapper.h"
#include "app/mappers/BattleModelMapper.h"
#include "app/BattleEventTextFormatter.h"
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

        void UpdateOverworldMode(const input::InputState& input);
        void UpdateLocationScene(const input::InputState& input, float deltaTime);
        void UpdateBattleMode(const input::InputState& input);
        bool ApplyLocationOutcome(const gameplay::location::InteractionOutcome& outcome);

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

        input::InputTranslator inputTranslator_;
        BattleController battleController_;
        BattleControllerState battleControllerState_;
        LocationController locationController_;
        OverworldController overworldController_;

        mappers::HudModelMapper hudModelMapper_;
        mappers::BattleModelMapper battleModelMapper_;
        mappers::LocationModelMapper locationModelMapper_;
        mappers::OverworldModelMapper overworldModelMapper_;

        BattleEventTextFormatter battleEventTextFormatter_;        

        bool battleInitialized_ = false;
        bool locationInitialized_ = false;
        bool contentLoaded_ = false;
        bool debugOverlayVisible_ = false;

        int recruitedUnits_ = 0;
        int overworldSelectedNodeIndex_ = 0;

        std::string statusMessage_;
        std::string pendingBattleScenarioId_;
    };

} // namespace app
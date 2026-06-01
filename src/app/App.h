#pragma once

#include <string>
#include <vector>

#include "app/RegionController.h"
#include "app/WorldMapController.h"
#include "app/LocationController.h"
#include "app/BattleController.h"
#include "app/MusteringInteraction.h"
#include "app/input/InputTranslator.h"
#include "app/mappers/RegionModelMapper.h"
#include "app/mappers/WorldMapModelMapper.h"
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
#include "rendering/RegionRenderer.h"
#include "rendering/WorldMapRenderer.h"
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

        void InitializeModeStateIfNeeded(const gameplay::SessionSnapshot& snapshot);
        void InitializeBattleIfNeeded(const gameplay::SessionSnapshot& snapshot);
        void InitializeLocationIfNeeded(const gameplay::SessionSnapshot& snapshot);
        void AdvanceFrontEndModesIfRequested(const gameplay::SessionSnapshot& snapshot, const input::InputState& input);
        void StartBattleScenario(const std::string& scenarioId, const std::string& statusMessage);
        void StartLocationMode(const std::string& locationId, const std::string& statusMessage);
        void ResetTransientModeState();
        void ApplyMissedSleepPenaltyIfNeeded();
        // Call after any intentional day-advance that should not be treated as a
        // missed-sleep (e.g. World Map travel). Sets observedDay_ to the current
        // day so ApplyMissedSleepPenaltyIfNeeded() is a no-op for this transition.
        // Deliberately does NOT set restedThisDay_ = true.
        void MarkCurrentDayObservedAfterIntentionalTimeAdvance();
        void ResolveBattleOutcomeIfNeeded();
        void ApplyWakePenaltyAndRecover(const std::string& reason);
        // Returns a safe recovery node within the current Region (not globally):
        // 1. the current Region's arrivalNodeId (preferred)
        // 2. the current destination if it belongs to the current Region
        // 3. the first node listed for the current Region
        // 4. the current destination unchanged (defensive last resort)
        // Never returns a node that belongs to a different Region.
        [[nodiscard]] std::string ResolveSafeFallbackRegionNodeId() const;
        [[nodiscard]] std::string ResolveBattleScenarioId(const gameplay::SessionSnapshot& snapshot) const;

        void UpdateRegionMode(const input::InputState& input);
        void UpdateWorldMapMode(const input::InputState& input);
        void UpdateLocationScene(const input::InputState& input, float deltaTime);
        void UpdateBattleMode(const input::InputState& input);
        // Appends "Victory!" / "Defeat." plus the latched reason to statusMessage_.
        // No-op if the session is not in a latched outcome state.
        void AppendScenarioEndedStatusIfLatched();
        [[nodiscard]] MusteringCommand TranslateMusteringCommand(const input::InputState& input) const;
        void OnDestinationArrived(const std::string& destinationId);
        bool ApplyLocationOutcome(const gameplay::location::InteractionOutcome& outcome);
        bool ApplyResolvedLocationService(const data::LocationServiceDefinition& service);

        gameplay::GameSession session_;
        data::ContentRepository content_;
        core::SaveGameRepository saveRepository_;

        gameplay::battle::BattleState debugBattle_;
        gameplay::location::LocationScene locationScene_;

        ashvale::rendering::RenderContext renderContext_;
        ashvale::rendering::HudRenderer hudRenderer_;
        ashvale::rendering::DebugOverlay debugOverlayRenderer_;
        ashvale::rendering::TitleRenderer titleRenderer_;
        ashvale::rendering::RegionRenderer regionRenderer_;
        ashvale::rendering::WorldMapRenderer worldMapRenderer_;
        ashvale::rendering::LocationRenderer locationRenderer_;
        ashvale::rendering::BattleRenderer battleRenderer_;

        input::InputTranslator inputTranslator_;
        BattleController battleController_;
        BattleControllerState battleControllerState_;
        LocationController locationController_;
        RegionController regionController_;
        WorldMapController worldMapController_;
        MusteringInteraction musteringInteraction_;

        mappers::HudModelMapper hudModelMapper_;
        mappers::BattleModelMapper battleModelMapper_;
        mappers::LocationModelMapper locationModelMapper_;
        mappers::RegionModelMapper regionModelMapper_;
        mappers::WorldMapModelMapper worldMapModelMapper_;

        BattleEventTextFormatter battleEventTextFormatter_;        

        bool battleInitialized_ = false;
        bool locationInitialized_ = false;
        bool contentLoaded_ = false;
        bool debugOverlayVisible_ = false;

        int regionSelectedNodeIndex_ = 0;
        int worldMapSelectedIndex_ = 0;
        int observedDay_ = -1;
        bool restedThisDay_ = false;
        gameplay::GameMode battleReturnMode_ = gameplay::GameMode::RegionMode;
        std::vector<std::string> battleStartStackIds_;

        std::string statusMessage_;
        std::string pendingBattleScenarioId_;
        std::string pendingHostileContactNodeId_;
        std::string pendingHostileContactTeamColor_;
    };

} // namespace app
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
#include "gameplay/region/RegionDeadEndRules.h"
#include "gameplay/region/RegionTravelRules.h"

namespace app {

namespace {
using ashvale::rendering::DebugLine;
using ashvale::rendering::DebugOverlayModel;
using ashvale::rendering::TitleScreenModel;

std::vector<std::string> BuildBlockedTransitNodeIds(const std::vector<app::mappers::RegionNodeMeta>& nodes) {
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
        std::vector<std::string> leaderCapableUnitIds;
        leaderCapableUnitIds.reserve(content_.Units().size());
        std::string playerCharacterLeaderCapableUnitId;
        for (const auto& unit : content_.Units()) {
            if (unit.category == data::UnitDefinitionCategory::Leader || unit.category == data::UnitDefinitionCategory::Hero) {
                leaderCapableUnitIds.push_back(unit.id);
                if (playerCharacterLeaderCapableUnitId.empty() && unit.isPlayerCharacter) {
                    playerCharacterLeaderCapableUnitId = unit.id;
                }
            }
        }
        session_.SetLeaderCapableUnitIds(std::move(leaderCapableUnitIds));

        if (!playerCharacterLeaderCapableUnitId.empty()) {
            bool playerCharacterInActiveParty = false;
            for (const auto& unitId : session_.ActivePartyUnitIds()) {
                if (unitId == playerCharacterLeaderCapableUnitId) {
                    playerCharacterInActiveParty = true;
                    break;
                }
            }

            if (!playerCharacterInActiveParty) {
                if (session_.OwnedUnitCount(playerCharacterLeaderCapableUnitId) <= 0) {
                    static_cast<void>(session_.AddOwnedUnit(playerCharacterLeaderCapableUnitId, 1));
                }
                static_cast<void>(session_.TryAddUnitToActiveParty(playerCharacterLeaderCapableUnitId));
            }
        }

        session_.InitializeQuestState(content_.QuestDefinitions());
        session_.InitializeEventDefinitions(content_.EventDefinitions());
        session_.SetScenarioOutcomeDefinition(content_.ScenarioOutcome());
        // M13-b: push item/artifact catalogs into the session so equip and
        // event-action paths can look up authored metadata.
        session_.SetItemCatalog(content_.Items());
        session_.SetArtifactCatalog(content_.Artifacts());
        // Player color is hardcoded "Green" project-wide (see roadmap debt #6).
        session_.SetPlayerColor("Green");
        // M14-a: feed the unit catalog (for the daily Energy agility term) and
        // seed day-1 Energy now that the catalog and starting party are in place.
        // Subsequent days auto-reset when the clock crosses a day boundary.
        session_.SetUnitCatalog(content_.Units());
        // M17 Phase 3b: feed service definitions so day-boundary mine payout can
        // resolve owned-service ids to authored mine outputs.
        session_.SetLocationServiceCatalog(content_.LocationServices());
        // Feed authored trader curves so player-facing Trading Post transactions
        // resolve ownership-tier rates (omitted curves fall back to defaults).
        session_.SetTraderCurveCatalog(content_.TraderCurves());
        // M30: feed authored enemy groups so enemy teams resolve their
        // deterministic service-attack strength.
        session_.SetEnemyGroupCatalog(content_.EnemyGroups());
        session_.ApplyDailyStartingEnergy();
        // M15-b: feed the World Map (seeds the unlocked-region set) and the
        // region catalog (resolves a destination's arrival node at travel time).
        session_.SetRegionCatalog(content_.Regions());
        session_.SetWorldMap(content_.WorldMap());
        // M16-c: feed the campaign/scenario catalogs (id->index maps) so campaign
        // start/transition can resolve scenarios and rules without content access.
        session_.SetScenarioCatalog(content_.Scenarios());
        session_.SetCampaignCatalog(content_.Campaigns());
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
    const auto activeBattleEntries = session_.BuildActiveBattleStackEntries();

    battleStartStackIds_.clear();
    battleStartStackIds_.reserve(activeBattleEntries.size());
    for (const auto& entry : activeBattleEntries) {
        battleStartStackIds_.push_back(entry.stackId);
    }

    std::vector<gameplay::battle::PlayerBattleEntry> playerEntries;
    playerEntries.reserve(activeBattleEntries.size());
    for (const auto& entry : activeBattleEntries) {
        gameplay::battle::PlayerBattleEntry pe;
        pe.activeSlotIndex = entry.activeSlotIndex;
        pe.stackId         = entry.stackId;
        pe.unitId          = entry.unitId;
        pe.quantity        = entry.quantity;
        // M13-b: forward pre-summed equipped-artifact stat bonuses so they
        // flow into per-battle hero stats via BattleFactory::BuildBattleUnit.
        pe.artifactAttackBonus     = entry.artifactAttackBonus;
        pe.artifactDefenseBonus    = entry.artifactDefenseBonus;
        pe.artifactMagicBonus      = entry.artifactMagicBonus;
        pe.artifactResistanceBonus = entry.artifactResistanceBonus;
        playerEntries.push_back(std::move(pe));
    }

    const auto battle = gameplay::battle::BattleFactory::CreateFromScenario(
        content_,
        scenarioId,
        playerEntries,
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

        if (battleReturnMode_ == gameplay::GameMode::LocationMode) {
            session_.EnterLocationMode(snapshot.destinationId);
            statusMessage_ = "Failed to initialize battle scenario: " + scenarioId + " | Returned to location";
        }
        else {
            session_.EnterRegionMode();
            statusMessage_ = "Failed to initialize battle scenario: " + scenarioId + " | Returned to region";
        }

        battleStartStackIds_.clear();
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
    // M16-c: Title is presence-gated. With campaigns installed, confirm opens
    // Campaign Selection; cancel skips to standalone play (the existing flow), so
    // standalone stays reachable. With no campaigns, confirm advances as before.
    if (snapshot.mode == gameplay::GameMode::Title) {
        if (input.confirm) {
            if (!content_.Campaigns().empty()) {
                session_.EnterCampaignSelectMode();
                campaignSelectedIndex_ = 0;
            } else {
                session_.AdvanceMode();
            }
        } else if (input.cancel && !content_.Campaigns().empty()) {
            session_.AdvanceMode();   // standalone path when campaigns exist
        }
        return;
    }

    // OpeningSequence -> RegionMode on confirm (unchanged).
    if (input.confirm && snapshot.mode == gameplay::GameMode::OpeningSequence) {
        session_.AdvanceMode();
    }
}

void App::StartBattleScenario(const std::string& scenarioId, const std::string& statusMessage) {
    const gameplay::SessionSnapshot snapshot = session_.Snapshot();
    battleReturnMode_ = snapshot.mode == gameplay::GameMode::LocationMode
        ? gameplay::GameMode::LocationMode
        : gameplay::GameMode::RegionMode;

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
    battleStartStackIds_.clear();
    musteringInteraction_.Close();
    tradingPostInteraction_.Close();
    stationingInteraction_.Close();
    storageInteraction_.Close();
    ownedServiceOverviewModel_ = {};
    ownedServiceSelectedIndex_ = 0;
    worldMapLossConfirmPending_ = false;
    worldMapPendingRegionId_.clear();
    pendingServiceDefenseNodeId_.clear();
    pendingServiceDefenseTeamColor_.clear();
    serviceMaintenanceConfirmPending_ = false;
    serviceMaintenancePendingServiceId_.clear();

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

void App::MarkCurrentDayObservedAfterIntentionalTimeAdvance() {
    // Advance the observed day to the session's current day without penalising
    // the player. This is correct for World Map travel (and any future path that
    // intentionally spans midnight) because the day advance was deliberate, not
    // a missed sleep. Deliberately does NOT set restedThisDay_ = true — the
    // player still needs to sleep before the next day ends normally.
    observedDay_ = session_.Snapshot().day;
    restedThisDay_ = false;
}

std::string App::ResolveSafeFallbackRegionNodeId() const {
    const gameplay::SessionSnapshot snapshot = session_.Snapshot();
    const auto* region = content_.FindRegionById(snapshot.regionId);

    // Helper: true iff `nodeId` is a node in `region`.
    auto nodeExistsInRegion = [&](const data::RegionDefinition* r, const std::string& nodeId) {
        if (r == nullptr || nodeId.empty()) {
            return false;
        }
        for (const auto& node : r->nodes) {
            if (node.locationId == nodeId) {
                return true;
            }
        }
        return false;
    };

    // 1. Prefer the current Region's authored arrival node — this is the
    //    protected entry point and the natural post-travel recovery location.
    if (region != nullptr &&
        !region->arrivalNodeId.empty() &&
        nodeExistsInRegion(region, region->arrivalNodeId)) {
        return region->arrivalNodeId;
    }

    // 2. Fall back to the current destination if it belongs to the current Region
    //    (preserves position when no arrival node is configured).
    if (nodeExistsInRegion(region, snapshot.destinationId)) {
        return snapshot.destinationId;
    }

    // 3. First node of the current Region as a last-resort known-safe position.
    if (region != nullptr && !region->nodes.empty()) {
        return region->nodes.front().locationId;
    }

    // 4. Defensive fallback: keep whatever the current destination is.
    return snapshot.destinationId;
}

void App::ApplyWakePenaltyAndRecover(const std::string& reason) {
    session_.ApplyWakePenalty();
    pendingHostileContactNodeId_.clear();
    pendingHostileContactTeamColor_.clear();

    const std::string fallbackLocationId = ResolveSafeFallbackRegionNodeId();
    session_.SetDestination(fallbackLocationId);
    session_.EnterRegionMode();
    ResetTransientModeState();

    const gameplay::SessionSnapshot penalized = session_.Snapshot();
    statusMessage_ = reason + ". Wake-up penalty applied (-1000 gold, wake at " + penalized.time + ") and moved to " + fallbackLocationId;
}

void App::ResolveBattleOutcomeIfNeeded() {
    if (!battleInitialized_ || !debugBattle_.IsFinished()) {
        return;
    }

    const gameplay::battle::BattleSummary summary = debugBattle_.Summary();
    std::vector<gameplay::BattleStackLifeResult> writeBackResults;
    writeBackResults.reserve(battleStartStackIds_.size());
    for (const auto& unit : debugBattle_.Units()) {
        if (unit.side != gameplay::battle::TeamSide::Allies || unit.rosterStackId.empty()) {
            continue;
        }

        const bool shouldRemoveKoHeroAfterAlliedWin =
            summary.alliesWon &&
            unit.category == gameplay::battle::UnitCategory::Hero &&
            !unit.isPlayerCharacter &&
            unit.lostAfterBattle;

        writeBackResults.push_back(gameplay::BattleStackLifeResult{
            unit.rosterStackId,
            shouldRemoveKoHeroAfterAlliedWin ? 0 : std::max(0, unit.life)
            });
    }

    const bool writeBackOk = session_.ApplyBattleStackLifeResults(writeBackResults, battleStartStackIds_);
    const bool writeBackFailed = !writeBackOk;

    battleInitialized_ = false;
    battleControllerState_ = {};
    battleStartStackIds_.clear();

    if (summary.enemiesWon) {
        // M30 service defense lost: capture/loss consequences resolve through the
        // session before the standard defeat recovery relocates the party.
        const std::string defenseNode = pendingServiceDefenseNodeId_;
        const std::string defenseColor = pendingServiceDefenseTeamColor_;
        pendingServiceDefenseNodeId_.clear();
        pendingServiceDefenseTeamColor_.clear();
        if (!defenseColor.empty()) {
            session_.ApplyServiceDefenseDefeat(defenseColor, defenseNode);
        }
        ApplyWakePenaltyAndRecover("Party defeated in battle");
        if (!defenseColor.empty()) {
            statusMessage_ += " | " + defenseNode + " captured by " + defenseColor + ".";
        }
        if (writeBackFailed) {
            statusMessage_ += " | Roster write-back failed";
        }
        return;
    }

    if (summary.alliesWon) {
        if (!pendingServiceDefenseTeamColor_.empty()) {
            // M30 service defense held: the attacker team is defeated; owned
            // services and their placed defenders are untouched.
            session_.ApplyServiceDefenseVictory(
                pendingServiceDefenseTeamColor_, pendingServiceDefenseNodeId_);
            statusMessage_ = pendingServiceDefenseTeamColor_ + " attack on " +
                pendingServiceDefenseNodeId_ + " repelled.";
            if (summary.playerSetToOneHp) {
                statusMessage_ += " Player recovered to 1 HP.";
            }
            pendingServiceDefenseNodeId_.clear();
            pendingServiceDefenseTeamColor_.clear();
            if (writeBackFailed) { statusMessage_ += " | Roster write-back failed"; }
            // Defeating the last hostile team can latch default victory.
            AppendScenarioEndedStatusIfLatched();
            session_.EnterRegionMode();
            return;
        }
        if (!pendingHostileContactTeamColor_.empty()) {
            // Capture the guarded node before clearing the pending fields so the
            // claim runs at the node the defeated team was occupying.
            const std::string clearedNodeId = pendingHostileContactNodeId_;
            session_.ClearEnemyTeamByColor(pendingHostileContactTeamColor_);
            // Defeating the guarding team claims the eligible ownable services
            // at that node. Runs after the team is cleared so the still-contested
            // guard sees the updated occupation. Same single claim path as peaceful
            // node entry (App::OnDestinationArrived).
            const auto claimedServices = session_.ResolveNodeEntryClaims(clearedNodeId);
            pendingHostileContactNodeId_.clear();
            pendingHostileContactTeamColor_.clear();
            statusMessage_ = summary.playerSetToOneHp
                ? "Hostile team defeated. Player recovered to 1 HP."
                : "Hostile team defeated.";
            if (!claimedServices.empty()) {
                std::string claimList;
                for (const auto& id : claimedServices) {
                    if (!claimList.empty()) { claimList += ", "; }
                    claimList += id;
                }
                statusMessage_ += " | Claimed: " + claimList;
            }
            if (writeBackFailed) { statusMessage_ += " | Roster write-back failed"; }
            // ClearEnemyTeamByColor latches default victory if this was the last
            // hostile team. Surface it in status so the player sees the outcome.
            AppendScenarioEndedStatusIfLatched();
            session_.EnterRegionMode();
            return;
        }
        pendingHostileContactNodeId_.clear();
        pendingHostileContactTeamColor_.clear();

        const gameplay::SessionSnapshot snapshot = session_.Snapshot();
        const auto* location = content_.FindLocationById(snapshot.destinationId);
        const bool nodeIsCombatType = location != nullptr && location->type == data::LocationType::Combat;
        const bool wasCleared = session_.IsCombatNodeCleared(snapshot.destinationId);
        session_.ApplyRegionCombatVictoryNodeClear(
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

        if (writeBackFailed) {
            statusMessage_ += " | Roster write-back failed";
        }

        if (battleReturnMode_ == gameplay::GameMode::LocationMode) {
            session_.EnterLocationMode(snapshot.destinationId);
        }
        else {
            session_.EnterRegionMode();
        }

        return;
    }

    pendingHostileContactNodeId_.clear();
    pendingHostileContactTeamColor_.clear();
    // Unresolved service defense (neither side won): the attack lapses; the
    // attacker remains and may attack again next phase.
    pendingServiceDefenseNodeId_.clear();
    pendingServiceDefenseTeamColor_.clear();
    session_.EnterRegionMode();
    statusMessage_ = "Battle ended.";
    if (writeBackFailed) {
        statusMessage_ += " | Roster write-back failed";
    }
}

bool App::HandleEnemyPhaseResults(const std::vector<gameplay::EnemyTeamActionResult>& results) {
    std::string defenseBattleNodeId;
    std::string defenseBattleTeamColor;
    for (const auto& enemyResult : results) {
        if (enemyResult.actionType == "service_attack" && !enemyResult.summaryText.empty()) {
            statusMessage_ += " | " + enemyResult.summaryText;
        } else if (enemyResult.actionType == "service_attack_pending_battle" &&
                   defenseBattleNodeId.empty()) {
            defenseBattleNodeId = enemyResult.nodeId;
            defenseBattleTeamColor = enemyResult.teamColor;
        }
    }
    if (defenseBattleNodeId.empty()) {
        return false;
    }
    const auto* defenseLocation = content_.FindLocationById(defenseBattleNodeId);
    const std::string defenseScenarioId =
        defenseLocation != nullptr ? defenseLocation->battleScenarioId : std::string{};
    if (defenseScenarioId.empty()) {
        // Same missing-encounter pattern as hostile travel: the attack stalls
        // until content authors an encounter for this node.
        statusMessage_ += " | " + defenseBattleTeamColor + " threatens " +
            defenseBattleNodeId + " — no encounter defined.";
        return false;
    }
    pendingServiceDefenseNodeId_ = defenseBattleNodeId;
    pendingServiceDefenseTeamColor_ = defenseBattleTeamColor;
    StartBattleScenario(defenseScenarioId,
        defenseBattleTeamColor + " attacks " + defenseBattleNodeId + " — defend!");
    return true;
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

    if (snapshot.mode == gameplay::GameMode::RegionMode) {
        UpdateRegionMode(input);
    }

    if (snapshot.mode == gameplay::GameMode::WorldMapMode) {
        UpdateWorldMapMode(input);
    }

    if (snapshot.mode == gameplay::GameMode::CampaignSelectMode) {
        UpdateCampaignSelectMode(input);
    }

    if (snapshot.mode == gameplay::GameMode::LocationMode && locationInitialized_) {
        UpdateLocationScene(input, GetFrameTime());
    }

    if (snapshot.mode == gameplay::GameMode::BattleMode && battleInitialized_) {
        UpdateBattleMode(input);
    }

    if (snapshot.mode == gameplay::GameMode::ScenarioResultMode) {
        UpdateScenarioResultMode(input);
    }

    if (snapshot.mode == gameplay::GameMode::OwnedServiceOverviewMode) {
        UpdateOwnedServiceOverviewMode(input);
    }

    // Save/load is suppressed while a transient screen (ScenarioResult / owned-
    // service overview) is current, so those modes are never persisted. The start-
    // of-frame snapshot is stale here (a mode update this frame may have entered a
    // transient screen), so re-read the current mode rather than trusting
    // `snapshot`.
    const gameplay::GameMode currentMode = session_.Snapshot().mode;
    const bool inTransientScreen =
        currentMode == gameplay::GameMode::ScenarioResultMode ||
        currentMode == gameplay::GameMode::OwnedServiceOverviewMode;

    if (input.save && !inTransientScreen) {
        const bool saved = saveRepository_.SaveToFile(session_.ToSaveData(), "saves/slot_1.json");
        statusMessage_ = saved ? "Saved to saves/slot_1.json" : "Save failed";
    }

    if (input.load && !inTransientScreen) {
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

void App::UpdateRegionMode(const input::InputState& input) {
    // Scenario ended: hand off to the result screen. Campaign progression is
    // deferred to the player's Continue there (see UpdateScenarioResultMode), so
    // the outcome is presented before the next scenario loads. This is the single
    // interception point for both in-region and post-battle latches.
    if (session_.IsScenarioEnded()) {
        session_.EnterScenarioResultMode();
        return;
    }

    // M15-c: open the World Map only from an authored exit node on the Region
    // layer. The World Map screen handles destination selection + travel.
    if (input.openWorldMap && session_.CanOpenWorldMapHere()) {
        session_.EnterWorldMapMode();
        worldMapSelectedIndex_ = 0;
        worldMapLossConfirmPending_ = false;
        worldMapPendingRegionId_.clear();
        statusMessage_ = "World Map: choose a destination Region";
        return;
    }

    // M27: open the read-only owned-service overview from anywhere in Region mode.
    // The model is assembled once here (read-only panel, static while open) so the
    // Draw path never re-scans the catalog.
    if (input.openOwnedServices) {
        ownedServiceOverviewModel_ = ownedServiceOverviewModelMapper_.Map(content_, session_);
        ownedServiceSelectedIndex_ = 0;
        session_.EnterOwnedServiceOverviewMode();
        return;
    }

    // M30 service maintenance at the current node: queue restoration of a
    // destroyed service (single press) or destroy a destroyable one (two-press
    // confirm; also the §20 cancel-queued-restoration path).
    if (input.serviceMaintenance) {
        const auto* maintSvc = session_.DestroyableServiceAtCurrentNode();
        if (maintSvc == nullptr) {
            statusMessage_ = "No destroyable service at this node.";
            serviceMaintenanceConfirmPending_ = false;
            serviceMaintenancePendingServiceId_.clear();
            return;
        }
        const std::string maintServiceId = maintSvc->id;
        const auto* ownedState = session_.FindOwnedService(maintServiceId);
        const bool isDestroyed = ownedState != nullptr && ownedState->destroyed;
        const bool isQueued = ownedState != nullptr && ownedState->restorationQueued;

        if (isDestroyed && !isQueued) {
            // Restoration path: non-destructive, single press.
            if (session_.TryQueueServiceRestorationAtCurrentNode(maintServiceId)) {
                statusMessage_ = "Restoration of " + maintServiceId +
                    " queued — completes at next day start.";
            } else {
                statusMessage_ = "Cannot queue restoration of " + maintServiceId +
                    " (restore cost not affordable).";
            }
            serviceMaintenanceConfirmPending_ = false;
            serviceMaintenancePendingServiceId_.clear();
            return;
        }

        if (!serviceMaintenanceConfirmPending_ ||
            serviceMaintenancePendingServiceId_ != maintServiceId) {
            if (!session_.CanDestroyServiceAtCurrentNode(maintServiceId)) {
                statusMessage_ = "Cannot destroy " + maintServiceId +
                    " (occupied, garrisoned units present, or not enough Energy).";
                serviceMaintenanceConfirmPending_ = false;
                serviceMaintenancePendingServiceId_.clear();
                return;
            }
            serviceMaintenanceConfirmPending_ = true;
            serviceMaintenancePendingServiceId_ = maintServiceId;
            statusMessage_ = (isQueued
                ? "Cancel queued restoration of " + maintServiceId + " by destruction?"
                : "Destroy " + maintServiceId + "?") +
                " (1000 Energy, 1h) — press K again to confirm.";
            return;
        }

        serviceMaintenanceConfirmPending_ = false;
        serviceMaintenancePendingServiceId_.clear();
        if (session_.TryDestroyServiceAtCurrentNode(maintServiceId)) {
            statusMessage_ = isQueued
                ? "Queued restoration of " + maintServiceId + " cancelled."
                : maintServiceId + " destroyed.";
            if (session_.IsScenarioEnded()) {
                AppendScenarioEndedStatusIfLatched();
                return;
            }
            // Destruction costs an hour: the enemy phase runs after every
            // time-costing Region action (core_loop_rules §12).
            const auto* region = content_.FindRegionById(session_.Snapshot().regionId);
            const auto enemyResults = session_.ProcessEnemyPhase(
                region != nullptr ? region->links
                                  : std::vector<data::RegionLinkDefinition>{});
            if (session_.IsScenarioEnded()) {
                AppendScenarioEndedStatusIfLatched();
                return;
            }
            static_cast<void>(HandleEnemyPhaseResults(enemyResults));
        } else {
            statusMessage_ = "Cannot destroy " + maintServiceId + ".";
        }
        return;
    }

    const auto snapshot = session_.Snapshot();
    const auto nodes = regionModelMapper_.BuildNodes(content_, snapshot.regionId, session_.ClearedCombatNodeIds());
    if (nodes.empty()) {
        return;
    }

    const int currentIndex =
        regionModelMapper_.FindNodeIndexById(nodes, snapshot.destinationId);

    const RegionUpdateResult result =
        regionController_.Update(
            input,
            static_cast<int>(nodes.size()),
            currentIndex,
            regionSelectedNodeIndex_);

    regionSelectedNodeIndex_ = result.selectedNodeIndex;

    const auto& currentNode = nodes[currentIndex];
    const bool hasUsableLocalAction =
        currentNode.supportsBattle && !currentNode.combatNodeCleared && !currentNode.battleScenarioId.empty();

    const auto* region = content_.FindRegionById(snapshot.regionId);
    const std::vector<data::RegionLinkDefinition> emptyLinks;
    const auto& links = region != nullptr ? region->links : emptyLinks;
    const auto blockedTransitNodeIds = BuildBlockedTransitNodeIds(nodes);
    const auto hostileOccupied = session_.HostileOccupiedNodeIds("Green");
    const std::string arrivalNodeId = region != nullptr ? region->arrivalNodeId : "";

    bool hasLegalTravelNow = false;
    bool hasReachableTravelIgnoringCutoff = false;
    for (int i = 0; i < static_cast<int>(nodes.size()); ++i) {
        if (i == currentIndex) {
            continue;
        }

        const bool nodeAvailable = nodes[i].travelAvailable;

        const auto nowTravel = gameplay::region::EvaluateTravel(
            snapshot.destinationId,
            nodes[i].id,
            nodeAvailable,
            snapshot.minutesIntoSliceDay,
            links,
            blockedTransitNodeIds,
            /*perHopTravelMinutes=*/15,
            hostileOccupied,
            arrivalNodeId);
        if (nowTravel.legal) {
            hasLegalTravelNow = true;
        }

        const auto ignoreCutoffTravel = gameplay::region::EvaluateTravel(
            snapshot.destinationId,
            nodes[i].id,
            nodeAvailable,
            0,
            links,
            blockedTransitNodeIds,
            /*perHopTravelMinutes=*/15,
            hostileOccupied,
            arrivalNodeId);
        if (ignoreCutoffTravel.legal) {
            hasReachableTravelIgnoringCutoff = true;
        }

        if (hasLegalTravelNow && hasReachableTravelIgnoringCutoff) {
            break;
        }
    }

    const bool declinedUsableLocalAction = hasUsableLocalAction && result.travelCancelled;
    const auto deadEndDecision = gameplay::region::EvaluateDeadEndWakePenalty(
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
        const auto& destination = nodes[regionSelectedNodeIndex_];
        const bool destinationAvailable = destination.travelAvailable;
        const auto travel = gameplay::region::EvaluateTravel(
            snapshot.destinationId,
            destination.id,
            destinationAvailable,
            snapshot.minutesIntoSliceDay,
            links,
            blockedTransitNodeIds,
            /*perHopTravelMinutes=*/15,
            hostileOccupied,
            arrivalNodeId);

        if (!travel.legal) {
            if (travel.reason == gameplay::region::TravelBlockReason::HostileOccupied) {
                if (destination.battleScenarioId.empty()) {
                    statusMessage_ = "Hostile team at " + destination.label +
                        " cannot be engaged — no encounter defined.";
                } else {
                    pendingHostileContactNodeId_ = destination.id;
                    // "Green" is a known hardcode — see plan M11-e Known Risks.
                    pendingHostileContactTeamColor_ =
                        session_.HostileTeamColorAtNode(destination.id, "Green");
                    StartBattleScenario(destination.battleScenarioId,
                        "Hostile encounter at " + destination.label);
                }
                return;
            }
            else if (travel.reason == gameplay::region::TravelBlockReason::DestinationUnavailable) {
                statusMessage_ = destination.label + " is not reachable yet";
            }
            else if (travel.reason == gameplay::region::TravelBlockReason::NoRouteLink) {
                statusMessage_ = "No route link from current node to " + destination.label;
            }
            else if (travel.reason == gameplay::region::TravelBlockReason::ArrivalPastDayEnd) {
                statusMessage_ = "Travel would arrive past 02:00";
            }
            else if (travel.reason == gameplay::region::TravelBlockReason::BlockedByUnclearedTransitNode) {
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
                regionModelMapper_.FormatTravelTime(appliedTravelMinutes) +
                ") | Supply prep used (-" + std::to_string(travel.minutes - appliedTravelMinutes) + "m)";
        }
        else {
            travelStatus =
                "Traveled to " + destination.label + " (" + regionModelMapper_.FormatTravelTime(appliedTravelMinutes) + ")";
        }
        statusMessage_ = travelStatus;
        OnDestinationArrived(destination.id);
        // M12-b outcome check #1: regionNodeEntry events may have removed teams,
        // changed alliances, or set story flags satisfying victory/defeat.
        // FireMatchingEvents already latched if appropriate; just observe and skip
        // enemy phase + battle/location entry when ended.
        if (session_.IsScenarioEnded()) {
            AppendScenarioEndedStatusIfLatched();
            return;
        }

        // Contract: call after every Region-mode time-costing player action.
        // Location-mode time costs are intentionally excluded (core_loop_rules §12).
        // Extension point: add a call here when new Region-mode actions are introduced.
        const auto enemyResults = session_.ProcessEnemyPhase(links);

        // M12-b outcome check #2: enemy phase may flip outcome state. ProcessEnemyPhase
        // already latched if appropriate; just observe and skip battle/location entry.
        if (session_.IsScenarioEnded()) {
            AppendScenarioEndedStatusIfLatched();
            return;
        }

        // M30: surface auto-resolved service attacks, and start the defense battle
        // when the enemy phase attacked the node the party is standing on.
        if (HandleEnemyPhaseResults(enemyResults)) {
            return;
        }

        if (destination.supportsBattle && !destination.combatNodeCleared && !destination.battleScenarioId.empty()) {
            StartBattleScenario(destination.battleScenarioId, travelStatus + " | Encounter started at " + destination.label);
            return;
        }

        if (destination.entersLocationMode) {
            StartLocationMode(destination.id, travelStatus + " | Entering location: " + destination.label);
        }
    }

    if (result.requestDebugBattle) {
        const auto& destination = nodes[regionSelectedNodeIndex_];
        StartBattleScenario(
            !destination.battleScenarioId.empty() ? destination.battleScenarioId : "debug_intro_battle",
            "Debug battle requested");
    }
}

void App::UpdateWorldMapMode(const input::InputState& input) {
    const auto model = worldMapModelMapper_.Map(
        content_, session_, worldMapSelectedIndex_, worldMapLossConfirmPending_);

    // M29: confirming a legal destination while traveling generics would be lost
    // first shows the loss-confirmation block instead of traveling. Illegal
    // destinations skip the warning (the commit-time block reason is reported as
    // before), and a no-loss party travels on the first confirm.
    const bool selectedDestinationLegal =
        worldMapSelectedIndex_ >= 0 &&
        worldMapSelectedIndex_ < static_cast<int>(model.destinations.size()) &&
        model.destinations[worldMapSelectedIndex_].legal;
    const bool lossWarningRequired =
        selectedDestinationLegal && model.genericLossCount > 0;

    const auto result = worldMapController_.Update(
        input, static_cast<int>(model.destinations.size()), worldMapSelectedIndex_,
        worldMapLossConfirmPending_, lossWarningRequired);
    worldMapSelectedIndex_ = result.selectedIndex;

    if (result.cancelled) {
        worldMapLossConfirmPending_ = false;
        worldMapPendingRegionId_.clear();
        session_.EnterRegionMode();
        statusMessage_ = "Returned to Region";
        return;
    }

    if (result.dismissLossConfirmation) {
        worldMapLossConfirmPending_ = false;
        worldMapPendingRegionId_.clear();
        statusMessage_ = "Travel cancelled. Generic units can be stored at a storage service.";
        return;
    }

    if (result.requestLossConfirmation) {
        // lossWarningRequired implies a valid selected index.
        worldMapLossConfirmPending_ = true;
        worldMapPendingRegionId_ = model.destinations[worldMapSelectedIndex_].regionId;
        return;
    }

    if (result.travelConfirmed &&
        worldMapSelectedIndex_ >= 0 &&
        worldMapSelectedIndex_ < static_cast<int>(model.destinations.size())) {
        const auto& dest = model.destinations[worldMapSelectedIndex_];
        if (worldMapLossConfirmPending_ && dest.regionId != worldMapPendingRegionId_) {
            // Stale warning: the selection no longer matches the destination the
            // player was warned about. Drop the pending state instead of
            // committing an unwarned trip.
            worldMapLossConfirmPending_ = false;
            worldMapPendingRegionId_.clear();
            return;
        }
        worldMapLossConfirmPending_ = false;
        worldMapPendingRegionId_.clear();
        const auto travel = session_.TravelToRegion(dest.regionId);
        if (travel.success) {
            const auto snapshot = session_.Snapshot();
            std::string message = "Traveled to " + dest.name +
                " (" + std::to_string(travel.days) + " day(s)). Arrived " + snapshot.time + ".";
            if (travel.genericsDropped > 0) {
                message += " Lost " + std::to_string(travel.genericsDropped) + " generic unit(s).";
            }
            statusMessage_ = message;
            // TravelToRegion leaves the session in RegionMode at the arrival node.
            // Mark the new day as already observed so ApplyMissedSleepPenaltyIfNeeded
            // does not treat the intentional day advance as a missed sleep.
            MarkCurrentDayObservedAfterIntentionalTimeAdvance();
        } else {
            // Report the actual commit-time reason, not the precomputed preview.
            statusMessage_ = "Travel to " + dest.name + " unavailable: " +
                gameplay::worldmap::DescribeWorldMapTravelBlockReason(travel.reason);
        }
    }
}

void App::UpdateCampaignSelectMode(const input::InputState& input) {
    const int campaignCount = static_cast<int>(content_.Campaigns().size());
    const auto result = campaignController_.Update(input, campaignCount, campaignSelectedIndex_);
    campaignSelectedIndex_ = result.selectedIndex;

    if (result.cancelled) {
        session_.EnterTitleMode();   // back to Title, no session mutation
        statusMessage_ = "Campaign selection cancelled";
        return;
    }

    if (result.confirmed && campaignSelectedIndex_ >= 0 && campaignSelectedIndex_ < campaignCount) {
        const auto& campaign = content_.Campaigns()[campaignSelectedIndex_];
        session_.StartCampaign(campaign.id);
        // StartCampaign leaves the session in RegionMode at the start scenario.
        MarkCurrentDayObservedAfterIntentionalTimeAdvance();
        ResetTransientModeState();
        statusMessage_ = "Campaign started: " +
            (campaign.name.empty() ? campaign.id : campaign.name);
    }
}

void App::UpdateScenarioResultMode(const input::InputState& input) {
    // Hold on the result screen until the player confirms. Idempotent: the mode
    // changes the instant Continue is handled (campaign advance switches to
    // RegionMode and clears the latch; terminal/standalone returns to Title), so
    // a held key cannot double-advance.
    if (!input.confirm) {
        return;
    }

    if (session_.IsCampaignActive()) {
        HandleCampaignProgressIfLatched();
        // Terminal Completed/Failed leaves the latch in place; mid-campaign
        // Victory has already advanced to the next scenario in RegionMode.
        if (session_.IsScenarioEnded()) {
            session_.EnterTitleMode();
        }
    } else {
        session_.EnterTitleMode();
    }
}

void App::UpdateOwnedServiceOverviewMode(const input::InputState& input) {
    // Read-only: navigate the cached row list or leave. No session mutation.
    if (input.cancel || input.openOwnedServices) {
        session_.ExitOwnedServiceOverviewMode();
        return;
    }

    const int rowCount = static_cast<int>(ownedServiceOverviewModel_.rows.size());
    if (rowCount <= 0) {
        return;
    }
    if (input.targetPrev) {
        ownedServiceSelectedIndex_ =
            (ownedServiceSelectedIndex_ - 1 + rowCount) % rowCount;
    } else if (input.targetNext) {
        ownedServiceSelectedIndex_ = (ownedServiceSelectedIndex_ + 1) % rowCount;
    }
}

void App::HandleCampaignProgressIfLatched() {
    if (!session_.IsCampaignActive() || !session_.IsScenarioEnded()) {
        return;
    }

    const std::string previousScenarioId = session_.CurrentScenarioId();
    session_.ResolveCampaignAfterOutcome();

    switch (session_.GetCampaignState()) {
    case gameplay::CampaignState::InProgress:
        if (session_.CurrentScenarioId() != previousScenarioId) {
            // Advanced to the next scenario: the latch is cleared and the session
            // is back in RegionMode at the new start. Treat as an intentional
            // time-neutral transition and reset transient battle/location state.
            MarkCurrentDayObservedAfterIntentionalTimeAdvance();
            ResetTransientModeState();
            statusMessage_ = "Scenario complete. Now: " + session_.CurrentScenarioId();
        }
        break;
    case gameplay::CampaignState::Completed:
        statusMessage_ = "Campaign complete!";
        break;
    case gameplay::CampaignState::Failed:
        statusMessage_ = "Campaign failed.";
        break;
    case gameplay::CampaignState::None:
        break;
    }
}

void App::AppendScenarioEndedStatusIfLatched() {
    const auto& outcome = session_.Outcome();
    if (!outcome.has_value()) return;
    const std::string label =
        outcome->state == gameplay::scenario::ScenarioOutcomeState::Victory ? "Victory!"
      : outcome->state == gameplay::scenario::ScenarioOutcomeState::Defeat  ? "Defeat."
      : std::string{};
    if (label.empty()) return;
    statusMessage_ += " | " + label;
    if (!outcome->reason.empty()) {
        statusMessage_ += " " + outcome->reason;
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

    // Fire authored regionNodeEntry events for this node. Quest notification runs
    // first so quest-log updates appear before any event-driven status text.
    const auto eventResults = session_.NotifyRegionNodeEntry(destinationId);
    for (const auto& result : eventResults) {
        if (!result.message.empty()) {
            statusMessage_ += " | " + result.message;
        }
    }

    // M26 general player-side claiming: legally entering an unguarded node claims
    // its eligible ownable services. Runs AFTER node-entry events so a guard
    // spawned by one of those events blocks the claim (ResolveNodeEntryClaims is a
    // no-op while the node is hostile-occupied). Guarded nodes never reach here —
    // hostile contact starts battle before placement and claims post-victory.
    const auto claimedServices = session_.ResolveNodeEntryClaims(destinationId);
    if (!claimedServices.empty()) {
        std::string claimList;
        for (const auto& id : claimedServices) {
            if (!claimList.empty()) { claimList += ", "; }
            claimList += id;
        }
        statusMessage_ += " | Claimed: " + claimList;
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

TradingPostCommand App::TranslateTradingPostCommand(const input::InputState& input) const {
    if (input.interact) {
        return TradingPostCommand::Exit;
    }
    if (input.option1) {
        return TradingPostCommand::ConfirmTrade;
    }
    if (input.option2) {
        return TradingPostCommand::CycleMode;
    }
    if (input.selectPrev) {
        return TradingPostCommand::SelectPrev;
    }
    if (input.selectNext) {
        return TradingPostCommand::SelectNext;
    }
    if (input.targetPrev) {
        return TradingPostCommand::QuantityDown;
    }
    if (input.targetNext) {
        return TradingPostCommand::QuantityUp;
    }
    return TradingPostCommand::None;
}

StationingCommand App::TranslateStationingCommand(const input::InputState& input) const {
    if (input.interact) {
        return StationingCommand::Exit;
    }
    if (input.option1) {
        return StationingCommand::Confirm;
    }
    if (input.option2) {
        return StationingCommand::CycleList;
    }
    if (input.selectPrev) {
        return StationingCommand::SelectPrev;
    }
    if (input.selectNext) {
        return StationingCommand::SelectNext;
    }
    if (input.targetPrev) {
        return StationingCommand::QuantityDown;
    }
    if (input.targetNext) {
        return StationingCommand::QuantityUp;
    }
    return StationingCommand::None;
}

StorageCommand App::TranslateStorageCommand(const input::InputState& input) const {
    if (input.interact) {
        return StorageCommand::Exit;
    }
    if (input.option1) {
        return StorageCommand::Confirm;
    }
    if (input.option2) {
        return StorageCommand::CycleList;
    }
    if (input.selectPrev) {
        return StorageCommand::SelectPrev;
    }
    if (input.selectNext) {
        return StorageCommand::SelectNext;
    }
    return StorageCommand::None;
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

    if (tradingPostInteraction_.IsActive()) {
        const TradingPostCommand command = TranslateTradingPostCommand(input);
        const auto tradeResult = tradingPostInteraction_.ApplyCommand(command, session_);
        if (!tradeResult.statusText.empty()) {
            statusMessage_ = tradeResult.statusText;
        }

        if (tradeResult.shouldExit) {
            tradingPostInteraction_.Close();
        }

        return;
    }

    if (stationingInteraction_.IsActive()) {
        const StationingCommand command = TranslateStationingCommand(input);
        const auto stationingResult = stationingInteraction_.ApplyCommand(command, session_);
        if (!stationingResult.statusText.empty()) {
            statusMessage_ = stationingResult.statusText;
        }

        if (stationingResult.shouldExit) {
            stationingInteraction_.Close();
        }

        return;
    }

    if (storageInteraction_.IsActive()) {
        const StorageCommand command = TranslateStorageCommand(input);
        const auto storageResult = storageInteraction_.ApplyCommand(command, session_);
        if (!storageResult.statusText.empty()) {
            statusMessage_ = storageResult.statusText;
        }

        if (storageResult.shouldExit) {
            storageInteraction_.Close();
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

    if (gameplay::location::IsTradingPostService(&service)) {
        if (!session_.ResolveTradingPostOffer(service.id).usable) {
            statusMessage_ = !service.failureText.empty()
                ? service.failureText
                : "Trading Post is not available";
            return false;
        }
        tradingPostInteraction_.Open(session_, service);
        statusMessage_ = !service.successText.empty()
            ? service.successText
            : "Opened Trading Post";
        return true;
    }

    if (gameplay::location::IsMineService(&service)) {
        if (!session_.CanOpenStationingAtMine(service.id)) {
            statusMessage_ = !service.failureText.empty()
                ? service.failureText
                : "This mine is not available";
            return false;
        }
        stationingInteraction_.Open(session_, service);
        statusMessage_ = !service.successText.empty()
            ? service.successText
            : "Stationing units at the mine";
        return true;
    }

    if (gameplay::location::IsStorageService(&service)) {
        if (!session_.CanOpenStorageAtService(service.id)) {
            statusMessage_ = !service.failureText.empty()
                ? service.failureText
                : "This storage is not available";
            return false;
        }
        storageInteraction_.Open(session_, service);
        statusMessage_ = !service.successText.empty()
            ? service.successText
            : "Storing units";
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
    else if (snapshot.mode == gameplay::GameMode::LocationMode && tradingPostInteraction_.IsActive()) {
        locationPromptOverride = mappers::InteractPromptOverride{
            tradingPostInteraction_.BuildPromptText(session_),
            true
        };
    }
    else if (snapshot.mode == gameplay::GameMode::LocationMode && stationingInteraction_.IsActive()) {
        locationPromptOverride = mappers::InteractPromptOverride{
            stationingInteraction_.BuildPromptText(session_),
            true
        };
    }
    else if (snapshot.mode == gameplay::GameMode::LocationMode && storageInteraction_.IsActive()) {
        locationPromptOverride = mappers::InteractPromptOverride{
            storageInteraction_.BuildPromptText(session_),
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
        DrawTextEx(font, "Press Enter to enter the Region. (Press M at an exit node to open the World Map.)", {80.0f, 230.0f}, context.normalFontSize, 1.0f, context.theme.mutedTextColor);
        break;
    }
    case gameplay::GameMode::CampaignSelectMode: {
        const auto model = campaignModelMapper_.Map(content_, campaignSelectedIndex_);
        campaignSelectRenderer_.Draw(context, model);
        break;
    }
    case gameplay::GameMode::WorldMapMode: {
        const auto model = worldMapModelMapper_.Map(
            content_, session_, worldMapSelectedIndex_, worldMapLossConfirmPending_);
        worldMapRenderer_.Draw(context, model);
        break;
    }
    case gameplay::GameMode::RegionMode: {
        const auto model = regionModelMapper_.Map(
            content_,
            session_,
            snapshot,
            regionSelectedNodeIndex_,
            session_.ClearedCombatNodeIds());

        regionRenderer_.Draw(context, model);
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
    case gameplay::GameMode::ScenarioResultMode: {
        const auto model = scenarioResultModelMapper_.Map(content_, session_);
        scenarioResultRenderer_.Draw(context, model);
        break;
    }
    case gameplay::GameMode::OwnedServiceOverviewMode: {
        // Render the model cached on mode-enter; only the selection is per-frame.
        ashvale::rendering::OwnedServiceOverviewModel model = ownedServiceOverviewModel_;
        model.selectedIndex = ownedServiceSelectedIndex_;
        ownedServiceOverviewRenderer_.Draw(context, model);
        break;
    }
    }

    if (snapshot.mode != gameplay::GameMode::Title &&
        snapshot.mode != gameplay::GameMode::CampaignSelectMode &&
        snapshot.mode != gameplay::GameMode::ScenarioResultMode &&
        snapshot.mode != gameplay::GameMode::OwnedServiceOverviewMode) {
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

#pragma once

#include <string>
#include <vector>

#include "data/ContentRepository.h"
#include "data/definitions/LocationDefinition.h"
#include "gameplay/GameSession.h"
#include "rendering/RegionRenderer.h"

namespace app::mappers
{
    struct RegionNodeMeta
    {
        std::string id;
        std::string label;
        data::LocationType type = data::LocationType::Unknown;
        bool travelAvailable = true;
        bool entersLocationMode = false;
        bool supportsBattle = false;
        bool combatNodeCleared = false;
        bool blocksTransitUntilCleared = false;
        std::string battleScenarioId;
        bool discovered = true;
        float x = 0.0f;
        float y = 0.0f;
    };

    class RegionModelMapper
    {
    public:
        [[nodiscard]] std::vector<RegionNodeMeta> BuildNodes(
            const data::ContentRepository& content,
            const std::string& regionId,
            const std::vector<std::string>& clearedCombatNodeIds) const;

        [[nodiscard]] int FindNodeIndexById(
            const std::vector<RegionNodeMeta>& nodes,
            const std::string& id) const;

        [[nodiscard]] std::string FormatTravelTime(int minutes) const;

        [[nodiscard]] ashvale::rendering::RegionRenderModel Map(
            const data::ContentRepository& content,
            const gameplay::GameSession& session,
            const gameplay::SessionSnapshot& snapshot,
            int selectedNodeIndex,
            const std::vector<std::string>& clearedCombatNodeIds) const;
    };
}
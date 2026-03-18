#pragma once

#include <string>
#include <vector>

#include "data/ContentRepository.h"
#include "data/definitions/LocationDefinition.h"
#include "gameplay/GameSession.h"
#include "rendering/OverworldRenderer.h"

namespace app::mappers
{
    struct OverworldNodeMeta
    {
        std::string id;
        std::string label;
        data::LocationType type = data::LocationType::Unknown;
        bool travelAvailable = true;
        bool entersLocationMode = false;
        bool supportsBattle = false;
        std::string battleScenarioId;
        bool discovered = true;
        float x = 0.0f;
        float y = 0.0f;
    };

    class OverworldModelMapper
    {
    public:
        [[nodiscard]] std::vector<OverworldNodeMeta> BuildNodes(
            const data::ContentRepository& content,
            const std::string& regionId) const;

        [[nodiscard]] int FindNodeIndexById(
            const std::vector<OverworldNodeMeta>& nodes,
            const std::string& id) const;

        [[nodiscard]] int ComputeTravelPreviewMinutes(
            int currentIndex,
            int selectedIndex) const;

        [[nodiscard]] std::string FormatTravelTime(int minutes) const;

        [[nodiscard]] ashvale::rendering::OverworldRenderModel Map(
            const data::ContentRepository& content,
            const gameplay::SessionSnapshot& snapshot,
            int selectedNodeIndex) const;
    };
}
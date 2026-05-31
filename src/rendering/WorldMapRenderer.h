#pragma once

#include <string>
#include <vector>

#include "RenderContext.h"

namespace ashvale::rendering
{
    // One selectable destination row on the World Map screen.
    struct WorldMapDestinationView
    {
        std::string regionId;
        std::string name;
        bool unlocked = false;
        bool legal = false;        // travel currently legal to this destination
        std::string statusText;    // "1 day, 1000 Energy" or the block reason
        int days = 0;
        bool selected = false;
    };

    struct WorldMapModel
    {
        std::string title = "World Map";
        std::string currentRegionName;
        int energy = 0;
        int maxEnergy = 0;
        int travelEnergyCost = 1000;
        int genericLossCount = 0;  // generics that would be lost on departure
        std::vector<WorldMapDestinationView> destinations;
        std::string footerHint;
    };

    class WorldMapRenderer
    {
    public:
        void Draw(const RenderContext& context, const WorldMapModel& model) const;
    };
}

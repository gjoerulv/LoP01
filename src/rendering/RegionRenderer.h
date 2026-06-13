#pragma once

#include <array>
#include <string>
#include <vector>
#include "RenderContext.h"

namespace ashvale::rendering
{
    enum class RegionNodeType
    {
        Unknown,
        Home,
        Town,
        Dungeon,
        Service,
        Recruit,
        Combat
    };

    struct RegionNodeView
    {
        std::string id;
        std::string label;
        Vector2 position{};
        RegionNodeType type = RegionNodeType::Unknown;
        bool discovered = true;
        bool enterable = true;
        bool current = false;
        bool selected = false;
        bool hostileOccupied = false;
        // M32 fog/reveal: runtime knowledge layer, distinct from authored
        // `discovered` (which stays the structural-availability flag). `revealed`
        // is false for nodes the player has not yet uncovered. `enemyEstimate` is a
        // bounded player-facing presence/estimate line, populated only when a
        // hostile team is on a revealed node (empty otherwise).
        bool revealed = true;
        std::string enemyEstimate;
    };

    struct RegionRenderModel
    {
        std::string regionName;
        std::string hintText = "Left/Right select destination";
        std::string controlsText = "Enter confirm travel | Esc cancel selection";
        std::string currentNodeLabel;
        std::string selectedNodeLabel;
        std::string selectedNodeType;
        std::string selectedNodeEnterable;
        std::string travelTimeText;

        std::vector<RegionNodeView> nodes;
        std::vector<std::array<int, 2>> links;
    };

    class RegionRenderer
    {
    public:
        void Draw(const RenderContext& context, const RegionRenderModel& model) const;

    private:
        Color GetNodeColor(RegionNodeType type, const UiTheme& theme) const;
    };
}
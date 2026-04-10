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
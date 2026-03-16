#pragma once

#include <array>
#include <string>
#include <vector>
#include "RenderContext.h"

namespace ashvale::rendering
{
    enum class OverworldNodeType
    {
        Unknown,
        Home,
        Town,
        Dungeon,
        Service,
        Recruit,
        Combat
    };

    struct OverworldNodeView
    {
        std::string id;
        std::string label;
        Vector2 position{};
        OverworldNodeType type = OverworldNodeType::Unknown;
        bool discovered = true;
        bool enterable = true;
        bool current = false;
        bool selected = false;
    };

    struct OverworldRenderModel
    {
        std::string regionName;
        std::string hintText = "Left/Right select destination";
        std::string controlsText = "Enter confirm travel | Esc cancel selection";
        std::string currentNodeLabel;
        std::string selectedNodeLabel;
        std::string selectedNodeType;
        std::string selectedNodeEnterable;
        std::string travelTimeText;

        std::vector<OverworldNodeView> nodes;
        std::vector<std::array<int, 2>> links;
    };

    class OverworldRenderer
    {
    public:
        void Draw(const RenderContext& context, const OverworldRenderModel& model) const;

    private:
        Color GetNodeColor(OverworldNodeType type, const UiTheme& theme) const;
    };
}
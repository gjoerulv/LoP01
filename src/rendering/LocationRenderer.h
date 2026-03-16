#pragma once

#include <string>
#include <vector>
#include "RenderContext.h"

namespace ashvale::rendering
{
    enum class LocationInteractableType
    {
        Unknown,
        Door,
        Inn,
        Shop,
        Recruit,
        Npc,
        Exit
    };

    struct LocationZoneView
    {
        Rectangle bounds{};
        std::string label;
        LocationInteractableType type = LocationInteractableType::Unknown;
        bool highlighted = false;
    };

    struct LocationNpcView
    {
        Rectangle bounds{};
        std::string name;
        bool highlighted = false;
    };

    struct LocationDialogueView
    {
        bool visible = false;
        std::string speaker;
        std::string text;
        std::vector<std::string> options;
        int selectedOption = 0;
    };

    struct LocationRenderModel
    {
        std::string locationName;
        Rectangle playerBounds{};

        std::vector<Rectangle> buildingBlocks;
        std::vector<Rectangle> walls;
        std::vector<LocationZoneView> zones;
        std::vector<LocationNpcView> npcs;

        bool showInteractPrompt = false;
        std::string interactPrompt = "Press E to interact";

        LocationDialogueView dialogue;
    };

    class LocationRenderer
    {
    public:
        void Draw(const RenderContext& context, const LocationRenderModel& model) const;

    private:
        Color GetZoneColor(LocationInteractableType type, const UiTheme& theme) const;
    };
}
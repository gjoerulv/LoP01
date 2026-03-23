#include "app/mappers/LocationModelMapper.h"

#include <cctype>
#include <string>

namespace app::mappers
{
    namespace
    {
        using ashvale::rendering::LocationDialogueView;
        using ashvale::rendering::LocationInteractableType;
        using ashvale::rendering::LocationNpcView;
        using ashvale::rendering::LocationRenderModel;
        using ashvale::rendering::LocationZoneView;

        std::string HumanizeId(std::string value)
        {
            bool nextUpper = true;
            for (char& ch : value)
            {
                if (ch == '_')
                {
                    ch = ' ';
                    nextUpper = true;
                    continue;
                }

                if (nextUpper)
                {
                    ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
                    nextUpper = false;
                }
            }

            return value;
        }

        Rectangle ToRectangle(const gameplay::location::RectF& rect)
        {
            return Rectangle{ rect.x, rect.y, rect.width, rect.height };
        }

        bool Intersects(const Rectangle& a, const Rectangle& b)
        {
            return a.x < b.x + b.width &&
                a.x + a.width > b.x &&
                a.y < b.y + b.height &&
                a.y + a.height > b.y;
        }

        int FindNearLocationZoneIndex(const gameplay::location::LocationScene& scene)
        {
            const auto& player = scene.Player();
            Rectangle probe{ player.x - 20.0f, player.y - 20.0f, player.width + 40.0f, player.height + 40.0f };

            const auto& zones = scene.Zones();
            for (int i = 0; i < static_cast<int>(zones.size()); ++i)
            {
                if (Intersects(probe, ToRectangle(zones[i].area)))
                {
                    return i;
                }
            }

            return -1;
        }

        LocationInteractableType ToInteractableType(const gameplay::location::InteractionType type)
        {
            switch (type)
            {
            case gameplay::location::InteractionType::InnDoor:
                return LocationInteractableType::Inn;
            case gameplay::location::InteractionType::Shop:
                return LocationInteractableType::Shop;
            case gameplay::location::InteractionType::Recruit:
                return LocationInteractableType::Recruit;
            case gameplay::location::InteractionType::Npc:
                return LocationInteractableType::Npc;
            case gameplay::location::InteractionType::Exit:
                return LocationInteractableType::Exit;
            }

            return LocationInteractableType::Unknown;
        }
    }

    LocationRenderModel LocationModelMapper::Map(
        const data::ContentRepository& content,
        const gameplay::SessionSnapshot& snapshot,
        const gameplay::location::LocationScene& scene,
        const std::string& statusText) const
    {
        LocationRenderModel model;
        const data::LocationDefinition* location = content.FindLocationById(snapshot.destinationId);
        model.locationName = location != nullptr
            ? location->name
            : HumanizeId(snapshot.destinationId);

        const auto& player = scene.Player();
        model.playerBounds = Rectangle{ player.x, player.y, player.width, player.height };

        const auto& blocks = scene.BlockingRects();
        for (int i = 0; i < static_cast<int>(blocks.size()); ++i)
        {
            const Rectangle rect{ blocks[i].x, blocks[i].y, blocks[i].width, blocks[i].height };
            if (i < 4)
            {
                model.walls.push_back(rect);
            }
            else
            {
                model.buildingBlocks.push_back(rect);
            }
        }

        const int nearZone = FindNearLocationZoneIndex(scene);
        const auto& zones = scene.Zones();
        for (int i = 0; i < static_cast<int>(zones.size()); ++i)
        {
            LocationZoneView zoneView;
            zoneView.bounds = ToRectangle(zones[i].area);
            zoneView.label = HumanizeId(zones[i].id);
            zoneView.type = ToInteractableType(zones[i].type);
            zoneView.highlighted = i == nearZone;
            model.zones.push_back(zoneView);

            if (zones[i].type == gameplay::location::InteractionType::Npc)
            {
                model.npcs.push_back(LocationNpcView{
                    ToRectangle(zones[i].area),
                    HumanizeId(zones[i].id),
                    i == nearZone
                    });
            }
        }
            
        model.showInteractPrompt = nearZone >= 0;
        if (scene.HasActiveDialogue())
        {
            model.interactPrompt = "Choose dialogue option (1/2)";
        }
        else if (nearZone >= 0)
        {
            const data::LocationServiceDefinition* service =
                content.FindLocationService(snapshot.destinationId, zones[nearZone].id);

            model.interactPrompt = service != nullptr
                ? service->promptText
                : zones[nearZone].promptText;
        }
        else
        {
            model.interactPrompt = "Move near a marker and press E";
        }

        model.dialogue = LocationDialogueView{};
        model.dialogue.visible = scene.HasActiveDialogue();
        model.dialogue.speaker = "Resident";
        model.dialogue.text = statusText;
        model.dialogue.options = scene.ActiveDialogueChoices();

        return model;
    }
}
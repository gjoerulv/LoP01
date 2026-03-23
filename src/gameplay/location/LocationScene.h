#pragma once

#include <optional>
#include <string>
#include <vector>

#include "data/definitions/LocationSceneDefinition.h"

namespace gameplay::location {

    enum class InteractionType {
        InnDoor,
        Shop,
        Recruit,
        Npc,
        Exit
    };

    struct RectF {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
    };

    struct InteractionZone {
        std::string id;
        InteractionType type = InteractionType::Npc;
        RectF area;

        std::string promptText;
        std::string resultText;
        std::string failureText;

        int timeCostMinutes = 0;
        int goldCost = 0;
        int recruitCount = 0;
        int dialogueChoiceTimeCostMinutes = 1;

        std::vector<std::string> dialogueChoices;
    };

    struct InteractionOutcome {
        InteractionType type = InteractionType::Npc;
        bool requiresDialogueChoice = false;
        bool exitsLocation = false;

        std::string zoneId;
        std::string message;
        std::string failureText;

        int timeCostMinutes = 0;
        int goldCost = 0;
        int recruitCount = 0;
    };

    class LocationScene {
    public:
        LocationScene() = default;

        void Reset(const data::LocationSceneDefinition& definition);

        [[nodiscard]] const RectF& Player() const;
        [[nodiscard]] const std::vector<RectF>& BlockingRects() const;
        [[nodiscard]] const std::vector<InteractionZone>& Zones() const;

        bool TryMovePlayer(float dx, float dy);

        [[nodiscard]] std::optional<InteractionOutcome> Interact();
        [[nodiscard]] bool HasActiveDialogue() const;
        [[nodiscard]] const std::vector<std::string>& ActiveDialogueChoices() const;
        [[nodiscard]] std::optional<InteractionOutcome> ChooseDialogueOption(int optionIndex);

    private:
        [[nodiscard]] bool IntersectsBlocking(const RectF& rect) const;
        [[nodiscard]] bool Intersects(const RectF& a, const RectF& b) const;

        RectF player_;
        std::vector<RectF> blockingRects_;
        std::vector<InteractionZone> zones_;
        std::optional<std::string> activeDialogueZoneId_;
        std::vector<std::string> activeDialogueChoices_;
        int activeDialogueChoiceTimeCostMinutes_ = 1;
    };

} // namespace gameplay::location
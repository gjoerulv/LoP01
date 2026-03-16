#pragma once

#include <optional>
#include <string>
#include <vector>

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
    std::vector<std::string> dialogueChoices;
};

struct InteractionOutcome {
    InteractionType type = InteractionType::Npc;
    bool requiresDialogueChoice = false;
    bool exitsLocation = false;
    std::string message;
};

class LocationScene {
public:
    LocationScene();

    void Reset();

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
};

} // namespace gameplay::location

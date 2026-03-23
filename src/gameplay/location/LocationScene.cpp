#include "gameplay/location/LocationScene.h"

#include <algorithm>

namespace gameplay::location {

    namespace
    {
        RectF ToRectF(const data::SceneRectDefinition& rect)
        {
            return RectF{ rect.x, rect.y, rect.width, rect.height };
        }

        InteractionType ToInteractionType(const data::LocationSceneZoneType type)
        {
            switch (type)
            {
            case data::LocationSceneZoneType::InnDoor: return InteractionType::InnDoor;
            case data::LocationSceneZoneType::Shop:    return InteractionType::Shop;
            case data::LocationSceneZoneType::Recruit: return InteractionType::Recruit;
            case data::LocationSceneZoneType::Npc:     return InteractionType::Npc;
            case data::LocationSceneZoneType::Exit:    return InteractionType::Exit;
            default:                                   return InteractionType::Npc;
            }
        }
    }

    void LocationScene::Reset(const data::LocationSceneDefinition& definition) {
        player_ = ToRectF(definition.spawn);

        blockingRects_.clear();
        for (const auto& rect : definition.blockingRects) {
            blockingRects_.push_back(ToRectF(rect));
        }

        zones_.clear();
        for (const auto& zone : definition.zones) {
            zones_.push_back(InteractionZone{
                zone.id,
                ToInteractionType(zone.type),
                ToRectF(zone.area),
                zone.promptText,
                zone.resultText,
                zone.failureText,
                zone.timeCostMinutes,
                zone.goldCost,
                zone.recruitCount,
                zone.dialogueChoiceTimeCostMinutes,
                zone.dialogueChoices
                });
        }

        activeDialogueZoneId_.reset();
        activeDialogueChoices_.clear();
        activeDialogueChoiceTimeCostMinutes_ = 1;
    }

    const RectF& LocationScene::Player() const {
        return player_;
    }

    const std::vector<RectF>& LocationScene::BlockingRects() const {
        return blockingRects_;
    }

    const std::vector<InteractionZone>& LocationScene::Zones() const {
        return zones_;
    }

    bool LocationScene::TryMovePlayer(const float dx, const float dy) {
        const RectF next = RectF{ player_.x + dx, player_.y + dy, player_.width, player_.height };
        if (IntersectsBlocking(next)) {
            return false;
        }

        player_ = next;
        return true;
    }

    std::optional<InteractionOutcome> LocationScene::Interact() {
        for (const auto& zone : zones_) {
            if (!Intersects(player_, zone.area)) {
                continue;
            }

            if (zone.type == InteractionType::Npc) {
                activeDialogueZoneId_ = zone.id;
                activeDialogueChoices_ = zone.dialogueChoices;
                activeDialogueChoiceTimeCostMinutes_ = zone.dialogueChoiceTimeCostMinutes;

                InteractionOutcome outcome;
                outcome.type = zone.type;
                outcome.requiresDialogueChoice = true;
				outcome.zoneId = zone.id;
                outcome.message = "Choose dialogue option";
                return outcome;
            }

            InteractionOutcome outcome;
            outcome.type = zone.type;
            outcome.requiresDialogueChoice = false;
            outcome.exitsLocation = zone.type == InteractionType::Exit;
            outcome.zoneId = zone.id;
            outcome.message = zone.resultText;
            outcome.failureText = zone.failureText;
            outcome.timeCostMinutes = zone.timeCostMinutes;
            outcome.goldCost = zone.goldCost;
            outcome.recruitCount = zone.recruitCount;
            return outcome;
        }

        return std::nullopt;
    }

    bool LocationScene::HasActiveDialogue() const {
        return activeDialogueZoneId_.has_value();
    }

    const std::vector<std::string>& LocationScene::ActiveDialogueChoices() const {
        return activeDialogueChoices_;
    }

    std::optional<InteractionOutcome> LocationScene::ChooseDialogueOption(const int optionIndex) {
        if (!HasActiveDialogue() || optionIndex < 0 || optionIndex >= static_cast<int>(activeDialogueChoices_.size())) {
            return std::nullopt;
        }

        const std::string selected = activeDialogueChoices_[optionIndex];
        const std::string zoneId = activeDialogueZoneId_.value_or("");
        activeDialogueZoneId_.reset();
        activeDialogueChoices_.clear();

        InteractionOutcome outcome;
        outcome.type = InteractionType::Npc;
        outcome.zoneId = zoneId;
        outcome.message = selected;
        outcome.timeCostMinutes = activeDialogueChoiceTimeCostMinutes_;
        return outcome;
    }

    bool LocationScene::IntersectsBlocking(const RectF& rect) const {
        return std::ranges::any_of(blockingRects_, [this, &rect](const RectF& block) {
            return Intersects(rect, block);
            });
    }

    bool LocationScene::Intersects(const RectF& a, const RectF& b) const {
        return a.x < b.x + b.width &&
            a.x + a.width > b.x &&
            a.y < b.y + b.height &&
            a.y + a.height > b.y;
    }

} // namespace gameplay::location
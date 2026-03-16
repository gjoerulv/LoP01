#include "gameplay/location/LocationScene.h"

#include <algorithm>

namespace gameplay::location {

LocationScene::LocationScene() {
    Reset();
}

void LocationScene::Reset() {
    player_ = RectF{84.0f, 520.0f, 24.0f, 24.0f};

    blockingRects_ = {
        RectF{0.0f, 0.0f, 1280.0f, 40.0f},
        RectF{0.0f, 0.0f, 40.0f, 720.0f},
        RectF{1240.0f, 0.0f, 40.0f, 720.0f},
        RectF{0.0f, 680.0f, 1280.0f, 40.0f},
        RectF{120.0f, 140.0f, 260.0f, 180.0f},
        RectF{520.0f, 140.0f, 260.0f, 180.0f},
        RectF{920.0f, 140.0f, 260.0f, 180.0f}
    };

    zones_ = {
        InteractionZone{"inn_door", InteractionType::InnDoor, RectF{220.0f, 320.0f, 80.0f, 20.0f}, {}},
        InteractionZone{"shop_counter", InteractionType::Shop, RectF{620.0f, 320.0f, 80.0f, 20.0f}, {}},
        InteractionZone{"recruit_board", InteractionType::Recruit, RectF{1020.0f, 320.0f, 80.0f, 20.0f}, {}},
        InteractionZone{"npc_old_man", InteractionType::Npc, RectF{460.0f, 500.0f, 40.0f, 40.0f}, {"Ask about the town", "Ask about the mine"}},
        InteractionZone{"npc_guard", InteractionType::Npc, RectF{760.0f, 500.0f, 40.0f, 40.0f}, {"Ask about patrol routes", "Ask about recruitment"}},
        InteractionZone{"town_exit", InteractionType::Exit, RectF{70.0f, 560.0f, 80.0f, 80.0f}, {}}
    };

    activeDialogueZoneId_.reset();
    activeDialogueChoices_.clear();
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
    const RectF next = RectF{player_.x + dx, player_.y + dy, player_.width, player_.height};
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
            return InteractionOutcome{zone.type, true, false, "Choose dialogue option (1 or 2)"};
        }

        if (zone.type == InteractionType::InnDoor) {
            return InteractionOutcome{zone.type, false, false, "Opened inn door (+1 min)"};
        }

        if (zone.type == InteractionType::Shop) {
            return InteractionOutcome{zone.type, false, false, "Shopping transaction (+5 min)"};
        }

        if (zone.type == InteractionType::Recruit) {
            return InteractionOutcome{zone.type, false, false, "Recruit transaction (+10 min)"};
        }

        if (zone.type == InteractionType::Exit) {
            return InteractionOutcome{zone.type, false, true, "Exited to overworld"};
        }
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
    activeDialogueZoneId_.reset();
    activeDialogueChoices_.clear();

    return InteractionOutcome{InteractionType::Npc, false, false, selected + " (+1 min)"};
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

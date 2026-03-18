#include "app/LocationController.h"

namespace app
{
    LocationController::LocationController(const float moveSpeed)
        : moveSpeed_(moveSpeed)
    {
    }

    LocationUpdateResult LocationController::Update(
        const input::InputState& input,
        const float deltaTime,
        const bool hasActiveDialogue) const
    {
        LocationUpdateResult result{};

        if (hasActiveDialogue)
        {
            result.chooseOption1 = input.option1;
            result.chooseOption2 = input.option2;
            return result;
        }

        result.moveDx = input.moveAxisX * moveSpeed_ * deltaTime;
        result.moveDy = input.moveAxisY * moveSpeed_ * deltaTime;
        result.interact = input.interact;

        return result;
    }
}
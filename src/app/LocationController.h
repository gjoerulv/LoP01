#pragma once

#include "app/input/InputState.h"

namespace app
{
    struct LocationUpdateResult
    {
        float moveDx = 0.0f;
        float moveDy = 0.0f;

        bool interact = false;
        bool chooseOption1 = false;
        bool chooseOption2 = false;
    };

    class LocationController
    {
    public:
        explicit LocationController(float moveSpeed = 180.0f);

        [[nodiscard]] LocationUpdateResult Update(
            const input::InputState& input,
            float deltaTime,
            bool hasActiveDialogue) const;

    private:
        float moveSpeed_ = 180.0f;
    };
}
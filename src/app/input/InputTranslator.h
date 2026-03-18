#pragma once

#include "app/input/InputState.h"

namespace app::input
{
    class InputTranslator
    {
    public:
        [[nodiscard]] InputState Poll() const;
    };
}
#pragma once

namespace app::input
{
    enum class InputAction
    {
        None,
        Confirm,
        Cancel,
        Interact,
        ToggleDebug,
        Save,
        Load,
        MoveLeft,
        MoveRight,
        MoveUp,
        MoveDown,
        SelectPrev,
        SelectNext,
        TargetPrev,
        TargetNext,
        Option1,
        Option2,
        DebugBattle
    };
}
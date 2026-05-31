#include "app/input/InputTranslator.h"

#include <raylib.h>

namespace app::input
{
    InputState InputTranslator::Poll() const
    {
        InputState input{};

        input.confirm = IsKeyPressed(KEY_ENTER);
        input.cancel = IsKeyPressed(KEY_ESCAPE);
        input.interact = IsKeyPressed(KEY_E);

        input.toggleDebug = IsKeyPressed(KEY_F1);
        input.save = IsKeyPressed(KEY_F5);
        input.load = IsKeyPressed(KEY_F9);
        input.openWorldMap = IsKeyPressed(KEY_M);

        input.moveLeft = IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A);
        input.moveRight = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D);
        input.moveUp = IsKeyDown(KEY_UP) || IsKeyDown(KEY_W);
        input.moveDown = IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S);

        input.selectPrev = IsKeyPressed(KEY_LEFT);
        input.selectNext = IsKeyPressed(KEY_RIGHT);
        input.targetPrev = IsKeyPressed(KEY_UP);
        input.targetNext = IsKeyPressed(KEY_DOWN);

        input.option1 = IsKeyPressed(KEY_ONE);
        input.option2 = IsKeyPressed(KEY_TWO);

        input.debugBattle = IsKeyPressed(KEY_B);

        input.moveAxisX =
            (input.moveRight ? 1.0f : 0.0f) -
            (input.moveLeft ? 1.0f : 0.0f);

        input.moveAxisY =
            (input.moveDown ? 1.0f : 0.0f) -
            (input.moveUp ? 1.0f : 0.0f);

        return input;
    }
}
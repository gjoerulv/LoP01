#pragma once

namespace app::input
{
    struct InputState
    {
        bool confirm = false;
        bool cancel = false;
        bool interact = false;
        bool toggleDebug = false;
        bool save = false;
        bool load = false;
        bool openWorldMap = false;
        bool openOwnedServices = false;
        // M30: destroy / queue restoration of a destroyable service at the
        // current Region node (two-press confirm for destruction).
        bool serviceMaintenance = false;

        bool moveLeft = false;
        bool moveRight = false;
        bool moveUp = false;
        bool moveDown = false;

        bool selectPrev = false;
        bool selectNext = false;
        bool targetPrev = false;
        bool targetNext = false;

        bool option1 = false;
        bool option2 = false;
        bool debugBattle = false;

        float moveAxisX = 0.0f;
        float moveAxisY = 0.0f;
    };
}
#pragma once

#include <string>
#include <vector>
#include "RenderContext.h"

namespace ashvale::rendering
{
    struct DebugLine
    {
        std::string label;
        std::string value;
    };

    struct DebugOverlayModel
    {
        bool visible = false;
        std::vector<DebugLine> lines;
    };

    class DebugOverlay
    {
    public:
        void Draw(const RenderContext& context, const DebugOverlayModel& model) const;
    };
}
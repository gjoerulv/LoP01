#pragma once

#include <string>
#include <vector>

#include "RenderContext.h"

namespace ashvale::rendering
{
    // One player-owned service row in the M27 overview panel. Pure presentation
    // data assembled by OwnedServiceOverviewModelMapper; the renderer never reads
    // GameSession.
    struct OwnedServiceRowView
    {
        std::string serviceId;                   // dim debug text only, never primary
        std::string displayName;                 // location name, e.g. "Copper Mine"
        std::string kindLabel;                   // "Mine" / "Trading Post" / ...
        std::string locationLabel;               // location (+ region) for "where"
        std::string statusLabel;                 // "Owned" / "Owned (Locked)" / ...

        bool isMine = false;
        int stationedCount = 0;
        int stationedCapacity = 0;               // 5 for mines
        std::vector<std::string> stationedUnitNames;
        std::vector<std::string> outputLines;    // "Stone: 2 (+2) = 4", "Gold: 1000"

        bool isTrader = false;
        int traderTier = 0;
    };

    // Read-only owned-service overview screen model. Built once on mode-enter; the
    // App updates only `selectedIndex` per frame, so the renderer needs no session.
    struct OwnedServiceOverviewModel
    {
        std::string title = "Owned Services";
        std::vector<OwnedServiceRowView> rows;
        std::string emptyText = "You own no services yet.";
        std::string footerHint = "Up/Down: select   Esc/O: back";
        int selectedIndex = 0;
    };

    class OwnedServiceOverviewRenderer
    {
    public:
        void Draw(const RenderContext& context, const OwnedServiceOverviewModel& model) const;
    };
}

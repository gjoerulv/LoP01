#include "app/mappers/WorldMapModelMapper.h"

#include <string>
#include <vector>

#include "gameplay/worldmap/WorldMapTravelRules.h"

namespace app::mappers
{
    namespace
    {
        std::string ReasonText(gameplay::worldmap::WorldMapTravelBlockReason reason, int days) {
            using gameplay::worldmap::WorldMapTravelBlockReason;
            switch (reason) {
                case WorldMapTravelBlockReason::None:
                    return std::to_string(days) + " day(s), 1000 Energy";
                case WorldMapTravelBlockReason::AlreadyHere:
                    return "Current region";
                case WorldMapTravelBlockReason::DestinationLocked:
                    return "Locked";
                case WorldMapTravelBlockReason::NoPath:
                    return "No route";
                case WorldMapTravelBlockReason::PastDepartureDeadline:
                    return "Too late (after 11:00)";
                case WorldMapTravelBlockReason::InsufficientEnergy:
                    return "Not enough Energy";
                case WorldMapTravelBlockReason::NotAtExitNode:
                    return "Not at an exit node";
            }
            return "";
        }
    }

    ashvale::rendering::WorldMapModel WorldMapModelMapper::Map(
        const data::ContentRepository& content,
        const gameplay::GameSession& session,
        const int selectedIndex) const
    {
        using namespace ashvale::rendering;
        const auto snapshot = session.Snapshot();
        const auto& worldMap = session.WorldMap();

        WorldMapModel model;
        model.energy = snapshot.energy;
        model.maxEnergy = snapshot.maxEnergy;
        model.genericLossCount = session.GenericTravelingPartyUnitCount();

        if (const auto* region = content.FindRegionById(snapshot.regionId)) {
            model.currentRegionName = region->name;
        } else {
            model.currentRegionName = snapshot.regionId;
        }

        // Unlocked-region vector for the pure rule (filter entries by the session's
        // runtime unlock state).
        std::vector<std::string> unlocked;
        for (const auto& entry : worldMap.entries) {
            if (session.IsRegionUnlocked(entry.id)) {
                unlocked.push_back(entry.id);
            }
        }

        int index = 0;
        for (const auto& entry : worldMap.entries) {
            if (entry.id == snapshot.regionId) {
                continue;  // never list the current region as a destination
            }

            WorldMapDestinationView view;
            view.regionId = entry.id;
            if (const auto* region = content.FindRegionById(entry.id)) {
                view.name = region->name;
            } else {
                view.name = entry.id;
            }
            view.unlocked = session.IsRegionUnlocked(entry.id);

            const auto eval = gameplay::worldmap::EvaluateWorldMapTravel(
                snapshot.regionId,
                entry.id,
                unlocked,
                worldMap.adjacency,
                snapshot.minutesIntoSliceDay,
                snapshot.energy);
            view.legal = eval.legal;
            view.days = eval.days;
            view.statusText = ReasonText(eval.reason, eval.days);
            view.selected = (index == selectedIndex);

            model.destinations.push_back(std::move(view));
            ++index;
        }

        model.footerHint = "Left/Right select  -  Enter travel  -  Esc/M back";
        return model;
    }
}

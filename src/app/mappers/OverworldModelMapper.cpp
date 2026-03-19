#include "app/mappers/OverworldModelMapper.h"

#include <algorithm>
#include <string>
#include <vector>

#include "gameplay/overworld/OverworldTravelRules.h"

namespace app::mappers
{
    namespace
    {
        using ashvale::rendering::OverworldNodeType;
        using ashvale::rendering::OverworldNodeView;
        using ashvale::rendering::OverworldRenderModel;

        OverworldNodeType ToNodeType(const data::LocationType type)
        {
            switch (type)
            {
            case data::LocationType::Home:    return OverworldNodeType::Home;
            case data::LocationType::Town:
            case data::LocationType::Inn:     return OverworldNodeType::Town;
            case data::LocationType::Dungeon: return OverworldNodeType::Dungeon;
            case data::LocationType::Service: return OverworldNodeType::Service;
            case data::LocationType::Recruit: return OverworldNodeType::Recruit;
            case data::LocationType::Combat:  return OverworldNodeType::Combat;
            default:                          return OverworldNodeType::Unknown;
            }
        }

        const data::RegionDefinition* ResolveRegion(
            const data::ContentRepository& content,
            const std::string& regionId)
        {
            if (const auto* region = content.FindRegionById(regionId)) {
                return region;
            }

            const auto& regions = content.Regions();
            if (!regions.empty()) {
                return &regions.front();
            }

            return nullptr;
        }

        bool IsClearedCombatNode(
            const std::vector<std::string>& clearedCombatNodeIds,
            const std::string& nodeId)
        {
            return std::ranges::find(clearedCombatNodeIds, nodeId) != clearedCombatNodeIds.end();
        }

        std::vector<std::string> BuildBlockedTransitNodeIds(const std::vector<OverworldNodeMeta>& nodes)
        {
            std::vector<std::string> blockedNodeIds;
            for (const auto& node : nodes)
            {
                if (node.blocksTransitUntilCleared && !node.combatNodeCleared)
                {
                    blockedNodeIds.push_back(node.id);
                }
            }

            return blockedNodeIds;
        }
    }

    std::vector<OverworldNodeMeta> OverworldModelMapper::BuildNodes(
        const data::ContentRepository& content,
        const std::string& regionId,
        const std::vector<std::string>& clearedCombatNodeIds) const
    {
        std::vector<OverworldNodeMeta> nodes;

        const data::RegionDefinition* region = ResolveRegion(content, regionId);
        if (region == nullptr) {
            return nodes;
        }

        for (const auto& regionNode : region->nodes)
        {
            const data::LocationDefinition* location = content.FindLocationById(regionNode.locationId);
            if (location == nullptr) {
                continue;
            }

            const bool supportsBattle = data::SupportsBattleScenario(location->type) && !location->battleScenarioId.empty();
            const bool combatNodeCleared = supportsBattle &&
                location->type == data::LocationType::Combat &&
                IsClearedCombatNode(clearedCombatNodeIds, location->id);

            nodes.push_back(OverworldNodeMeta{
                location->id,
                location->name,
                location->type,
                regionNode.travelAvailable,
                data::EntersLocationMode(location->type),
                supportsBattle,
                combatNodeCleared,
                location->blocksTransitUntilCleared,
                location->battleScenarioId,
                regionNode.discovered,
                regionNode.x,
                regionNode.y
            });
        }

        return nodes;
    }

    int OverworldModelMapper::FindNodeIndexById(
        const std::vector<OverworldNodeMeta>& nodes,
        const std::string& id) const
    {
        for (int i = 0; i < static_cast<int>(nodes.size()); ++i)
        {
            if (nodes[i].id == id)
            {
                return i;
            }
        }

        return 0;
    }

    std::string OverworldModelMapper::FormatTravelTime(const int minutes) const
    {
        if (minutes < 60)
        {
            return std::to_string(minutes) + " min";
        }

        const int hours = minutes / 60;
        const int remainder = minutes % 60;
        if (remainder == 0)
        {
            return std::to_string(hours) + "h";
        }

        return std::to_string(hours) + "h " + std::to_string(remainder) + "m";
    }

    OverworldRenderModel OverworldModelMapper::Map(
        const data::ContentRepository& content,
        const gameplay::SessionSnapshot& snapshot,
        const int selectedNodeIndex,
        const std::vector<std::string>& clearedCombatNodeIds) const
    {
        OverworldRenderModel model;

        const data::RegionDefinition* region = ResolveRegion(content, snapshot.regionId);
        model.regionName = region != nullptr ? region->name : "Unknown Region";
        model.hintText = "Left/Right: select destination";
        model.controlsText = "Enter: confirm travel | Esc: cancel selection";

        const auto nodes = BuildNodes(content, snapshot.regionId, clearedCombatNodeIds);

        const int currentIndex = FindNodeIndexById(nodes, snapshot.destinationId);
        const int safeSelectedIndex = nodes.empty()
            ? 0
            : std::clamp(selectedNodeIndex, 0, static_cast<int>(nodes.size()) - 1);

        for (int i = 0; i < static_cast<int>(nodes.size()); ++i)
        {
            model.nodes.push_back(OverworldNodeView{
                nodes[i].id,
                nodes[i].label,
                Vector2{ nodes[i].x, nodes[i].y },
                ToNodeType(nodes[i].type),
                nodes[i].discovered,
                nodes[i].travelAvailable,
                i == currentIndex,
                i == safeSelectedIndex
                });
        }

        if (region != nullptr)
        {
            for (const auto& link : region->links)
            {
                const int fromIndex = FindNodeIndexById(nodes, link.fromLocationId);
                const int toIndex = FindNodeIndexById(nodes, link.toLocationId);

                if (fromIndex >= 0 && toIndex >= 0 &&
                    fromIndex < static_cast<int>(nodes.size()) &&
                    toIndex < static_cast<int>(nodes.size()) &&
                    fromIndex != toIndex)
                {
                    model.links.push_back({ fromIndex, toIndex });
                }
            }
        }

        if (!nodes.empty())
        {
            const auto blockedTransitNodeIds = BuildBlockedTransitNodeIds(nodes);
            const std::vector<data::RegionLinkDefinition> emptyLinks;
            const auto travel = gameplay::overworld::EvaluateTravel(
                snapshot.destinationId,
                nodes[safeSelectedIndex].id,
                nodes[safeSelectedIndex].travelAvailable,
                snapshot.minutesIntoSliceDay,
                region != nullptr ? region->links : emptyLinks,
                blockedTransitNodeIds);

            model.currentNodeLabel = nodes[currentIndex].label;
            model.selectedNodeLabel = nodes[safeSelectedIndex].label;
            model.selectedNodeType = "Type: " + data::ToDisplayString(nodes[safeSelectedIndex].type);
            if (nodes[safeSelectedIndex].combatNodeCleared)
            {
                model.selectedNodeType += " | Node: Cleared";
            }
            if (nodes[safeSelectedIndex].blocksTransitUntilCleared)
            {
                model.selectedNodeType += nodes[safeSelectedIndex].combatNodeCleared
                    ? " | Route: Blocker Cleared"
                    : " | Route: Blocker Uncleared";
            }

            std::string travelText = "Yes";
            if (!travel.legal)
            {
                if (travel.reason == gameplay::overworld::TravelBlockReason::DestinationUnavailable)
                {
                    travelText = "No (destination unavailable)";
                }
                else if (travel.reason == gameplay::overworld::TravelBlockReason::NoRouteLink)
                {
                    travelText = "No (no route link)";
                }
                else if (travel.reason == gameplay::overworld::TravelBlockReason::ArrivalPastDayEnd)
                {
                    travelText = "No (arrives past 02:00)";
                }
                else if (travel.reason == gameplay::overworld::TravelBlockReason::BlockedByUnclearedTransitNode)
                {
                    travelText = "No (blocked by uncleared route blocker)";
                }
                else
                {
                    travelText = "No";
                }
            }

            model.selectedNodeEnterable =
                "Travel: " + travelText +
                " | Scene: " + (nodes[safeSelectedIndex].entersLocationMode ? "Yes" : "No") +
                " | Battle: " + ((nodes[safeSelectedIndex].supportsBattle && !nodes[safeSelectedIndex].combatNodeCleared) ? "Yes" : "No") +
                (nodes[safeSelectedIndex].combatNodeCleared ? " (Cleared)" : "");
            model.travelTimeText = travel.legal
                ? FormatTravelTime(travel.minutes)
                : "Unavailable";
        }

        return model;
    }
}
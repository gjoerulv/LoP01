#include "app/mappers/RegionModelMapper.h"

#include <algorithm>
#include <string>
#include <vector>

#include "gameplay/region/RegionTravelRules.h"

namespace app::mappers
{
    namespace
    {
        using ashvale::rendering::RegionNodeType;
        using ashvale::rendering::RegionNodeView;
        using ashvale::rendering::RegionRenderModel;

        RegionNodeType ToNodeType(const data::LocationType type)
        {
            switch (type)
            {
            case data::LocationType::Home:    return RegionNodeType::Home;
            case data::LocationType::Town:
            case data::LocationType::Inn:     return RegionNodeType::Town;
            case data::LocationType::Dungeon: return RegionNodeType::Dungeon;
            case data::LocationType::Service: return RegionNodeType::Service;
            case data::LocationType::Recruit: return RegionNodeType::Recruit;
            case data::LocationType::Combat:  return RegionNodeType::Combat;
            default:                          return RegionNodeType::Unknown;
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

        std::vector<std::string> BuildBlockedTransitNodeIds(const std::vector<RegionNodeMeta>& nodes)
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

    std::vector<RegionNodeMeta> RegionModelMapper::BuildNodes(
        const data::ContentRepository& content,
        const std::string& regionId,
        const std::vector<std::string>& clearedCombatNodeIds) const
    {
        std::vector<RegionNodeMeta> nodes;

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

            nodes.push_back(RegionNodeMeta{
                location->id,
                location->name,
                location->type,
                regionNode.travelAvailable,
                data::EntersLocationMode(*location),
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

    int RegionModelMapper::FindNodeIndexById(
        const std::vector<RegionNodeMeta>& nodes,
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

    std::string RegionModelMapper::FormatTravelTime(const int minutes) const
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

    RegionRenderModel RegionModelMapper::Map(
        const data::ContentRepository& content,
        const gameplay::GameSession& session,
        const gameplay::SessionSnapshot& snapshot,
        const int selectedNodeIndex,
        const std::vector<std::string>& clearedCombatNodeIds) const
    {
        RegionRenderModel model;

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
            model.nodes.push_back(RegionNodeView{
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
            const auto travel = gameplay::region::EvaluateTravel(
                snapshot.destinationId,
                nodes[safeSelectedIndex].id,
                nodes[safeSelectedIndex].travelAvailable,
                snapshot.minutesIntoSliceDay,
                region != nullptr ? region->links : emptyLinks,
                blockedTransitNodeIds);

            model.currentNodeLabel = nodes[currentIndex].label;
            model.selectedNodeLabel = nodes[safeSelectedIndex].label;
            model.selectedNodeType = data::ToDisplayString(nodes[safeSelectedIndex].type);

            std::vector<std::string> positiveProperties;
            if (travel.legal)
            {
                positiveProperties.push_back("Travel");
            }
            if (nodes[safeSelectedIndex].entersLocationMode)
            {
                positiveProperties.push_back("Scene");
            }
            if (nodes[safeSelectedIndex].supportsBattle && !nodes[safeSelectedIndex].combatNodeCleared)
            {
                positiveProperties.push_back("Battle");
            }

            if (!positiveProperties.empty())
            {
                model.selectedNodeEnterable = positiveProperties.front();
                for (int i = 1; i < static_cast<int>(positiveProperties.size()); ++i)
                {
                    model.selectedNodeEnterable += " | " + positiveProperties[i];
                }
            }
            else
            {
                model.selectedNodeEnterable = "-";
            }

            if (travel.legal) {
                const int previewTravelMinutes = session.PreviewSameDayTravelPrepToTravelMinutes(travel.minutes);
                if (previewTravelMinutes < travel.minutes) {
                    model.travelTimeText = FormatTravelTime(previewTravelMinutes) +
                        " (Supply Prep from " + FormatTravelTime(travel.minutes) + ")";
                }
                else {
                    model.travelTimeText = FormatTravelTime(travel.minutes);
                }
            }
            else {
                model.travelTimeText = "Unavailable";
            }
        }

        return model;
    }
}
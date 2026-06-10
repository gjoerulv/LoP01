#include "app/mappers/OwnedServiceOverviewModelMapper.h"

#include <set>
#include <string>
#include <unordered_map>

#include "data/definitions/LocationServiceDefinition.h"
#include "gameplay/ResourceState.h"
#include "gameplay/economy/MineProductionRules.h"
#include "gameplay/economy/StationingRules.h"
#include "gameplay/economy/StorageRules.h"

namespace app::mappers
{
    namespace
    {
        const data::LocationServiceDefinition* FindServiceById(
            const data::ContentRepository& content, const std::string& serviceId)
        {
            for (const auto& svc : content.LocationServices())
            {
                if (svc.id == serviceId)
                {
                    return &svc;
                }
            }
            return nullptr;
        }

        std::string KindLabel(data::LocationServiceKind kind)
        {
            switch (kind)
            {
            case data::LocationServiceKind::Mine:             return "Mine";
            case data::LocationServiceKind::TradingPost:      return "Trading Post";
            case data::LocationServiceKind::Market:           return "Market";
            case data::LocationServiceKind::FreelancersGuild: return "Freelancer's Guild";
            case data::LocationServiceKind::BlackMarket:      return "Black Market";
            case data::LocationServiceKind::Storage:          return "Storage";
            default:                                          return "Service";
            }
        }

        // Region display name for the location this service sits in, "" if none.
        std::string RegionNameForLocation(const data::ContentRepository& content,
            const std::string& locationId)
        {
            for (const auto& region : content.Regions())
            {
                for (const auto& node : region.nodes)
                {
                    if (node.locationId == locationId)
                    {
                        return region.name;
                    }
                }
            }
            return {};
        }

        // "Stone: 2 (+2) = 4" when boosted, "Gold: 1000" otherwise. `preview` is the
        // strongest-only daily output; `base` is the authored output for the delta.
        std::vector<std::string> FormatMineOutputs(
            const std::vector<data::MineOutputDefinition>& base,
            const std::vector<gameplay::economy::MineResourceOutput>& preview)
        {
            std::unordered_map<int, int> baseByResource;
            for (const auto& out : base)
            {
                gameplay::ResourceType resource;
                if (gameplay::TryResourceTypeFromString(out.resource, resource))
                {
                    baseByResource[static_cast<int>(resource)] = out.amount;
                }
            }

            std::vector<std::string> lines;
            lines.reserve(preview.size());
            for (const auto& line : preview)
            {
                const std::string name = gameplay::ResourceTypeToString(line.resource);
                const auto baseIt = baseByResource.find(static_cast<int>(line.resource));
                const int baseAmount = baseIt == baseByResource.end() ? 0 : baseIt->second;
                const int boost = line.amount - baseAmount;
                if (boost > 0)
                {
                    lines.push_back(name + ": " + std::to_string(baseAmount) + " (+" +
                        std::to_string(boost) + ") = " + std::to_string(line.amount));
                }
                else
                {
                    lines.push_back(name + ": " + std::to_string(line.amount));
                }
            }
            return lines;
        }
    }

    ashvale::rendering::OwnedServiceOverviewModel OwnedServiceOverviewModelMapper::Map(
        const data::ContentRepository& content,
        const gameplay::GameSession& session) const
    {
        ashvale::rendering::OwnedServiceOverviewModel model;

        const std::string playerColor = session.PlayerColor();
        const auto hostileVec = session.HostileOccupiedNodeIds(playerColor);
        const std::set<std::string> hostileNodes(hostileVec.begin(), hostileVec.end());

        for (const auto& owned : session.OwnedServices())
        {
            // Player-owned services only.
            if (owned.ownerTeamColor.empty() || owned.ownerTeamColor != playerColor)
            {
                continue;
            }
            const data::LocationServiceDefinition* def =
                FindServiceById(content, owned.serviceId);
            // Ownable services (mine/trader) plus M28 storage services (which are
            // player-owned via playerStart but intentionally not IsOwnableServiceKind).
            if (def == nullptr ||
                (!data::IsOwnableServiceKind(def->kind) &&
                 def->kind != data::LocationServiceKind::Storage))
            {
                continue;
            }

            ashvale::rendering::OwnedServiceRowView row;
            row.serviceId = owned.serviceId;
            const data::LocationDefinition* loc = content.FindLocationById(def->locationId);
            row.displayName = loc != nullptr ? loc->name : def->locationId;
            row.kindLabel = KindLabel(def->kind);

            const std::string regionName = RegionNameForLocation(content, def->locationId);
            row.locationLabel = regionName.empty()
                ? row.displayName
                : row.displayName + " — " + regionName;

            const bool hostile = hostileNodes.count(def->locationId) != 0;
            if (owned.destroyed)
            {
                row.statusLabel = "Owned (Destroyed)";
            }
            else if (owned.locked)
            {
                row.statusLabel = "Owned (Locked)";
            }
            else if (hostile)
            {
                row.statusLabel = "Owned (Unavailable: occupied)";
            }
            else
            {
                row.statusLabel = "Owned";
            }

            if (def->kind == data::LocationServiceKind::Mine)
            {
                row.isMine = true;
                row.stationedCapacity = gameplay::economy::kMaxStationedUnitsPerService;
                row.stationedCount = static_cast<int>(owned.stationedUnits.size());
                for (const auto& ref : owned.stationedUnits)
                {
                    const data::UnitDefinition* unit = content.FindUnitById(ref.unitId);
                    row.stationedUnitNames.push_back(unit != nullptr ? unit->name : ref.unitId);
                }
                row.outputLines = FormatMineOutputs(
                    def->mineOutputs, session.PreviewMineDailyOutput(owned.serviceId));
            }
            else if (data::IsTraderServiceKind(def->kind))
            {
                row.isTrader = true;
                row.traderTier = session.OwnedTraderServiceTierForService(owned.serviceId);
            }
            else if (def->kind == data::LocationServiceKind::Storage)
            {
                row.isStorage = true;
                row.storageCapacity = gameplay::economy::kMaxStoredUnitsPerService;
                row.storedCount = static_cast<int>(owned.storedUnits.size());
                for (const auto& ref : owned.storedUnits)
                {
                    const data::UnitDefinition* unit = content.FindUnitById(ref.unitId);
                    row.storedUnitNames.push_back(unit != nullptr ? unit->name : ref.unitId);
                }
            }

            model.rows.push_back(std::move(row));
        }

        return model;
    }
}

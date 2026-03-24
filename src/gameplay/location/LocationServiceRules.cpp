#include "gameplay/location/LocationServiceRules.h"

#include <algorithm>

namespace gameplay::location {

    bool IsRestService(const data::LocationServiceDefinition* service) {
        return service != nullptr && service->kind == data::LocationServiceKind::Rest;
    }

    bool IsShopService(const data::LocationServiceDefinition* service) {
        return service != nullptr && service->kind == data::LocationServiceKind::Shop;
    }

    bool IsRecruitService(const data::LocationServiceDefinition* service) {
        return service != nullptr && service->kind == data::LocationServiceKind::Recruit;
    }

    std::string BuildServicePromptText(
        const data::LocationServiceDefinition& service,
        const int remainingRecruitStock,
        const int currentWeek,
        const int remainingDailyUses) {
        if (service.kind == data::LocationServiceKind::Recruit) {
            const std::string unitName = !service.unitDisplayName.empty()
                ? service.unitDisplayName
                : (!service.unitId.empty() ? service.unitId : "Recruit");
            const std::string costText = service.goldCost > 0
                ? std::to_string(service.goldCost) + "g"
                : "Free";

            std::string stockText = std::to_string(std::max(0, remainingRecruitStock));
            if (service.weeklyStock > 0) {
                stockText += "/" + std::to_string(service.weeklyStock);
            }

            return "E: Recruit " + unitName + " (" + costText + ") | Stock " +
                stockText + " | Week " + std::to_string(std::max(1, currentWeek));
        }

        if (service.kind == data::LocationServiceKind::Shop && service.dailyUseLimit > 0) {
            const std::string costText = service.goldCost > 0
                ? std::to_string(service.goldCost) + "g"
                : "Free";

            const std::string actionText = !service.promptText.empty()
                ? service.promptText
                : "E: Buy Trail Supplies";

            std::string effectText;
            if (service.travelPrepDiscountMinutes > 0 && service.travelPrepCharges > 0) {
                effectText = " | Next travel -" + std::to_string(service.travelPrepDiscountMinutes) + "m";
            }

            const bool availableToday = remainingDailyUses > 0;
            const std::string availabilityText = availableToday
                ? "Available today"
                : "Used today";

            return actionText + " (" + costText + ")" + effectText +
                " | 1/day: " + availabilityText;
        }

        if (service.kind == data::LocationServiceKind::Rest && service.promptText.empty()) {
            if (service.restKind == data::RestServiceKind::HomeBase) {
                return "E: Rest at Home Base (Free)";
            }

            if (service.restKind == data::RestServiceKind::Inn) {
                if (service.goldCost > 0) {
                    return "E: Rent a Room (" + std::to_string(service.goldCost) + "g)";
                }

                return "E: Rest at Inn";
            }
        }

        return service.promptText;
    }

} // namespace gameplay::location
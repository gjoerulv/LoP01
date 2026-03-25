#include "app/formatters/LocationServicePromptFormatter.h"

#include <algorithm>

namespace {

    std::string StripInteractPrefix(const std::string& text) {
        if (text.rfind("E: ", 0) == 0) {
            return text.substr(3);
        }
        return text;
    }

} // namespace

namespace app {

    std::string BuildLocationServicePrompt(
        const data::LocationServiceDefinition& service,
        const LocationServicePromptContext& context) {
        if (service.kind == data::LocationServiceKind::Recruit) {
            const std::string unitName = !service.unitDisplayName.empty()
                ? service.unitDisplayName
                : (!service.unitId.empty() ? service.unitId : "Recruit");

            const std::string costText = service.goldCost > 0
                ? std::to_string(service.goldCost) + "g"
                : "Free";

            std::string stockText = std::to_string(std::max(0, context.remainingRecruitStock));
            if (service.weeklyStock > 0) {
                stockText += "/" + std::to_string(service.weeklyStock);
            }

            const int safeWeek = std::max(1, context.currentWeek);
            const int refreshWeek = safeWeek + 1;

            return "Recruit " + unitName +
                "\nCost: " + costText + " | Stock: " + stockText +
                "\nResets: Week " + std::to_string(refreshWeek);
        }

        if (service.kind == data::LocationServiceKind::Shop && service.dailyUseLimit > 0) {
            const std::string costText = service.goldCost > 0
                ? std::to_string(service.goldCost) + "g"
                : "Free";

            const std::string actionText = !service.promptText.empty()
                ? StripInteractPrefix(service.promptText)
                : "Buy Trail Supplies";

            std::string effectText;
            if (service.travelPrepDiscountMinutes > 0 && service.travelPrepCharges > 0) {
                effectText = "Next travel -" + std::to_string(service.travelPrepDiscountMinutes) + "m";
            }

            std::string availabilityText;
            if (service.travelPrepDiscountMinutes > 0 &&
                service.travelPrepCharges > 0 &&
                context.hasActiveTravelPrep) {
                availabilityText = "Prep already active";
            }
            else {
                availabilityText = context.remainingDailyUses > 0
                    ? "Available today"
                    : "Used today";
            }

            std::string line2 = "Cost: " + costText;
            if (!effectText.empty()) {
                line2 += " | Effect: " + effectText;
            }

            return actionText +
                "\n" + line2 +
                "\nResets: Next day | " + availabilityText;
        }

        if (service.kind == data::LocationServiceKind::Rest) {
            std::string actionText;
            if (!service.promptText.empty()) {
                actionText = StripInteractPrefix(service.promptText);
            }
            else if (service.restKind == data::RestServiceKind::HomeBase) {
                actionText = "Rest at Home Base";
            }
            else if (service.restKind == data::RestServiceKind::Inn) {
                actionText = "Rent a Room";
            }
            else {
                actionText = "Rest";
            }

            const std::string costText = service.goldCost > 0
                ? std::to_string(service.goldCost) + "g"
                : "Free";

            return actionText +
                "\nCost: " + costText +
                "\nResets: N/A";
        }

        return service.promptText;
    }

} // namespace app
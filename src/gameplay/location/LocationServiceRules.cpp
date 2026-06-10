#include "gameplay/location/LocationServiceRules.h"

#include "data/ContentRepository.h"
#include "gameplay/GameSession.h"

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

    bool IsMusterService(const data::LocationServiceDefinition* service) {
        return service != nullptr && service->kind == data::LocationServiceKind::Muster;
    }

    bool IsTradingPostService(const data::LocationServiceDefinition* service) {
        return service != nullptr && service->kind == data::LocationServiceKind::TradingPost;
    }

    bool IsMineService(const data::LocationServiceDefinition* service) {
        return service != nullptr && service->kind == data::LocationServiceKind::Mine;
    }

    bool IsStorageService(const data::LocationServiceDefinition* service) {
        return service != nullptr && service->kind == data::LocationServiceKind::Storage;
    }

    RecruitServiceApplyResult TryApplyRecruitService(
        gameplay::GameSession& session,
        const data::ContentRepository& content,
        const data::LocationServiceDefinition& service) {
        if (service.kind != data::LocationServiceKind::Recruit) {
            return RecruitServiceApplyResult{false, "Invalid recruit service request"};
        }

        if (service.unitId.empty() || content.FindUnitById(service.unitId) == nullptr) {
            return RecruitServiceApplyResult{false, "Invalid recruit service configuration: missing or unknown unit_id"};
        }

        session.RefreshWeeklyRecruitStocks(content.LocationServices());
        const int currentWeek = session.CurrentWeek();

        const int remainingBefore = session.RemainingRecruitStock(service.id, service.weeklyStock);
        if (remainingBefore <= 0) {
            std::string status = !service.failureText.empty()
                ? service.failureText
                : "No recruits available this week";
            status += " (Week " + std::to_string(currentWeek) + ", refreshes next week)";
            return RecruitServiceApplyResult{false, status};
        }

        if (!session.CanAddOwnedUnit(service.unitId, 1)) {
            return RecruitServiceApplyResult{false, "No reserve slot available for a new recruit stack"};
        }

        if (service.goldCost > 0 && !session.TrySpendGold(service.goldCost)) {
            const std::string status = !service.failureText.empty()
                ? service.failureText
                : "Not enough gold to recruit";
            return RecruitServiceApplyResult{false, status};
        }

        if (!session.TryConsumeRecruitStock(service.id, service.weeklyStock)) {
            const std::string status = !service.failureText.empty()
                ? service.failureText
                : "No recruits available this week";
            return RecruitServiceApplyResult{false, status};
        }

        if (service.timeCostMinutes > 0) {
            session.AddMinutes(service.timeCostMinutes);
        }

        if (!session.AddOwnedUnit(service.unitId, 1)) {
            return RecruitServiceApplyResult{false, "Recruit failed: roster placement changed unexpectedly"};
        }

        const int remainingAfter = session.RemainingRecruitStock(service.id, service.weeklyStock);
        std::string status = service.successText + " (" + std::to_string(remainingAfter) +
            " left this week, Week " + std::to_string(currentWeek) + ")";
        return RecruitServiceApplyResult{true, status};
    }

} // namespace gameplay::location
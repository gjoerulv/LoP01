#include "gameplay/location/LocationServiceRules.h"

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

} // namespace gameplay::location
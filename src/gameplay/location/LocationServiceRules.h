#pragma once

#include "data/definitions/LocationServiceDefinition.h"

namespace gameplay::location {

	[[nodiscard]] bool IsRestService(const data::LocationServiceDefinition* service);
	[[nodiscard]] bool IsShopService(const data::LocationServiceDefinition* service);
	[[nodiscard]] bool IsRecruitService(const data::LocationServiceDefinition* service);

} // namespace gameplay::location
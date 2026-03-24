#pragma once

#include <string>

#include "data/definitions/LocationServiceDefinition.h"

namespace gameplay::location {

	[[nodiscard]] bool IsRestService(const data::LocationServiceDefinition* service);
	[[nodiscard]] bool IsShopService(const data::LocationServiceDefinition* service);
	[[nodiscard]] bool IsRecruitService(const data::LocationServiceDefinition* service);
	[[nodiscard]] std::string BuildServicePromptText(
		const data::LocationServiceDefinition& service,
		int remainingRecruitStock,
       int currentWeek,
		int remainingDailyUses);

} // namespace gameplay::location
#pragma once

#include <string>

#include "data/definitions/LocationServiceDefinition.h"

namespace data {
	class ContentRepository;
}

namespace gameplay {
	class GameSession;
}

namespace gameplay::location {

	[[nodiscard]] bool IsRestService(const data::LocationServiceDefinition* service);
	[[nodiscard]] bool IsShopService(const data::LocationServiceDefinition* service);
	[[nodiscard]] bool IsRecruitService(const data::LocationServiceDefinition* service);
	[[nodiscard]] bool IsMusterService(const data::LocationServiceDefinition* service);
	[[nodiscard]] bool IsTradingPostService(const data::LocationServiceDefinition* service);
	[[nodiscard]] bool IsMineService(const data::LocationServiceDefinition* service);
	[[nodiscard]] bool IsStorageService(const data::LocationServiceDefinition* service);

	struct RecruitServiceApplyResult {
		bool success = false;
		std::string statusMessage;
	};

	[[nodiscard]] RecruitServiceApplyResult TryApplyRecruitService(
		gameplay::GameSession& session,
		const data::ContentRepository& content,
		const data::LocationServiceDefinition& service);

} // namespace gameplay::location
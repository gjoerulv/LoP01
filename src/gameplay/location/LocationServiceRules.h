#pragma once

#include "gameplay/location/LocationScene.h"

namespace gameplay::location {

[[nodiscard]] bool IsRestValidForLocation(InteractionType interactionType, bool locationAllowsSleep);

} // namespace gameplay::location

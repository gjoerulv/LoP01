#include "gameplay/location/LocationServiceRules.h"

namespace gameplay::location {

bool IsRestValidForLocation(const InteractionType interactionType, const bool locationAllowsSleep) {
    return interactionType == InteractionType::InnDoor && locationAllowsSleep;
}

} // namespace gameplay::location

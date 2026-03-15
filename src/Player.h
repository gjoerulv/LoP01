#pragma once

#include "Location.h"

#include <string>

/// Tracks mutable player state (position only for v0).
class Player
{
public:
    explicit Player(Location::Id startLocationId);

    const Location::Id& GetCurrentLocationId() const;
    void MoveTo(const Location::Id& locationId);

private:
    Location::Id m_currentLocationId;
};

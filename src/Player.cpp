#include "Player.h"

#include <utility>

Player::Player(Location::Id startLocationId)
    : m_currentLocationId(std::move(startLocationId))
{}

const Location::Id& Player::GetCurrentLocationId() const
{
    return m_currentLocationId;
}

void Player::MoveTo(const Location::Id& locationId)
{
    m_currentLocationId = locationId;
}

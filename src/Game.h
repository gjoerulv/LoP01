#pragma once

#include "Location.h"
#include "Player.h"

#include <string>
#include <unordered_map>

/// Central game controller: owns locations, player state and drives the game loop.
class Game
{
public:
    Game();

    /// @returns true while the game session is still active.
    bool IsRunning() const;

    /// Process a single player command and return the text to display.
    std::string ProcessCommand(const std::string& command);

    /// Render the current scene as a display string (without side effects).
    std::string RenderCurrentScene() const;

private:
    void SetupLocations();

    const Location* GetCurrentLocation() const;

    std::unordered_map<Location::Id, Location> m_locations;
    Player m_player;
    bool   m_running = false;
    bool   m_started = false;
};

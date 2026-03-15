#pragma once

#include <string>

class Location;

/// Represents the current display state of the game.
class Scene
{
public:
    enum class Type
    {
        Title,
        Location,
        GameOver
    };

    /// Construct a scene that renders the given location.
    explicit Scene(const Location& location);

    /// Construct a non-location scene (title or game-over).
    explicit Scene(Type type);

    Type GetType() const;

    /// Returns the fully formatted string to display to the player.
    std::string Render() const;

private:
    Type            m_type;
    const Location* m_location = nullptr;

    std::string RenderTitle()    const;
    std::string RenderLocation() const;
    std::string RenderGameOver() const;
};

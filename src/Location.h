#pragma once

#include <string>
#include <unordered_map>

/// A single location in the game world.
class Location
{
public:
    using Id = std::string;

    Location() = default;
    Location(Id id, std::string name, std::string description);

    const Id&          GetId()          const;
    const std::string& GetName()        const;
    const std::string& GetDescription() const;

    /// Register an exit from this location.
    /// @param direction  Lowercase direction word (e.g. "north").
    /// @param destinationId  Id of the location the exit leads to.
    void AddExit(const std::string& direction, const Id& destinationId);

    bool HasExit(const std::string& direction) const;

    /// @throws std::invalid_argument if direction has no exit.
    const Id& GetExitDestination(const std::string& direction) const;

    const std::unordered_map<std::string, Id>& GetExits() const;

private:
    Id          m_id;
    std::string m_name;
    std::string m_description;
    std::unordered_map<std::string, Id> m_exits;
};

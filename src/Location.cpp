#include "Location.h"

#include <stdexcept>
#include <utility>

Location::Location(Id id, std::string name, std::string description)
    : m_id(std::move(id))
    , m_name(std::move(name))
    , m_description(std::move(description))
{}

const Location::Id& Location::GetId() const
{
    return m_id;
}

const std::string& Location::GetName() const
{
    return m_name;
}

const std::string& Location::GetDescription() const
{
    return m_description;
}

void Location::AddExit(const std::string& direction, const Id& destinationId)
{
    m_exits[direction] = destinationId;
}

bool Location::HasExit(const std::string& direction) const
{
    return m_exits.count(direction) > 0;
}

const Location::Id& Location::GetExitDestination(const std::string& direction) const
{
    auto it = m_exits.find(direction);
    if (it == m_exits.end())
        throw std::invalid_argument("No exit in direction: " + direction);
    return it->second;
}

const std::unordered_map<std::string, Location::Id>& Location::GetExits() const
{
    return m_exits;
}

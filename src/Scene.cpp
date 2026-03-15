#include "Scene.h"
#include "Location.h"

#include <sstream>

Scene::Scene(const Location& location)
    : m_type(Type::Location)
    , m_location(&location)
{}

Scene::Scene(Type type)
    : m_type(type)
{}

Scene::Type Scene::GetType() const
{
    return m_type;
}

std::string Scene::Render() const
{
    switch (m_type)
    {
    case Type::Title:    return RenderTitle();
    case Type::Location: return RenderLocation();
    case Type::GameOver: return RenderGameOver();
    }
    return {};
}

std::string Scene::RenderTitle() const
{
    return
        "================================\n"
        "    LANDS OF PERIL  (LoP01)    \n"
        "================================\n"
        "\n"
        "  A dark adventure awaits...\n"
        "\n"
        "Type 'start' to begin.\n";
}

std::string Scene::RenderLocation() const
{
    if (!m_location)
        return {};

    std::ostringstream oss;
    oss << "=== " << m_location->GetName() << " ===\n"
        << "\n"
        << m_location->GetDescription() << "\n";

    const auto& exits = m_location->GetExits();
    if (exits.empty())
    {
        oss << "\nThere are no exits.\n";
    }
    else
    {
        oss << "\nExits: ";
        bool first = true;
        for (const auto& [dir, _] : exits)
        {
            if (!first) oss << ", ";
            oss << dir;
            first = false;
        }
        oss << "\n";
    }

    return oss.str();
}

std::string Scene::RenderGameOver() const
{
    return
        "================================\n"
        "          GAME OVER            \n"
        "================================\n"
        "\n"
        "Thank you for playing!\n";
}

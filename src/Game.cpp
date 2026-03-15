#include "Game.h"
#include "Scene.h"

#include <algorithm>
#include <cctype>
#include <utility>

// ────────────────────────────────────────────────
// Construction
// ────────────────────────────────────────────────

Game::Game()
    : m_player("town_square")
{
    SetupLocations();
    m_running = true;
}

// ────────────────────────────────────────────────
// World setup
// ────────────────────────────────────────────────

void Game::SetupLocations()
{
    auto add = [&](Location loc)
    {
        const auto id = loc.GetId();
        m_locations[id] = std::move(loc);
    };

    // Town Square
    {
        Location loc("town_square", "Town Square",
            "The bustling centre of Stonehaven. Merchants hawk their wares\n"
            "and townsfolk hurry about their business. A stone fountain\n"
            "gurgles at the centre of the cobblestoned plaza.");
        loc.AddExit("north", "market");
        loc.AddExit("east",  "blacksmith");
        loc.AddExit("south", "south_gate");
        add(std::move(loc));
    }

    // Market District
    {
        Location loc("market", "Market District",
            "Colourful stalls line both sides of the narrow street. The air\n"
            "is thick with the smell of spices, leather, and fresh bread.\n"
            "A peculiar old woman eyes you from behind a cart of potions.");
        loc.AddExit("south", "town_square");
        loc.AddExit("east",  "tavern");
        add(std::move(loc));
    }

    // Blacksmith's Forge
    {
        Location loc("blacksmith", "Blacksmith's Forge",
            "The rhythmic clanging of hammer on anvil fills the smoky shop.\n"
            "Weapons and armour of all kinds hang from the walls. The\n"
            "muscular smith nods at you without breaking his rhythm.");
        loc.AddExit("west", "town_square");
        add(std::move(loc));
    }

    // Tavern
    {
        Location loc("tavern", "The Rusty Flagon",
            "Rough-hewn tables fill a dimly lit common room. A fire crackles\n"
            "in the hearth. Several adventurers nurse their drinks and whisper\n"
            "of dangers in the nearby Darkwood Forest.");
        loc.AddExit("west",  "market");
        loc.AddExit("north", "darkwood_forest");
        add(std::move(loc));
    }

    // South Gate
    {
        Location loc("south_gate", "South Gate",
            "A pair of bored guards lean on their spears beside the massive\n"
            "iron gate. Beyond it, a rutted road leads into the unknown.\n"
            "One guard points north. 'Town's that way, traveller.'");
        loc.AddExit("north", "town_square");
        add(std::move(loc));
    }

    // Darkwood Forest
    {
        Location loc("darkwood_forest", "Darkwood Forest",
            "Ancient trees blot out the sun. Strange sounds echo through\n"
            "the gloom and pairs of yellow eyes glint between the trunks.\n"
            "This place feels dangerous. Proceed with caution.");
        loc.AddExit("south", "tavern");
        add(std::move(loc));
    }
}

// ────────────────────────────────────────────────
// Helpers
// ────────────────────────────────────────────────

const Location* Game::GetCurrentLocation() const
{
    auto it = m_locations.find(m_player.GetCurrentLocationId());
    return (it != m_locations.end()) ? &it->second : nullptr;
}

// ────────────────────────────────────────────────
// Public API
// ────────────────────────────────────────────────

bool Game::IsRunning() const
{
    return m_running;
}

std::string Game::RenderCurrentScene() const
{
    if (!m_started)
        return Scene(Scene::Type::Title).Render();

    if (!m_running)
        return Scene(Scene::Type::GameOver).Render();

    const Location* loc = GetCurrentLocation();
    if (!loc)
        return "Error: unknown location.\n";

    return Scene(*loc).Render();
}

std::string Game::ProcessCommand(const std::string& command)
{
    if (command.empty())
        return {};

    // Normalise: lowercase and trim whitespace
    std::string cmd = command;
    std::transform(cmd.begin(), cmd.end(), cmd.begin(),
        [](unsigned char c){ return static_cast<char>(std::tolower(c)); });

    const auto start = cmd.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return {};
    const auto end = cmd.find_last_not_of(" \t\r\n");
    cmd = cmd.substr(start, end - start + 1);

    // ── Pre-start phase ──────────────────────────
    if (!m_started)
    {
        if (cmd == "start")
        {
            m_started = true;
            return RenderCurrentScene();
        }
        return "Type 'start' to begin.\n";
    }

    // ── Meta commands ────────────────────────────
    if (cmd == "quit" || cmd == "q")
    {
        m_running = false;
        return Scene(Scene::Type::GameOver).Render();
    }

    if (cmd == "look" || cmd == "l")
        return RenderCurrentScene();

    // ── Movement ─────────────────────────────────
    // Resolve direction aliases
    static const std::unordered_map<std::string, std::string> kAliases =
    {
        { "n",         "north" },
        { "s",         "south" },
        { "e",         "east"  },
        { "w",         "west"  },
        { "go north",  "north" },
        { "go south",  "south" },
        { "go east",   "east"  },
        { "go west",   "west"  },
        { "go n",      "north" },
        { "go s",      "south" },
        { "go e",      "east"  },
        { "go w",      "west"  },
    };

    std::string direction = cmd;
    if (auto it = kAliases.find(cmd); it != kAliases.end())
        direction = it->second;

    const Location* loc = GetCurrentLocation();
    if (loc && loc->HasExit(direction))
    {
        m_player.MoveTo(loc->GetExitDestination(direction));
        return RenderCurrentScene();
    }

    return "You can't go that way.\n";
}

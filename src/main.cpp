#include "Game.h"

#include <iostream>
#include <string>

int main()
{
    Game game;
    std::cout << game.RenderCurrentScene();

    std::string input;
    while (game.IsRunning())
    {
        std::cout << "\n> ";
        if (!std::getline(std::cin, input))
            break;

        const std::string output = game.ProcessCommand(input);
        if (!output.empty())
            std::cout << "\n" << output;
    }

    return 0;
}

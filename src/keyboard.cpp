#include "keyboard.hpp"

#include <SDL.h>



bool Keyboard::handleEvents()
{
    SDL_Event e;

    auto hasEvent = SDL_PollEvent(&e);
    if (!hasEvent)
        return true;

    switch (e.type) {
        case SDL_QUIT:
            return false;

        case SDL_KEYDOWN:
        {
            // auto keyValue = mapKey(e.key.keysym.sym);
            // if (keyValue != NoKey)
            //     keysDown[keyValue] = true;
            break;
        }

        case SDL_KEYUP:
        {
            // auto keyValue = mapKey(e.key.keysym.sym);
            // if (keyValue != NoKey)
            //     keysDown[keyValue] = false;
            break;
        }
    }

    return true;
}


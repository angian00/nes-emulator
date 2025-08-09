#include "display.hpp"

#include "ppu.hpp"
#include "palette.hpp"

#include <print>


bool Display::initSdl()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::println("SDL could not be initialized!");
        std::println("SDL_Error: {}", SDL_GetError());
        return false;
    }
    sdlInited = true;

    auto windowWidth  = Ppu::SCREEN_WIDTH  * SCALE_FACTOR;
    auto windowHeight = Ppu::SCREEN_HEIGHT * SCALE_FACTOR;

    window = SDL_CreateWindow("NES Emulator by AnGian",
                                SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED,
                                windowWidth, windowHeight,
                                SDL_WINDOW_SHOWN);
    if (!window)
    {
        std::println("Window could not be created!");
        std::println("SDL_Error: {}", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        std::println("Renderer could not be created!");
        std::println("SDL_Error: {}", SDL_GetError());
        return false;
    }

    clear();

    return true;
}

void Display::shutdownSdl()
{
    if (renderer)
        SDL_DestroyRenderer(renderer);

    if (window)
        SDL_DestroyWindow(window);

    if (sdlInited)
        SDL_Quit();
}


void Display::clear()
{
    uint8_t* color = (uint8_t*)PALETTE_COLORS;
    SDL_SetRenderDrawColor(renderer, color[0], color[1], color[2], color[3]);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}


void Display::render(const uint8_t *pixels)
{
    //clear();

    for (auto x=0; x < Ppu::SCREEN_WIDTH; x++)
    {
        for (auto y=0; y < Ppu::SCREEN_HEIGHT; y++)
        {
            uint8_t colorIndex = pixels[x*Ppu::SCREEN_HEIGHT + y];
            uint8_t* color = PALETTE_COLORS + 4*colorIndex;
            //uint8_t * color = PALETTE;
            SDL_SetRenderDrawColor(renderer, color[0], color[1], color[2], color[3]);

            SDL_Rect rect(x*SCALE_FACTOR, y*SCALE_FACTOR, SCALE_FACTOR, SCALE_FACTOR);
            SDL_RenderFillRect(renderer, &rect);
        }
    }
    
    SDL_RenderPresent(renderer);
}


void Display::refresh()
{
    SDL_RenderPresent(renderer);
    SDL_RaiseWindow(window);
}
